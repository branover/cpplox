#include <iostream>

#include "vm.h"
#include "common.h"
#include "compiler.h"

#define CHUNK_START (m_chunk->data())

VM::VM() {
    m_stack_top = m_stack.data();
    m_chunk = std::make_shared<Chunk>(Chunk {});
}

InterpretResult VM::interpret(const std::string &source) {
    Compiler compiler {};
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
#define BINARY_OP(op) \
    do { \
        Value b = pop(); \
        Value a = pop(); \
        push(a op b); \
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
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_ADD:        BINARY_OP(+); break;
            case OP_SUBTRACT:   BINARY_OP(-); break;
            case OP_MULTIPLY:   BINARY_OP(*); break;
            case OP_DIVIDE:     BINARY_OP(/); break;
            case OP_NEGATE:     push(-pop()); break;
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

void VM::push(Value value) {
    *m_stack_top = value;
    m_stack_top++;
}

Value VM::pop() {
    m_stack_top--;
    return *m_stack_top;
}