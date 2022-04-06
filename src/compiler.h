#pragma once

#include <memory>
#include <functional>

#include "chunk.h"
#include "parser.h"
#include "token.h"

struct Local {
    Token name {};
    int depth;
};

struct Compiler {
    // bool compile(const std::string &source, Chunk &chunk); 
    Compiler();
    bool compile(const std::string &source, std::shared_ptr<Chunk> chunk);   
    void advance();
    void consume(TokenType type, const char* message);

    void emit_byte(uint8_t byte);
    void emit_bytes(uint8_t byte1, uint8_t byte2);
    void emit_return();
    void emit_constant(Value &value);

    std::shared_ptr<Chunk> current_chunk();
    void end_compiler();

    void expression();
    void number(bool can_assign);
    void grouping(bool can_assign);
    void unary(bool can_assign);
    void binary(bool can_assign);
    void literal(bool can_assign);
    void string(bool can_assign);
    void variable(bool can_assign);


    void declaration();
    void var_declaration();
    void statement();
    void print_statement();
    void expression_statement();
    void block();

    void begin_scope();
    void end_scope();

    bool match(TokenType type);

    uint8_t parse_variable(const char* error_message);
    void declare_variable();
    void define_variable(uint8_t global_index);
    uint8_t identifier_constant(Token& toknen);
    uint8_t make_constant(Value &value);
    void named_variable(Token &name, bool can_assign);   
    void add_local(Token &name);
    int resolve_local(Token &name);
    bool identifiers_equal(Token &a, Token &b);

private:
    std::unique_ptr<Parser> m_parser;
    std::shared_ptr<Scanner> m_scanner;
    std::shared_ptr<Chunk> m_compiling_chunk;

    // Locals
    std::vector<Local> m_locals {};
    int m_scope_depth {0};
};
