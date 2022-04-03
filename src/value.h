#pragma once

#include <sstream>
#include <iomanip>

#include "common.h"

enum ValueType {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
};

// typedef double Value;
struct Value {
    friend std::ostream& operator << (std::ostream &os, const Value &value);
    bool is_falsey() const;
    bool operator==(const Value& other) const;

    ValueType type;
    union {
        bool boolean;
        double number;

    } as;
};

#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NIL(value)     ((value).type == VAL_NIL)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)

#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)

#define BOOL_VAL(value)   ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL           ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})

// void print_value(const Value &value, std::stringstream &output);

template<typename stream_type>
void print_value(const Value &value, stream_type &output);