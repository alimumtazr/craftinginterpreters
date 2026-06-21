#include "Scanner.h"
#include <iostream>

using namespace std;

Scanner::Scanner(const string& source) : source(source) {
  // initialize keywords
  keywords["and"] = AND;
  keywords["class"] = CLASS;
  keywords["else"] = ELSE;
  keywords["false"] = FALSE_TOK;
  keywords["for"] = FOR;
  keywords["fun"] = FUN;
  keywords["if"] = IF;
  keywords["nil"] = NIL;
  keywords["or"] = OR;
  keywords["print"] = PRINT;
  keywords["return"] = RETURN;
  keywords["super"] = SUPER;
  keywords["this"] = THIS;
  keywords["true"] = TRUE_TOK;
  keywords["var"] = VAR;
  keywords["while"] = WHILE;
}

vector<Token> Scanner::scanTokens() {
  while (!isAtEnd()) {
    start = current;
    scanToken();
  }
  tokens.emplace_back(EOF_TOK, "", "", line);
  return tokens;
}

bool Scanner::isAtEnd() const { return current >= (int)source.length(); }

char Scanner::advance() { return source.at(current++); }

void Scanner::addToken(TokenType type) { addToken(type, ""); }

void Scanner::addToken(TokenType type, const string& literal) {
  string text = source.substr(start, current - start);
  tokens.emplace_back(type, text, literal, line);
}

bool Scanner::match(char expected) {
  if (isAtEnd()) {
    return false;
  }
  if (source.at(current) != expected) {
    return false;
  }
  current++;
  return true;
}

char Scanner::peek() const {
  if (isAtEnd()) {
    return '\0';
  }
  return source.at(current);
}

char Scanner::peekNext() const {
  if (current + 1 >= (int)source.length()) {
    return '\0';
  }
  return source.at(current + 1);
}

void Scanner::scanToken() {
  char c = advance();
  switch (c) {
    case '(': addToken(LEFT_PAREN); break;
    case ')': addToken(RIGHT_PAREN); break;
    case '{': addToken(LEFT_BRACE); break;
    case '}': addToken(RIGHT_BRACE); break;
    case ',': addToken(COMMA); break;
    case '.': addToken(DOT); break;
    case '-': addToken(MINUS); break;
    case '+': addToken(PLUS); break;
    case ';': addToken(SEMICOLON); break;
    case '*': addToken(STAR); break;
    case '!':
      if (match('=')) {
        addToken(BANG_EQUAL);
      } else {
        addToken(BANG);
      }
      break;
    case '=':
      if (match('=')) {
        addToken(EQUAL_EQUAL);
      } else {
        addToken(EQUAL);
      }
      break;
    case '<':
      if (match('=')) {
        addToken(LESS_EQUAL);
      } else {
        addToken(LESS);
      }
      break;
    case '>':
      if (match('=')) {
        addToken(GREATER_EQUAL);
      } else {
        addToken(GREATER);
      }
      break;
    case '/':
      if (match('/')) {
        while (peek() != '\n' && !isAtEnd()) advance();
      } else {
        addToken(SLASH);
      }
      break;
    case ' ':
    case '\r':
    case '\t':
      break;
    case '\n':
      line++;
      break;
    case '"': scanString(); break;
    default:
      if (isDigit(c)) {
        number();
      } else if (isAlpha(c)) {
        identifier();
      } else {
        // Unknown char: report to cerr but continue
        cerr << "[line " << line << "] Unexpected character: '" << c << "'\n";
      }
      break;
  }
}

void Scanner::scanString() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n') line++;
    advance();
  }

  if (isAtEnd()) {
    cerr << "[line " << line << "] Unterminated string." << endl;
    return;
  }

  // closing quote
  advance();
  string value = source.substr(start + 1, current - start - 2);
  addToken(STRING, value);
}

void Scanner::number() {
  while (isDigit(peek())) {
    advance();
  }

  if (peek() == '.' && isDigit(peekNext())) {
    advance();
    while (isDigit(peek())) {
      advance();
    }
  }

  string value = source.substr(start, current - start);
  addToken(NUMBER, value);
}

void Scanner::identifier() {
  while (isAlphaNumeric(peek())) {
    advance();
  }
  string text = source.substr(start, current - start);
  unordered_map<string, TokenType>::const_iterator it = keywords.find(text);
  TokenType type;
  if (it != keywords.end()) {
    type = it->second;
  } else {
    type = IDENTIFIER;
  }
  addToken(type);
}

bool Scanner::isDigit(char c) const { return c >= '0' && c <= '9'; }

bool Scanner::isAlpha(char c) const {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Scanner::isAlphaNumeric(char c) const { return isAlpha(c) || isDigit(c); }
// end of Scanner.cpp