#include "cart.h"
#include "bus.h"
#include <cstdio>
#include <cstring>
#include "usefullfuncs.h"

Cart::Cart()
{
	memset(Rom_Banks, 0, sizeof(Rom_Banks));
	memset(Ram_Banks, 0, sizeof(Ram_Banks));
}

void Cart::InsertCart(const char* filepath)
{
	FILE * IN;
	IN = fopen(filepath, "rb");
	fread(&bus->mem[0], 1, 16 * 1024, IN); // The first 16kb. This is fixed and never gonna change
	mbc = bus->mem[0x0147];

	if (mbc == 0x0)
	{
		fread(&bus->mem[0x4000], 1, 16 * 1024, IN);
		fclose(IN);
	}
	else
	{
		switch (mbc)
		{
			case 0x1 ... 0x3: m = MBC1; break;
			case 0x5 ... 0x6: m = MBC2; break;
			case 0xF ... 0x13: m = MBC3; break;
		}
		fread(Rom_Banks, 1, 16 * 1024 * 128, IN);
		fclose(IN);
		memcpy(&bus->mem[0x4000], &Rom_Banks[0], 16 * 1024);
	}
}

void Cart::ConnectBus(Bus* b) { bus = b; }

void Cart::HandleChange(uint16_t addr, uint8_t data)
{
	if (mbc == 0x0)
	{
		return;
	}
	else
	{
		switch (m) {
			case MBC1: 
				{
					if (addr >= 0x6000 && addr <= 0x7FFF)
					{
						if (TestBit(data, 0))
							Use16_8 = false;
						else
							Use16_8 = true;
					}
					else if (addr >= 0x2000 && addr <= 0x3FFF)
					{
						int prev = CurrentRomBank;
						CurrentRomBank &= 0xE0;
						data &= 0x1F;
						CurrentRomBank |= data;
						if (CurrentRomBank == 0) CurrentRomBank++;
						if (prev != CurrentRomBank)
							memcpy(&bus->mem[0x4000], &Rom_Banks[(CurrentRomBank - 1) * 16 * 1024], 16 * 1024);
					}
					else if (addr >= 0x4000 && addr <= 0x5FFF)
					{
						if (!Use16_8)
						{
							int prev = CurrentRamBank; data &= 0x3;
							CurrentRamBank = data;
							if (prev != CurrentRamBank)
							{
								memcpy(&Ram_Banks[(prev - 1) * 8 * 1024], &bus->mem[0xA000], 8 * 1024);
								memcpy(&bus->mem[0xA000], &Ram_Banks[(CurrentRamBank - 1) * 8 * 1024], 8 * 1024);
							}
						}
						else 
						{
							int prev = CurrentRomBank;
							CurrentRomBank &= 0x1F;
							data &= 0xE0;
							CurrentRomBank |= data;
							if (CurrentRomBank == 0) CurrentRomBank++;
							if (prev != CurrentRomBank)
								memcpy(&bus->mem[0x4000], &Rom_Banks[(CurrentRomBank - 1) * 16 * 1024], 16 * 1024);
						}
					}
					else if (addr <= 0x00FF)
					{
						data &= 0xF;
						if (data == 0xA)
							EnableRamWrite = true;
						else
							EnableRamWrite = false;
					}
				} break;
			case MBC2:
				{
					if (TestBit(addr, 8))
					{
						int prev = CurrentRomBank;
						CurrentRomBank &= 0xF0;
						data &= 0x0F;
						CurrentRomBank |= data;
						if (prev != CurrentRomBank)
							memcpy(&bus->mem[0x4000], &Rom_Banks[(CurrentRomBank - 1) * 16 * 1024], 16 * 1024);
					}
					else
					{
						data &= 0xF;
						if (data == 0xA)
							EnableRamWrite = true;
						else 
							EnableRamWrite = false;
					}
				} break;
			case MBC3:
				{
					if (addr >= 0x2000 && addr <= 0x3FFF)
					{
						int prev = CurrentRomBank;
						CurrentRomBank &= 0x80;
						data &= 0x7F;
						CurrentRomBank |= data;
						if (CurrentRomBank == 0) CurrentRomBank++;
						if (prev != CurrentRomBank)
							memcpy(&bus->mem[0x4000], &Rom_Banks[(CurrentRomBank - 1) * 16 * 1024], 16 * 1024);
					}
					else if (addr >= 0x4000 && addr <= 0x5FFF)
					{
							int prev = CurrentRamBank; data &= 0x3;
							CurrentRamBank = data;
							if (prev != CurrentRamBank)
							{
								memcpy(&Ram_Banks[(prev - 1) * 8 * 1024], &bus->mem[0xA000], 8 * 1024);
								memcpy(&bus->mem[0xA000], &Ram_Banks[(CurrentRamBank - 1) * 8 * 1024], 8 * 1024);
							}
					}
					else if (addr <= 0x00FF)
					{
						data &= 0xF;
						if (data == 0xA)
							EnableRamWrite = true;
						else
							EnableRamWrite = false;
					}
				} break;
		}
	}
}
