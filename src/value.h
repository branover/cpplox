#pragma once

#include <sstream>
#include <iomanip>

#include "common.h"

typedef double Value;

void print_value(const Value &value, std::stringstream &output);

template<typename stream_type>
void print_value(const Value &value, stream_type &output) {
    output << value;
}