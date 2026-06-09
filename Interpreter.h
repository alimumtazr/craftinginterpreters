#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "Expr.h"
#include "Lox.h"
#include "Stmt.h"
#include "Environment.h"
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

using namespace std;

class Interpreter : public ExprVisitor, public StmtVisitor {
public:
  Interpreter();
  void interpret(const vector<unique_ptr<Stmt>>& statements);
  Value evaluate(Expr& expr);

  void executeBlock(const vector<unique_ptr<Stmt>>& statements,
                    shared_ptr<Environment> environment);
  void resolve(const Expr* expr, int depth);

  Value visitBinaryExpr(Binary& expr) override;
  Value visitGroupingExpr(Grouping& expr) override;
  Value visitLiteralExpr(Literal& expr) override;
  Value visitUnaryExpr(Unary& expr) override;
  Value visitVariableExpr(Variable& expr) override;
  Value visitAssignExpr(Assign& expr) override;
  Value visitLogicalExpr(Logical& expr) override;
  Value visitCallExpr(Call& expr) override;
  Value visitGetExpr(Get& expr) override;
  Value visitSetExpr(Set& expr) override;
  Value visitThisExpr(This& expr) override;
  Value visitSuperExpr(Super& expr) override;

  void visitExpressionStmt(Expression& stmt) override;
  void visitPrintStmt(Print& stmt) override;
  void visitVarStmt(Var& stmt) override;
  void visitBlockStmt(Block& stmt) override;
  void visitIfStmt(If& stmt) override;
  void visitWhileStmt(While& stmt) override;
  void visitFunctionStmt(Function& stmt) override;
  void visitReturnStmt(Return& stmt) override;
  void visitClassStmt(Class& stmt) override;

private:
  void execute(Stmt& stmt);

  Value lookUpVariable(const Token& name, const Expr* expr);
  bool isTruthy(const Value& value) const;
  bool isEqual(const Value& a, const Value& b) const;
  void checkNumberOperand(const Token& oper, const Value& operand) const;
  void checkNumberOperands(const Token& oper, const Value& left, const Value& right) const;
  string stringify(const Value& value) const;

  shared_ptr<Environment> globals;
  shared_ptr<Environment> environment;
  unordered_map<const Expr*, int> locals;
};

#endif // INTERPRETER_H
