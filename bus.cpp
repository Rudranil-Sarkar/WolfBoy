#include "bus.h"
#include <cstring>
#include <cstdio>
#include "usefullfuncs.h"

Bus::Bus() {
	t_cycle = 1024;
	t_divider = 0;
	uint16_t start_addr;
	if (have_boot_rom)
		start_addr = 0x0;
	else
		start_addr = 0x0100;

	c = new Cpu(start_addr);

	c->ConnectBus(this);
	p.connectBus(this);
	cart.ConnectBus(this);
	memset(&mem, 0, 64 * 1024);
	mem[0xFF00] = 0xFF;
	joyPad_Reg.reg = 0x3F;
	JoyPadCache.data = 0xFF;
}

void Bus::SetRenderer(void (*func)())
{
	renderScreen = func;
}

void Bus::LoadRom(const char * FileName)
{
	cart.InsertCart(FileName);
	if (!have_boot_rom)
		mem[0xFF50] = 1;
}

void Bus::ChangePallete(uint32_t C0, uint32_t C1, uint32_t C2, uint32_t C3)
{
	p.RealColor.R_WHITE = C0;			p.RealColor.R_LIGHT_GREY = C1;
	p.RealColor.R_DARK_GREY = C2; p.RealColor.R_BLACK = C3;
}

uint8_t Bus::memRead(uint16_t addr)
{
	if (addr <= 0x00FF && mem[0xFF50] == 0)
		return boot_rom[addr];
	if (addr == 0xFF00)
	{
		return GetJoyPad();
	}
	return mem[addr];
}

void Bus::memWrite(uint16_t addr, uint8_t data)
{
	if (addr < 0x8000)
		// Need to handle rom/ram banking
		cart.HandleChange(addr, data);

	else if ((addr >= 0xE000) && (addr <= 0xFDFF))
	{
		// Here we need to handle the work ram mirroring
		mem[addr] = data;
		mem[addr - 0x2000] = data; // Reflect the change made here to the address range 0xC000 to 0xDDFF
	}

	else if (addr >= 0xA000 && addr <= 0xBFFF)
	{
		if (cart.EnableRamWrite)
		{
			mem[addr] = data;
		}
	}
	
	else if ((addr >= 0xFEA0) && (addr < 0xFE00))
	{
		// Unusable area
	}

	else if (addr == 0xFF07)
	{
		uint8_t before = memRead(0xFF07) & 0x3;
		mem[0xFF07] = data;
		data = memRead(0xFF07) & 0x3;
		
		if (data != before)
		{
			setFreqTimer();
		}
	}

	else if (addr == 0xFF04)
		mem[0xFF04] = 0;

	else if (addr == 0xFF44)
		mem[0xFF44] = 0;

	else if (addr == 0xFF46)
	{
		// Here we handle the DMA (Dynamic memory Access) transfer.
		// First we need the source address of the data
		// The data provided by the write is 8bit/1 Byte but a address is 16bit/2 Bytes so we need to shift it by 8 bits
		uint16_t SourceAddr = data << 8;
		for (int i = 0; i < 0xA0; i++) // The OAM data is 0xA0 bytes long
			memWrite(0xFE00 + i, memRead(SourceAddr + i));
	}

	else
		mem[addr] = data; // No restrictions or special rule just write
}

bool Bus::getClockStatus()
{
	return TestBit(memRead(0xFF07), 2) ? true : false;
}

void Bus::setFreqTimer()
{
	uint8_t f = memRead(0xFF07) & 0x3;

	switch (f) {
		case 0: t_cycle = 1024; break;
		case 1: t_cycle = 16;		break;
		case 2: t_cycle = 64;		break;
		case 3: t_cycle = 256;	break;
	}
}

void Bus::updateTimers(int cycles)
{
	if (getClockStatus())
	{
		t_cycle -= cycles;

		if (t_cycle <= 0)
		{
			setFreqTimer();

			if (memRead(0xFF05) == 255) // The time is about to overflow
			{
				memWrite(0xFF05, memRead(0xFF06));
				queueIntr(2);
			}
			else
			{
				memWrite(0xFF05, memRead(0xFF05) + 1);
			}
		}
	}
}

void Bus::updateDividerReg(int cycles)
{
	t_divider += cycles;
	if (t_divider >= 255) // The time to inc the reg
	{
		t_divider = 0;
		mem[0xFF04]++;
	}
}

void Bus::queueIntr(int id)
{
	uint8_t req = memRead(0xFF0F);
	req = BitSet(req, id);
	mem[0xFF0F] = req;
}

void Bus::CheckIntr()
{
	if (masterIntr == true)
	{
		uint8_t req = memRead(0xFF0F);
		uint8_t enb = memRead(0xFFFF);

		if (req > 0)
		{
			for (int i = 0; i < 5; i++)
			{
				if (TestBit(req, i))
				{
					if (TestBit(enb, i))
						c->intr(i);
				}
			}
		}
	}
}

void Bus::KeyPressed(Key key)
{
	uint8_t CurrState = JoyPadCache.data;
	switch(key)
	{
		case KEY_B: JoyPadCache.B = 0; break;
		case KEY_A: JoyPadCache.A = 0; break;
		case KEY_START: JoyPadCache.Start = 0; break;
		case KEY_SELECT: JoyPadCache.Select = 0; break;
		case KEY_RIGHT: JoyPadCache.Right = 0; break;
		case KEY_UP: JoyPadCache.Up = 0; break;
		case KEY_DOWN: JoyPadCache.Down = 0; break;
		case KEY_LEFT: JoyPadCache.Left = 0; break;
	}
	if (CurrState != JoyPadCache.data && mem[0xFF00] != JoyPadCache.data)
		queueIntr(4);
}

void Bus::KeyReleased(Key key)
{
	switch(key)
	{
		case KEY_B: JoyPadCache.B = 1; break;
		case KEY_A: JoyPadCache.A = 1; break;
		case KEY_START: JoyPadCache.Start = 1; break;
		case KEY_SELECT: JoyPadCache.Select = 1; break;
		case KEY_RIGHT: JoyPadCache.Right = 1; break;
		case KEY_UP: JoyPadCache.Up = 1; break;
		case KEY_DOWN: JoyPadCache.Down = 1; break;
		case KEY_LEFT: JoyPadCache.Left = 1; break;
	}
}

uint8_t Bus::GetJoyPad()
{
	joyPad_Reg.reg = mem[0xFF00];
	uint8_t res = JoyPadCache.data;
	if (!joyPad_Reg.P14 && joyPad_Reg.P15)
	{
		// P14 is Selected So We are Insterested in Directions
		res >>= 4; // The Directions are stored in the upper 4 bits of this var;
		joyPad_Reg.reg &= 0xF0;
		joyPad_Reg.reg |= res;
		return joyPad_Reg.reg;
	}
	else 
	{
		// P15 is selected so we are Insterested in buttons;
		res &= 0x0F; // The buttons are stored in the lower 4 bits of this var
		joyPad_Reg.reg &= 0xF0;
		joyPad_Reg.reg |= res;
		return joyPad_Reg.reg;
	}
}

void Bus::clock()
{
	int CyclesThisUpdate = 0;
	while (CyclesThisUpdate < maxCycle) {
		int cycles = c->ExecNext();	
		CyclesThisUpdate += cycles;
		updateDividerReg(cycles);
		updateTimers(cycles);
		p.clock(cycles);
		CheckIntr();
	}
	renderScreen();
}

Bus::~Bus() {delete c;}