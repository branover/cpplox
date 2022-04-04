#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <fstream>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

VM vm;

static void run_file(const char* path, VM &vm) {
    std::ifstream in {path};
    if (!in.is_open()) {
        std::cerr << "Could not open file " << path << "." << std::endl;
        exit(74);
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    in.close();

    std::string source {buffer.str()};
    InterpretResult result = vm.interpret(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

static void repl(VM &vm) {
    std::cout << "> ";
    for (std::string line; std::getline(std::cin, line);) {
        vm.interpret(line);
        vm.free_objects();        
        std::cout << "> ";
    }
    std::cout << std::endl;
}

int main(int argc, const char* argv[]) {
    if (argc == 1) {
        repl(vm);
    } else if (argc == 2) {
        run_file(argv[1], vm);
    } else {
        std::cerr << "Usage: clox [path]\n";
        exit(64);
    }

    return 0;
}

