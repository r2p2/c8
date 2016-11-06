#pragma once

#include "snapshot.h"

#include <stdint.h>

namespace chip8
{

class Input
{
public:
	Input()
	: _avlbl(false)
	, _value(0)
	{}

	void save(Snapshot& ss) const
	{
		ss.has_input = _avlbl;
		ss.input     = _value;
	}

	void press(uint8_t value)
	{
		_avlbl = true;
		_value = value;
	}

	void release()
	{
		_avlbl = false;
	}

	bool has_input() const
	{
		return _avlbl;
	}

	uint8_t value() const
	{
		return _value;
	}

	void reset()
	{
		_avlbl = false;
		_value = 0;
	}

private:
	bool    _avlbl;
	uint8_t _value;
};

} // namespace chip8
