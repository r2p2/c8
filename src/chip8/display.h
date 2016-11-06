#pragma once

#include "snapshot.h"

#include <vector>
#include <stdint.h>

namespace chip8
{

class Display
{
public:
	Display()
	: _updated(false)
	, _display(width() * height(), 0)
	{}

	void save(Snapshot& ss) const
	{
		ss.display = _display;
	}

	void clear()
	{
		_updated = true;
		for (std::size_t i = 0; i < _display.size(); ++i)
			_display[i] = 0;
	}

	void set(uint8_t x, uint8_t y, bool value)
	{
		_display[_xy2i(x, y)] = value;
	}

	bool get(uint8_t x, uint8_t y) const
	{
		return _display[_xy2i(x, y)];
	}

	void reset()
	{
		_updated = false;
		clear();
	}

	bool redraw_required() const
	{
		return _updated;
	}

	void redrawn()
	{
		_updated = false;
	}

	static std::size_t width()
	{
		return 64;
	}

	static std::size_t height()
	{
		return 32;
	}

private:
	static std::size_t _xy2i(uint8_t x, uint8_t y)
	{
		return y * width() + x;
	}

private:
	bool              _updated;
	std::vector<bool> _display;
};

} // namespace chip8
