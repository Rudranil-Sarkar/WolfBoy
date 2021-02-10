#ifndef __PPU_H__
#define __PPU_H__

#include <cstdint>

class Bus;

class Ppu
{
	public:
		Ppu();

	public:
		void connectBus(Bus * b);
		void clock(int cycles);
		struct {
			uint32_t R_WHITE = 0xFFFFF6d3;
			uint32_t R_LIGHT_GREY = 0xFFF9A875;
			uint32_t R_DARK_GREY = 0xFFEB6B6F;
			uint32_t R_BLACK = 0xFF7C3F58;
		} RealColor;

	private:
		Bus *bus;
		int t_cycle = 0;
		void SetLCDStatus();
		void DrawScanLine();
		void RenderTiles();
		void RenderSprite();

		enum Color {
			Color_White,
			Color_Light_Gray,
			Color_Dark_Gray,
			Color_Black
		};

		Color GetColor(int ColorId, uint16_t addr);

		union p {
			struct {
				uint8_t first:	2;
				uint8_t second: 2;
				uint8_t third:	2;
				uint8_t fourth: 2;
			};
			uint8_t data;
		} pallette;

		union lt {
			struct {
				uint8_t ModeSelect:					2; // Bit 0-1
				uint8_t CoincidenceFlag:		1; // Bit 2
				uint8_t ModeH_Blank:				1; // Bit 3
				uint8_t ModeV_Blank:				1; // Bit 4
				uint8_t ModeSearchOAM_RAM:	1; // Bit 5
				uint8_t LYCEQLYCoincidence: 1; // Bit 6
				uint8_t Unused:							1; // Bit 7
			};
			uint8_t reg;
		} LCDStatus;

		union lc {
			struct {
				uint8_t BGDisplay:									1; // Bit 0
				uint8_t OBJDisplayEnable:						1; // Bit 1
				uint8_t OBJSize:										1; // Bit 2
				uint8_t BGTileMapDisplaySelect:			1; // Bit 3
				uint8_t BGandWindowTileDataSelect:	1; // Bit 4
				uint8_t WindowDisplay:							1; // Bit 5
				uint8_t WindowTileMapDisplaySelect: 1; // Bit 6
				uint8_t OnOFF:											1; // Bit 7
			};
			uint8_t reg;
		} LCDControl;
		uint8_t  tempBuffer [160 * 144];
};

#endif
