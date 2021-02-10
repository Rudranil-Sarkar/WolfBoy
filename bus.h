#ifndef _BUS_H_
#define _BUS_H_

#include <cstdint>

#include "cpu.h"
#include "ppu.h"
#include "cart.h"

class Bus {
	public:
		Bus();
		~Bus();
		void SetRenderer(void(*func)());

	public:
		void clock();
		void reset();
		void LoadRom(const char * FileName);
		uint8_t memRead(uint16_t addr);
		void memWrite(uint16_t addr, uint8_t data);
		void queueIntr(int id);
		void ChangePallete(uint32_t C0, uint32_t C1, uint32_t C2, uint32_t C3);
		bool masterIntr = false;
		enum Key {
			KEY_B,
			KEY_A,
			KEY_START,
			KEY_SELECT,
			KEY_RIGHT,
			KEY_UP,
			KEY_DOWN,
			KEY_LEFT
		};
		void KeyPressed(Key key);
		void KeyReleased(Key key);

	private:
		int t_cycle = 1024;
		int t_divider;
		void setFreqTimer();
		void updateTimers(int cycles);
		uint8_t GetJoyPad();
		union {
			struct {
				uint8_t A:			1;
				uint8_t B:			1;
				uint8_t Select: 1;
				uint8_t Start:	1;
				uint8_t Right:	1;
				uint8_t Left:		1;
				uint8_t Up:			1;
				uint8_t Down:		1;
			};
			uint8_t data;
		} JoyPadCache;
		union {
			/* '|' This means select this line and 'XXX' means to disable this line */
			struct {
				uint8_t P10: 1; // Bit 0 Right - A
				uint8_t P11: 1; // Bit 1 Left	 - B
				uint8_t P12: 1; // Bit 2 Up		 - Select
				uint8_t P13: 1; // Bit 3 Down	 - Start
				uint8_t P14: 1; // Bit 4  |    -	XXX
				uint8_t P15: 1; // Bit 5 XXX	 -   |
				uint8_t un1: 1; // Bit 6
				uint8_t un2: 1; // Bit 7
			};
			uint8_t reg;
		} joyPad_Reg;
		void updateDividerReg(int cycles);
		void CheckIntr();
		void (*renderScreen) ();
		bool getClockStatus();
		const int maxCycle = 69905; // The gameboy can exec 4194300 cycles per seconds max so 4194300 / 59.73 we get the max cycles for per frame
		uint8_t boot_rom[256] = {
			// If you want the nintendo logo at the startup add the boot rom yourself
	};
		bool have_boot_rom = false; // And set this to true

	public:
		Cpu* c;
		Ppu p;
		Cart cart;
		uint8_t	mem[64 * 1024];
		uint32_t pix[160 * 144];
};
#endif
