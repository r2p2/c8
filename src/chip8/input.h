#pragma once

#include "snapshot.h"

#include <stdint.h>

namespace chip8
{

class Input
{
public:
	Input()
	: _value(0)
	{}

	void save(Snapshot& ss) const
	{
		ss.input = _value;
	}

	void press(uint8_t value)
	{
		_value = value+1;
	}

	void release()
	{
		_value = 0;
	}

	bool has_input() const
	{
		return _value;
	}

	uint8_t value() const
	{
		return _value-1;
	}

	void reset()
	{
		_value = 0;
	}

private:
	uint16_t _value;
};

} // namespace chip8
