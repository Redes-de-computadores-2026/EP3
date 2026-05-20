#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>

uint32_t crc32(const uint8_t* dados, size_t len);
uint32_t crc32(const std::vector<uint8_t>& dados);
