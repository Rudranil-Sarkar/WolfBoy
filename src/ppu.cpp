#include "ppu.h"
#include "bus.h"
#include "usefullfuncs.h"
#include <bits/stdint-uintn.h>
#include <cstring>

Ppu::Ppu()
{
	t_cycle = 0;
}

void Ppu::connectBus(Bus *b)
{
	bus = b;
}

void Ppu::SetLCDStatus()
{
	LCDStatus.reg = bus->memRead(0xFF41);
	LCDControl.reg = bus->memRead(0xFF40);
	if (!LCDControl.OnOFF)
	{
		// This Block is important!! We need to set LCD Mode to v_blank
		bus->mem[0xFF44] = 0;
		LCDStatus.ModeSelect = 1;
		bus->memWrite(0xFF41, LCDStatus.reg);
		return;
	}
	uint8_t currentScanLine = bus->memRead(0xFF44);
	uint8_t currentMode = LCDStatus.ModeSelect;
	bool reqIntr = false;

	if (currentScanLine >= 144)
	{
		// VBlank mode
		LCDStatus.ModeSelect = 1;
		reqIntr = (LCDStatus.ModeV_Blank > 0) ? true: false;
	}

	else
	{
		if (t_cycle <= 201 && t_cycle <= 207)
		{
			// Mode 0
			LCDStatus.ModeSelect = 0;
			reqIntr = (LCDStatus.ModeH_Blank > 0) ? true : false;
		}
		else if (t_cycle <= 204 + 80)
		{
			LCDStatus.ModeSelect = 2;
			reqIntr = (LCDStatus.ModeSearchOAM_RAM > 0) ? true : false;
		}
		else if (t_cycle <= 204 + 80 + 172)
		{
			LCDStatus.ModeSelect = 3;
			if (bus->memRead(0xFF44) == bus->memRead(0xFF45))
			{
				LCDStatus.CoincidenceFlag = 1;
				if (LCDStatus.LYCEQLYCoincidence)
					reqIntr = true;
			}
			else
				LCDStatus.CoincidenceFlag = 0;
		}
	}

	if (reqIntr && currentMode != LCDStatus.ModeSelect)
		// New Mdoe Entered and the corrosponding intterupt flag is enabled so we need to trigger a intterupt
		bus->queueIntr(1);

	bus->memWrite(0xFF41, LCDStatus.reg);
}

Ppu::Color Ppu::GetColor(int ColorId, uint16_t addr)
{
	Color res;
	uint8_t data;
	pallette.data = bus->memRead(addr);
	
	switch (ColorId) {
		case 0: data = pallette.first; break;
		case 1: data = pallette.second; break;
		case 2: data = pallette.third; break;
		case 3: data = pallette.fourth; break;
	}
	switch (data) {
		case 0: res = Color_White; break;
		case 1: res = Color_Light_Gray; break;
		case 2: res = Color_Dark_Gray; break;
		case 3: res = Color_Black; break;
	}
	return res;
}

void Ppu::RenderTiles()
{
	LCDControl.reg = bus->memRead(0xFF40);
	uint16_t TileData = 0;
	uint16_t BackgroundMemory = 0;
	bool UseSig = false;

	uint8_t ScrollY = bus->memRead(0xFF42);
	uint8_t ScrollX = bus->memRead(0xFF43);
	uint8_t WindowY = bus->memRead(0xFF4A);
	uint8_t WindowX = bus->memRead(0xFF4B) - 7; // Why minus 7 This is how it is
	bool usingWindow = false;
	uint8_t CurrentScan = bus->memRead(0xFF44);

	if (LCDControl.WindowDisplay)
	{
		if (WindowY <= bus->memRead(0xFF44))
			usingWindow = true;
	}

	if (LCDControl.BGandWindowTileDataSelect)
		TileData = 0x8000;
	else
	{
		TileData = 0x8800;
		UseSig = true;
	}

	// using windowBG so which memory to use
	if (usingWindow)
	{
		if (LCDControl.WindowTileMapDisplaySelect)
			BackgroundMemory = 0x9C00;
		else
			BackgroundMemory = 0x9800;
	}
	else
	{
		// Not using window but rather background so we need to choose what region of memory to read the data from
		if (LCDControl.BGTileMapDisplaySelect)
			BackgroundMemory = 0x9C00;
		else
			BackgroundMemory = 0x9800;
	}

	uint8_t PosY = 0; // This var is used for to calculate exact Y position out of the 32 vertical tiles the scanline is currently drawing

	if (usingWindow)
		PosY = CurrentScan - WindowY;
	else
		PosY = ScrollY + CurrentScan;

	uint16_t TileRow = (((uint8_t)(PosY / 8)) * 32); // The Exact Y position;

	for (int i = 0; i < 160; i++)
	{
		uint8_t PosX = i + ScrollX;
		if (usingWindow)
		{
			if (i >= WindowX)
				PosX = i - WindowX;
		}

		uint8_t TileCol = (PosX / 8);
		int16_t TileNum = 0;

		uint16_t TileAddr = BackgroundMemory + TileRow + TileCol;
		if (UseSig)
			TileNum = (int8_t)bus->memRead(TileAddr);
		else
			TileNum = (uint8_t)bus->memRead(TileAddr);
		
		uint16_t TileLocation = TileData;
		if (UseSig)
			TileLocation += (((TileNum + 128) * 16));
		else
			TileLocation +=  ((TileNum * 16));

		uint8_t line = (PosY % 8); line *= 2;
		uint8_t data1 = bus->memRead(TileLocation + line);
		uint8_t data2 = bus->memRead(TileLocation + line + 1);

		int ColorShift = PosX % 8;
		// The bit 0 in this var represent the 7th bit in the data var bit 1 represent the 6th bit and so on so we need to reverse that
		ColorShift -= 7;
		ColorShift *= -1;

		// Now we need to combine data2 and data1 to get the colorId
		int ColorId = 0;
		ColorId = BitGetVal(data2, ColorShift);
		ColorId <<= 1;
		ColorId |= BitGetVal(data1, ColorShift);

		Color col = GetColor(ColorId, 0xFF47);

		if ((i > 159) || (CurrentScan > 143))
			continue;

		switch (col) {
			case Color_White: bus->pix[i + CurrentScan * 160] = RealColor.R_WHITE; break;
			case Color_Light_Gray: bus->pix[i + CurrentScan * 160] = RealColor.R_LIGHT_GREY; break;
			case Color_Dark_Gray: bus->pix[i + CurrentScan * 160] = RealColor.R_DARK_GREY; break;
			case Color_Black: bus->pix[i + CurrentScan * 160] = RealColor.R_BLACK; break;
		}
		tempBuffer[i + CurrentScan * 160] = ColorId;
	}
}

void Ppu::RenderSprite()
{
	LCDControl.reg = bus->memRead(0xFF40);
	bool Use8x16 = false;

	if (LCDControl.OBJSize)
		Use8x16 = true;

	for (int i = 0; i < 40; i++)
	{
		uint8_t index = i * 4;
		uint8_t PosY = bus->memRead(0xFE00 + index) - 16;
		uint8_t PosX = bus->memRead(0xFE00 + index + 1) - 8;

		uint8_t TileLoc = bus->memRead(0xFE00 + index + 2);
		uint8_t Attrib = bus->memRead(0xFE00 + index + 3);

		bool FlipY = TestBit(Attrib, 6);
		bool FlipX = TestBit(Attrib, 5);

		int CurrentScan = bus->memRead(0xFF44);
		int SizeY = 8;
		if (Use8x16)
			SizeY = 16;
		
		if ((CurrentScan >= PosY) && (CurrentScan < (PosY + SizeY)))
		{
			int line = CurrentScan - PosY;

			if (FlipY)
			{
				line -= SizeY;
				line *= -1;
			}

			line *= 2;
			uint8_t data1 = bus->memRead( (0x8000 + (TileLoc * 16)) + line ) ;
			uint8_t data2 = bus->memRead( (0x8000 + (TileLoc * 16)) + line+1 ) ;

			for (int tp = 7; tp >= 0; tp--)
			{
				int ColorShift = tp;
				if (FlipX)
				{
					ColorShift -= 7;
					ColorShift *= -1;
				}

				int ColorId = BitGetVal(data2, ColorShift);
				ColorId <<= 1;
				ColorId |= BitGetVal(data1, ColorShift);

				uint16_t ColorAddr = TestBit(Attrib, 4) ? 0xFF49 : 0xFF48;

				Color col = GetColor(ColorId, ColorAddr);

				uint32_t temp;
				switch (col) {
					case Color_White: temp = RealColor.R_WHITE; break;
					case Color_Light_Gray: temp = RealColor.R_LIGHT_GREY; break;
					case Color_Dark_Gray: temp = RealColor.R_DARK_GREY; break;
					case Color_Black: temp = RealColor.R_BLACK; break;
				}
				int XPix = 0 - tp;
				XPix += 7;
				int pixel = PosX + XPix;

				if (CurrentScan > 143 || pixel > 159)
					continue;

				bool ObjBehindBG = ((Attrib >> 7) & 0x1) ? true : false; 
				if (ColorId == 0)
					continue;
				if (TestBit(Attrib, 7) && tempBuffer[pixel + CurrentScan * 160] != 0)
					continue;
				bus->pix[pixel + CurrentScan * 160] = temp;
			}
		}
	}
}

void Ppu::DrawScanLine()
{
	if (LCDControl.BGDisplay)
		RenderTiles(); // And oop's from the last commit

	if (LCDControl.OBJDisplayEnable)
		RenderSprite();
}
void Ppu::clock(int cycles)
{
	SetLCDStatus();
	LCDControl.reg = bus->memRead(0xFF40);

	if (LCDControl.OnOFF)
		t_cycle += cycles;
	else 
		return;

	if (t_cycle >= 456)
	{
		t_cycle = 0;
		bus->mem[0xFF44]++;
		uint8_t scanline = bus->memRead(0xFF44);

		if (scanline < 144)
			DrawScanLine();
		else if (scanline == 144)
			bus->queueIntr(0);
		else if (scanline > 153)
			bus->mem[0xFF44] = 0;
	}
}
