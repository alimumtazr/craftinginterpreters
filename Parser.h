#ifndef PARSER_H
#define PARSER_H

#include "Expr.h"
#include "Scanner.h"
#include "Stmt.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;

class Parser {
public:
  Parser(const vector<Token>& tokens);
  vector<unique_ptr<Stmt>> parse();

private:
  class ParseError {};

  unique_ptr<Stmt> declaration();
  unique_ptr<Stmt> statement();
  unique_ptr<Stmt> ifStatement();
  unique_ptr<Stmt> whileStatement();
  unique_ptr<Stmt> forStatement();
  unique_ptr<Stmt> printStatement();
  unique_ptr<Stmt> returnStatement();
  unique_ptr<Stmt> expressionStatement();
  unique_ptr<Stmt> classDeclaration();
  unique_ptr<Function> function(const string& kind);
  unique_ptr<Stmt> varDeclaration();
  vector<unique_ptr<Stmt>> block();

  unique_ptr<Expr> expression();
  unique_ptr<Expr> assignment();
  unique_ptr<Expr> or_();
  unique_ptr<Expr> and_();
  unique_ptr<Expr> equality();
  unique_ptr<Expr> comparison();
  unique_ptr<Expr> term();
  unique_ptr<Expr> factor();
  unique_ptr<Expr> unary();
  unique_ptr<Expr> call();
  unique_ptr<Expr> finishCall(unique_ptr<Expr> callee);
  unique_ptr<Expr> primary();

  bool match(TokenType type);
  bool check(TokenType type) const;
  Token advance();
  bool isAtEnd() const;
  Token peek() const;
  Token previous() const;
  Token consume(TokenType type, const string& message);
  ParseError error(const Token& token, const string& message);
  void reportError(const Token& token, const string& message);
  void synchronize();

  const vector<Token>& tokens;
  int current = 0;
};

#endif // PARSER_H
