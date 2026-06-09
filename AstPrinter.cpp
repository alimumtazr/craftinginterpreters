#include "AstPrinter.h"
#include <iostream>

string AstPrinter::print(Expr& expr) {
  Value value = expr.accept(*this);
  return get<string>(value);
}

Value AstPrinter::visitBinaryExpr(Binary& expr) {
  return parenthesize(expr.oper.lexeme, { expr.left.get(), expr.right.get() });
}

Value AstPrinter::visitGroupingExpr(Grouping& expr) {
  return parenthesize("group", { expr.expression.get() });
}

Value AstPrinter::visitLiteralExpr(Literal& expr) {
  if (expr.token.type == NIL) return string("nil");
  if (expr.token.type == TRUE_TOK) return string("true");
  if (expr.token.type == FALSE_TOK) return string("false");
  if (!expr.token.literal.empty()) return expr.token.literal;
  return expr.token.lexeme;
}

Value AstPrinter::visitUnaryExpr(Unary& expr) {
  return parenthesize(expr.oper.lexeme, { expr.right.get() });
}

Value AstPrinter::visitVariableExpr(Variable& expr) {
  return expr.name.lexeme;
}

Value AstPrinter::visitAssignExpr(Assign& expr) {
  return parenthesize("=", { expr.value.get() });
}

Value AstPrinter::visitLogicalExpr(Logical& expr) {
  return parenthesize(expr.oper.lexeme, { expr.left.get(), expr.right.get() });
}

Value AstPrinter::visitCallExpr(Call& expr) {
  ostringstream builder;
  builder << "(call " << get<string>(expr.callee->accept(*this));
  for (const auto& argument : expr.arguments) {
    builder << " " << get<string>(argument->accept(*this));
  }
  builder << ")";
  return builder.str();
}

Value AstPrinter::visitGetExpr(Get& expr) {
  return parenthesize("." + expr.name.lexeme, { expr.object.get() });
}

Value AstPrinter::visitSetExpr(Set& expr) {
  return parenthesize("=." + expr.name.lexeme,
                      { expr.object.get(), expr.value.get() });
}

Value AstPrinter::visitThisExpr(This& expr) {
  return string("this");
}

Value AstPrinter::visitSuperExpr(Super& expr) {
  return string("(super ") + expr.method.lexeme + ")";
}

string AstPrinter::parenthesize(const string& name, initializer_list<Expr*> exprs) {
  ostringstream builder;
  builder << "(" << name;
  for (Expr* e : exprs) {
    builder << " ";
    builder << get<string>(e->accept(*this));
  }
  builder << ")";
  return builder.str();
}
