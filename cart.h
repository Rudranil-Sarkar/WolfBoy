#ifndef __CART_H__
#define __CART_H__

#include <cstdint>

class Bus;

class Cart {
	public:
		Cart();
		void InsertCart(const char* filepath);
		void ConnectBus(Bus * b);

		void HandleChange(uint16_t addr, uint8_t data);
		bool EnableRamWrite = false;
	private:
		enum MBCType{
			MBC1 = 1,
			MBC2 = 2,
			MBC3 = 3
		};
		MBCType m;
		Bus * bus;
		uint8_t Rom_Banks[16 * 1024 * 128];
		uint8_t Ram_Banks[8 * 1024 * 16];
		int CurrentRomBank = 0;
		int CurrentRamBank = 0;
		int mbc;
		bool Use16_8 = true;
};

#endif
