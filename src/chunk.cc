#include <sstream>

#include "chunk.h"

std::ostream &operator<<(std::ostream &os, Chunk const &chunk) {
    return os << disassemble_chunk(chunk, chunk.name().c_str()).str();
}

Chunk::Chunk(std::string name): std::vector<uint8_t>() {
    m_name = name;
}

Chunk::Chunk(): std::vector<uint8_t>() {}

void Chunk::set_name(const std::string &name) {
    m_name = name;
}

std::string Chunk::name() const {
    return m_name;
}

const ValueArray& Chunk::constants() const {
    return m_constants;
}

const std::vector<int>& Chunk::lines() const {
    return m_lines;
}

int Chunk::get_line(uint8_t offset) const {
    for (int i = 0; i < m_lines.size(); i++) {
        if (m_lines.at(i) > offset) return i;
    }
    return m_lines.size();
}