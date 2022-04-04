#include "value.h"
#include "objects/object.h"
#include "objects/objstring.h"


Value::Value() {
    // std::cout << "VALUE CONSTRUCTED1" << std::endl;
    this->type = VAL_NIL;
    this->as.any = 0;
}

Value::Value(ValueType type, bool value) {
    // std::cout << "VALUE CONSTRUCTED2" << std::endl;
    this->type = type;
    this->as.boolean = value;
}

Value::Value(ValueType type) {
    // std::cout << "VALUE CONSTRUCTED3" << std::endl;
    this->type = type;
    this->as.number = 0;
}

Value::Value(ValueType type, double value) {
    // std::cout << "VALUE CONSTRUCTED4" << std::endl;
    this->type = type;
    this->as.number = value;
}

Value::Value(ValueType type, Obj* value) {
    // std::cout << "VALUE CONSTRUCTED5" << std::endl;
    this->type = type;
    this->as.obj = value;
}

Value::Value(const Value &old) {
    // std::cout << "VALUE COPIED" << std::endl;
    this->type = old.type;
    switch (old.type) {
        case VAL_OBJ: this->as.obj = old.as.obj->copy(); break;
        default: this->as.any = old.as.any;
    }
}

Value::Value(Value&& other):
    type {std::move(other.type)},
    as {std::move(other.as)}
{
    // std::cout << "***VALUE MOVED***" << std::endl;
    other.as.any = nullptr;
    other.type = VAL_NIL;
}

Value::~Value() {
    // std::cout << "VALUE DESTRUCTOR" << std::endl;   
    if (type == VAL_OBJ) {
        // delete as.obj;
    } 
}

std::ostream &operator<<(std::ostream &os, Value const &value) {
    print_value(value, os);
    return os;
}

bool Value::is_falsey() const {
    return IS_NIL(*this) || (IS_BOOL(*this) && !AS_BOOL(*this));
}

Value& Value::operator=(const Value& other) {
    this->type = other.type;
    switch (other.type) {
        case VAL_OBJ: this->as.obj = other.as.obj->copy(); break;
        default: this->as.any = other.as.any;
    }
    return *this;
}

Value& Value::operator=(Value&& other) {
    // std::cout << "***VALUE ASSIGNMENT MOVED***" << std::endl;
    return *this;
}

bool Value::operator==(const Value& other) const {
    if (this->type != other.type) return false;
    switch (this->type) {
        case VAL_BOOL:   return AS_BOOL(*this) == AS_BOOL(other);
        case VAL_NIL:    return true;
        case VAL_NUMBER: return AS_NUMBER(*this) == AS_NUMBER(other);
        case VAL_OBJ: {
            ObjString * a_string = AS_STRING(*this);
            ObjString * b_string = AS_STRING(other);
            return a_string->m_str == b_string->m_str;

        }
        default:         return false; // Unreachable.
    }

}

template<typename stream_type>
void print_value(const Value &value, stream_type &output) {
    switch (value.type) {
        case VAL_BOOL: output << (AS_BOOL(value) ? "true" : "false"); break;
        case VAL_NIL: output << "nil"; break;
        case VAL_NUMBER: output << AS_NUMBER(value); break;
        case VAL_OBJ: Obj::print_object(value, output); break;
    }
}

template void print_value(const Value&, std::ostream&);
template void print_value(const Value&, std::stringstream&);

