#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "chunk.h"
#include "parser.h"
#include "objects/object.h"
#include "objects/objstring.h"


#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif


Compiler::Compiler() {
 
}

bool Compiler::compile(const std::string &source, std::shared_ptr<Chunk> chunk) {
    m_scanner = std::make_shared<Scanner>(source);
    m_parser = std::make_unique<Parser>(m_scanner, *this);
    m_compiling_chunk = chunk;
    advance();

    while (!match(TOKEN_EOF)) {
        declaration();
    }
    // expression();
    // consume(TOKEN_EOF, "Expect end of expression.");

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

void Compiler::emit_constant(Value &value) {
    emit_bytes(OP_CONSTANT, make_constant(value));
}

void Compiler::expression() {
    m_parser->parse_precedence(PREC_ASSIGNMENT);
}

void Compiler::number(bool can_assign) {
    auto value = NUMBER_VAL(strtod(m_parser->previous().start, nullptr));
    emit_constant(value);
}

void Compiler::grouping(bool can_assign) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::unary(bool can_assign) {
    TokenType operator_type = m_parser->previous().type;

    // Compile the operand.
    m_parser->parse_precedence(PREC_UNARY);

    // Emit the operator instruction.
    switch (operator_type) {
        case TOKEN_BANG: emit_byte(OP_NOT); break;
        case TOKEN_MINUS: emit_byte(OP_NEGATE); break;
        default: return;
    }
}

void Compiler::binary(bool can_assign) {
    TokenType operator_type = m_parser->previous().type;
    ParseRule& rule {m_parser->get_rule(operator_type)};
    m_parser->parse_precedence(static_cast<Precedence>(rule.precedence + 1));

    switch (operator_type) {
        case TOKEN_BANG_EQUAL:    emit_bytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:   emit_byte(OP_EQUAL); break;
        case TOKEN_GREATER:       emit_byte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL: emit_bytes(OP_LESS, OP_NOT); break;
        case TOKEN_LESS:          emit_byte(OP_LESS); break;
        case TOKEN_LESS_EQUAL:    emit_bytes(OP_GREATER, OP_NOT); break;
        case TOKEN_PLUS:          emit_byte(OP_ADD); break;
        case TOKEN_MINUS:         emit_byte(OP_SUBTRACT); break;
        case TOKEN_STAR:          emit_byte(OP_MULTIPLY); break;
        case TOKEN_SLASH:         emit_byte(OP_DIVIDE); break;
        default: return; // Unreachable.    
    }
}

void Compiler::literal(bool can_assign) {
    switch (m_parser->previous().type) {
        case TOKEN_FALSE: emit_byte(OP_FALSE); break;
        case TOKEN_NIL: emit_byte(OP_NIL); break;
        case TOKEN_TRUE: emit_byte(OP_TRUE); break;
        default: return;
    }
}

void Compiler::string(bool can_assign) {
    auto value = OBJ_VAL(ObjString::copy_string(m_parser->previous().start + 1,
                                      m_parser->previous().length - 2));
    std::cout << "Printing value: " << value << std::endl;
    emit_constant(value);
}

void Compiler::variable(bool can_assign) {
    named_variable(m_parser->previous(), can_assign);
}

void Compiler::declaration() {
    if (match(TOKEN_VAR)) {
        var_declaration();
    } else {
        statement();
    }

    if (m_parser->panic_mode()) {
        m_parser->synchronize();
    }
}

void Compiler::var_declaration() {
    uint8_t global_index = parse_variable("Expect variable name.");
    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emit_byte(OP_NIL);
    }
    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    define_variable(global_index);
}

void Compiler::statement() {
    if (match(TOKEN_PRINT)) {
        print_statement();
    } else {
        expression_statement();
    }
}

void Compiler::print_statement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emit_byte(OP_PRINT);
}

void Compiler::expression_statement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emit_byte(OP_POP);
}

bool Compiler::match(TokenType type) {
    if (m_parser->check(type)) {
        advance();
        return true;
    }
    return false;
}

uint8_t Compiler::parse_variable(const char* error_message) {
    consume(TOKEN_IDENTIFIER, error_message);
    return identifier_constant(m_parser->previous());
}

void Compiler::define_variable(uint8_t global_index) {
    emit_bytes(OP_DEFINE_GLOBAL, global_index);
}

uint8_t Compiler::identifier_constant(Token& token) {
    Value value = OBJ_VAL(ObjString::copy_string(token.start, token.length));
    return make_constant(value);
}

uint8_t Compiler::make_constant(Value &value) {
    int constant = current_chunk()->add_constant(value);
    if (constant > UINT8_MAX) {
        m_parser->error("Too many constants in one chunk");
        return 0;
    }

    return (uint8_t) constant;
}

void Compiler::named_variable(Token &name, bool can_assign) {
    uint8_t arg = identifier_constant(name);

    if (can_assign && match(TOKEN_EQUAL)) {
        expression();
        emit_bytes(OP_SET_GLOBAL, arg);
    } else {
        emit_bytes(OP_GET_GLOBAL, arg);
    }
}

