#pragma once
#include <arpa/inet.h>
#include <vector>
#include <cstring>
#include <cstdint>

void escreve_u16_be(std::vector<uint8_t>& buf, size_t offset, uint16_t valor);
void escreve_u32_be(std::vector<uint8_t>& buf, size_t offset, uint32_t valor);

uint16_t le_u16_be(const std::vector<uint8_t>& buf, size_t offset);
uint32_t le_u32_be(const std::vector<uint8_t>& buf, size_t offset);
