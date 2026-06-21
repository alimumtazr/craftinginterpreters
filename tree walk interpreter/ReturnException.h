#ifndef RETURN_EXCEPTION_H
#define RETURN_EXCEPTION_H

#include "Expr.h"

// Control-flow exception used to unwind the call stack from a `return`
// statement back to LoxFunction::call(). Not an error.
struct ReturnException {
  Value value;
  explicit ReturnException(Value value) : value(std::move(value)) {}
};

#endif // RETURN_EXCEPTION_H
