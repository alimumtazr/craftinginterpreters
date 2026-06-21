#ifndef RUNTIME_ERROR_H
#define RUNTIME_ERROR_H

#include "Token.h"
#include <stdexcept>
#include <string>

using namespace std;

struct RuntimeError : runtime_error {
  Token token;

  RuntimeError(const Token& token, const string& message)
      : runtime_error(message), token(token) {}
};

#endif // RUNTIME_ERROR_H
