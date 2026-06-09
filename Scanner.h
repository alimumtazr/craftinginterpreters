#ifndef SCANNER_H
#define SCANNER_H

#include "Token.h"
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

class Scanner {
public:
  Scanner(const string& source);
  vector<Token> scanTokens();

private:
  string source;
  vector<Token> tokens;
  int start = 0;
  int current = 0;
  int line = 1;
  unordered_map<string, TokenType> keywords;

  void scanToken();
  bool isAtEnd() const;
  char advance();
  void addToken(TokenType type);
  void addToken(TokenType type, const string& literal);
  bool match(char expected);
  char peek() const;
  char peekNext() const;
  void scanString();
  void number();
  void identifier();
  bool isDigit(char c) const;
  bool isAlpha(char c) const;
  bool isAlphaNumeric(char c) const;
};

#endif // SCANNER_H
