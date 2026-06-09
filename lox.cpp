#include "Lox.h"
#include "Scanner.h"
#include "Parser.h"
#include "Interpreter.h"
#include "Resolver.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

using namespace std;

bool Lox::hadRuntimeError = false;

Lox::Lox() : hadError_(false) {}

void Lox::run(const string& source) {
  Scanner scanner(source);
  vector<Token> tokens = scanner.scanTokens();
  Parser parser(tokens);
  vector<unique_ptr<Stmt>> statements = parser.parse();
  if (statements.empty()) {
    return;
  }

  Interpreter interpreter;

  Resolver resolver(interpreter);
  resolver.resolve(statements);

  // Stop if there was a resolution error.
  if (resolver.hadError()) {
    return;
  }

  interpreter.interpret(statements);
}

void Lox::runFile(const string& path) {
  ifstream file(path);
  if (!file) {
    cerr << "Could not open file: " << path << endl;
    return;
  }
  string code{
    istreambuf_iterator<char>(file),
    istreambuf_iterator<char>()
  };
  run(code);
}

void Lox::runPrompt() {
  string line;
  while (true) {
    cout << "> ";
    if (!getline(cin, line)) break;
    run(line);
    hadError_ = false;
  }
}

void Lox::error(int line, const string& msg) {
  report(line, "", msg);
}

void Lox::report(int line, const string& where, const string& msg) {
  hadError_ = true;
  cerr << "[line " << line << "] Error " << where << ": " << msg << endl;
}

void Lox::runtimeError(const RuntimeError& error) {
  cerr << error.what() << "\n[line " << error.token.line << "]" << endl;
  hadRuntimeError = true;
}

bool Lox::hadError() const { return hadError_; }

int main(int argc, char* argv[])
{
	Lox lox;

	if (argc > 2)
	{
		cout << "Command line usage error" << endl;
		return 64;
	}
	else
	{
		if (argc == 2)
		{
			lox.runFile(argv[1]);
		}
		else {
			cout << "hello";
			lox.runPrompt();
		}
	}

	if (Lox::hadRuntimeError) {
		return 70;
	}
	if (lox.hadError()) {
		return 65;
	}
	return 0;
}