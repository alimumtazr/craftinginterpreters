#ifndef LOX_H
#define LOX_H

#include <string>

struct RuntimeError;

class Lox {
public:
  Lox();
  void run(const std::string& source);
  void runFile(const std::string& path);
  void runPrompt();
  void error(int line, const std::string& msg);
  bool hadError() const;
  static void runtimeError(const RuntimeError& error);

  static bool hadRuntimeError;

private:
  bool hadError_;
  void report(int line, const std::string& where, const std::string& msg);
};

#endif // LOX_H
