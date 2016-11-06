#include <chip8/system.h>
#include <nc/ui.h>

#include<stdio.h>
#include<time.h>
#include<signal.h>

#include <fstream>

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

int main(int argc, char** argv)
{
	if (argc < 2)
		return 1;

	std::ifstream f;
	f.open(argv[1], std::fstream::in | std::ios::binary);
	if (not f.is_open())
		return 2;

	std::vector<uint8_t> rom;

	char b;
	while(f.read(&b, 1))
		rom.push_back(b);
	f.close();

	chip8::System chip8;
	chip8.reset();
	chip8.load_rom(rom);

	Ui ui(chip8);
	if (ui.init() != 0)
		return 3;

	while(1)
	{
		ui.update();
		chip8.tick();
		msleep(1);
	}

	return 0;
}
