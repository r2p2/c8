#pragma once
#include <random>
#include <iostream>
#include <stdio.h>

class Chip8
{
public:
	Chip8()
	: _rnd_engine()
	, _input(0)
	, _l(0)
	, _d(0)
	, _s(0)
	, _pc(0x200)
	, _sp(0)
	{
		for (int i = 0; i < (64 * 32); ++i)
			_display[i] = 0;

		for (int i = 0; i < 4096; ++i)
			_memory[i] = 0;

		for (int i = 0; i < 16; ++i)
			_stack[i] = 0;

		for (int i = 0; i < 16; ++i)
			_v[i] = 0;

		mem(0x000, 0xF0); // 0
		mem(0x001, 0x90);
		mem(0x002, 0x90);
		mem(0x003, 0x90);
		mem(0x004, 0xF0);

		mem(0x005, 0x20); // 1
		mem(0x006, 0x60);
		mem(0x007, 0x20);
		mem(0x008, 0x20);
		mem(0x009, 0x70);

		mem(0x00a, 0xF0); // 2
		mem(0x00b, 0x10);
		mem(0x00c, 0xF0);
		mem(0x00d, 0x80);
		mem(0x00e, 0xF0);

		mem(0x00f, 0xF0); // 3
		mem(0x010, 0x10);
		mem(0x011, 0xF0);
		mem(0x012, 0x10);
		mem(0x013, 0xF0);

		mem(0x014, 0x90); // 4
		mem(0x015, 0x90);
		mem(0x016, 0xF0);
		mem(0x017, 0x10);
		mem(0x018, 0x10);

		mem(0x019, 0xF0); // 5
		mem(0x01a, 0x80);
		mem(0x01b, 0xF0);
		mem(0x01c, 0x10);
		mem(0x01d, 0xF0);

		mem(0x01e, 0xF0); // 6
		mem(0x01f, 0x80);
		mem(0x020, 0xF0);
		mem(0x021, 0x90);
		mem(0x022, 0xF0);

		mem(0x023, 0xF0); // 7
		mem(0x024, 0x10);
		mem(0x025, 0x20);
		mem(0x027, 0x40);
		mem(0x028, 0x40);

		mem(0x029, 0xF0); // 8
		mem(0x02a, 0x90);
		mem(0x02b, 0xF0);
		mem(0x02c, 0x90);
		mem(0x02d, 0xF0);

		mem(0x02e, 0xF0); // 9
		mem(0x02f, 0x90);
		mem(0x030, 0xF0);
		mem(0x031, 0x10);
		mem(0x032, 0xF0);

		mem(0x033, 0xF0); // A
		mem(0x034, 0x90);
		mem(0x035, 0xF0);
		mem(0x036, 0x90);
		mem(0x037, 0x90);

		mem(0x038, 0xE0); // B
		mem(0x039, 0x90);
		mem(0x03a, 0xE0);
		mem(0x03b, 0x90);
		mem(0x03c, 0xE0);

		mem(0x03d, 0xF0); // C
		mem(0x03e, 0x80);
		mem(0x03f, 0x80);
		mem(0x040, 0x80);
		mem(0x041, 0xF0);

		mem(0x042, 0xE0); // D
		mem(0x043, 0x90);
		mem(0x044, 0x90);
		mem(0x045, 0x90);
		mem(0x046, 0xE0);

		mem(0x047, 0xF0); // E
		mem(0x048, 0x80);
		mem(0x049, 0xF0);
		mem(0x04a, 0x80);
		mem(0x04b, 0xF0);

		mem(0x04c, 0xF0); // F
		mem(0x04d, 0x80);
		mem(0x04e, 0xF0);
		mem(0x04f, 0x80);
		mem(0x050, 0x80);
	}

	void mem(int index, unsigned char value)
	{
		_memory[index] = value;
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
		return _sp;
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

	void show()
	{
		printf("PC:%04x SP: %04x L:%04x D:%04x S:%04x => %02x%02x\n"
		     , _pc, _sp, _l,  _d, _s, _memory[_pc], _memory[_pc+1]);
		for (int i = 0; i <= 0xF; ++i)
		{
			printf("v[%02x]:%04x ", i, _v[i]);
		}
		printf("\n");
	}


	auto get_display() -> bool (&)[64*32]
	{
		return _display;
	}

	void display()
	{
		for (int y = 0; y < 32; ++y)
		{
			for (int x = 0; x < 64; ++x)
			{
				if (_display[y*64 + x])
					printf("X");
				else
					printf(" ");
			}
			printf("\n");
		}
	}

	void tick()
	{
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
		unsigned int i = _memory[_pc] << 8 | _memory[_pc + 1];

		if (i == 0x00E0)
			_cls();
		else if (i == 0x00EE)
			_return();
		else if ((i & 0xFF00) == 0x0000)
			_next();
		else if ((i & 0xF000) == 0x1000)
			_jmp(i & 0x0FFF);
		else if ((i & 0xF000) == 0x2000)
			_call(i & 0x0FFF);
		else if ((i & 0xF000) == 0x3000)
		{
			unsigned char  reg_nr = (i & 0x0F00) >> 8;
			unsigned short value  = (i & 0x00FF);
			_se(reg_nr, value);
		}
		else if ((i & 0xF000) == 0x4000)
		{
			unsigned char  reg_nr = (i & 0x0F00) >> 8;
			unsigned short value  = (i & 0x00FF);
			_sne(reg_nr, value);
		}
		else if ((i & 0xF000) == 0x5000)
		{
			unsigned char reg_nr_a = (i & 0x0F00) >> 8;
			unsigned char reg_nr_b = (i & 0x00F0) >> 4;
			_se(reg_nr_a, reg_nr_b);
		}
		else if ((i & 0xF000) == 0x6000)
		{
			unsigned char  reg_nr = (i & 0x0F00) >> 8;
			unsigned short value  = (i & 0x00FF);
			_ld(reg_nr, value);
		}
		else if ((i & 0xF000) == 0x7000)
		{
			unsigned char  reg_nr = (i & 0x0F00) >> 8;
			unsigned short value  = (i & 0x00FF);
			_add(reg_nr, value);
		}
		else if ((i & 0xF00F) == 0x8000)
		{
			unsigned char reg_nr_a = (i & 0x0F00) >> 8;
			unsigned char reg_nr_b = (i & 0x00F0) >> 4;
			_ld(reg_nr_a, reg_nr_b);
		}
		else if ((i & 0xF00F) == 0x8001)
		{
			unsigned char reg_nr_a = (i & 0x0F00) >> 8;
			unsigned char reg_nr_b = (i & 0x00F0) >> 4;
			_or(reg_nr_a, reg_nr_b);
		}
		else if ((i & 0xF00F) == 0x8002)
		{
			unsigned char reg_nr_a = (i & 0x0F00) >> 8;
			unsigned char reg_nr_b = (i & 0x00F0) >> 4;
			_and(reg_nr_a, reg_nr_b);
		}
		else if ((i & 0xF00F) == 0x8003)
		{
			unsigned char reg_nr_a = (i & 0x0F00) >> 8;
			unsigned char reg_nr_b = (i & 0x00F0) >> 4;
			_xor(reg_nr_a, reg_nr_b);
		}
		else if ((i & 0xF00F) == 0x8004)
		{
			unsigned char reg_nr_a = (i & 0x0F00) >> 8;
			unsigned char reg_nr_b = (i & 0x00F0) >> 4;
			_add(reg_nr_a, reg_nr_b);
		}
		else if ((i & 0xF00F) == 0x8005)
		{
			unsigned char reg_nr_a = (i & 0x0F00) >> 8;
			unsigned char reg_nr_b = (i & 0x00F0) >> 4;
			_sub(reg_nr_a, reg_nr_b);
		}
		else if ((i & 0xF00F) == 0x8006)
		{
			unsigned char reg_nr_a = (i & 0x0F00) >> 8;
			unsigned char reg_nr_b = (i & 0x00F0) >> 4;
			_shr(reg_nr_a, reg_nr_b);
		}
		else if ((i & 0xF00F) == 0x8007)
		{
			unsigned char reg_nr_a = (i & 0x0F00) >> 8;
			unsigned char reg_nr_b = (i & 0x00F0) >> 4;
			_subn(reg_nr_a, reg_nr_b);
		}
		else if ((i & 0xF00F) == 0x800E)
		{
			unsigned char reg_nr_a = (i & 0x0F00) >> 8;
			unsigned char reg_nr_b = (i & 0x00F0) >> 4;
			_shr(reg_nr_a, reg_nr_b);
		}
		else if ((i & 0xF000) == 0x9000)
		{
			unsigned char reg_nr_a = (i & 0x0F00) >> 8;
			unsigned char reg_nr_b = (i & 0x00F0) >> 4;
			_sne(reg_nr_a, reg_nr_b);
		}
		else if ((i & 0xF000) == 0xA000)
		{
			unsigned short value  = (i & 0x0FFF);
			_ld(value);
		}
		else if ((i & 0xF000) == 0xB000)
		{
			unsigned short addr  = (i & 0x0FFF);
			_jmp_v0(addr);
		}
		else if ((i & 0xF000) == 0xC000)
		{
			unsigned char  reg_nr = (i & 0x0F00) >> 8;
			unsigned short value  = (i & 0x00FF);
			_rnd(reg_nr, value);
		}
		else if ((i & 0xF000) == 0xD000)
		{
			unsigned char bytes    = i & 0x000F;
			unsigned char reg_nr_y = (i & 0X00F0) >> 4;
			unsigned char reg_nr_x = (i & 0X0F00) >> 8;

			_drw(reg_nr_x, reg_nr_y, bytes);
		}
		else if ((i & 0xF0FF) == 0xE09E)
		{
			unsigned char key_nr = (i & 0X0F00) >> 8;

			_skp(key_nr);
		}
		else if ((i & 0xF0FF) == 0xE0A1)
		{
			unsigned char key_nr = (i & 0X0F00) >> 8;

			_sknp(key_nr);
		}
		else if ((i & 0xF0FF) == 0xF007)
		{
			unsigned char reg_nr = (i & 0X0F00) >> 8;

			_ldd(reg_nr);
		}
		else if ((i & 0xF0FF) == 0xF00A)
		{
			unsigned char reg_nr = (i & 0X0F00) >> 8;

			_wk(reg_nr);
		}
		else if ((i & 0xF0FF) == 0xF015)
		{
			unsigned char reg_nr = (i & 0X0F00) >> 8;

			_sdd(reg_nr);
		}
		else if ((i & 0xF0FF) == 0xF018)
		{
			unsigned char reg_nr = (i & 0X0F00) >> 8;

			_sds(reg_nr);
		}
		else if ((i & 0xF0FF) == 0xF01E)
		{
			unsigned char reg_nr = (i & 0X0F00) >> 8;

			_addl(reg_nr);
		}
		else if ((i & 0xF0FF) == 0xF029)
		{
			unsigned char reg_nr = (i & 0X0F00) >> 8;

			_ldl(reg_nr);
		}
		else if ((i & 0xF0FF) == 0xF033)
		{
			unsigned char reg_nr = (i & 0X0F00) >> 8;

			_ldb(reg_nr);
		}
		else if ((i & 0xF0FF) == 0xF055)
		{
			unsigned char reg_nr = (i & 0X0F00) >> 8;

			_store(reg_nr);
		}
		else if ((i & 0xF0FF) == 0xF065)
		{
			unsigned char reg_nr = (i & 0X0F00) >> 8;

			_load(reg_nr);
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
		for (int i = 0; i < (64 * 32); ++i)
			_display[i] = 0;

		_next();
	}

	void _return()
	{
		--_sp;
		_pc = _stack[_sp];

		_next();
	}

	void _jmp(unsigned short addr)
	{
		_pc = addr;
	}

	void _call(unsigned short addr)
	{
		_stack[_sp] = _pc;
		++_sp;
		_pc = addr;
	}

	void _se(unsigned char reg_nr, unsigned short value)
	{
		if ((_v[reg_nr] & 0xFF) == (value & 0xFF))
			_next();

		_next();
	}

	void _sne(unsigned char reg_nr, unsigned short value)
	{
		if (_v[reg_nr] != (value & 0xFF))
			_next();

		_next();
	}

	void _se(unsigned char reg_nr_a, unsigned char reg_nr_b)
	{
		if (_v[reg_nr_a] == _v[reg_nr_b])
			_next();

		_next();
	}

	void _ld(unsigned char reg_nr, unsigned short value)
	{
		_v[reg_nr] = value;

		_next();
	}

	void _add(unsigned char reg_nr, unsigned short value)
	{
		unsigned short sum = static_cast<unsigned short>(_v[reg_nr])
		                   + value;

		_v[0xF] = sum > 255;
		_v[reg_nr] = static_cast<unsigned short>(sum);

		_next();
	}

	void _ld(unsigned char reg_nr_a, unsigned char reg_nr_b)
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

	void _add(unsigned char reg_nr_a, unsigned char reg_nr_b)
	{
		unsigned short sum = static_cast<unsigned short>(_v[reg_nr_a])
		                   + static_cast<unsigned short>(_v[reg_nr_b]);

		_v[0xF] = sum > 255;

		_v[reg_nr_a] = static_cast<unsigned short>(sum);

		_next();
	}

	void _sub(unsigned char reg_nr_a, unsigned char reg_nr_b)
	{
		_v[0xF] = _v[reg_nr_a] > _v[reg_nr_b];

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
		_v[0xF] = _v[reg_nr_b] > _v[reg_nr_a];

		_v[reg_nr_a] = _v[reg_nr_b] - _v[reg_nr_a];

		_next();
	}

	void _shl(unsigned char reg_nr_a, unsigned char reg_nr_b)
	{
		_v[0xF] = _v[reg_nr_a] & 0x8;

		_v[reg_nr_a] <<= 1;

		_next();
	}

	void _sne(unsigned char reg_nr_a, unsigned char reg_nr_b)
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
		if ((_input >> _v[key_nr]) & 0x1)
			_next();

		_next();
	}

	void _sknp(unsigned char key_nr)
	{
		if ((_input >> _v[key_nr]) & 0x1)
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
		if (not _input)
			return;

		_v[reg_nr] = _input;

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
	std::random_device _rnd_engine;
	unsigned short     _input;
	bool               _display[64 * 32];

	unsigned char      _memory[4096];
	unsigned short     _stack[16];

	unsigned short     _v[16];
	unsigned short     _l;

	unsigned char      _d;
	unsigned char      _s;

	unsigned short     _pc;
	unsigned short     _sp;
};
