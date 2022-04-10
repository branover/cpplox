#pragma once

#include <sstream>
#include <iomanip>
#include <memory>

#include "common.h"

struct Obj;

enum ValueType {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ,
};

union CastValue {
    constexpr CastValue() {any = nullptr;}
    ~CastValue() {any = nullptr;} 
    bool boolean;
    double number;
    Obj *obj;
    void* any;
};

struct Value {
    Value();
    Value(ValueType type, bool value);
    Value(ValueType type);
    Value(ValueType type, double value);
    Value(ValueType type, Obj* value);
    Value(const Value &old);
    Value(Value&& other);
    ~Value();

    friend std::ostream& operator << (std::ostream &os, const Value &value);
    bool operator==(const Value& other) const;
    Value& operator=(const Value& other);
    Value& operator=(Value&& other);

    bool is_falsey() const;

    ValueType type {};
    CastValue as {};
  
};

#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NIL(value)     ((value).type == VAL_NIL)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)
#define IS_OBJ(value)     ((value).type == VAL_OBJ)

#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)
#define AS_OBJ(value)     ((value).as.obj)

#define BOOL_VAL(value)   ((Value){VAL_BOOL, value})
#define NIL_VAL           ((Value){VAL_NIL})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, value})
#define OBJ_VAL(object)   ((Value){VAL_OBJ, (Obj*)object})

template<typename stream_type>
void print_value(const Value &value, stream_type &output);