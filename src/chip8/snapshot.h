#pragma once

#include <vector>
#include <stdint.h>

namespace chip8
{

struct Snapshot
{
	uint16_t              pc;
	uint16_t              i;
	uint8_t               d;
	uint8_t               s;
	bool                  has_input;
	uint8_t               input;
	std::vector<uint8_t>  v;
	std::vector<uint16_t> stack;
	std::vector<uint8_t>  memory;
	std::vector<bool>     display;
};

} // namespace chip8
