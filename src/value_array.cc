#include <sstream>

#include "value_array.h"

ValueArray::ValueArray(std::string name): std::vector<Value>() {
    m_name = name;
}

ValueArray::ValueArray(): std::vector<Value>() {}

void ValueArray::set_name(const std::string &name) {
    m_name = name;
}

std::string ValueArray::name() const {
    return m_name;
}