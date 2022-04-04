// #include "object.h"
#include "objstring.h"

ObjString::~ObjString() {
    // std::cout << "OBJSTRING DESTRUCTOR" << std::endl;
}

ObjString* ObjString::copy_string(const char* chars, size_t length) {
  return allocate_string(chars, length);
}

ObjString* ObjString::allocate_string(const char* chars, size_t length) {
    return new ObjString {OBJ_STRING, chars, length};
}

ObjString::ObjString(ObjType type, const char* chars, size_t length): Obj() {
    // std::cout << "OBJSTR CONSTRUCTOR" << std::endl;
    type = type;
    m_str = std::make_shared<std::string>(chars, length);
}

ObjString::ObjString(const ObjString &other) : Obj() {
    // std::cout << "OBJSTR COPY CONSTRUCTOR" << std::endl;
    m_type = other.m_type;
    m_str = other.m_str;
    // std::cout << "This str: " << (void*)str.c_str() << std::endl;
    // std::cout << str << std::endl;
    // std::cout << "Other str: " << (void*)other.str.c_str() << std::endl;
    // std::cout << other.str << std::endl;
}

ObjString* ObjString::clone() {
    // std::cout << "OBJSTRING CLONED" << std::endl;
    return new ObjString (*this);
}

ObjString* ObjString::copy() {
    // std::cout << "OBJSTRING COPIED" << std::endl;
    auto result = new ObjString {};
    result->m_type = m_type;
    result->m_str = m_str;
    return result;
}