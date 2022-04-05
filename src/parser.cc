#include <memory>
#include <functional>

#include "parser.h"
#include "token.h"
#include "scanner.h"
#include "compiler.h"

// #define BIND_FUNC(name) std::bind(&name, &m_compiler)
#define BIND_FUNC(name) ParseFn name = std::bind(&Compiler::name, &m_compiler, std::placeholders::_1)

Parser::Parser(std::shared_ptr<Scanner> scanner, Compiler& compiler): 
m_scanner {scanner},
m_compiler {compiler}
{
    BIND_FUNC(grouping);
    BIND_FUNC(unary);
    BIND_FUNC(binary);
    BIND_FUNC(number);
    BIND_FUNC(literal);
    BIND_FUNC(string);
    BIND_FUNC(variable);
    ParseFn NULL_FN {};

    m_rules =  {
        //[TOKEN_LEFT_PAREN] 
        {grouping,    NULL_FN,   PREC_NONE},
        //[TOKEN_RIGHT_PAREN]
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_LEFT_BRACE] 
        {NULL_FN,     NULL_FN,   PREC_NONE}, 
        //[TOKEN_RIGHT_BRACE]
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_COMMA]      
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_DOT]        
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_MINUS]      
        {unary,       binary,    PREC_TERM},
        //[TOKEN_PLUS]       
        {NULL_FN,     binary,    PREC_TERM},
        //[TOKEN_SEMICOLON]  
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_SLASH]      
        {NULL_FN,     binary,    PREC_FACTOR},
        //[TOKEN_STAR]       
        {NULL_FN,     binary,    PREC_FACTOR},
        //[TOKEN_BANG]       
        {unary,       NULL_FN,   PREC_NONE},
        //[TOKEN_BANG_EQUAL] 
        {NULL_FN,     binary,    PREC_EQUALITY},
        //[TOKEN_EQUAL]      
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_EQUAL_EQUAL]
        {NULL_FN,     binary,    PREC_EQUALITY},
        //[TOKEN_GREATER]    
        {NULL_FN,     binary,    PREC_COMPARISON},
        //[TOKEN_GREATER_EQUA
        {NULL_FN,     binary,    PREC_COMPARISON},
        //[TOKEN_LESS]       
        {NULL_FN,     binary,    PREC_COMPARISON},
        //[TOKEN_LESS_EQUAL] 
        {NULL_FN,     binary,    PREC_COMPARISON},
        //[TOKEN_IDENTIFIER] 
        {variable,    NULL_FN,   PREC_NONE},
        //[TOKEN_STRING]     
        {string,      NULL_FN,   PREC_NONE},
        //[TOKEN_NUMBER]     
        {number,      NULL_FN,   PREC_NONE},
        //[TOKEN_AND]        
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_CLASS]      
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_ELSE]       
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_FALSE]      
        {literal,     NULL_FN,   PREC_NONE},
        //[TOKEN_FOR]        
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_FUN]        
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_IF]         
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_NIL]        
        {literal,     NULL_FN,   PREC_NONE},
        //[TOKEN_OR]         
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_PRINT]      
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_RETURN]     
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_SUPER]      
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_THIS]       
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_TRUE]       
        {literal,     NULL_FN,   PREC_NONE},
        //[TOKEN_VAR]        
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_WHILE]      
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_ERROR]      
        {NULL_FN,     NULL_FN,   PREC_NONE},
        //[TOKEN_EOF]        
        {NULL_FN,     NULL_FN,   PREC_NONE},
    }; 
}

void Parser::advance() {
    m_previous = m_current;

    for (;;) {
        m_current = m_scanner->scan_token();
        if (m_current.type != TOKEN_ERROR) break;

        error_at_current();
    }
}

Token& Parser::current() {
    return m_current;
}

Token& Parser::previous() {
    return m_previous;
}

bool Parser::had_error() {
    return m_had_error;
}

bool Parser::panic_mode() {
    return m_panic_mode;
}

void Parser::synchronize() {
    m_panic_mode = false;

    while (m_current.type != TOKEN_EOF) {
        if (m_previous.type == TOKEN_SEMICOLON) return;

        switch (m_current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;
            default:
                break;
        }

        advance();
    }
}

void Parser::error_at_current() {
    error_at(m_current, m_current.start);
}

void Parser::error(const char* message) {
    error_at(m_previous, message);
}

void Parser::error_at(Token &token, const char * message) {
    m_panic_mode = true;
    fprintf(stderr, "[line %d] Error", token.line);

    if (token.type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token.type == TOKEN_ERROR) {
        // Nothing.
    } else {
        fprintf(stderr, " at '%.*s'", token.length, token.start);
    }

    fprintf(stderr, ": %s\n", message);
    m_had_error = true;
}

void Parser::consume(TokenType type, const char* message) {
    if (m_current.type == type) {
        advance();
        return;
    }

    error_at_current();
}


void Parser::parse_precedence(Precedence precedence) {
    advance();
    ParseFn prefix_rule = get_rule(m_previous.type).prefix;
    if (!prefix_rule) {
        error("Expect expression");
        return;
    }

    bool can_assign = precedence <= PREC_ASSIGNMENT;
    prefix_rule(can_assign);

    while (precedence <= get_rule(m_current.type).precedence) {
        advance();
        ParseFn infix_rule = get_rule(m_previous.type).infix;
        infix_rule(can_assign);
    }

    if (can_assign && m_compiler.match(TOKEN_EQUAL)) {
        error("Invalid assignment target");
    }
}
