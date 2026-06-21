#include "Interpreter.h"
#include "LoxCallable.h"
#include "LoxFunction.h"
#include "LoxClass.h"
#include "LoxInstance.h"
#include "ReturnException.h"
#include <chrono>
#include <map>
#include <memory>
#include <variant>

namespace {
class ClockCallable : public LoxCallable {
public:
  int arity() const override { return 0; }
  Value call(Interpreter&, vector<Value>&) override {
    auto now = chrono::system_clock::now().time_since_epoch();
    return (double)chrono::duration_cast<chrono::milliseconds>(now).count() / 1000.0;
  }
  string toString() const override { return "<native fn>"; }
};
}

Interpreter::Interpreter()
    : globals(make_shared<Environment>()), environment(globals) {
  globals->define("clock", shared_ptr<LoxCallable>(make_shared<ClockCallable>()));
}

void Interpreter::interpret(const vector<unique_ptr<Stmt>>& statements) {
  try {
    for (const auto& statement : statements) {
      execute(*statement);
    }
  } catch (const RuntimeError& error) {
    Lox::runtimeError(error);
  }
}

Value Interpreter::evaluate(Expr& expr) {
  return expr.accept(*this);
}

void Interpreter::resolve(const Expr* expr, int depth) {
  locals[expr] = depth;
}

Value Interpreter::lookUpVariable(const Token& name, const Expr* expr) {
  auto it = locals.find(expr);
  if (it != locals.end()) {
    return environment->getAt(it->second, name.lexeme);
  }
  return globals->get(name);
}

void Interpreter::execute(Stmt& stmt) {
  stmt.accept(*this);
}

void Interpreter::executeBlock(const vector<unique_ptr<Stmt>>& statements,
                               shared_ptr<Environment> environment) {
  shared_ptr<Environment> previous = this->environment;
  this->environment = environment;

  try {
    for (const auto& statement : statements) {
      execute(*statement);
    }
  } catch (...) {
    this->environment = previous;
    throw;
  }

  this->environment = previous;
}

Value Interpreter::visitLiteralExpr(Literal& expr) {
  switch (expr.token.type) {
    case FALSE_TOK: return false;
    case TRUE_TOK: return true;
    case NIL: return nullptr;
    case NUMBER:
      return stod(expr.token.literal);
    case STRING:
      return expr.token.literal;
    default:
      return expr.token.lexeme;
  }
}

Value Interpreter::visitGroupingExpr(Grouping& expr) {
  return evaluate(*expr.expression);
}

Value Interpreter::visitUnaryExpr(Unary& expr) {
  Value right = evaluate(*expr.right);

  switch (expr.oper.type) {
    case MINUS:
      checkNumberOperand(expr.oper, right);
      return -get<double>(right);
    case BANG:
      return !isTruthy(right);
    default:
      return nullptr;
  }
}

Value Interpreter::visitBinaryExpr(Binary& expr) {
  Value left = evaluate(*expr.left);
  Value right = evaluate(*expr.right);

  switch (expr.oper.type) {
    case MINUS:
      checkNumberOperands(expr.oper, left, right);
      return get<double>(left) - get<double>(right);
    case SLASH:
      checkNumberOperands(expr.oper, left, right);
      return get<double>(left) / get<double>(right);
    case STAR:
      checkNumberOperands(expr.oper, left, right);
      return get<double>(left) * get<double>(right);
    case PLUS:
      if (holds_alternative<double>(left) && holds_alternative<double>(right)) {
        return get<double>(left) + get<double>(right);
      }
      if (holds_alternative<string>(left) && holds_alternative<string>(right)) {
        return get<string>(left) + get<string>(right);
      }
      throw RuntimeError(expr.oper, "Operands must be two numbers or two strings.");
    case GREATER:
      checkNumberOperands(expr.oper, left, right);
      return get<double>(left) > get<double>(right);
    case GREATER_EQUAL:
      checkNumberOperands(expr.oper, left, right);
      return get<double>(left) >= get<double>(right);
    case LESS:
      checkNumberOperands(expr.oper, left, right);
      return get<double>(left) < get<double>(right);
    case LESS_EQUAL:
      checkNumberOperands(expr.oper, left, right);
      return get<double>(left) <= get<double>(right);
    case BANG_EQUAL:
      return !isEqual(left, right);
    case EQUAL_EQUAL:
      return isEqual(left, right);
    default:
      return nullptr;
  }
}

Value Interpreter::visitVariableExpr(Variable& expr) {
  return lookUpVariable(expr.name, &expr);
}

Value Interpreter::visitAssignExpr(Assign& expr) {
  Value value = evaluate(*expr.value);

  auto it = locals.find(&expr);
  if (it != locals.end()) {
    environment->assignAt(it->second, expr.name, value);
  } else {
    globals->assign(expr.name, value);
  }

  return value;
}

Value Interpreter::visitLogicalExpr(Logical& expr) {
  Value left = evaluate(*expr.left);

  if (expr.oper.type == OR) {
    if (isTruthy(left)) return left;
  } else {
    if (!isTruthy(left)) return left;
  }

  return evaluate(*expr.right);
}

Value Interpreter::visitCallExpr(Call& expr) {
  Value callee = evaluate(*expr.callee);

  vector<Value> arguments;
  for (const auto& argument : expr.arguments) {
    arguments.push_back(evaluate(*argument));
  }

  if (!holds_alternative<shared_ptr<LoxCallable>>(callee)) {
    throw RuntimeError(expr.paren, "Can only call functions and classes.");
  }

  shared_ptr<LoxCallable> function = get<shared_ptr<LoxCallable>>(callee);
  if ((int)arguments.size() != function->arity()) {
    throw RuntimeError(expr.paren, "Expected " + to_string(function->arity()) +
                       " arguments but got " + to_string(arguments.size()) + ".");
  }

  return function->call(*this, arguments);
}

Value Interpreter::visitGetExpr(Get& expr) {
  Value object = evaluate(*expr.object);
  if (holds_alternative<shared_ptr<LoxInstance>>(object)) {
    return get<shared_ptr<LoxInstance>>(object)->get(expr.name);
  }

  throw RuntimeError(expr.name, "Only instances have properties.");
}

Value Interpreter::visitSetExpr(Set& expr) {
  Value object = evaluate(*expr.object);

  if (!holds_alternative<shared_ptr<LoxInstance>>(object)) {
    throw RuntimeError(expr.name, "Only instances have fields.");
  }

  Value value = evaluate(*expr.value);
  get<shared_ptr<LoxInstance>>(object)->set(expr.name, value);
  return value;
}

Value Interpreter::visitThisExpr(This& expr) {
  return lookUpVariable(expr.keyword, &expr);
}

Value Interpreter::visitSuperExpr(Super& expr) {
  int distance = locals.at(&expr);

  Value superValue = environment->getAt(distance, "super");
  shared_ptr<LoxClass> superclass = dynamic_pointer_cast<LoxClass>(
      get<shared_ptr<LoxCallable>>(superValue));

  Value thisValue = environment->getAt(distance - 1, "this");
  shared_ptr<LoxInstance> object = get<shared_ptr<LoxInstance>>(thisValue);

  shared_ptr<LoxFunction> method = superclass->findMethod(expr.method.lexeme);
  if (method == nullptr) {
    throw RuntimeError(expr.method,
                       "Undefined property '" + expr.method.lexeme + "'.");
  }

  return method->bind(object);
}

void Interpreter::visitExpressionStmt(Expression& stmt) {
  evaluate(*stmt.expression);
}

void Interpreter::visitPrintStmt(Print& stmt) {
  Value value = evaluate(*stmt.expression);
  cout << stringify(value) << endl;
}

void Interpreter::visitVarStmt(Var& stmt) {
  Value value = nullptr;
  if (stmt.initializer != nullptr) {
    value = evaluate(*stmt.initializer);
  }
  environment->define(stmt.name.lexeme, value);
}

void Interpreter::visitBlockStmt(Block& stmt) {
  executeBlock(stmt.statements, make_shared<Environment>(environment));
}

void Interpreter::visitFunctionStmt(Function& stmt) {
  shared_ptr<LoxCallable> function = make_shared<LoxFunction>(&stmt, environment, false);
  environment->define(stmt.name.lexeme, function);
}

void Interpreter::visitClassStmt(Class& stmt) {
  shared_ptr<LoxClass> superclass = nullptr;
  if (stmt.superclass != nullptr) {
    Value superValue = evaluate(*stmt.superclass);
    if (holds_alternative<shared_ptr<LoxCallable>>(superValue)) {
      superclass = dynamic_pointer_cast<LoxClass>(
          get<shared_ptr<LoxCallable>>(superValue));
    }
    if (superclass == nullptr) {
      throw RuntimeError(stmt.superclass->name, "Superclass must be a class.");
    }
  }

  environment->define(stmt.name.lexeme, Value(nullptr));

  shared_ptr<Environment> previous = environment;
  if (stmt.superclass != nullptr) {
    environment = make_shared<Environment>(environment);
    environment->define("super", Value(shared_ptr<LoxCallable>(superclass)));
  }

  map<string, shared_ptr<LoxFunction>> methods;
  for (const auto& method : stmt.methods) {
    bool isInitializer = method->name.lexeme == "init";
    methods[method->name.lexeme] =
        make_shared<LoxFunction>(method.get(), environment, isInitializer);
  }

  shared_ptr<LoxCallable> klass =
      make_shared<LoxClass>(stmt.name.lexeme, superclass, methods);

  if (stmt.superclass != nullptr) {
    environment = previous;
  }

  environment->assign(stmt.name, klass);
}

void Interpreter::visitReturnStmt(Return& stmt) {
  Value value = nullptr;
  if (stmt.value != nullptr) {
    value = evaluate(*stmt.value);
  }
  throw ReturnException(value);
}

void Interpreter::visitIfStmt(If& stmt) {
  if (isTruthy(evaluate(*stmt.condition))) {
    execute(*stmt.thenBranch);
  } else if (stmt.elseBranch != nullptr) {
    execute(*stmt.elseBranch);
  }
}

void Interpreter::visitWhileStmt(While& stmt) {
  while (isTruthy(evaluate(*stmt.condition))) {
    execute(*stmt.body);
  }
}

bool Interpreter::isTruthy(const Value& value) const {
  if (holds_alternative<nullptr_t>(value)) return false;
  if (holds_alternative<bool>(value)) return get<bool>(value);
  return true;
}

bool Interpreter::isEqual(const Value& a, const Value& b) const {
  if (holds_alternative<nullptr_t>(a) && holds_alternative<nullptr_t>(b)) return true;
  if (holds_alternative<nullptr_t>(a) || holds_alternative<nullptr_t>(b)) return false;
  if (holds_alternative<bool>(a) && holds_alternative<bool>(b)) return get<bool>(a) == get<bool>(b);
  if (holds_alternative<double>(a) && holds_alternative<double>(b)) return get<double>(a) == get<double>(b);
  if (holds_alternative<string>(a) && holds_alternative<string>(b)) return get<string>(a) == get<string>(b);
  if (holds_alternative<shared_ptr<LoxCallable>>(a) && holds_alternative<shared_ptr<LoxCallable>>(b))
    return get<shared_ptr<LoxCallable>>(a) == get<shared_ptr<LoxCallable>>(b);
  if (holds_alternative<shared_ptr<LoxInstance>>(a) && holds_alternative<shared_ptr<LoxInstance>>(b))
    return get<shared_ptr<LoxInstance>>(a) == get<shared_ptr<LoxInstance>>(b);
  return false;
}

void Interpreter::checkNumberOperand(const Token& oper, const Value& operand) const {
  if (holds_alternative<double>(operand)) return;
  throw RuntimeError(oper, "Operand must be a number.");
}

void Interpreter::checkNumberOperands(const Token& oper, const Value& left, const Value& right) const {
  if (holds_alternative<double>(left) && holds_alternative<double>(right)) return;
  throw RuntimeError(oper, "Operands must be numbers.");
}

string Interpreter::stringify(const Value& value) const {
  if (holds_alternative<nullptr_t>(value)) return string("nil");
  if (holds_alternative<bool>(value)) return get<bool>(value) ? string("true") : string("false");
  if (holds_alternative<double>(value)) {
    string text = to_string(get<double>(value));
    if (text.find('.') != string::npos) {
      while (!text.empty() && text.back() == '0') text.pop_back();
      if (!text.empty() && text.back() == '.') text.pop_back();
    }
    return text;
  }
  if (holds_alternative<shared_ptr<LoxCallable>>(value)) {
    return get<shared_ptr<LoxCallable>>(value)->toString();
  }
  if (holds_alternative<shared_ptr<LoxInstance>>(value)) {
    return get<shared_ptr<LoxInstance>>(value)->toString();
  }
  return get<string>(value);
}
