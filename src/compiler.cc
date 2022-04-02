#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "chunk.h"
#include "parser.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif


Compiler::Compiler() {
 
}

bool Compiler::compile(const std::string &source, std::shared_ptr<Chunk> chunk) {
    std::cout << "SOURCE: " << source << std::endl;
    m_scanner = std::make_shared<Scanner>(Scanner(source));
    m_parser = std::make_unique<Parser>(Parser {m_scanner, *this});
    m_compiling_chunk = chunk;
    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");
    end_compiler();
    return !m_parser->had_error();
}

void Compiler::advance() {
    m_parser->advance();
}

void Compiler::consume(TokenType type, const char* message) {
    m_parser->consume(type, message);
}

void Compiler::emit_byte(uint8_t byte) {
    current_chunk()->write_chunk(byte, m_parser->previous().line);
}

void Compiler::emit_bytes(uint8_t byte1, uint8_t byte2) {
    emit_byte(byte1);
    emit_byte(byte2);
}

std::shared_ptr<Chunk> Compiler::current_chunk() {
    return m_compiling_chunk;
}

void Compiler::end_compiler() {
    emit_return();
#ifdef DEBUG_PRINT_CODE
    if (!m_parser->had_error()) {
        std::cerr << disassemble_chunk(*current_chunk(), "code").str();
    }
#endif
}

void Compiler::emit_return() {
    emit_byte(OP_RETURN);
}

void Compiler::emit_constant(Value value) {
    emit_bytes(OP_CONSTANT, make_constant(value));
}

void Compiler::expression() {
    m_parser->parse_precedence(PREC_ASSIGNMENT);
}

void Compiler::number() {
    double value = strtod(m_parser->previous().start, nullptr);
    emit_constant(value);
}

void Compiler::grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::unary() {
    TokenType operator_type = m_parser->previous().type;

    // Compile the operand.
    m_parser->parse_precedence(PREC_UNARY);

    // Emit the operator instruction.
    switch (operator_type) {
        case TOKEN_MINUS: emit_byte(OP_NEGATE); break;
        default: return;
    }
}

void Compiler::binary() {
    TokenType operator_type = m_parser->previous().type;
    ParseRule& rule {m_parser->get_rule(operator_type)};
    m_parser->parse_precedence(static_cast<Precedence>(rule.precedence + 1));

    switch (operator_type) {
        case TOKEN_PLUS:          emit_byte(OP_ADD); break;
        case TOKEN_MINUS:         emit_byte(OP_SUBTRACT); break;
        case TOKEN_STAR:          emit_byte(OP_MULTIPLY); break;
        case TOKEN_SLASH:         emit_byte(OP_DIVIDE); break;
        default: return; // Unreachable.    
    }
}

uint8_t Compiler::make_constant(Value value) {
    int constant = current_chunk()->add_constant(value);
    if (constant > UINT8_MAX) {
        m_parser->error("Too many constants in one chunk");
        return 0;
    }

    return (uint8_t) constant;
}


