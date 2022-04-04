#include <iostream>

#include <stdarg.h>

#include "vm.h"
#include "common.h"
#include "compiler.h"
#include "objects/object.h"
#include "objects/objstring.h"

#define CHUNK_START (m_chunk->data())

VM::VM() {
    // std::cout << "VM CONSTRUCTED" << std::endl;
    reset_stack();
    m_chunk = std::make_shared<Chunk>(Chunk {});
}

VM::~VM() {
    free_objects();
}

void VM::free_objects() {
    // std::cout << "Freeing objects" << std::endl;
    Obj* object = m_objects;
    while (object != nullptr) {
        // std::cout << "Freeing object at: " << (void*) object << std::endl;
        Obj *next = object->m_next;
        delete object;
        object = next;
    }
    m_objects = nullptr;
}

void VM::reset_stack() {
    m_stack_top = m_stack.data();
}

InterpretResult VM::interpret(const std::string &source) {
    Compiler compiler {};

    //TODO: Remove this to keep state between REPL calls
    m_chunk = std::make_shared<Chunk>(Chunk {});

    if (!compiler.compile(source, m_chunk)) {
        return INTERPRET_COMPILE_ERROR;
    }

    m_ip = CHUNK_START; 

    InterpretResult result = run();

    return result;
}

InterpretResult VM::run() {
#define READ_BYTE() (*m_ip++)
#define READ_CONSTANT() (m_chunk->constants()[READ_BYTE()])
#define BINARY_OP(value_type, op) \
    do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtime_error("Operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(pop()); \
        double a = AS_NUMBER(pop()); \
        push(value_type(a op b)); \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        for (Value* slot = m_stack.data(); slot < m_stack_top; slot++) {
            std::cout << "[ " << *slot << " ]";
        }
        std::cout << std::endl;
        disassemble_instruction(*m_chunk, m_ip - CHUNK_START, std::cout);
        std::cout << std::endl;
#endif
        uint8_t instruction {};
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                const Value& constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_NIL:        push(NIL_VAL); break;
            case OP_TRUE:       push(BOOL_VAL(true)); break;
            case OP_FALSE:      push(BOOL_VAL(false)); break;
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(a == b));
                break;
            }
            case OP_GREATER:    BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS:       BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD: {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                } else {
                    runtime_error("Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT:   BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY:   BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:     BINARY_OP(NUMBER_VAL, /); break;
            case OP_NOT:
                push(BOOL_VAL(pop().is_falsey()));
                break;
            case OP_NEGATE: 
                if (!IS_NUMBER(peek(0))) {
                    runtime_error("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }    
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            case OP_RETURN: {
                print_value(pop(), std::cout);
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }
#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

void VM::push(Value &value) {
    *m_stack_top = std::move(value);
    m_stack_top++;
}

void VM::push(const Value &value) {
    *m_stack_top = value;
    m_stack_top++;
}

Value& VM::pop() {
    m_stack_top--;
    return *m_stack_top;
}

Value& VM::peek(int distance) {
    return m_stack_top[-1 - distance];
}

void VM::runtime_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = m_ip - m_chunk->data() - 1;
    int line = m_chunk->lines()[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    reset_stack();
}

void VM::concatenate() {
    ObjString* b = AS_STRING(pop());
    ObjString* a = AS_STRING(pop());

    *a->m_str += *b->m_str;
    push(std::move(OBJ_VAL(a)));
}