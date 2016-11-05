#pragma once
#include <random>
#include <vector>
#include <iostream>
#include <stdio.h>

class Chip8
{
public:
	Chip8()
	: _rnd_engine()
	, _input(0)
	, _redraw(false)
	, _memory(4096, 0x00)
	, _stack()
	, _l(0)
	, _d(0)
	, _s(0)
	, _pc(0x200)
	{
		for (int i = 0; i < (64 * 32); ++i)
			_display[i] = 0;

		for (int i = 0; i < 16; ++i)
			_v[i] = 0;

		load(0x000, {
		     0xF0, // 0
		     0x90,
		     0x90,
		     0x90,
		     0xF0,

		     0x20, // 1
		     0x60,
		     0x20,
		     0x20,
		     0x70,

		     0xF0, // 2
		     0x10,
		     0xF0,
		     0x80,
		     0xF0,

		     0xF0, // 3
		     0x10,
		     0xF0,
		     0x10,
		     0xF0,

		     0x90, // 4
		     0x90,
		     0xF0,
		     0x10,
		     0x10,

		     0xF0, // 5
		     0x80,
		     0xF0,
		     0x10,
		     0xF0,

		     0xF0, // 6
		     0x80,
		     0xF0,
		     0x90,
		     0xF0,

		     0xF0, // 7
		     0x10,
		     0x20,
		     0x40,
		     0x40,

		     0xF0, // 8
		     0x90,
		     0xF0,
		     0x90,
		     0xF0,

		     0xF0, // 9
		     0x90,
		     0xF0,
		     0x10,
		     0xF0,

		     0xF0, // A
		     0x90,
		     0xF0,
		     0x90,
		     0x90,

		     0xE0, // B
		     0x90,
		     0xE0,
		     0x90,
		     0xE0,

		     0xF0, // C
		     0x80,
		     0x80,
		     0x80,
		     0xF0,

		     0xE0, // D
		     0x90,
		     0x90,
		     0x90,
		     0xE0,

		     0xF0, // E
		     0x80,
		     0xF0,
		     0x80,
		     0xF0,

		     0xF0, // F
		     0x80,
		     0xF0,
		     0x80,
		     0x80,
		    });
	}

	void mem(int index, unsigned char value)
	{
		_memory[index] = value;
	}

	void load(int start, std::vector<unsigned char> const& data)
	{
		if (start + data.size() > _memory.size())
			return;

		for (int i = 0; i < data.size(); ++start, ++i)
			_memory[start] = data[i];
	}

	unsigned char mem(int index) const
	{
		return _memory[index];
	}

	void key(unsigned short k)
	{
		_input = k;
	}

	unsigned short input() const
	{
		return _input;
	}

	unsigned short pc() const
	{
		return _pc;
	}

	unsigned short sp() const
	{
		return _stack.size();
	}

	unsigned short l() const
	{
		return _l;
	}

	unsigned short v(int index) const
	{
		return _v[index];
	}

	unsigned char d() const
	{
		return _d;
	}

	unsigned char s() const
	{
		return _s;
	}

	std::vector<unsigned short> const& stack() const
	{
		return _stack;
	}

	bool redraw() const
	{
		return _redraw;
	}

	auto get_display() const -> const bool (&)[64*32]
	{
		return _display;
	}

	void tick()
	{
		_redraw = false;

		_exec();

		if (_d > 0)
			--_d;

		if (_s > 0)
		{
			--_s;
			_beep_on();
		}
		else
			_beep_off();
	}

private:
	void _exec()
	{
		unsigned short opcode = _memory[_pc] << 8 | _memory[_pc + 1];
		unsigned char  i      = (opcode & 0xF000) >> 12;
		unsigned short nnn    = (opcode & 0x0FFF);
		unsigned char  kk     = (opcode & 0x00FF);
		unsigned char  x      = (opcode & 0x0F00) >>  8;
		unsigned char  y      = (opcode & 0x00F0) >>  4;
		unsigned char  p      = (opcode & 0x000F);

		if (opcode == 0x00E0)
		{
			_cls();
			_next();
			return;
		}

		if (opcode == 0x00EE)
		{
			_pc = _stack.back();
			_stack.pop_back();

			_next();
			return;
		}

		if ((opcode & 0xFF00) == 0x0000)
		{
			_next();
			return;
		}

		switch (i)
		{
		case 0x1: _pc = nnn;                                   break; // jmp
		case 0x2: _stack.push_back(_pc); _pc = nnn;            break; // call
		case 0x3: if (_v[x] == kk) { _next(); }       _next(); break; // sk.e
		case 0x4: if (_v[x] != kk) { _next(); }       _next(); break; // sk.ne
		case 0x5: if (_v[x] == _v[y]) { _next(); }    _next(); break; // sk.e
		case 0x6: _v[x] = kk;                         _next(); break; // mov
		case 0x7:                                                     // add
			{
			unsigned short s = _v[x] + kk;
			_v[0xF]          = s > 255;
			_v[x]            = static_cast<unsigned char>(s);
			_next();
			break;
			}
		case 0x8:
			switch (p)
			{
			case 0: _v[x]  = _v[y];                          break; // mov
			case 1: _v[x] |= _v[y];                          break; // or
			case 2: _v[x] &= _v[y];                          break; // and
			case 3: _v[x] ^= _v[y];                          break; // xor
			case 4:                                                 // add
				{
				unsigned short s = _v[x] + _v[y];
				_v[0xF]          = s > 250;
				_v[x]            = static_cast<unsigned char>(s);
				break;
				}
			case 5: _v[0xF] = _v[x] < _v[y]; _v[x] -= _v[y];        break; // sub
			case 6: _v[0xF] = _v[x] & 1;     _v[x] >>= 1;           break; // shr
			case 7: _v[0xF] = _v[x] > _v[y]; _v[x] = _v[y] - _v[x]; break; // sub
			case 8: _v[0xF] = _v[x] & 0x8;   _v[x] <<= 1;           break; // shl

			}
			_next();
			break;
		case 0x9: if (_v[x] != _v[y]) { _next(); }    _next(); break; // sk.ne
		case 0xA: _l = nnn;                           _next(); break; // mov
		case 0xB: _pc = nnn + _v[0];                           break; // jmp
		case 0xC: _v[x] = _rnd_engine() & kk;         _next(); break; // rnd
		case 0xD: _drw(x, y, p);                      _next(); break; // drw
		case 0xE:
			switch (kk)
			{
			case 0x9E: if (_input & (1 << x)) _next();         break; // skp
			case 0xA1: if (!(_input & (1 << x))) _next();      break; // skp
			}
			_next();
			break;
		case 0xF:
			switch (kk)
			{
			case 0x07: _v[x] = _d;                    _next(); break; // mov
			case 0x0A:                                                // wk
				_v[x] = 0;
				while (_input > 1)
				{
					++_v[x];
					_input >>= 1;
				}
				if (_v[x])
					_next();
			case 0x15: _d = _v[x];                    _next(); break;
			case 0x18: _s = _v[x];                    _next(); break;
			case 0x1E: _l += _v[x];                    _next(); break;
			case 0x29: _l = _v[x] * 5;                _next(); break;
			case 0x33:
				_memory[_l]     = _v[x] / 100;
				_memory[_l + 1] = _v[x] / 10 % 10;
				_memory[_l + 2] = _v[x] % 10;

				_next();
				break;
			case 0x55:
				for (int i = 0; i <= x; ++i)
					_memory[_l+i] = _v[i];

				_next();
				break;
			case 0x65:
				for (int i = 0; i <= x; ++i)
					_v[i] = _memory[_l+i];

				_next();
				break;
			}
			break;
		}
	}

	void _next()
	{
		_pc += 2;
	}

	void _beep_on()
	{
	//	std::cerr << "BEEP" << std::endl;
	}

	void _beep_off()
	{
	//	std::cerr << "beep" << std::endl;
	}

	void _cls()
	{
		_redraw = true;

		for (int i = 0; i < (64 * 32); ++i)
			_display[i] = 0;
	}

	void _drw(unsigned char reg_nr_x
	        , unsigned char reg_nr_y
	        , unsigned char bytes)
	{
		_redraw = true;
		_v[0xF] = 0x00;

        unsigned char x = _v[reg_nr_x];
        unsigned char y = _v[reg_nr_y];

		x %= 64;
		y %= 32;

		for(int r = 0; r < bytes; ++r)
		{
			for (int c = 0; c < 8; ++c)
			{
				bool bit = (_memory[_l + r] << c) & 0x80;

				unsigned char rx = x + c;
				unsigned char ry = y + r;

				if (rx > 63 or ry > 31)
					continue;

				size_t const index = ry*64 + rx;
				if (not _v[0xF] and bit)
					_v[0xF] = _display[index] == bit;

				_display[index] ^= bit;
			}
		}
	}

private:
	std::random_device          _rnd_engine;
	unsigned short              _input;
	bool                        _display[64 * 32];
	bool                        _redraw;

	std::vector<unsigned char>  _memory;
	std::vector<unsigned short> _stack;

	unsigned char               _v[16];
	unsigned short              _l;

	unsigned char               _d;
	unsigned char               _s;

	unsigned short              _pc;
};
