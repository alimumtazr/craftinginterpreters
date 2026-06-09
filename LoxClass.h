#ifndef LOX_CLASS_H
#define LOX_CLASS_H

#include "LoxCallable.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace std;

class LoxFunction; // forward
class Interpreter;

class LoxClass : public LoxCallable,
                 public enable_shared_from_this<LoxClass> {
public:
  LoxClass(string name, shared_ptr<LoxClass> superclass,
           map<string, shared_ptr<LoxFunction>> methods)
      : name(std::move(name)), superclass(std::move(superclass)),
        methods(std::move(methods)) {}

  shared_ptr<LoxFunction> findMethod(const string& name) const;

  int arity() const override;
  Value call(Interpreter& interpreter, vector<Value>& arguments) override;
  string toString() const override;

  string name;
  shared_ptr<LoxClass> superclass;

private:
  map<string, shared_ptr<LoxFunction>> methods;
};

#endif // LOX_CLASS_H
