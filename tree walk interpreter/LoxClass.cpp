#include "LoxClass.h"
#include "LoxFunction.h"
#include "LoxInstance.h"

shared_ptr<LoxFunction> LoxClass::findMethod(const string& name) const {
  auto it = methods.find(name);
  if (it != methods.end()) {
    return it->second;
  }

  if (superclass != nullptr) {
    return superclass->findMethod(name);
  }

  return nullptr;
}

int LoxClass::arity() const {
  shared_ptr<LoxFunction> initializer = findMethod("init");
  if (initializer == nullptr) return 0;
  return initializer->arity();
}

Value LoxClass::call(Interpreter& interpreter, vector<Value>& arguments) {
  shared_ptr<LoxInstance> instance = make_shared<LoxInstance>(shared_from_this());
  shared_ptr<LoxFunction> initializer = findMethod("init");
  if (initializer != nullptr) {
    initializer->bind(instance)->call(interpreter, arguments);
  }
  return instance;
}

string LoxClass::toString() const {
  return name;
}
