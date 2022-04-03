#include "value.h"

// void print_value(const Value &value, std::stringstream &output) {
//     output << value;
// }

std::ostream &operator<<(std::ostream &os, Value const &value) {
    print_value(value, os);
    return os;
}

bool Value::is_falsey() const {
    return IS_NIL(*this) || (IS_BOOL(*this) && !AS_BOOL(*this));
}

bool Value::operator==(const Value& other) const {
    if (this->type != other.type) return false;
    switch (this->type) {
        case VAL_BOOL:   return AS_BOOL(*this) == AS_BOOL(other);
        case VAL_NIL:    return true;
        case VAL_NUMBER: return AS_NUMBER(*this) == AS_NUMBER(other);
        default:         return false; // Unreachable.
    }

}

template<typename stream_type>
void print_value(const Value &value, stream_type &output) {
    switch (value.type) {
        case VAL_BOOL: output << (AS_BOOL(value) ? "true" : "false"); break;
        case VAL_NIL: output << "nil"; break;
        case VAL_NUMBER: output << AS_NUMBER(value); break;
    }
}

template void print_value(const Value&, std::ostream&);
template void print_value(const Value&, std::stringstream&);

