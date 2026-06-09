#ifndef STMT_H
#define STMT_H

#include "Expr.h"
#include <memory>
#include <vector>

using namespace std;

class StmtVisitor;

class Stmt {
public:
  virtual ~Stmt() = default;
  virtual void accept(StmtVisitor& visitor) = 0;
};

class Expression;
class Print;
class Var;
class Block;
class If;
class While;
class Function;
class Return;
class Class;

class StmtVisitor {
public:
  virtual ~StmtVisitor() = default;
  virtual void visitExpressionStmt(Expression& stmt) = 0;
  virtual void visitPrintStmt(Print& stmt) = 0;
  virtual void visitVarStmt(Var& stmt) = 0;
  virtual void visitBlockStmt(Block& stmt) = 0;
  virtual void visitIfStmt(If& stmt) = 0;
  virtual void visitWhileStmt(While& stmt) = 0;
  virtual void visitFunctionStmt(Function& stmt) = 0;
  virtual void visitReturnStmt(Return& stmt) = 0;
  virtual void visitClassStmt(Class& stmt) = 0;
};

class Expression : public Stmt {
public:
  Expression(unique_ptr<Expr> expression)
      : expression(move(expression)) {}

  void accept(StmtVisitor& visitor) override {
    visitor.visitExpressionStmt(*this);
  }

  unique_ptr<Expr> expression;
};

class Print : public Stmt {
public:
  Print(unique_ptr<Expr> expression)
      : expression(move(expression)) {}

  void accept(StmtVisitor& visitor) override {
    visitor.visitPrintStmt(*this);
  }

  unique_ptr<Expr> expression;
};

class Var : public Stmt {
public:
  Var(const Token& name, unique_ptr<Expr> initializer)
      : name(name), initializer(move(initializer)) {}

  void accept(StmtVisitor& visitor) override {
    visitor.visitVarStmt(*this);
  }

  Token name;
  unique_ptr<Expr> initializer;
};

class Block : public Stmt {
public:
  Block(vector<unique_ptr<Stmt>> statements)
      : statements(move(statements)) {}

  void accept(StmtVisitor& visitor) override {
    visitor.visitBlockStmt(*this);
  }

  vector<unique_ptr<Stmt>> statements;
};

class If : public Stmt {
public:
  If(unique_ptr<Expr> condition, unique_ptr<Stmt> thenBranch, unique_ptr<Stmt> elseBranch)
      : condition(move(condition)), thenBranch(move(thenBranch)), elseBranch(move(elseBranch)) {}

  void accept(StmtVisitor& visitor) override {
    visitor.visitIfStmt(*this);
  }

  unique_ptr<Expr> condition;
  unique_ptr<Stmt> thenBranch;
  unique_ptr<Stmt> elseBranch;
};

class While : public Stmt {
public:
  While(unique_ptr<Expr> condition, unique_ptr<Stmt> body)
      : condition(move(condition)), body(move(body)) {}

  void accept(StmtVisitor& visitor) override {
    visitor.visitWhileStmt(*this);
  }

  unique_ptr<Expr> condition;
  unique_ptr<Stmt> body;
};

class Function : public Stmt {
public:
  Function(const Token& name, vector<Token> params,
           vector<unique_ptr<Stmt>> body)
      : name(name), params(move(params)), body(move(body)) {}

  void accept(StmtVisitor& visitor) override {
    visitor.visitFunctionStmt(*this);
  }

  Token name;
  vector<Token> params;
  vector<unique_ptr<Stmt>> body;
};

class Return : public Stmt {
public:
  Return(const Token& keyword, unique_ptr<Expr> value)
      : keyword(keyword), value(move(value)) {}

  void accept(StmtVisitor& visitor) override {
    visitor.visitReturnStmt(*this);
  }

  Token keyword;
  unique_ptr<Expr> value;
};

class Class : public Stmt {
public:
  Class(const Token& name, unique_ptr<Variable> superclass,
        vector<unique_ptr<Function>> methods)
      : name(name), superclass(move(superclass)), methods(move(methods)) {}

  void accept(StmtVisitor& visitor) override {
    visitor.visitClassStmt(*this);
  }

  Token name;
  unique_ptr<Variable> superclass;
  vector<unique_ptr<Function>> methods;
};

#endif // STMT_H
