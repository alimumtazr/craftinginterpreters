#ifndef EXPR_H
#define EXPR_H

#include "Token.h"
#include <memory>
#include <string>
#include <variant>
#include <vector>

using namespace std;

class LoxCallable; // forward
class LoxInstance; // forward
using Value = variant<nullptr_t, bool, double, string,
                      shared_ptr<LoxCallable>, shared_ptr<LoxInstance>>;

class ExprVisitor; // forward

class Expr {
public:
  virtual ~Expr() = default;
  virtual Value accept(ExprVisitor& visitor) = 0;
};

class Binary;
class Grouping;
class Literal;
class Unary;
class Variable;
class Assign;
class Logical;
class Call;
class Get;
class Set;
class This;
class Super;

class ExprVisitor {
public:
  virtual ~ExprVisitor() = default;
  virtual Value visitBinaryExpr(Binary& expr) = 0;
  virtual Value visitGroupingExpr(Grouping& expr) = 0;
  virtual Value visitLiteralExpr(Literal& expr) = 0;
  virtual Value visitUnaryExpr(Unary& expr) = 0;
  virtual Value visitVariableExpr(Variable& expr) = 0;
  virtual Value visitAssignExpr(Assign& expr) = 0;
  virtual Value visitLogicalExpr(Logical& expr) = 0;
  virtual Value visitCallExpr(Call& expr) = 0;
  virtual Value visitGetExpr(Get& expr) = 0;
  virtual Value visitSetExpr(Set& expr) = 0;
  virtual Value visitThisExpr(This& expr) = 0;
  virtual Value visitSuperExpr(Super& expr) = 0;
};

class Binary : public Expr {
public:
  Binary(unique_ptr<Expr> left, const Token& oper, unique_ptr<Expr> right)
    : left(std::move(left)), oper(oper), right(std::move(right)) {}

  Value accept(ExprVisitor& visitor) override {
    return visitor.visitBinaryExpr(*this);
  }

  unique_ptr<Expr> left;
  Token oper;
  unique_ptr<Expr> right;
};

class Grouping : public Expr {
public:
  Grouping(unique_ptr<Expr> expression)
    : expression(std::move(expression)) {}

  Value accept(ExprVisitor& visitor) override {
    return visitor.visitGroupingExpr(*this);
  }

  unique_ptr<Expr> expression;
};

class Literal : public Expr {
public:
  Literal(const Token& token) : token(token) {}

  Value accept(ExprVisitor& visitor) override {
    return visitor.visitLiteralExpr(*this);
  }

  Token token;
};

class Unary : public Expr {
public:
  Unary(const Token& oper, unique_ptr<Expr> right)
    : oper(oper), right(std::move(right)) {}

  Value accept(ExprVisitor& visitor) override {
    return visitor.visitUnaryExpr(*this);
  }

  Token oper;
  unique_ptr<Expr> right;
};

class Variable : public Expr {
public:
  Variable(const Token& name) : name(name) {}

  Value accept(ExprVisitor& visitor) override {
    return visitor.visitVariableExpr(*this);
  }

  Token name;
};

class Assign : public Expr {
public:
  Assign(const Token& name, unique_ptr<Expr> value)
      : name(name), value(std::move(value)) {}

  Value accept(ExprVisitor& visitor) override {
    return visitor.visitAssignExpr(*this);
  }

  Token name;
  unique_ptr<Expr> value;
};

class Logical : public Expr {
public:
  Logical(unique_ptr<Expr> left, const Token& oper, unique_ptr<Expr> right)
      : left(std::move(left)), oper(oper), right(std::move(right)) {}

  Value accept(ExprVisitor& visitor) override {
    return visitor.visitLogicalExpr(*this);
  }

  unique_ptr<Expr> left;
  Token oper;
  unique_ptr<Expr> right;
};

class Call : public Expr {
public:
  Call(unique_ptr<Expr> callee, const Token& paren,
       vector<unique_ptr<Expr>> arguments)
    : callee(std::move(callee)), paren(paren), arguments(std::move(arguments)) {}

  Value accept(ExprVisitor& visitor) override {
    return visitor.visitCallExpr(*this);
  }

  unique_ptr<Expr> callee;
  Token paren;
  vector<unique_ptr<Expr>> arguments;
};

class Get : public Expr {
public:
  Get(unique_ptr<Expr> object, const Token& name)
    : object(std::move(object)), name(name) {}

  Value accept(ExprVisitor& visitor) override {
    return visitor.visitGetExpr(*this);
  }

  unique_ptr<Expr> object;
  Token name;
};

class Set : public Expr {
public:
  Set(unique_ptr<Expr> object, const Token& name, unique_ptr<Expr> value)
    : object(std::move(object)), name(name), value(std::move(value)) {}

  Value accept(ExprVisitor& visitor) override {
    return visitor.visitSetExpr(*this);
  }

  unique_ptr<Expr> object;
  Token name;
  unique_ptr<Expr> value;
};

class This : public Expr {
public:
  This(const Token& keyword) : keyword(keyword) {}

  Value accept(ExprVisitor& visitor) override {
    return visitor.visitThisExpr(*this);
  }

  Token keyword;
};

class Super : public Expr {
public:
  Super(const Token& keyword, const Token& method)
    : keyword(keyword), method(method) {}

  Value accept(ExprVisitor& visitor) override {
    return visitor.visitSuperExpr(*this);
  }

  Token keyword;
  Token method;
};

#endif // EXPR_H
