#include "chip8.h"

#include<stdio.h>
#include<time.h>
#include<signal.h>
#include<chrono>

#include <fstream>

#include <ncurses.h>

int msleep(unsigned long milisec)
{
    struct timespec req={0};
    time_t sec=(int)(milisec/1000);
    milisec=milisec-(sec*1000);
    req.tv_sec=sec;
    req.tv_nsec=milisec*1000000L;
    while(nanosleep(&req,&req)==-1)
         continue;
    return 1;
}

std::chrono::time_point<std::chrono::steady_clock> next_clock()
{
	return std::chrono::steady_clock::now()
	     + std::chrono::milliseconds(16);
}

void update_display(WINDOW* w, Chip8 const& c/*bool (&display)[64*32]*/)
{
	box(w, 0, 0);
	mvwprintw(w, 0, 2, "Display");

	if (not c.redraw())
		return;

	auto const& display = c.get_display();
	for (int y = 0; y < 32; ++y)
		for (int x = 0; x < 64; ++x)
		{
			bool const active = display[y*64 + x];
			if (active)
				wattrset(w, A_REVERSE);

			mvwprintw(w, y+1, x+1, " ");

			if (active)
				wattroff(w, A_REVERSE);
		}
}

void update_regs(WINDOW* w, Chip8 const& c)
{
	mvwprintw(w, 1, 1, "op:0x%02X%02X", c.mem(c.pc()), c.mem(c.pc()+1));
	mvwprintw(w, 2, 1, "PC:0x%04X", c.pc());
	mvwprintw(w, 3, 1, "SP:0x%04X", c.sp());
	mvwprintw(w, 4, 1, " L:0x%04X", c.l());

	for (int i = 0; i <= 0xF; ++i)
	{
		mvwprintw(w, 5+i, 1, "v%X:0x%04X", i, c.v(i));
	}

	box(w, 0, 0);
	mvwprintw(w, 0, 2, "Regs");
}

void update_timer(WINDOW* w, Chip8 const& c)
{
	mvwprintw(w, 1, 1, " D:  0x%02X", c.d());
	mvwprintw(w, 2, 1, " S:  0x%02X", c.s());

	box(w, 0, 0);
	mvwprintw(w, 0, 2, "Timer");
}

void update_stack(WINDOW* w, Chip8 const& c)
{
	auto const& stack =  c.stack();
	for (int i = 0; i < stack.size(); ++i)
		mvwprintw(w, 1+i, 2, "0x%04X", stack[i]);

	box(w, 0, 0);
	mvwprintw(w, 0, 2, "Stack");
}

void update_keys(WINDOW* w, Chip8 const& c)
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

	auto const input = c.input();
	for (int i = 0; i <= 0xF; ++i)
	{
		if ((c.input() >> i) & 0x1)
			wattrset(w, A_STANDOUT | A_BOLD);

		mvwprintw(w, keys[i].y, keys[i].x, "%c", keys[i].c);

		if ((c.input() >> i) & 0x1)
			wattroff(w, A_STANDOUT | A_BOLD);
	}

	box(w, 0, 0);
	mvwprintw(w, 0, 2, "Keys");
}


int main(int argc, char** argv)
{
	if (argc < 2)
		return 1;

	std::ifstream f;
	f.open(argv[1], std::fstream::in | std::ios::binary);
	if (not f.is_open())
		return 2;

	Chip8 c;

	int index = 0x200;
	char b;
	while(f.read(&b, 1))
	{
		c.mem(index, b);
		++index;
	}

	f.close();

	initscr();
	if(has_colors() == FALSE)
	{
		endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}
	start_color();
	cbreak();
	curs_set(0);
	timeout(0);
	noecho();

	init_pair(1, COLOR_RED, COLOR_BLACK);

	WINDOW *canvas;
	canvas = newwin(34, 66, 2, 2);

	WINDOW *regs;
	regs = newwin(22, 11, 2, 69);

	WINDOW *timer;
	timer = newwin(4, 11, 24, 69);

	WINDOW *keys;
	keys = newwin(8, 11, 28, 69);

	WINDOW *stack;
	stack = newwin(18, 10, 2, 81);


	auto clock = next_clock();

	int key = 0;
	int nokey = 0;
	while(1)
	{
		int newkey = getch();
		if (newkey != ERR)
		{
			key = newkey;
			nokey = 0;
		}
		else
		{
			++nokey;
			if (nokey > 500)
			{
				key = ERR;
				nokey = 0;
			}

		}

		switch(key)
		{
		case '0': c.key(1 <<  0); break;
		case '1': c.key(1 <<  1); break;
		case '2': c.key(1 <<  2); break;
		case '3': c.key(1 <<  3); break;
		case '4': c.key(1 <<  4); break;
		case '5': c.key(1 <<  5); break;
		case '6': c.key(1 <<  6); break;
		case '7': c.key(1 <<  7); break;
		case '8': c.key(1 <<  8); break;
		case '9': c.key(1 <<  9); break;
		case 'a': c.key(1 << 10); break;
		case 'b': c.key(1 << 11); break;
		case 'c': c.key(1 << 12); break;
		case 'd': c.key(1 << 13); break;
		case 'e': c.key(1 << 14); break;
		case 'f': c.key(1 << 15); break;
		default:
			c.key(0);
		}

		update_display(canvas, c);
		update_regs(regs, c);
		update_timer(timer, c);
		update_keys(keys, c);
		update_stack(stack, c);

		wrefresh(canvas);
		wrefresh(regs);
		wrefresh(timer);
		wrefresh(keys);
		wrefresh(stack);

		if (clock < std::chrono::steady_clock::now())
		{
			c.clock();
			clock = next_clock();
		}

		c.tick();
#if 1
		msleep(1);
#else
		timeout(-10);
		getch();
		timeout(0);
#endif
	}
	delwin(keys);
	delwin(timer);
	delwin(regs);
	delwin(canvas);
	endwin();
	return 0;
}
