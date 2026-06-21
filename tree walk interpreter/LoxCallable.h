#ifndef LOX_CALLABLE_H
#define LOX_CALLABLE_H

#include "Expr.h"
#include <string>
#include <vector>

using namespace std;

class Interpreter; // forward

class LoxCallable {
public:
  virtual ~LoxCallable() = default;
  virtual int arity() const = 0;
  virtual Value call(Interpreter& interpreter, vector<Value>& arguments) = 0;
  virtual string toString() const = 0;
};

#endif // LOX_CALLABLE_H
