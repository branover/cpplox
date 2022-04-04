#pragma once

#include "common.h"
#include "../value.h"
#include "object.h"

#define AS_STRING(value)       ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)      (((ObjString*)AS_OBJ(value))->m_str->c_str())

struct ObjString: Obj {
    ObjString() = default;
    ObjString(ObjType type, const char* chars, size_t length);
    ObjString(const ObjString& other);
    ~ObjString();
    ObjString* clone() override;
    ObjString* copy() override;

    static ObjString* copy_string(const char* chars, size_t length);
    static ObjString* allocate_string(const char* chars, size_t length);    

    std::shared_ptr<std::string> m_str {};
};



