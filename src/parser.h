#pragma once

#include <functional>

#include "common.h"
#include "token.h"
#include "scanner.h"

typedef std::function<void()> ParseFn;

struct Compiler;

enum Precedence {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
};

struct ParseRule {
    ParseFn prefix {};
    ParseFn infix {};
    Precedence precedence {};
};

struct Parser {
    Parser(std::shared_ptr<Scanner> scanner, Compiler &compiler);
    void advance();
    Token scan_token();
    Token& current();
    Token& previous();
    bool had_error();
    bool panic_mode();
    void error_at_current();
    void error(const char* message);
    void error_at(Token &token, const char * message);
    void consume(TokenType type, const char* message);
    void parse_precedence(Precedence precedence);

    inline ParseRule& get_rule(TokenType type) {
        return m_rules[type];
    }

private:
    std::shared_ptr<Scanner> m_scanner;
    Compiler &m_compiler;
    Token m_current {};
    Token m_previous {};
    bool m_had_error {false};
    bool m_panic_mode {false};

    std::vector<ParseRule> m_rules {};

};