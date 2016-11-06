#pragma once

#include <chip8/system.h>

#include <ncurses.h>

class Ui
{
public:
	Ui(chip8::System& chip8)
	: _chip8(chip8)
	, _initialized(false)
	, _win_dis(nullptr)
	, _win_reg(nullptr)
	, _win_tim(nullptr)
	, _win_key(nullptr)
	, _win_sta(nullptr)
	, _key(0)
	, _nokey(0)
	{
	}

	~Ui()
	{
		if (not _initialized)
			return;

		delwin(_win_key);
		delwin(_win_tim);
		delwin(_win_reg);
		delwin(_win_dis);
		delwin(_win_sta);
		endwin();
	}

	int init()
	{
		initscr();
		start_color();
		cbreak();
		curs_set(0);
		timeout(0);
		noecho();

		if(has_colors() == FALSE)
			return 1;

		_win_dis = newwin(34 , 66 ,  2 ,  2);
		_win_reg = newwin(22 , 11 ,  2 , 69);
		_win_tim = newwin( 4 , 11 , 24 , 69);
		_win_key = newwin( 8 , 11 , 28 , 69);
		_win_sta = newwin(18 , 10 ,  2 , 81);

		_key = 0;
		_nokey = 0;
		_initialized = true;
		return 0;
	}

	void update()
	{
		if (not _initialized)
			return;

		_fetch_input();

		chip8::Snapshot ss = _chip8.save();

		_update_display();
		_update_regs(ss);
		_update_timer(ss);
		_update_keys(ss);
		_update_stack(ss);

		wrefresh(_win_dis);
		wrefresh(_win_reg);
		wrefresh(_win_tim);
		wrefresh(_win_key);
		wrefresh(_win_sta);
	}

private:
	void _fetch_input()
	{
		int _newkey = getch();
		if (_newkey != ERR)
		{
			_key = _newkey;
			_nokey = 0;
		}
		else
		{
			++_nokey;
			if (_nokey > 200)
			{
				_key = ERR;
				_nokey = 0;
			}
		}

		switch(_key)
		{
		case '0': _chip8.key_pressed( 0); break;
		case '1': _chip8.key_pressed( 1); break;
		case '2': _chip8.key_pressed( 2); break;
		case '3': _chip8.key_pressed( 3); break;
		case '4': _chip8.key_pressed( 4); break;
		case '5': _chip8.key_pressed( 5); break;
		case '6': _chip8.key_pressed( 6); break;
		case '7': _chip8.key_pressed( 7); break;
		case '8': _chip8.key_pressed( 8); break;
		case '9': _chip8.key_pressed( 9); break;
		case 'a': _chip8.key_pressed(10); break;
		case 'b': _chip8.key_pressed(11); break;
		case 'c': _chip8.key_pressed(12); break;
		case 'd': _chip8.key_pressed(13); break;
		case 'e': _chip8.key_pressed(14); break;
		case 'f': _chip8.key_pressed(15); break;
		default:
			_chip8.key_released();
		}
	}

	void _update_display()
	{
		box(_win_dis, 0, 0);
		mvwprintw(_win_dis, 0, 2, "Display");

		if (not _chip8.redraw_required())
			return;

		for (int y = 0; y < 32; ++y)
			for (int x = 0; x < 64; ++x)
			{
				bool const active = _chip8.pixel(x, y);
				if (active)
					wattrset(_win_dis, A_REVERSE);

				mvwprintw(_win_dis, y+1, x+1, " ");

				if (active)
					wattroff(_win_dis, A_REVERSE);
			}
	}

	void _update_regs(chip8::Snapshot const& ss)
	{
		mvwprintw(_win_reg, 1, 1, "op:0x%02X%02X", ss.memory[ss.pc], ss.memory[ss.pc+1]);
		mvwprintw(_win_reg, 2, 1, "PC:0x%04X", ss.pc);
		mvwprintw(_win_reg, 3, 1, "SP:0x%04X", ss.stack.size());
		mvwprintw(_win_reg, 4, 1, " I:0x%04X", ss.i);

		for (int i = 0; i <= 0xF; ++i)
			mvwprintw(_win_reg, 5+i, 1, "v%X:0x%04X", i, ss.v[i]);

		box(_win_reg, 0, 0);
		mvwprintw(_win_reg, 0, 2, "Regs");
	}

	void _update_timer(chip8::Snapshot const& ss)
	{
		mvwprintw(_win_tim, 1, 1, " D:  0x%02X", ss.d);
		mvwprintw(_win_tim, 2, 1, " S:  0x%02X", ss.s);

		box(_win_tim, 0, 0);
		mvwprintw(_win_tim, 0, 2, "Timer");
	}

	void _update_stack(chip8::Snapshot const& ss)
	{
		auto const& stack =  ss.stack;
		for (int i = 0; i < 16; ++i)
		{
			if (i < stack.size())
				mvwprintw(_win_sta, 1+i, 2, "0x%04X", stack[i]);
			else
				mvwprintw(_win_sta, 1+i, 2, "      ");
		}

		box(_win_sta, 0, 0);
		mvwprintw(_win_sta, 0, 2, "Stack");
	}

	void _update_keys(chip8::Snapshot const& ss)
	{
		struct Key {
			int value;
			int x;
			int y;
			char c;
		};

		Key keys[] = {
			{  1, 4, 5, '0'},
			{  2, 2, 2, '1'},
			{  3, 4, 2, '2'},
			{  4, 6, 2, '3'},
			{  5, 2, 3, '4'},
			{  6, 4, 3, '5'},
			{  7, 6, 3, '6'},
			{  8, 2, 4, '7'},
			{  9, 4, 4, '8'},
			{ 10, 6, 4, '9'},
			{ 11, 2, 5, 'A'},
			{ 12, 6, 5, 'B'},
			{ 13, 8, 2, 'C'},
			{ 14, 8, 3, 'D'},
			{ 15, 8, 4, 'E'},
			{ 16, 8, 5, 'F'}
		};

		for (int i = 0; i <= 0xF; ++i)
		{
			if (ss.input == keys[i].value)
				wattrset(_win_key, A_STANDOUT | A_BOLD);

			mvwprintw(_win_key, keys[i].y, keys[i].x, "%c", keys[i].c);

			if (ss.input == keys[i].value)
				wattroff(_win_key, A_STANDOUT | A_BOLD);
		}

		box(_win_key, 0, 0);
		mvwprintw(_win_key, 0, 2, "Keys");
	}

private:
	chip8::System& _chip8;

	bool           _initialized;
	WINDOW*        _win_dis;
	WINDOW*        _win_reg;
	WINDOW*        _win_tim;
	WINDOW*        _win_key;
	WINDOW*        _win_sta;

	int            _key;
	int            _nokey;
};
