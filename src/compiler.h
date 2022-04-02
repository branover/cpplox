#pragma once

#include <memory>
#include <functional>

#include "chunk.h"
#include "parser.h"

struct Compiler {
    // bool compile(const std::string &source, Chunk &chunk); 
    Compiler();
    bool compile(const std::string &source, std::shared_ptr<Chunk> chunk);   
    void advance();
    void consume(TokenType type, const char* message);

    void emit_byte(uint8_t byte);
    void emit_bytes(uint8_t byte1, uint8_t byte2);
    void emit_return();
    void emit_constant(Value value);

    std::shared_ptr<Chunk> current_chunk();
    void end_compiler();

    void expression();
    void number();
    void grouping();
    void unary();
    void binary();

    uint8_t make_constant(Value value);

private:
    std::unique_ptr<Parser> m_parser;
    std::shared_ptr<Scanner> m_scanner;
    std::shared_ptr<Chunk> m_compiling_chunk;

    // std::vector<ParseRule> m_rules {};

};
