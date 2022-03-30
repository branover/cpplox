#pragma once

#include <memory>

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

enum InterpretResult {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR    
};

struct VM {
    VM();
    InterpretResult interpret(const std::string &source);
    InterpretResult run();
    void push(Value value);
    Value pop();

private:
    std::unique_ptr<Chunk> m_chunk;
    uint8_t* m_ip {nullptr};
    std::vector<Value> m_stack {STACK_MAX};
    Value* m_stack_top {nullptr};
};