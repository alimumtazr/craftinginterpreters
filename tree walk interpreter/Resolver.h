#ifndef RESOLVER_H
#define RESOLVER_H

#include "Expr.h"
#include "Stmt.h"
#include "Interpreter.h"
#include <map>
#include <string>
#include <vector>

using namespace std;

class Resolver : public ExprVisitor, public StmtVisitor {
public:
  explicit Resolver(Interpreter& interpreter);

  void resolve(const vector<unique_ptr<Stmt>>& statements);
  bool hadError() const { return hadError_; }

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
  enum class FunctionType { NONE, FUNCTION, INITIALIZER, METHOD };
  enum class ClassType { NONE, CLASS, SUBCLASS };

  void resolve(Stmt& stmt);
  void resolve(Expr& expr);
  void resolveFunction(Function& function, FunctionType type);
  void beginScope();
  void endScope();
  void declare(const Token& name);
  void define(const Token& name);
  void resolveLocal(Expr& expr, const Token& name);
  void error(const Token& token, const string& message);

  Interpreter& interpreter;
  vector<map<string, bool>> scopes;
  FunctionType currentFunction = FunctionType::NONE;
  ClassType currentClass = ClassType::NONE;
  bool hadError_ = false;
};

#endif // RESOLVER_H
