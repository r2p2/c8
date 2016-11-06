#pragma once

#include <vector>
#include <stdint.h>

namespace chip8
{

class Memory
{
public:
	Memory()
	: _data(4096, 0)
	{}

	uint8_t fetch(std::size_t index) const
	{
		return _data[index];
	}

	void store(std::size_t index, uint8_t data)
	{
		_data[index] = data;
	}

	void load(std::size_t start_index, std::vector<uint8_t> const& data)
	{
		if (start_index + data.size() > _data.size())
			return;

		for (std::size_t i = 0; i < data.size(); ++start_index, ++i)
			_data[start_index] = data[i];
	}

	void reset()
	{
		for (auto& e : _data)
			e = 0;
	}

private:
	std::vector<uint8_t> _data;
};

} // namespace chip8
