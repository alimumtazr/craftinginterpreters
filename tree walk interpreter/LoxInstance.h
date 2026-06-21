#ifndef LOX_INSTANCE_H
#define LOX_INSTANCE_H

#include "Expr.h"
#include "Token.h"
#include <map>
#include <memory>
#include <string>

using namespace std;

class LoxClass; // forward

class LoxInstance : public enable_shared_from_this<LoxInstance> {
public:
  explicit LoxInstance(shared_ptr<LoxClass> klass)
      : klass(std::move(klass)) {}

  Value get(const Token& name);
  void set(const Token& name, const Value& value);
  string toString() const;

private:
  shared_ptr<LoxClass> klass;
  map<string, Value> fields;
};

#endif // LOX_INSTANCE_H
