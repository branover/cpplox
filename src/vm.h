#pragma once

#include <memory>
#include <unordered_map>

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
    ~VM();
    void reset_stack();
    InterpretResult interpret(const std::string &source);
    InterpretResult run();
    void push(Value &value);
    void push(const Value &value);

    Value& pop();
    Value& peek(int distance);

    void runtime_error(const char* format, ...);

    void concatenate();

    void free_objects();

    Obj* m_objects {nullptr};
    std::unordered_map<std::string, Value> m_strings {};
    std::unordered_map<std::string, Value> m_globals {};

private:
    std::shared_ptr<Chunk> m_chunk;
    uint8_t* m_ip {nullptr};
    std::vector<Value> m_stack {STACK_MAX};
    Value* m_stack_top {nullptr};
};