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
        case OP_NIL:
            return simple_instruction("OP_NIL", offset, output);
        case OP_TRUE:
            return simple_instruction("OP_TRUE", offset, output);
        case OP_FALSE:
            return simple_instruction("OP_FALSE", offset, output);
        case OP_POP:
            return simple_instruction("OP_POP", offset, output);
        case OP_GET_LOCAL:
            return byte_instruction("OP_GET_LOCAL", chunk, offset, output);
        case OP_SET_LOCAL:
            return byte_instruction("OP_SET_LOCAL", chunk, offset, output);
        case OP_GET_GLOBAL:
            return constant_instruction("OP_GET_GLOBAL", chunk, offset, output);
        case OP_DEFINE_GLOBAL:
            return constant_instruction("OP_DEFINE_GLOBAL", chunk, offset, output);
        case OP_SET_GLOBAL:
            return constant_instruction("OP_SET_GLOBAL", chunk, offset, output);
        case OP_EQUAL:
            return simple_instruction("OP_EQUAL", offset, output);
        case OP_GREATER:
            return simple_instruction("OP_GREATER", offset, output);
        case OP_LESS:
            return simple_instruction("OP_LESS", offset, output);
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
        case OP_PRINT:
            return simple_instruction("OP_PRINT", offset, output);
        case OP_JUMP:
            return jump_instruction("OP_JUMP", 1, chunk, offset, output);
        case OP_JUMP_IF_FALSE:
            return jump_instruction("OP_JUMP_IF_FALSE", 1, chunk, offset, output);
        case OP_RETURN:
            return simple_instruction("OP_RETURN", offset, output);
        case OP_NOT:
            return simple_instruction("OP_NOT", offset, output);
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

template <typename stream_type>
int byte_instruction(std::string name, const Chunk &chunk, int offset, stream_type &output) {
    uint8_t slot = chunk[offset + 1];
    output << std::left << std::setw(16) << std::setfill(' ') << name << " " << std::right;
    output << std::setw(4) << std::setfill('0') << (unsigned int)slot << " " << std::endl;
    return offset + 2;
}

template <typename stream_type>
int jump_instruction(std::string name, int sign, const Chunk &chunk, int offset, stream_type &output) {
    uint16_t jump = chunk[offset + 1] << 8;
    jump |= chunk[offset + 2];
    output << std::left << std::setw(16) << std::setfill(' ') << name << " " << std::right;
    output << std::setw(4) << std::setfill('0') << (unsigned int)offset << " -> ";
    output << offset + 3 + (sign * jump);
    return offset + 3;
}

template int disassemble_instruction(const Chunk&, int, std::stringstream&);
template int simple_instruction(std::string, int, std::stringstream&);
template int constant_instruction(std::string, const Chunk &, int, std::stringstream&);
template int byte_instruction(std::string, const Chunk &, int, std::stringstream&);
template int jump_instruction(std::string, int, const Chunk &, int, std::stringstream&);

template int disassemble_instruction(const Chunk&, int, std::ostream&);
template int simple_instruction(std::string, int, std::ostream&);
template int constant_instruction(std::string, const Chunk &, int, std::ostream&);
template int byte_instruction(std::string, const Chunk &, int, std::ostream&);
template int jump_instruction(std::string, int, const Chunk &, int, std::ostream&);