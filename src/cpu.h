#ifndef _CPU_H_
#define _CPU_H_

#include <cstdint>

class Bus;

class Cpu
{
public:
	Cpu(uint16_t addr);

private:
	union Register
	{
		uint16_t reg;
		struct
		{
			uint8_t lo;
			uint8_t hi;
		};
	};

	enum FLAG
	{
		unused1 = (1 << 0),
		unused2 = (1 << 1),
		unused3 = (1 << 2),
		unused4 = (1 << 3),
		Carry		= (1 << 4),
		Half		= (1 << 5),
		Nega		= (1 << 6),
		Zero		= (1 << 7)
	};

	Register AF; Register BC;
	Register DE; Register HL;

	uint16_t PC;
	Register SP;

	Bus *bus = nullptr;

private:
	uint16_t make_16_bit(uint8_t lo, uint8_t hi);
	void setFlag(FLAG f, bool clear = false);
	bool getFlag(FLAG f);
	void set_16_bit(uint8_t &, uint8_t &, uint16_t data);
	uint8_t ReadMem(uint16_t addr);
	void writeMem(uint16_t addr, uint8_t data);

	/* All Opcode fuctions */
	void	ExecuteExtendedOpcode();
	void	CPU_8BIT_LOAD(uint8_t& reg);
	void	CPU_16BIT_LOAD(uint16_t& reg);
	void	CPU_REG_LOAD(uint8_t& reg, uint8_t load, int cycles);
	void	CPU_REG_LOAD_ROM(uint8_t& reg, uint16_t addr);
	void	CPU_8BIT_ADD(uint8_t& reg, uint8_t toAdd, int cycles, bool useImmediate, bool addCarry);
	void	CPU_8BIT_SUB(uint8_t& reg, uint8_t toSubtract, int cycles, bool useImmediate, bool subCarry);
	void	CPU_8BIT_AND(uint8_t& reg, uint8_t toAnd, int cycles, bool useImmediate);
	void	CPU_8BIT_OR(uint8_t& reg, uint8_t toOr, int cycles, bool useImmediate);
	void	CPU_8BIT_XOR(uint8_t& reg, uint8_t toXOr, int cycles, bool useImmediate);
	void	CPU_8BIT_COMPARE(uint8_t reg, uint8_t toSubtract, int cycles, bool useImmediate);
	void	CPU_8BIT_INC(uint8_t& reg, int cycles);
	void	CPU_8BIT_DEC(uint8_t& reg, int cycles);
	void	CPU_8BIT_MEMORY_INC( uint16_t addr, int cycles) ;
	void	CPU_8BIT_MEMORY_DEC(uint16_t addr, int cycles);
	void	CPU_RST(uint8_t n);
	void	CPU_16BIT_DEC(uint16_t& word, int cycles);
	void	CPU_16BIT_INC(uint16_t& word, int cycles);
	void	CPU_16BIT_ADD	(uint16_t& reg, uint16_t toAdd, int cycles);
	void	CPU_JUMP(bool useCondition, FLAG flag, bool condition);
	void	CPU_JUMP_IMMEDIATE(bool useCondition, FLAG flag, bool condition);
	void	CPU_CALL(bool useCondition, FLAG flag, bool condition);
	void	CPU_RETURN(bool useCondition, FLAG flag, bool condition); 
	void	CPU_SWAP_NIBBLES(uint8_t& reg);
	void	CPU_SWAP_NIB_MEM(uint16_t addr);
	void	CPU_SHIFT_LEFT_CARRY(uint8_t& reg);
	void	CPU_SHIFT_LEFT_CARRY_MEMORY(uint16_t addr);
	void	CPU_SHIFT_RIGHT_CARRY(uint8_t& reg, bool resetMSB);
	void	CPU_SHIFT_RIGHT_CARRY_MEMORY(uint16_t addr, bool resetMSB);
	void	CPU_RESET_BIT(uint8_t& reg, int bit);
	void	CPU_RESET_BIT_MEMORY(uint16_t addr, int bit);
	void	CPU_TEST_BIT(uint8_t reg, int bit, int cycles);
	void	CPU_SET_BIT(uint8_t& reg, int bit);
	void	CPU_SET_BIT_MEMORY(uint16_t addr, int bit);
	void	CPU_DAA();
	void	CPU_RLC(uint8_t& reg);
	void	CPU_RLC_MEMORY(uint16_t addr);
	void	CPU_RRC(uint8_t& reg);
	void	CPU_RRC_MEMORY(uint16_t addr);
	void	CPU_RL(uint8_t& reg);
	void	CPU_RL_MEMORY(uint16_t addr);
	void	CPU_RR(uint8_t& reg);
	void	CPU_RR_MEMORY(uint16_t addr);
	void	CPU_SLA(uint8_t& reg);
	void	CPU_SLA_MEMORY(uint16_t addr);
	void	CPU_SRA(uint8_t& reg);
	void	CPU_SRA_MEMORY(uint16_t addr) ;
	void	CPU_SRL(uint8_t& reg);
	void	CPU_SRL_MEMORY(uint16_t addr);

	void push_stack(uint16_t addr);
	uint16_t pop_stack();
public:
	void reset();
	int cycles;
	void intr(uint8_t id);
	void ConnectBus(Bus * b);
	int ExecNext();
	bool PendingIntEnable = false;
	bool PendingIntDisable = false;
	bool halt;
};
#endif
