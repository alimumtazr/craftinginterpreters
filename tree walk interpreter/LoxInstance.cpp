#include "LoxInstance.h"
#include "LoxClass.h"
#include "LoxFunction.h"
#include "RuntimeError.h"

Value LoxInstance::get(const Token& name) {
  auto it = fields.find(name.lexeme);
  if (it != fields.end()) {
    return it->second;
  }

  shared_ptr<LoxFunction> method = klass->findMethod(name.lexeme);
  if (method != nullptr) {
    return method->bind(shared_from_this());
  }

  throw RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
}

void LoxInstance::set(const Token& name, const Value& value) {
  fields[name.lexeme] = value;
}

string LoxInstance::toString() const {
  return klass->name + " instance";
}
