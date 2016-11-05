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
			return;
		}

		if (opcode == 0x00EE)
		{
			_return();
			return;
		}

		if ((opcode & 0xFF00) == 0x0000)
		{
			_next();
			return;
		}

		if (i == 1)
		{
			_jmp(nnn);
			return;
		}

		if (i == 2)
		{
			_call(nnn);
			return;
		}

		if (i == 3)
		{
			_se1(x, kk);
			return;
		}

		if (i == 4)
		{
			_sne1(x, kk);
			return;
		}

		if (i == 5)
		{
			_se2(x, y);
			return;
		}

		if (i == 6)
		{
			_ld1(x, kk);
			return;
		}

		if (i == 7)
		{
			_add1(x, kk);
			return;
		}

		if (i == 8 && p == 0)
		{
			_ld2(x, y);
			return;
		}

		if (i == 8 && p == 1)
		{
			_or(x, y);
			return;
		}

		if (i == 8 && p == 2)
		{
			_and(x, y);
			return;
		}
		if (i == 8 && p == 3)
		{
			_xor(x, y);
			return;
		}

		if (i == 8 && p == 4)
		{
			_add2(x, y);
			return;
		}

		if (i == 8 && p == 5)
		{
			_sub(x, y);
			return;
		}

		if (i == 8 && p == 6)
		{
			_shr(x, y);
			return;
		}

		if (i == 8 && p == 7)
		{
			_subn(x, y);
			return;
		}

		if (i == 8 && p == 8)
		{
			_shr(x, y);
			return;
		}

		if (i == 9)
		{
			_sne2(x, y);
			return;
		}

		if (i == 0xA)
		{
			_ld(nnn);
			return;
		}

		if (i == 0xB)
		{
			_jmp_v0(nnn);
			return;
		}

		if (i == 0xC)
		{
			_rnd(x, kk);
			return;
		}

		if (i == 0xD)
		{
			_drw(x, y, p);
			return;
		}

		if (i == 0xE and kk == 0x9E)
		{
			_skp(x);
			return;
		}

		if (i == 0xE and kk == 0xA1)
		{
			_sknp(x);
			return;
		}

		if (i == 0xF and kk == 0x07)
		{
			_ldd(x);
			return;
		}

		if (i == 0xF and kk == 0x0A)
		{
			_wk(x);
			return;
		}

		if (i == 0xF and kk == 0x15)
		{
			_sdd(x);
			return;
		}

		if (i == 0xF and kk == 0x18)
		{
			_sds(x);
			return;
		}

		if (i == 0xF and kk == 0x1E)
		{
			_addl(x);
			return;
		}

		if (i == 0xF and kk == 0x29)
		{
			_ldl(x);
			return;
		}

		if (i == 0xF and kk == 0x33)
		{
			_ldb(x);
			return;
		}

		if (i == 0xF and kk == 0x55)
		{
			_store(x);
			return;
		}

		if (i == 0xF and kk == 0x65)
		{
			_load(x);
			return;
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

		_next();
	}

	void _return()
	{
		_pc = _stack.back();
		_stack.pop_back();

		_next();
	}

	void _jmp(unsigned short addr)
	{
		_pc = addr;
	}

	void _call(unsigned short addr)
	{
		_stack.push_back(_pc);
		_pc = addr;
	}

	void _se1(unsigned char reg_nr, unsigned char value)
	{
		if (_v[reg_nr] == value)
			_next();

		_next();
	}

	void _sne1(unsigned char reg_nr, unsigned char value)
	{
		if (_v[reg_nr] != value)
			_next();

		_next();
	}

	void _se2(unsigned char reg_nr_a, unsigned char reg_nr_b)
	{
		if (_v[reg_nr_a] == _v[reg_nr_b])
			_next();

		_next();
	}

	void _ld1(unsigned char reg_nr, unsigned char value)
	{
		_v[reg_nr] = value;

		_next();
	}

	void _add1(unsigned char reg_nr, unsigned char value)
	{
		unsigned short sum = static_cast<unsigned short>(_v[reg_nr])
		                   + static_cast<unsigned short>(value);

		_v[0xF] = sum > 255;
		_v[reg_nr] = static_cast<unsigned char>(sum);

		_next();
	}

	void _ld2(unsigned char reg_nr_a, unsigned char reg_nr_b)
	{
		_v[reg_nr_a] = _v[reg_nr_b];

		_next();
	}

	void _or(unsigned char reg_nr_a, unsigned char reg_nr_b)
	{
		_v[reg_nr_a] |= _v[reg_nr_b];

		_next();
	}

	void _and(unsigned char reg_nr_a, unsigned char reg_nr_b)
	{
		_v[reg_nr_a] &= _v[reg_nr_b];

		_next();
	}

	void _xor(unsigned char reg_nr_a, unsigned char reg_nr_b)
	{
		_v[reg_nr_a] ^= _v[reg_nr_b];

		_next();
	}

	void _add2(unsigned char reg_nr_a, unsigned char reg_nr_b)
	{
		unsigned short sum = static_cast<unsigned short>(_v[reg_nr_a])
		                   + static_cast<unsigned short>(_v[reg_nr_b]);

		_v[0xF] = sum > 255;

		_v[reg_nr_a] = static_cast<unsigned char>(sum);

		_next();
	}

	void _sub(unsigned char reg_nr_a, unsigned char reg_nr_b)
	{
		_v[0xF] = _v[reg_nr_a] < _v[reg_nr_b];

		_v[reg_nr_a] -= _v[reg_nr_b];

		_next();
	}

	void _shr(unsigned char reg_nr_a, unsigned char reg_nr_b)
	{
		_v[0xF] = _v[reg_nr_a] & 0x1;

		_v[reg_nr_a] >>= 1;

		_next();
	}

	void _subn(unsigned char reg_nr_a, unsigned char reg_nr_b)
	{
		_v[0xF] = _v[reg_nr_b] < _v[reg_nr_a];

		_v[reg_nr_a] = _v[reg_nr_b] - _v[reg_nr_a];

		_next();
	}

	void _shl(unsigned char reg_nr_a, unsigned char reg_nr_b)
	{
		_v[0xF] = _v[reg_nr_a] & 0x8;

		_v[reg_nr_a] <<= 1;

		_next();
	}

	void _sne2(unsigned char reg_nr_a, unsigned char reg_nr_b)
	{
		if (_v[reg_nr_a] != _v[reg_nr_b])
			_next();

		_next();
	}

	void _ld(unsigned short value)
	{
		_l = value;

		_next();
	}

	void _jmp_v0(unsigned short addr)
	{
		_pc = addr + _v[0];
	}

	void _rnd(unsigned char reg_nr, unsigned short value)
	{
		_v[reg_nr] = static_cast<unsigned char>(_rnd_engine()) & value;

		_next();
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

		_next();
	}

	void _skp(unsigned char key_nr)
	{
		if (_input & (0x0001 << key_nr))
			_next();

		_next();
	}

	void _sknp(unsigned char key_nr)
	{
		if (not (_input & (0x0001 << key_nr)))
			_next();

		_next();
	}

	void _ldd(unsigned char reg_nr)
	{
		_v[reg_nr] = _d;

		_next();
	}

	void _wk(unsigned char reg_nr)
	{
		_v[reg_nr] = 0;
		while (_input > 0x0001)
		{
			++_v[reg_nr];
			_input >>= 1;
		}

		if (_v[reg_nr])
			_next();
	}

	void _sdd(unsigned char reg_nr)
	{
		_d = _v[reg_nr];

		_next();
	}

	void _sds(unsigned char reg_nr)
	{
		_s = _v[reg_nr];

		_next();
	}

	void _addl(unsigned char reg_nr)
	{
		_l += _v[reg_nr];

		_next();
	}

	void _ldl(unsigned char reg_nr)
	{
		_l = _v[reg_nr]*5;

		_next();
	}

	void _ldb(unsigned char reg_nr)
	{
		_memory[_l]     = _v[reg_nr] / 100;
		_memory[_l + 1] = _v[reg_nr] / 10 % 10;
		_memory[_l + 2] = _v[reg_nr] % 10;

		_next();
	}

	void _store(unsigned char reg_nr)
	{
		for (int i = 0; i <= reg_nr; ++i)
		{
			_memory[_l+i] = _v[i];
		}

		_next();
	}

	void _load(unsigned char reg_nr)
	{
		for (int i = 0; i <= reg_nr; ++i)
		{
			_v[i] = _memory[_l+i];
		}

		_next();
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
