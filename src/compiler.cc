#include <stdio.h>
#include <cstring>

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
    m_locals.reserve(256); 
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

int Compiler::emit_jump(uint8_t instruction) {
    emit_byte(instruction);
    emit_byte(0xff);
    emit_byte(0xff);
    return current_chunk()->size() - 2;
}

void Compiler::patch_jump(int offset) {
    int jump = current_chunk()->size() - offset - 2;

    if (jump > UINT16_MAX) {
        m_parser->error("Too much code to jump over.");
        return;
    }

    (*current_chunk())[offset] = static_cast<uint8_t>((jump >> 8) & 0xff);
    (*current_chunk())[offset + 1] = static_cast<uint8_t>(jump & 0xff);
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
    } else if (match(TOKEN_IF)){
        if_statement();
    } else if (match(TOKEN_LEFT_BRACE)) {
        begin_scope();
        block();
        end_scope();
    } else {
        expression_statement();
    }
}

void Compiler::print_statement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emit_byte(OP_PRINT);
}

void Compiler::if_statement() {
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int then_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);
    statement();

    int else_jump = emit_jump(OP_JUMP);

    patch_jump(then_jump);
    emit_byte(OP_POP);

    if (match(TOKEN_ELSE)) {
        statement();
    }
    patch_jump(else_jump);
}

void Compiler::block() {
    while (!m_parser->check(TOKEN_RIGHT_BRACE) && !m_parser->check(TOKEN_EOF)) {
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

void Compiler::expression_statement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emit_byte(OP_POP);
}

void Compiler::begin_scope() {
    m_scope_depth++;
}

void Compiler::end_scope() {
    m_scope_depth--;

    while (m_locals.size() > 0 &&
        m_locals[m_locals.size() -1].depth > m_scope_depth) {
        emit_byte(OP_POP);
        m_locals.pop_back();
    }
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

    declare_variable();
    if (m_scope_depth > 0) return 0;

    return identifier_constant(m_parser->previous());
}

void Compiler::declare_variable() {
    if (m_scope_depth == 0) return;

    Token &name = m_parser->previous();

    for (auto it = m_locals.rbegin(); it != m_locals.rend(); ++it) {
        Local &local = *it;
        if (local.depth != -1 && local.depth < m_scope_depth) break;

        if (identifiers_equal(name, local.name)) {
            m_parser->error("Variable with this name already declared in this scope.");
        }
    }

    add_local(name);
}

void Compiler::define_variable(uint8_t global_index) {
    if (m_scope_depth > 0) {
        // Mark initialized
        m_locals.back().depth = m_scope_depth;
        return;
    }

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
    uint8_t get_op {}, set_op {};
    int arg = resolve_local(name);
    if (arg != -1) {
        get_op = OP_GET_LOCAL;
        set_op = OP_SET_LOCAL;
    } else {
        arg = identifier_constant(name);
        get_op = OP_GET_GLOBAL;
        set_op = OP_SET_GLOBAL;
    }

    if (can_assign && match(TOKEN_EQUAL)) {
        expression();
        emit_bytes(set_op, arg);
    } else {
        emit_bytes(get_op, arg);
    }
}

void Compiler::add_local(Token &name) {
    if (m_locals.size() > UINT8_MAX) {
        m_parser->error("Too many local variables in one function.");
        return;
    }

    m_locals.emplace_back(Local {.name = name, .depth = -1});
}

int Compiler::resolve_local(Token &name) {
    for (int i = m_locals.size() - 1; i >= 0; i--) {
        Local &local = m_locals[i];
        if (identifiers_equal(name, local.name)) {
            if (local.depth == -1) {
                m_parser->error("Cannot read local variable in its own initializer.");
                return -1;
            }
            return i;
        }
    }

    return -1;
}

bool Compiler::identifiers_equal(Token &a, Token &b) {
    if (a.length != b.length) return false;
    return memcmp(a.start, b.start, a.length) == 0;
}