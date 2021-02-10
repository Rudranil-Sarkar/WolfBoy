#include "Emulator.h"
#include <iostream>

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "Need the Rom Path" << std::endl;
		exit(-1);
	}

	Emulator::LoadRom(argv[1]);
	Emulator::RunLoop();
	Emulator::Quit();
	return 0;
}
