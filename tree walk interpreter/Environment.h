#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "Expr.h"
#include "Token.h"
#include "RuntimeError.h"
#include <map>
#include <memory>
#include <string>

using namespace std;

class Environment {
public:
  Environment() : enclosing(nullptr) {}
  explicit Environment(shared_ptr<Environment> enclosing)
      : enclosing(std::move(enclosing)) {}

  void define(const string& name, const Value& value) {
    values[name] = value;
  }

  Value get(const Token& name) const {
    auto it = values.find(name.lexeme);
    if (it != values.end()) {
      return it->second;
    }

    if (enclosing != nullptr) {
      return enclosing->get(name);
    }

    throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
  }

  void assign(const Token& name, const Value& value) {
    auto it = values.find(name.lexeme);
    if (it != values.end()) {
      it->second = value;
      return;
    }

    if (enclosing != nullptr) {
      enclosing->assign(name, value);
      return;
    }

    throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
  }

  Value getAt(int distance, const string& name) {
    return ancestor(distance)->values.at(name);
  }

  void assignAt(int distance, const Token& name, const Value& value) {
    ancestor(distance)->values[name.lexeme] = value;
  }

private:
  Environment* ancestor(int distance) {
    Environment* environment = this;
    for (int i = 0; i < distance; i++) {
      environment = environment->enclosing.get();
    }
    return environment;
  }

  shared_ptr<Environment> enclosing;
  map<string, Value> values;
};

#endif // ENVIRONMENT_H
