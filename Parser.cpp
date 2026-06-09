#include "Parser.h"

Parser::Parser(const vector<Token>& tokens) : tokens(tokens) {}

vector<unique_ptr<Stmt>> Parser::parse() {
  vector<unique_ptr<Stmt>> statements;
  while (!isAtEnd()) {
    unique_ptr<Stmt> decl = declaration();
    if (decl != nullptr) {
      statements.push_back(move(decl));
    }
  }
  return statements;
}

unique_ptr<Stmt> Parser::declaration() {
  try {
    if (match(CLASS)) return classDeclaration();
    if (match(FUN)) return function("function");
    if (match(VAR)) return varDeclaration();
    return statement();
  } catch (const ParseError&) {
    synchronize();
    return nullptr;
  }
}

unique_ptr<Stmt> Parser::classDeclaration() {
  Token name = consume(IDENTIFIER, "Expect class name.");

  unique_ptr<Variable> superclass = nullptr;
  if (match(LESS)) {
    consume(IDENTIFIER, "Expect superclass name.");
    superclass = make_unique<Variable>(previous());
  }

  consume(LEFT_BRACE, "Expect '{' before class body.");

  vector<unique_ptr<Function>> methods;
  while (!check(RIGHT_BRACE) && !isAtEnd()) {
    methods.push_back(function("method"));
  }

  consume(RIGHT_BRACE, "Expect '}' after class body.");

  return make_unique<Class>(name, move(superclass), move(methods));
}

unique_ptr<Stmt> Parser::statement() {
  if (match(FOR)) return forStatement();
  if (match(IF)) return ifStatement();
  if (match(PRINT)) return printStatement();
  if (match(RETURN)) return returnStatement();
  if (match(WHILE)) return whileStatement();
  if (match(LEFT_BRACE)) return make_unique<Block>(block());
  return expressionStatement();
}

unique_ptr<Stmt> Parser::ifStatement() {
  consume(LEFT_PAREN, "Expect '(' after 'if'.");
  unique_ptr<Expr> condition = expression();
  consume(RIGHT_PAREN, "Expect ')' after if condition.");

  unique_ptr<Stmt> thenBranch = statement();
  unique_ptr<Stmt> elseBranch = nullptr;
  if (match(ELSE)) {
    elseBranch = statement();
  }

  return make_unique<If>(move(condition), move(thenBranch), move(elseBranch));
}

unique_ptr<Stmt> Parser::whileStatement() {
  consume(LEFT_PAREN, "Expect '(' after 'while'.");
  unique_ptr<Expr> condition = expression();
  consume(RIGHT_PAREN, "Expect ')' after condition.");
  unique_ptr<Stmt> body = statement();
  return make_unique<While>(move(condition), move(body));
}

unique_ptr<Stmt> Parser::forStatement() {
  consume(LEFT_PAREN, "Expect '(' after 'for'.");

  unique_ptr<Stmt> initializer;
  if (match(SEMICOLON)) {
    initializer = nullptr;
  } else if (match(VAR)) {
    initializer = varDeclaration();
  } else {
    initializer = expressionStatement();
  }

  unique_ptr<Expr> condition = nullptr;
  if (!check(SEMICOLON)) {
    condition = expression();
  }
  consume(SEMICOLON, "Expect ';' after loop condition.");

  unique_ptr<Expr> increment = nullptr;
  if (!check(RIGHT_PAREN)) {
    increment = expression();
  }
  consume(RIGHT_PAREN, "Expect ')' after for clauses.");

  unique_ptr<Stmt> body = statement();

  if (increment != nullptr) {
    vector<unique_ptr<Stmt>> statements;
    statements.push_back(move(body));
    statements.push_back(make_unique<Expression>(move(increment)));
    body = make_unique<Block>(move(statements));
  }

  if (condition == nullptr) {
    condition = make_unique<Literal>(Token{TRUE_TOK, "true", "true", 0});
  }

  body = make_unique<While>(move(condition), move(body));

  if (initializer != nullptr) {
    vector<unique_ptr<Stmt>> statements;
    statements.push_back(move(initializer));
    statements.push_back(move(body));
    body = make_unique<Block>(move(statements));
  }

  return body;
}

unique_ptr<Stmt> Parser::printStatement() {
  unique_ptr<Expr> value = expression();
  consume(SEMICOLON, "Expect ';' after value.");
  return make_unique<Print>(move(value));
}

unique_ptr<Stmt> Parser::returnStatement() {
  Token keyword = previous();
  unique_ptr<Expr> value = nullptr;
  if (!check(SEMICOLON)) {
    value = expression();
  }
  consume(SEMICOLON, "Expect ';' after return value.");
  return make_unique<Return>(keyword, move(value));
}

unique_ptr<Function> Parser::function(const string& kind) {
  Token name = consume(IDENTIFIER, "Expect " + kind + " name.");
  consume(LEFT_PAREN, "Expect '(' after " + kind + " name.");

  vector<Token> parameters;
  if (!check(RIGHT_PAREN)) {
    do {
      if (parameters.size() >= 255) {
        error(peek(), "Can't have more than 255 parameters.");
      }
      parameters.push_back(consume(IDENTIFIER, "Expect parameter name."));
    } while (match(COMMA));
  }
  consume(RIGHT_PAREN, "Expect ')' after parameters.");

  consume(LEFT_BRACE, "Expect '{' before " + kind + " body.");
  vector<unique_ptr<Stmt>> body = block();
  return make_unique<Function>(name, move(parameters), move(body));
}

unique_ptr<Stmt> Parser::expressionStatement() {
  unique_ptr<Expr> expr = expression();
  consume(SEMICOLON, "Expect ';' after expression.");
  return make_unique<Expression>(move(expr));
}

unique_ptr<Stmt> Parser::varDeclaration() {
  Token name = consume(IDENTIFIER, "Expect variable name.");

  unique_ptr<Expr> initializer = nullptr;
  if (match(EQUAL)) {
    initializer = expression();
  }

  consume(SEMICOLON, "Expect ';' after variable declaration.");
  return make_unique<Var>(name, move(initializer));
}

vector<unique_ptr<Stmt>> Parser::block() {
  vector<unique_ptr<Stmt>> statements;

  while (!check(RIGHT_BRACE) && !isAtEnd()) {
    unique_ptr<Stmt> decl = declaration();
    if (decl != nullptr) {
      statements.push_back(move(decl));
    }
  }

  consume(RIGHT_BRACE, "Expect '}' after block.");
  return statements;
}

unique_ptr<Expr> Parser::expression() {
  return assignment();
}

unique_ptr<Expr> Parser::assignment() {
  unique_ptr<Expr> expr = or_();

  if (match(EQUAL)) {
    Token equals = previous();
    unique_ptr<Expr> value = assignment();

    if (Variable* variable = dynamic_cast<Variable*>(expr.get())) {
      return make_unique<Assign>(variable->name, move(value));
    } else if (Get* get = dynamic_cast<Get*>(expr.get())) {
      return make_unique<Set>(move(get->object), get->name, move(value));
    }

    error(equals, "Invalid assignment target.");
  }

  return expr;
}

unique_ptr<Expr> Parser::or_() {
  unique_ptr<Expr> expr = and_();

  while (match(OR)) {
    Token oper = previous();
    unique_ptr<Expr> right = and_();
    expr = make_unique<Logical>(move(expr), oper, move(right));
  }

  return expr;
}

unique_ptr<Expr> Parser::and_() {
  unique_ptr<Expr> expr = equality();

  while (match(AND)) {
    Token oper = previous();
    unique_ptr<Expr> right = equality();
    expr = make_unique<Logical>(move(expr), oper, move(right));
  }

  return expr;
}

unique_ptr<Expr> Parser::equality() {
  unique_ptr<Expr> expr = comparison();

  while (match(BANG_EQUAL) || match(EQUAL_EQUAL)) {
    Token oper = previous();
    unique_ptr<Expr> right = comparison();
    expr = make_unique<Binary>(move(expr), oper, move(right));
  }

  return expr;
}

unique_ptr<Expr> Parser::comparison() {
  unique_ptr<Expr> expr = term();

  while (match(GREATER) || match(GREATER_EQUAL) || match(LESS) || match(LESS_EQUAL)) {
    Token oper = previous();
    unique_ptr<Expr> right = term();
    expr = make_unique<Binary>(move(expr), oper, move(right));
  }

  return expr;
}

unique_ptr<Expr> Parser::term() {
  unique_ptr<Expr> expr = factor();

  while (match(MINUS) || match(PLUS)) {
    Token oper = previous();
    unique_ptr<Expr> right = factor();
    expr = make_unique<Binary>(move(expr), oper, move(right));
  }

  return expr;
}

unique_ptr<Expr> Parser::factor() {
  unique_ptr<Expr> expr = unary();

  while (match(SLASH) || match(STAR)) {
    Token oper = previous();
    unique_ptr<Expr> right = unary();
    expr = make_unique<Binary>(move(expr), oper, move(right));
  }

  return expr;
}

unique_ptr<Expr> Parser::unary() {
  if (match(BANG) || match(MINUS)) {
    Token oper = previous();
    unique_ptr<Expr> right = unary();
    return make_unique<Unary>(oper, move(right));
  }

  return call();
}

unique_ptr<Expr> Parser::call() {
  unique_ptr<Expr> expr = primary();

  while (true) {
    if (match(LEFT_PAREN)) {
      expr = finishCall(move(expr));
    } else if (match(DOT)) {
      Token name = consume(IDENTIFIER, "Expect property name after '.'.");
      expr = make_unique<Get>(move(expr), name);
    } else {
      break;
    }
  }

  return expr;
}

unique_ptr<Expr> Parser::finishCall(unique_ptr<Expr> callee) {
  vector<unique_ptr<Expr>> arguments;
  if (!check(RIGHT_PAREN)) {
    do {
      if (arguments.size() >= 255) {
        error(peek(), "Can't have more than 255 arguments.");
      }
      arguments.push_back(expression());
    } while (match(COMMA));
  }

  Token paren = consume(RIGHT_PAREN, "Expect ')' after arguments.");
  return make_unique<Call>(move(callee), paren, move(arguments));
}

unique_ptr<Expr> Parser::primary() {
  if (match(FALSE_TOK)) {
    return make_unique<Literal>(previous());
  }
  if (match(TRUE_TOK)) {
    return make_unique<Literal>(previous());
  }
  if (match(NIL)) {
    return make_unique<Literal>(previous());
  }

  if (match(NUMBER) || match(STRING)) {
    return make_unique<Literal>(previous());
  }

  if (match(SUPER)) {
    Token keyword = previous();
    consume(DOT, "Expect '.' after 'super'.");
    Token method = consume(IDENTIFIER, "Expect superclass method name.");
    return make_unique<Super>(keyword, method);
  }

  if (match(THIS)) {
    return make_unique<This>(previous());
  }

  if (match(IDENTIFIER)) {
    return make_unique<Variable>(previous());
  }

  if (match(LEFT_PAREN)) {
    unique_ptr<Expr> expr = expression();
    consume(RIGHT_PAREN, "Expect ')' after expression.");
    return make_unique<Grouping>(move(expr));
  }

  throw error(peek(), "Expect expression.");
}

bool Parser::match(TokenType type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

bool Parser::check(TokenType type) const {
  if (isAtEnd()) return false;
  return peek().type == type;
}

Token Parser::advance() {
  if (!isAtEnd()) current++;
  return previous();
}

bool Parser::isAtEnd() const {
  return peek().type == EOF_TOK;
}

Token Parser::peek() const {
  return tokens[current];
}

Token Parser::previous() const {
  return tokens[current - 1];
}

Token Parser::consume(TokenType type, const string& message) {
  if (check(type)) return advance();
  throw error(peek(), message);
}

void Parser::synchronize() {
  advance();

  while (!isAtEnd()) {
    if (previous().type == SEMICOLON) return;

    switch (peek().type) {
      case CLASS:
      case FUN:
      case VAR:
      case FOR:
      case IF:
      case WHILE:
      case PRINT:
      case RETURN:
        return;
      default:
        break;
    }

    advance();
  }
}

Parser::ParseError Parser::error(const Token& token, const string& message) {
  reportError(token, message);
  return ParseError();
}

void Parser::reportError(const Token& token, const string& message) {
  if (token.type == EOF_TOK) {
    cerr << "[line " << token.line << "] Error at end: " << message << endl;
  } else {
    cerr << "[line " << token.line << "] Error at '" << token.lexeme << "': " << message << endl;
  }
}
