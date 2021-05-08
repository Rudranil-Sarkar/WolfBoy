#include "./cpu.h"
#include "./bus.h"
#include "./usefullfuncs.h"

void Cpu::CPU_8BIT_LOAD( uint8_t& reg )
{
	Cpu::cycles = 8;
	uint8_t n = ReadMem(PC++);
	reg = n;
}

void Cpu::CPU_16BIT_LOAD( uint16_t& reg )
{
	Cpu::cycles = 12;
	uint8_t lo = ReadMem(PC++); uint8_t hi = ReadMem(PC++);
	uint16_t n = make_16_bit(lo, hi);
	reg = n;
}

void Cpu::CPU_REG_LOAD(uint8_t& reg, uint8_t load, int cycles)
{
	Cpu::cycles = cycles;
	reg = load;
}

void Cpu::CPU_REG_LOAD_ROM(uint8_t& reg, uint16_t address)
{
	Cpu::cycles = 8;
	reg = ReadMem(address);
}

void Cpu::CPU_16BIT_DEC(uint16_t& word, int cycles)
{
	Cpu::cycles = cycles;
	word--;
}

void Cpu::CPU_16BIT_INC(uint16_t& word, int cycles)
{
	Cpu::cycles = cycles;
	word++;
}

void Cpu::CPU_8BIT_ADD(uint8_t& reg, uint8_t toAdd, int cycles, bool useImmediate, bool addCarry)
{
	Cpu::cycles = cycles;
	uint8_t before = reg;
	uint8_t adding = 0;

	if (useImmediate)
	{
		uint8_t n = ReadMem(PC++);
		adding = n;
	}
	else
		adding = toAdd;

	if (addCarry)
	{
		if (getFlag(Carry))
			adding++;
	}

	reg+=adding;

	AF.lo = 0;

	if (reg == 0)
		setFlag(Zero);

	uint16_t htest = (before & 0xF);
	htest += (adding & 0xF);

	if (htest > 0xF)
		setFlag(Half);

	if ((before + adding) > 0xFF)
		setFlag(Carry);
}

void Cpu::CPU_8BIT_SUB(uint8_t& reg, uint8_t subtracting, int cycles, bool useImmediate, bool subCarry)
{
	Cpu::cycles = cycles;
	uint8_t before = reg;
	uint8_t toSubtract = 0;

	if (useImmediate)
	{
		uint8_t n = ReadMem(PC++);
		toSubtract = n;
	}
	else
		toSubtract = subtracting;

	if (subCarry)
		if (getFlag(Carry))
			toSubtract++;

	reg -= toSubtract;

	AF.lo = 0;

	if (reg == 0)
		setFlag(Zero);

	setFlag(Nega);

	if (before < toSubtract)
		setFlag(Carry);

	int16_t htest = (before & 0xF);
	htest -= (toSubtract & 0xF);

	if (htest < 0)
		setFlag(Half);
}

void Cpu::CPU_8BIT_AND(uint8_t& reg, uint8_t toAnd, int cycles, bool useImmediate)
{
	Cpu::cycles = cycles;
	uint8_t accAnd = 0;

	if (useImmediate)
	{
		uint8_t n = ReadMem(PC++);
		accAnd = n;
	}
	else
		accAnd = toAnd;

	reg &= accAnd;

	AF.lo = 0;

	if (reg == 0)
		setFlag(Zero);

	setFlag(Half);
}

void Cpu::CPU_8BIT_OR(uint8_t& reg, uint8_t toOr, int cycles, bool useImmediate)
{
	Cpu::cycles = cycles;
	uint8_t myor = 0;

	if (useImmediate)
	{
		uint8_t n = ReadMem(PC++);
		myor = n;
	}
	else
	{
		myor = toOr;
	}

	reg |= myor;

	AF.lo = 0;

	if (reg == 0)
		setFlag(Zero);
}

void Cpu::CPU_8BIT_XOR(uint8_t& reg, uint8_t toXOr, int cycles, bool useImmediate)
{
	Cpu::cycles = cycles;
	uint8_t myxor = 0;

	if (useImmediate)
	{
		uint8_t n = ReadMem(PC++);
		myxor = n;
	}
	else
	{
		myxor = toXOr;
	}

	reg ^= myxor;

	AF.lo = 0;

	if (reg == 0)
		setFlag(Zero);
}

void Cpu::CPU_8BIT_COMPARE(uint8_t reg, uint8_t subtracting, int cycles, bool useImmediate)
{
	Cpu::cycles = cycles;
	uint8_t before = reg;
	uint8_t toSubtract = 0;

	if (useImmediate)
	{
		uint8_t n = ReadMem(PC++);
		toSubtract = n;
	}
	else
	{
		toSubtract = subtracting;
	}

	reg -= toSubtract;

	AF.lo = 0;

	if (reg == 0)
		setFlag(Zero);

	setFlag(Nega);

	if (before < toSubtract)
		setFlag(Carry);


	int16_t htest = before & 0xF;
	htest -= (toSubtract & 0xF);

	if (htest < 0)
		setFlag(Half);

}

void Cpu::CPU_8BIT_INC(uint8_t& reg, int cycles)
{
	Cpu::cycles = cycles;

	uint8_t before = reg;

	reg++;

	if (reg == 0)
		setFlag(Zero);
	else
		setFlag(Zero, true);

	setFlag(Nega, true);

	if ((before & 0xF) == 0xF)
		setFlag(Half);
	else
		setFlag(Half, true);
}

void Cpu::CPU_8BIT_MEMORY_INC(uint16_t address, int cycles)
{
	Cpu::cycles = cycles;

	uint8_t before = ReadMem( address );
	writeMem(address, (before+1));
	uint8_t now =  before+1;

	if (now == 0)
		setFlag(Zero);
	else
		setFlag(Zero, true);

	setFlag(Nega);

	if ((before & 0xF) == 0xF)
		setFlag(Half);
	else
		setFlag(Half, true);
}

void Cpu::CPU_8BIT_DEC(uint8_t& reg, int cycles)
{
	Cpu::cycles = cycles;
	uint8_t before = reg;

	reg--;

	if (reg == 0)
		setFlag(Zero);
	else
		setFlag(Zero, true);

	setFlag(Nega);

	if ((before & 0x0F) == 0)
		setFlag(Half);
	else
		setFlag(Half);
}

void Cpu::CPU_8BIT_MEMORY_DEC(uint16_t address, int cycles)
{
	Cpu::cycles = cycles;
	uint8_t before = ReadMem(address);
	writeMem(address, (before-1));
	uint8_t now = before-1;

	if (now == 0)
		setFlag(Zero);
	else
		setFlag(Zero, true);

	setFlag(Nega);

	if ((before & 0x0F) == 0)
		setFlag(Half);
	else
		setFlag(Half, true);
}

void Cpu::CPU_16BIT_ADD(uint16_t& reg, uint16_t toAdd, int cycles)
{
	Cpu::cycles = cycles;
	uint16_t before = reg;

	reg += toAdd;

	setFlag(Nega, true);

	if ((before + toAdd) > 0xFFFF)
		setFlag(Carry);
	else
		setFlag(Carry, true);


	if (( (before & 0xFF00) & 0xF) + ((toAdd >> 8) & 0xF))
		setFlag(Half);
	else
		setFlag(Half, true);
}

void Cpu::CPU_JUMP(bool useCondition, FLAG flag, bool condition)
{
	Cpu::cycles = 12;

	uint8_t lo = ReadMem(PC++); uint8_t hi = ReadMem(PC++);
	uint16_t nn = make_16_bit(lo, hi);

	if (!useCondition)
	{
		PC = nn;
		return;
	}

	if (getFlag(flag) == condition)
	{
		PC = nn;
	}

}

void Cpu::CPU_JUMP_IMMEDIATE(bool useCondition, FLAG flag, bool condition)
{
	Cpu::cycles = 8;

	int8_t n = (int8_t)ReadMem(PC);

	if (!useCondition) // Unconditional jump basically
	{
		PC += n;
	}
	else if (getFlag(flag) == condition)
	{
		PC += n;
	}
	PC++;
}

void Cpu::CPU_CALL(bool useCondition, FLAG flag, bool condition)
{
	Cpu::cycles = 12;
	uint8_t lo = ReadMem(PC++); uint8_t hi = ReadMem(PC++);
	uint16_t nn = make_16_bit(lo, hi);

	if (!useCondition)
	{
		push_stack(PC);
		PC = nn;
		return;
	}

	if (getFlag(flag) == condition)
	{
		push_stack(PC);
		PC = nn;
	}
}

void Cpu::CPU_RETURN(bool useCondition, FLAG flag, bool condition)
{
	Cpu::cycles = 8;
	if (!useCondition)
	{
		PC = pop_stack();
		return;
	}

	if (getFlag(flag) == condition)
	{
		PC = pop_stack();
	}
}

void Cpu::CPU_SWAP_NIBBLES(uint8_t& reg)
{
	Cpu::cycles = 8;

	AF.lo = 0;

	reg = (((reg & 0xF0) >> 4) | ((reg & 0x0F) << 4));

	if (reg == 0)
		setFlag(Zero);
}

void Cpu::CPU_SWAP_NIB_MEM(uint16_t address)
{
	Cpu::cycles = 16;

	AF.lo = 0;

	uint8_t mem = ReadMem(address);
	mem = (((mem & 0xF0) >> 4) | ((mem & 0x0F) << 4));

	writeMem(address,mem);

	if (mem == 0)
		setFlag(Zero);
}

void Cpu::CPU_RST(uint8_t n)
{
	push_stack(PC);
	Cpu::cycles = 32;
	PC = n;
}

void Cpu::CPU_SHIFT_LEFT_CARRY(uint8_t& reg)
{
	Cpu::cycles = 8;
	AF.lo = 0;
	if (TestBit(reg,7))
		setFlag(Carry);

	reg = reg << 1;
	if (reg == 0)
		setFlag(Zero);
}

void Cpu::CPU_SHIFT_LEFT_CARRY_MEMORY(uint16_t address)
{
	Cpu::cycles = 16;
	uint8_t before = ReadMem(address);

	AF.lo = 0;
	if (TestBit(before,7))
		setFlag(Carry);

	before = before << 1;
	if (before == 0)
		setFlag(Zero);

	writeMem(address, before);
}

void Cpu::CPU_RESET_BIT(uint8_t& reg, int bit)
{
	reg = BitReset(reg, bit);
	Cpu::cycles = 8;
}

void Cpu::CPU_RESET_BIT_MEMORY(uint16_t address, int bit)
{
	uint8_t mem = ReadMem(address);
	mem = BitReset(mem, bit);
	writeMem(address, mem);
	Cpu::cycles = 16;
}

void Cpu::CPU_TEST_BIT(uint8_t reg, int bit, int cycles)
{
	if (TestBit(reg, bit))
		setFlag(Zero, true);
	else
		setFlag(Zero);

	setFlag(Nega, true);
	setFlag(Half, true);

	Cpu::cycles = cycles;
}

void Cpu::CPU_SET_BIT(uint8_t& reg, int bit)
{
	reg = BitSet(reg, bit);
	Cpu::cycles = 8;
}

void Cpu::CPU_SET_BIT_MEMORY(uint16_t address, int bit)
{
	uint8_t mem = ReadMem(address);
	mem = BitSet(mem, bit);
	writeMem(address, mem);
	Cpu::cycles = 16;
}

void Cpu::CPU_DAA()
{
	Cpu::cycles = 4;

	if(getFlag(Nega))
	{
		if((AF.hi &0x0F ) >0x09 || AF.lo &0x20 )
		{
			AF.hi -=0x06; 
			if((AF.hi&0xF0)==0xF0) AF.lo|=0x10; else AF.lo&=~0x10;
		}

		if((AF.hi&0xF0)>0x90 || AF.lo&0x10) AF.hi-=0x60;
	}
	else
	{
		if((AF.hi&0x0F)>9 || AF.lo&0x20)
		{
			AF.hi+=0x06; 
			if((AF.hi&0xF0)==0) AF.lo|=0x10; else AF.lo&=~0x10;
		}

		if((AF.hi&0xF0)>0x90 || AF.lo&0x10) AF.hi+=0x60;
	}

	if(AF.hi==0) AF.lo|=0x80; else AF.lo&=~0x80;
}

void Cpu::CPU_RR(uint8_t& reg)
{
	Cpu::cycles = 8;

	bool isCarrySet = getFlag(Carry);
	bool isLSBSet = TestBit(reg, 0);

	AF.lo = 0;
	reg >>= 1;

	if (isLSBSet)
		setFlag(Carry);

	if (isCarrySet)
		reg = BitSet(reg, 7);

	if (reg == 0)
		setFlag(Zero);
}

void Cpu::CPU_RR_MEMORY(uint16_t address)
{
	Cpu::cycles = 16;

	uint8_t reg = ReadMem(address);

	bool isCarrySet = getFlag(Carry);
	bool isLSBSet = TestBit(reg, 0);

	AF.lo = 0;

	reg >>= 1;

	if (isLSBSet)
		setFlag(Carry);

	if (isCarrySet)
		reg = BitSet(reg, 7);

	if (reg == 0)
		setFlag(Zero);

	writeMem(address, reg);
}

void Cpu::CPU_RLC(uint8_t& reg)
{
	Cpu::cycles = 8;

	bool isMSBSet = TestBit(reg, 7);

	AF.lo = 0;
	reg <<= 1;

	if (isMSBSet)
	{
		setFlag(Carry);
		reg = BitSet(reg,0);
	}

	if (reg == 0)
		setFlag(Zero);

}

void Cpu::CPU_RLC_MEMORY(uint16_t address)
{
	Cpu::cycles = 16;

	uint8_t reg = ReadMem(address);

	bool isMSBSet = TestBit(reg, 7);

	AF.lo = 0;
	reg <<= 1;

	if (isMSBSet)
	{
		setFlag(Carry);
		reg = BitSet(reg,0);
	}

	if (reg == 0)
		setFlag(Zero);

	writeMem(address, reg);

}

void Cpu::CPU_RRC(uint8_t& reg)
{
	Cpu::cycles = 8;

	bool isLSBSet = TestBit(reg, 0);

	AF.lo = 0;

	reg >>= 1;

	if (isLSBSet)
	{
		setFlag(Carry);
		reg = BitSet(reg,7);
	}

	if (reg == 0)
		setFlag(Zero);
}

void Cpu::CPU_RRC_MEMORY(uint16_t address)
{
	Cpu::cycles = 16;

	uint8_t reg = ReadMem(address);

	bool isLSBSet = TestBit(reg, 0);

	AF.lo = 0;
	reg >>= 1;

	if (isLSBSet)
	{
		reg = BitSet(reg,7);
		setFlag(Carry);
	}

	if (reg == 0)
		setFlag(Zero);
	writeMem(address, reg);
}

void Cpu::CPU_SLA(uint8_t& reg)
{
	Cpu::cycles = 8;

	bool isMSBSet = TestBit(reg, 7);

	reg <<= 1;
	AF.lo = 0;

	if (isMSBSet)
		setFlag(Carry);

	if (reg == 0)
		setFlag(Zero);
}

void Cpu::CPU_SLA_MEMORY(uint16_t address)
{
	Cpu::cycles = 16;

	uint8_t reg = ReadMem(address);

	bool isMSBSet = TestBit(reg, 7);

	reg <<= 1;
	AF.lo = 0;

	if (isMSBSet)
		setFlag(Carry);

	if (reg == 0)
		setFlag(Zero);

	writeMem(address, reg);
}

void Cpu::CPU_SRA(uint8_t& reg)
{
	Cpu::cycles = 8;

	bool isLSBSet = TestBit(reg,0);
	bool isMSBSet = TestBit(reg,7);

	AF.lo = 0;
	reg >>= 1;

	if (isMSBSet)
		reg = BitSet(reg,7);
	if (isLSBSet)
		setFlag(Carry);

	if (reg == 0)
		setFlag(Zero);
}

void Cpu::CPU_SRA_MEMORY(uint16_t address)
{
	Cpu::cycles = 16;

	uint8_t reg = ReadMem(address);

	bool isLSBSet = TestBit(reg,0);
	bool isMSBSet = TestBit(reg,7);

	AF.lo = 0;
	reg >>= 1;

	if (isMSBSet)
		reg = BitSet(reg,7);
	if (isLSBSet)
		setFlag(Carry);

	if (reg == 0)
		setFlag(Zero);

	writeMem(address, reg);
}

void Cpu::CPU_SRL(uint8_t& reg)
{
	Cpu::cycles = 8;

	bool isLSBSet = TestBit(reg,0);

	AF.lo = 0;
	reg >>= 1;

	if (isLSBSet)
		setFlag(Carry);

	if (reg == 0)
		setFlag(Zero);
}

void Cpu::CPU_SRL_MEMORY(uint16_t address)
{
	Cpu::cycles = 8;

	uint8_t reg = ReadMem(address);

	bool isLSBSet = TestBit(reg,0);

	AF.lo = 0;
	reg >>= 1;

	if (isLSBSet)
		setFlag(Carry);

	if (reg == 0)
		setFlag(Zero);

	writeMem(address, reg);

}

void Cpu::CPU_RL(uint8_t& reg)
{
	Cpu::cycles = 8;

	bool isCarrySet = getFlag(Carry);
	bool isMSBSet = TestBit(reg, 7);

	AF.lo = 0;
	reg <<= 1;

	if (isMSBSet)
		setFlag(Carry);

	if (isCarrySet)
		reg = BitSet(reg, 0);

	if (reg == 0)
		setFlag(Zero);
}

void Cpu::CPU_RL_MEMORY(uint16_t address)
{
	Cpu::cycles = 16;
	uint8_t reg = ReadMem(address);

	bool isCarrySet = getFlag(Carry);
	bool isMSBSet = TestBit(reg, 7);

	AF.lo = 0;

	reg <<= 1;

	if (isMSBSet)
		setFlag(Carry);

	if (isCarrySet)
		reg = BitSet(reg, 0);

	if (reg == 0)
		setFlag(Zero);

	writeMem(address, reg);
}
