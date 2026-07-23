#pragma once
#include "cpu.hpp"
#include "memory.hpp"

void enableRawMode();
void disableRawMode();
bool load_hex_file(const string& filename, Memory& memory);