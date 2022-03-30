#include <stdio.h>

#include <sstream>
#include <iomanip>

#include "util.h"
#include "debug.h"
#include "chunk.h"
#include "value.h"

std::stringstream disassemble_chunk(const Chunk &chunk, const char* name) {
    std::stringstream output {};
    output << string_format("== %s ==", name) << std::endl;

    for (int offset = 0; offset < chunk.size();) {
        offset = disassemble_instruction(chunk, offset, output);
        output << std::endl;
    }
    output << std::endl;
    return output;
}

template <typename stream_type>
int disassemble_instruction(const Chunk &chunk, int offset, stream_type &output) {
    output << std::setfill('0') << std::setw(4) << offset << " ";
    if (offset > 0 && chunk.get_line(offset) == chunk.get_line(offset-1))
        output << "   | ";
    else
        output << std::setfill('0') << std::setw(4) << chunk.get_line(offset) << " ";

    uint8_t instruction = chunk[offset];
    switch (instruction) {
        case OP_CONSTANT:
            return constant_instruction("OP_CONSTANT", chunk, offset, output);
        case OP_ADD:
            return simple_instruction("OP_ADD", offset, output);
        case OP_SUBTRACT:
            return simple_instruction("OP_SUBTRACT", offset, output);
        case OP_MULTIPLY:
            return simple_instruction("OP_MULTIPLY", offset, output);
        case OP_DIVIDE:
            return simple_instruction("OP_DIVIDE", offset, output);
        case OP_NEGATE:
            return simple_instruction("OP_NEGATE", offset, output);
        case OP_RETURN:
            return simple_instruction("OP_RETURN", offset, output);
        default:
            output << "Unknown opcode " << instruction;
            return offset += 1;
    }
}

template <typename stream_type>
int simple_instruction(std::string name, int offset, stream_type &output) {
    output << name;
    return offset + 1;
}

template <typename stream_type>
int constant_instruction(std::string name, const Chunk &chunk, int offset, stream_type &output) {
    uint8_t constant = chunk[offset + 1];
    output << std::left << std::setw(16) << std::setfill(' ') << name << " " << std::right;
    output << std::setw(4) << std::setfill('0') << (unsigned int)constant << " ";
    print_value(chunk.constants()[constant], output);
    return offset + 2;
}

template int disassemble_instruction(const Chunk&, int, std::stringstream&);
template int simple_instruction(std::string, int, std::stringstream&);
template int constant_instruction(std::string, const Chunk &, int, std::stringstream&);

template int disassemble_instruction(const Chunk&, int, std::ostream&);
template int simple_instruction(std::string, int, std::ostream&);
template int constant_instruction(std::string, const Chunk &, int, std::ostream&);