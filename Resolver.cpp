#include "Resolver.h"
#include <iostream>

Resolver::Resolver(Interpreter& interpreter) : interpreter(interpreter) {}

void Resolver::resolve(const vector<unique_ptr<Stmt>>& statements) {
  for (const auto& statement : statements) {
    resolve(*statement);
  }
}

void Resolver::resolve(Stmt& stmt) {
  stmt.accept(*this);
}

void Resolver::resolve(Expr& expr) {
  expr.accept(*this);
}

void Resolver::beginScope() {
  scopes.emplace_back();
}

void Resolver::endScope() {
  scopes.pop_back();
}

void Resolver::declare(const Token& name) {
  if (scopes.empty()) return;

  map<string, bool>& scope = scopes.back();
  if (scope.find(name.lexeme) != scope.end()) {
    error(name, "Already a variable with this name in this scope.");
  }
  scope[name.lexeme] = false;
}

void Resolver::define(const Token& name) {
  if (scopes.empty()) return;
  scopes.back()[name.lexeme] = true;
}

void Resolver::resolveLocal(Expr& expr, const Token& name) {
  for (int i = (int)scopes.size() - 1; i >= 0; i--) {
    if (scopes[i].find(name.lexeme) != scopes[i].end()) {
      interpreter.resolve(&expr, (int)scopes.size() - 1 - i);
      return;
    }
  }
}

void Resolver::resolveFunction(Function& function, FunctionType type) {
  FunctionType enclosingFunction = currentFunction;
  currentFunction = type;

  beginScope();
  for (const Token& param : function.params) {
    declare(param);
    define(param);
  }
  resolve(function.body);
  endScope();

  currentFunction = enclosingFunction;
}

void Resolver::visitBlockStmt(Block& stmt) {
  beginScope();
  resolve(stmt.statements);
  endScope();
}

void Resolver::visitVarStmt(Var& stmt) {
  declare(stmt.name);
  if (stmt.initializer != nullptr) {
    resolve(*stmt.initializer);
  }
  define(stmt.name);
}

void Resolver::visitFunctionStmt(Function& stmt) {
  declare(stmt.name);
  define(stmt.name);

  resolveFunction(stmt, FunctionType::FUNCTION);
}

void Resolver::visitClassStmt(Class& stmt) {
  ClassType enclosingClass = currentClass;
  currentClass = ClassType::CLASS;

  declare(stmt.name);
  define(stmt.name);

  if (stmt.superclass != nullptr &&
      stmt.name.lexeme == stmt.superclass->name.lexeme) {
    error(stmt.superclass->name, "A class can't inherit from itself.");
  }

  if (stmt.superclass != nullptr) {
    currentClass = ClassType::SUBCLASS;
    resolve(*stmt.superclass);
  }

  if (stmt.superclass != nullptr) {
    beginScope();
    scopes.back()["super"] = true;
  }

  beginScope();
  scopes.back()["this"] = true;

  for (const auto& method : stmt.methods) {
    FunctionType declaration = FunctionType::METHOD;
    if (method->name.lexeme == "init") {
      declaration = FunctionType::INITIALIZER;
    }
    resolveFunction(*method, declaration);
  }

  endScope();

  if (stmt.superclass != nullptr) endScope();

  currentClass = enclosingClass;
}

void Resolver::visitExpressionStmt(Expression& stmt) {
  resolve(*stmt.expression);
}

void Resolver::visitIfStmt(If& stmt) {
  resolve(*stmt.condition);
  resolve(*stmt.thenBranch);
  if (stmt.elseBranch != nullptr) resolve(*stmt.elseBranch);
}

void Resolver::visitPrintStmt(Print& stmt) {
  resolve(*stmt.expression);
}

void Resolver::visitReturnStmt(Return& stmt) {
  if (currentFunction == FunctionType::NONE) {
    error(stmt.keyword, "Can't return from top-level code.");
  }

  if (stmt.value != nullptr) {
    if (currentFunction == FunctionType::INITIALIZER) {
      error(stmt.keyword, "Can't return a value from an initializer.");
    }
    resolve(*stmt.value);
  }
}

void Resolver::visitWhileStmt(While& stmt) {
  resolve(*stmt.condition);
  resolve(*stmt.body);
}

Value Resolver::visitVariableExpr(Variable& expr) {
  if (!scopes.empty()) {
    map<string, bool>& scope = scopes.back();
    auto it = scope.find(expr.name.lexeme);
    if (it != scope.end() && it->second == false) {
      error(expr.name, "Can't read local variable in its own initializer.");
    }
  }

  resolveLocal(expr, expr.name);
  return nullptr;
}

Value Resolver::visitAssignExpr(Assign& expr) {
  resolve(*expr.value);
  resolveLocal(expr, expr.name);
  return nullptr;
}

Value Resolver::visitBinaryExpr(Binary& expr) {
  resolve(*expr.left);
  resolve(*expr.right);
  return nullptr;
}

Value Resolver::visitCallExpr(Call& expr) {
  resolve(*expr.callee);
  for (const auto& argument : expr.arguments) {
    resolve(*argument);
  }
  return nullptr;
}

Value Resolver::visitGetExpr(Get& expr) {
  resolve(*expr.object);
  return nullptr;
}

Value Resolver::visitSetExpr(Set& expr) {
  resolve(*expr.value);
  resolve(*expr.object);
  return nullptr;
}

Value Resolver::visitThisExpr(This& expr) {
  if (currentClass == ClassType::NONE) {
    error(expr.keyword, "Can't use 'this' outside of a class.");
    return nullptr;
  }

  resolveLocal(expr, expr.keyword);
  return nullptr;
}

Value Resolver::visitSuperExpr(Super& expr) {
  if (currentClass == ClassType::NONE) {
    error(expr.keyword, "Can't use 'super' outside of a class.");
  } else if (currentClass != ClassType::SUBCLASS) {
    error(expr.keyword, "Can't use 'super' in a class with no superclass.");
  }

  resolveLocal(expr, expr.keyword);
  return nullptr;
}

Value Resolver::visitGroupingExpr(Grouping& expr) {
  resolve(*expr.expression);
  return nullptr;
}

Value Resolver::visitLiteralExpr(Literal&) {
  return nullptr;
}

Value Resolver::visitLogicalExpr(Logical& expr) {
  resolve(*expr.left);
  resolve(*expr.right);
  return nullptr;
}

Value Resolver::visitUnaryExpr(Unary& expr) {
  resolve(*expr.right);
  return nullptr;
}

void Resolver::error(const Token& token, const string& message) {
  hadError_ = true;
  if (token.type == EOF_TOK) {
    cerr << "[line " << token.line << "] Error at end: " << message << endl;
  } else {
    cerr << "[line " << token.line << "] Error at '" << token.lexeme
         << "': " << message << endl;
  }
}
