#pragma once
#include <fstream>
#include <vector>
#include <iostream>
#include <string>

std::vector<char> readSPIRV(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open SPIR-V file: " + filepath);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        throw std::runtime_error("Failed to read SPIR-V file: " + filepath);
    }

    return buffer;
}
