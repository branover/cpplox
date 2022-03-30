#pragma once

#include "common.h"
#include "token.h"

struct Scanner {
    Scanner(const std::string &source);
    Token scan_token();
    bool is_at_end() const;
    Token make_token(TokenType type) const;
    Token error_token(const std::string &message) const;
    bool match(char expected);
    void skip_whitespace();
    Token string();
    Token number();
    Token identifier();
    TokenType identifier_type() const;
    TokenType check_keyword(int start, int length, const char* rest, TokenType type) const;

    inline char advance() {
        m_current++;
        return m_current[-1];
    }
    inline char peek() const {
        return *m_current;
    }
    inline char peek_next() const {
        if (is_at_end()) return '\0';
        return m_current[1];
    }
    static inline bool is_digit(char c) {
        return c >= '0' && c <= '9';
    }
    static inline bool is_alpha(char c) {
        return (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                c == '_';
    }

    const std::string &m_source;
    const char* m_start {nullptr};
    const char* m_current {nullptr};
    int m_line {0}; 
};