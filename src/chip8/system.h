#pragma once

#include "memory.h"
#include "display.h"
#include "input.h"
#include "cpu.h"

#include<chrono>

namespace chip8
{

class System
{
public:
	System()
	: _mem()
	, _dis()
	, _inp()
	, _cpu(_mem, _dis, _inp)
	{}

	void reset()
	{
		_mem.reset();
		_mem.load(0x000, {
		     0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		     0x20, 0x60, 0x20, 0x20, 0x70, // 1
		     0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		     0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		     0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		     0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		     0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		     0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		     0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		     0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		     0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		     0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		     0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		     0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		     0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		     0xF0, 0x80, 0xF0, 0x80, 0x80, // F
		});
		_dis.reset();
		_inp.reset();
		_cpu.reset();

		_clock = next_clock();
	}

	void load_rom(std::vector<uint8_t> const& data)
	{
		_mem.load(0x200, data);
	}

	void key_pressed(uint8_t key)
	{
		_inp.press(key);
	}

	void key_released()
	{
		_inp.release();
	}

	bool redraw_required() const
	{
		return _dis.redraw_required();
	}

	bool pixel(uint8_t x, uint8_t y) const
	{
		return _dis.get(x, y);
	}

	void tick()
	{
		if (_clock < std::chrono::steady_clock::now())
		{
			_cpu.clock();
			_clock = next_clock();
		}

		_cpu.tick();
	}

private:
	static std::chrono::time_point<std::chrono::steady_clock> next_clock()
	{
		return std::chrono::steady_clock::now()
			 + std::chrono::milliseconds(16);
	}

private:
	Memory  _mem;
	Display _dis;
	Input   _inp;
	Cpu     _cpu;

	std::chrono::time_point<std::chrono::steady_clock> _clock;
};

} // namespace chip8
