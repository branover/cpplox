#pragma once

#include "common.h"
#include "../value.h"

enum ObjType {
    OBJ_STRING,
};

#define OBJ_TYPE(value)        (AS_OBJ(value)->m_type)

#define IS_STRING(value)        Obj::is_obj_type(value, OBJ_STRING)

struct ObjString;

struct Obj {
    ObjType m_type;
    Obj* m_next {nullptr};

    Obj();
    virtual ~Obj();

    void allocate_object();

    virtual Obj* clone();
    virtual Obj* copy();

    static inline bool is_obj_type(Value value, ObjType type) {
        return IS_OBJ(value) && AS_OBJ(value)->m_type == type;
    }

    template <typename stream_type>
    static void print_object(const Value &value, stream_type &output);
};