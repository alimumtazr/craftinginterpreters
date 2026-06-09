#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "Expr.h"
#include <initializer_list>
#include <string>
#include <sstream>

using namespace std;

class AstPrinter : public ExprVisitor {
public:
  string print(Expr& expr);

  // Visitor methods
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

private:
  string parenthesize(const string& name, initializer_list<Expr*> exprs);
};

#endif // AST_PRINTER_H
