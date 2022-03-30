
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"
#include "token.h"

Scanner::Scanner(const std::string &source):
m_source {source} {
    m_start = m_source.c_str();
    m_current = m_start;
    m_line = 1;
}


Token Scanner::scan_token() {
    skip_whitespace();
    m_start = m_current;

    if (is_at_end()) return make_token(TOKEN_EOF);

    char c = advance();

    if (is_alpha(c)) return identifier();
    if (is_digit(c)) return number();

    switch (c) {
        case '(': return make_token(TOKEN_LEFT_PAREN);
        case ')': return make_token(TOKEN_RIGHT_PAREN);
        case '{': return make_token(TOKEN_LEFT_BRACE);
        case '}': return make_token(TOKEN_RIGHT_BRACE);
        case ';': return make_token(TOKEN_SEMICOLON);
        case ',': return make_token(TOKEN_COMMA);
        case '.': return make_token(TOKEN_DOT);
        case '-': return make_token(TOKEN_MINUS);
        case '+': return make_token(TOKEN_PLUS);
        case '/': return make_token(TOKEN_SLASH);
        case '*': return make_token(TOKEN_STAR);
        case '!':
            return make_token(
                match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return make_token(
                match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return make_token(
                match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return make_token(
                match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"': return string();
    }

    return error_token("Unexpected character.");
}

bool Scanner::is_at_end() const {
    return *m_current == '\0';
}

Token Scanner::make_token(TokenType type) const {
    Token token;
    token.type = type;
    token.start = m_start;
    token.length = (int)(m_current - m_start);
    token.line = m_line;
    return token;
}

Token Scanner::error_token(const std::string &message) const {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message.c_str();
    token.length = message.length();
    token.line = m_line;
    return token;
}

bool Scanner::match(char expected) {
    if (is_at_end()) return false;
    if (*m_current != expected) return false;
    m_current++;
    return true;
}

void Scanner::skip_whitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                m_line++;
                advance();
                break;
            case '/':
                if (peek_next() == '/') {
                    while (peek() != '\n' && !is_at_end()) advance();
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

Token Scanner::string() {
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') m_line++;
        advance();
    }

    if (is_at_end()) return error_token("Unterminated string");

    advance();
    return make_token(TOKEN_STRING);
}

Token Scanner::number() {
    while (is_digit(peek())) advance();

    // Look for a fractional part
    if (peek() == '.' && is_digit(peek_next())) {
        // Consume the '.'
        advance();

        while (is_digit(peek())) advance();
    }

    return make_token(TOKEN_NUMBER);
}

Token Scanner::identifier() {
    while (is_alpha(peek()) || is_digit(peek())) advance();
    return make_token(identifier_type());
}

TokenType Scanner::identifier_type() const {
    switch (m_start[0]) {
        case 'a': return check_keyword(1, 2, "nd", TOKEN_AND);
        case 'c': return check_keyword(1, 4, "lass", TOKEN_CLASS);
        case 'e': return check_keyword(1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (m_current - m_start > 1) {
                switch (m_start[1]) {
                case 'a': return check_keyword(2, 3, "lse", TOKEN_FALSE);
                case 'o': return check_keyword(2, 1, "r", TOKEN_FOR);
                case 'u': return check_keyword(2, 1, "n", TOKEN_FUN);
                }
            }
            break;
        case 'i': return check_keyword(1, 1, "f", TOKEN_IF);
        case 'n': return check_keyword(1, 2, "il", TOKEN_NIL);
        case 'o': return check_keyword(1, 1, "r", TOKEN_OR);
        case 'p': return check_keyword(1, 4, "rint", TOKEN_PRINT);
        case 'r': return check_keyword(1, 5, "eturn", TOKEN_RETURN);
        case 's': return check_keyword(1, 4, "uper", TOKEN_SUPER);
        case 't':
            if (m_current - m_start > 1) {
                switch (m_start[1]) {
                case 'h': return check_keyword(2, 2, "is", TOKEN_THIS);
                case 'r': return check_keyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
        case 'v': return check_keyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return check_keyword(1, 4, "hile", TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;
}

TokenType Scanner::check_keyword(int start, int length, const char* rest, TokenType type) const {
    if (m_current - m_start == start + length &&
        memcmp(m_start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}
