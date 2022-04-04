#include <stdio.h>
#include <string.h>

#include "object.h"
#include "objstring.h"
#include "../value.h"
#include "../vm.h"

extern VM vm;

Obj::Obj() {
    // std::cout << "OBJ CONSTRUCTOR" << std::endl;
    allocate_object();
}

Obj::~Obj() {
    // std::cout << "OBJ DESTRUCTOR" << std::endl;
}

void Obj::allocate_object() {
    // std::cout << "CALLING ALLOCATE OBJECT" << std::endl;
    m_next = vm.m_objects;
    vm.m_objects = this;
}

Obj* Obj::clone() {
    // std::cout << "OBJ CLONE" << std::endl;
    return new Obj {*this};
}

Obj* Obj::copy() {
    // std::cout << "OBJ COPY" << std::endl;
    return new Obj {*this};
}

template <typename stream_type>
void Obj::print_object(const Value &value, stream_type &output) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            output << AS_CSTRING(value);
            break;
    }
}

template void Obj::print_object(const Value&, std::ostream&);
template void Obj::print_object(const Value&, std::stringstream&);