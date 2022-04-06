#pragma once

struct Chunk;

std::stringstream disassemble_chunk(const Chunk &chunk, const char* name);

template <typename stream_type>
int disassemble_instruction(const Chunk &chunk, int offset, stream_type &output);

template <typename stream_type>
int simple_instruction(std::string name, int offset, stream_type &output);

template <typename stream_type>
int constant_instruction(std::string name, const Chunk &chunk, int offset, stream_type &output);

template <typename stream_type>
int byte_instruction(std::string name, const Chunk &chunk, int offset, stream_type &output);
