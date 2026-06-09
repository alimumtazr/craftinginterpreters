#ifndef LOX_FUNCTION_H
#define LOX_FUNCTION_H

#include "LoxCallable.h"
#include "Stmt.h"
#include "Environment.h"
#include <memory>
#include <string>
#include <vector>

using namespace std;

class LoxInstance; // forward

class LoxFunction : public LoxCallable {
public:
  LoxFunction(const Function* declaration, shared_ptr<Environment> closure,
              bool isInitializer)
      : declaration(declaration), closure(std::move(closure)),
        isInitializer(isInitializer) {}

  shared_ptr<LoxFunction> bind(shared_ptr<LoxInstance> instance);

  int arity() const override;
  Value call(Interpreter& interpreter, vector<Value>& arguments) override;
  string toString() const override;

private:
  const Function* declaration;
  shared_ptr<Environment> closure;
  bool isInitializer;
};

#endif // LOX_FUNCTION_H
