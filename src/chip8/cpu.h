#pragma once

#include "memory.h"
#include "display.h"
#include "input.h"
#include "snapshot.h"

#include <vector>
#include <stdint.h>
#include <random>

namespace chip8
{

class Cpu
{
public:
	Cpu(Memory& mem, Display& dis, Input& inp)
	: _mem(mem)
	, _dis(dis)
	, _inp(inp)
	, _i(0)
	, _pc(0)
	, _d(0)
	, _s(0)
	, _v(16, 0)
	, _stack()
	, _rnd_engine()
	{}

	void save(Snapshot& ss) const
	{
		ss.pc    = _pc;
		ss.i     = _i;
		ss.d     = _d;
		ss.s     = _s;
		ss.v     = _v;
		ss.stack = _stack;
	}

	void clock()
	{
		if (_d > 0)
			--_d;

		if (_s > 0)
			--_s;
	}

	void tick()
	{
		_exec();
	}

	void reset()
	{
		_i  = 0;
		_pc = 0x200;
		_d  = 0;
		_s  = 0;
		_stack.clear();

		for (auto& e : _v)
			e = 0;
	}

private:
	void _exec()
	{
		uint16_t const opcode = _mem.fetch(_pc) << 8 | _mem.fetch(_pc + 1);
		uint8_t  const i      = (opcode & 0xF000) >> 12;
		uint16_t const nnn    = (opcode & 0x0FFF);
		uint8_t  const kk     = (opcode & 0x00FF);
		uint8_t  const x      = (opcode & 0x0F00) >>  8;
		uint8_t  const y      = (opcode & 0x00F0) >>  4;
		uint8_t  const p      = (opcode & 0x000F);

		uint8_t        dpc    = 2;

		switch (i)
		{
		case 0x0:
			switch (kk)
			{
			case 0xE0: _dis.clear();                                      break;
			case 0xEE:
				{
				_pc = _stack.back();
				_stack.pop_back();
				}
				break;
			}
			break;
		case 0x1: _pc = nnn; dpc = 0;                                     break;
		case 0x2: _stack.push_back(_pc); _pc = nnn; dpc = 0;              break;
		case 0x3: if (_v[x] == kk) dpc += 2;                              break;
		case 0x4: if (_v[x] != kk) dpc += 2;                              break;
		case 0x5: if (_v[x] == _v[y]) dpc += 2;                           break;
		case 0x6: _v[x] = kk;                                             break;
		case 0x7:
			{
			unsigned short s = _v[x] + kk;
			_v[0xF]          = s > 255;
			_v[x]            = static_cast<unsigned char>(s);
			break;
			}
		case 0x8:
			switch (p)
			{
			case 0: _v[x]  = _v[y];                                       break;
			case 1: _v[x] |= _v[y];                                       break;
			case 2: _v[x] &= _v[y];                                       break;
			case 3: _v[x] ^= _v[y];                                       break;
			case 4:
				{
				unsigned short s = _v[x] + _v[y];
				_v[0xF]          = s > 250;
				_v[x]            = static_cast<unsigned char>(s);
				break;
				}
			case 5: _v[0xF] = _v[x] < _v[y]; _v[x] -= _v[y];              break;
			case 6: _v[0xF] = _v[x] & 1;     _v[x] >>= 1;                 break;
			case 7: _v[0xF] = _v[x] > _v[y]; _v[x] = _v[y] - _v[x];       break;
			case 8: _v[0xF] = _v[x] & 0x8;   _v[x] <<= 1;                 break;

			}
			break;
		case 0x9: if (_v[x] != _v[y]) dpc += 2;                           break;
		case 0xA: _i = nnn;                                               break;
		case 0xB: _pc = nnn + _v[0]; dpc = 0;                             break;
		case 0xC: _v[x] = _rnd_engine() & kk;                             break;
		case 0xD: _drw(x, y, p);                                          break;
		case 0xE:
			switch (kk)
			{
			case 0x9E:
				if (_inp.has_input() and _inp.value() == _v[x])
					dpc += 2;
				break;
			case 0xA1:
				if (not _inp.has_input() or _inp.value() != _v[x])
					dpc += 2;
				break;
			}
			break;
		case 0xF:
			switch (kk)
			{
			case 0x07: _v[x] = _d;                                        break;
			case 0x0A:
				_v[x] = _inp.value();
				if (not _inp.has_input())
					dpc = 0;
				break;
			case 0x15: _d  = _v[x];                                       break;
			case 0x18: _s  = _v[x];                                       break;
			case 0x1E: _i += _v[x];                                       break;
			case 0x29: _i  = _v[x] * 5;                                   break;
			case 0x33:
				_mem.store(_i,     _v[x] / 100);
				_mem.store(_i + 1, _v[x] / 10 % 10);
				_mem.store(_i + 2, _v[x] % 10);
				break;
			case 0x55:
				for (int i = 0; i <= x; ++i)
					_mem.store(_i+i, _v[i]);
				break;
			case 0x65:
				for (int i = 0; i <= x; ++i)
					_v[i] = _mem.fetch(_i+i);
				break;
			}
			break;
		}

		_pc += dpc;
	}

	void _drw(uint8_t reg_nr_x
	        , uint8_t reg_nr_y
	        , uint8_t bytes)
	{
		_v[0xF] = 0x00;

        uint8_t const x = _v[reg_nr_x] % _dis.width();
        uint8_t const y = _v[reg_nr_y] % _dis.height();

		for(int r = 0; r < bytes; ++r)
		{
			for (int c = 0; c < 8; ++c)
			{
				bool bit = (_mem.fetch(_i + r) << c) & 0x80;

				uint8_t const rx = x + c;
				uint8_t const ry = y + r;

				if (rx >= _dis.width() or ry >= _dis.height())
					continue;

				auto const curr_dis_bit = _dis.get(rx, ry);
				if (not _v[0xF] and bit)
					_v[0xF] = curr_dis_bit == bit;

				_dis.set(rx, ry, curr_dis_bit ^ bit);
			}
		}
	}

private:
	Memory&               _mem;
	Display&              _dis;
	Input&                _inp;

	uint16_t              _i;
	uint16_t              _pc;
	uint8_t               _d;
	uint8_t               _s;
	std::vector<uint8_t>  _v;
	std::vector<uint16_t> _stack;

	std::random_device    _rnd_engine;
};

} // namespace chip8
