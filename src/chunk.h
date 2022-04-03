#pragma once

#include "common.h"
#include "debug.h"
#include "value.h"
#include "value_array.h"

enum OpCode {
    OP_CONSTANT,

    // Optimized Values
    OP_NIL,
    OP_TRUE,
    OP_FALSE,    

    // Equality and Comparison
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,

    // Binary
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,

    // Unary
    OP_NOT,
    OP_NEGATE,

    OP_RETURN,
};

struct Chunk: std::vector<uint8_t> {
private:
    std::string m_name {"unnamed chunk"};
    ValueArray m_constants {};
    std::vector<int> m_lines {};

public:
    Chunk(std::string name);
    Chunk();
    friend std::ostream & operator << (std::ostream &os, const Chunk &chunk);

    void set_name(const std::string &name);

    std::string name() const;
    const ValueArray& constants() const;
    const std::vector<int>& lines() const;
    int get_line(uint8_t offset) const;

    inline void write_chunk(uint8_t byte, int line) {
        this->push_back(byte);
        if (line >= m_lines.size()) {
            m_lines.resize(line+1);
        }
        m_lines.at(line) = this->size();
    }

    inline int add_constant(Value value) {
        m_constants.push_back(value);

        // Return the index of the new value
        return m_constants.size() - 1; 
    }
};
