#pragma once

#include "common.h"
#include "debug.h"
#include "value.h"

// struct Chunk;

struct ValueArray: std::vector<Value> {
private:
    std::string m_name {};

public:
    ValueArray();
    ValueArray(std::string name);

    void set_name(const std::string &name);
    std::string name() const;

    int add_constant(Chunk &chunk, Value value);
protected:


};