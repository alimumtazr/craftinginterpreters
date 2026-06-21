#include "LoxFunction.h"
#include "Interpreter.h"
#include "LoxInstance.h"
#include "ReturnException.h"

shared_ptr<LoxFunction> LoxFunction::bind(shared_ptr<LoxInstance> instance) {
  shared_ptr<Environment> environment = make_shared<Environment>(closure);
  environment->define("this", Value(instance));
  return make_shared<LoxFunction>(declaration, environment, isInitializer);
}

int LoxFunction::arity() const {
  return (int)declaration->params.size();
}

Value LoxFunction::call(Interpreter& interpreter, vector<Value>& arguments) {
  shared_ptr<Environment> environment = make_shared<Environment>(closure);
  for (size_t i = 0; i < declaration->params.size(); i++) {
    environment->define(declaration->params[i].lexeme, arguments[i]);
  }

  try {
    interpreter.executeBlock(declaration->body, environment);
  } catch (ReturnException& returnValue) {
    if (isInitializer) return closure->getAt(0, "this");
    return returnValue.value;
  }

  if (isInitializer) return closure->getAt(0, "this");
  return nullptr;
}

string LoxFunction::toString() const {
  return "<fn " + declaration->name.lexeme + ">";
}
