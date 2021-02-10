#include "./cpu.h"
#include "bus.h"
#include "usefullfuncs.h"

Cpu::Cpu(uint16_t addr)
{
	PC = addr;
	halt = false;
	PendingIntEnable = false;
	PendingIntDisable = false;
}

void Cpu::ConnectBus(Bus *b)
{
	bus = b;
}

void Cpu::reset()
{
	halt = false;
	PendingIntDisable = false;
	PendingIntEnable = false;
}

uint16_t Cpu::make_16_bit(uint8_t lo, uint8_t hi)
{
	uint16_t res = hi;
	res = (res << 8) | lo;
	return res;
}

void Cpu::setFlag(FLAG f, bool clear)
{
	if(clear)
	{
		AF.lo &= ~f;
		return; // I forgot this return statement Earlier, and Basicaly screwed up the flags. Took me 2 days to figure it out 😢
	}
	AF.lo |= f;
}

bool Cpu::getFlag(FLAG f)
{
	uint8_t temp = AF.lo;
	return ((temp & f) > 0);
}

uint8_t Cpu::ReadMem(uint16_t addr)
{
	return bus->memRead(addr);
}

void Cpu::writeMem(uint16_t addr, uint8_t data)
{
	bus->memWrite(addr, data);
}

void Cpu::push_stack(uint16_t addr)
{
	uint8_t lo = addr & 0xFF; uint8_t hi = addr >> 8;
	SP.reg--;
	writeMem(SP.reg, hi);
	SP.reg--;
	writeMem(SP.reg, lo);
}

uint16_t Cpu::pop_stack()
{
	uint8_t lo = ReadMem(SP.reg++);
	uint8_t hi = ReadMem(SP.reg++);
	return make_16_bit(lo, hi);
}

int Cpu::ExecNext()
{
	if (!halt)
	{
		uint8_t opcode = ReadMem(PC++);
		switch (opcode) {
			case 0x00: { Cpu::cycles = 4; } break;
			case 0x01: CPU_16BIT_LOAD(BC.reg); break;
			case 0x02: { writeMem(BC.reg, AF.hi); Cpu::cycles = 8; } break;
			case 0x03: CPU_16BIT_INC(BC.reg, 8); break;
			case 0x04: CPU_8BIT_INC(BC.hi,4); break;
			case 0x05: CPU_8BIT_DEC(BC.hi,4); break;
			case 0x06: CPU_8BIT_LOAD(BC.hi); break;
			case 0x07: CPU_RLC(AF.hi); break;
			case 0x08: { uint8_t lo = ReadMem(PC++); uint8_t hi = ReadMem(PC++); uint16_t nn = make_16_bit(lo, hi);
									 writeMem(nn, SP.lo); nn++; writeMem(nn, SP.hi); Cpu::cycles = 20; } break;
			case 0x09: CPU_16BIT_ADD(HL.reg,BC.reg,8); break;
			case 0x0A: CPU_REG_LOAD_ROM(AF.hi, BC.reg); break;
			case 0x0B: CPU_16BIT_DEC(BC.reg, 8); break;
			case 0x0C: CPU_8BIT_INC(BC.lo,4); break;
			case 0x0D: CPU_8BIT_DEC(BC.lo,4); break;
			case 0x0E: CPU_8BIT_LOAD(BC.lo); break;
			case 0x0F: CPU_RRC(AF.hi);	break;

			case 0x10: { PC++; Cpu::cycles = 4; } break;
			case 0x11: CPU_16BIT_LOAD(DE.reg);break;
			case 0x12: { writeMem(DE.reg, AF.hi); Cpu::cycles = 8; } break;
			case 0x13: CPU_16BIT_INC(DE.reg, 8); break;
			case 0x14: CPU_8BIT_INC(DE.hi,4); break;
			case 0x15: CPU_8BIT_DEC(DE.hi,4); break;
			case 0x16: CPU_8BIT_LOAD(DE.hi); break;
			case 0x17: CPU_RL(AF.hi); break;
			case 0x18: CPU_JUMP_IMMEDIATE(false, (FLAG)0, false); break;
			case 0x19: CPU_16BIT_ADD(HL.reg,DE.reg,8); break;
			case 0x1A: CPU_REG_LOAD_ROM(AF.hi, DE.reg); break;
			case 0x1B: CPU_16BIT_DEC(DE.reg, 8); break;
			case 0x1C: CPU_8BIT_INC(DE.lo,4); break;
			case 0x1D: CPU_8BIT_DEC(DE.lo,4); break;
			case 0x1E: CPU_8BIT_LOAD(DE.lo); break;
			case 0x1F: CPU_RR(AF.hi);	break;

			case 0x20: CPU_JUMP_IMMEDIATE(true, Zero, false);break;
			case 0x21: CPU_16BIT_LOAD(HL.reg);break;
			case 0x22: { writeMem(HL.reg, AF.hi); CPU_16BIT_INC(HL.reg, 0); Cpu::cycles = 8; } break;
			case 0x23: CPU_16BIT_INC(HL.reg, 8); break;
			case 0x24: CPU_8BIT_INC(HL.hi,4); break;
			case 0x25: CPU_8BIT_DEC(HL.hi,4); break;
			case 0x26: CPU_8BIT_LOAD(HL.hi); break;
			case 0x27: CPU_DAA(); break;
			case 0x28: CPU_JUMP_IMMEDIATE(true, Zero, true);break;
			case 0x29: CPU_16BIT_ADD(HL.reg,HL.reg,8); break;
			case 0x2A: CPU_REG_LOAD_ROM(AF.hi,HL.reg); CPU_16BIT_INC(HL.reg,0);break;
			case 0x2B: CPU_16BIT_DEC(HL.reg, 8); break;
			case 0x2C: CPU_8BIT_INC(HL.lo,4); break;
			case 0x2D: CPU_8BIT_DEC(HL.lo,4); break;
			case 0x2E: CPU_8BIT_LOAD(HL.lo); break;
			case 0x2F: { Cpu::cycles = 4; AF.hi ^= 0xFF; setFlag(Nega); setFlag(Half); } break;

			case 0x30: CPU_JUMP_IMMEDIATE(true, Carry, false); break;
			case 0x31: CPU_16BIT_LOAD(SP.reg); break;
			case 0x32: { writeMem(HL.reg, AF.hi); CPU_16BIT_DEC(HL.reg,0); Cpu::cycles = 8; } break;
			case 0x33: CPU_16BIT_INC(SP.reg, 8); break;
			case 0x34: CPU_8BIT_MEMORY_INC(HL.reg,12); break;
			case 0x35: CPU_8BIT_MEMORY_DEC(HL.reg,12); break;
			case 0x36: { Cpu::cycles = 12; uint8_t n = ReadMem(PC++); writeMem(HL.reg, n); } break;
			case 0x37: { Cpu::cycles = 4; setFlag(Carry); setFlag(Half, true); setFlag(Nega, true); } break; 
			case 0x38: CPU_JUMP_IMMEDIATE(true, Carry, true);break;
			case 0x39: CPU_16BIT_ADD(HL.reg,SP.reg,8); break;
			case 0x3A: CPU_REG_LOAD_ROM(AF.hi,HL.reg); CPU_16BIT_DEC(HL.reg,0);break;
			case 0x3B: CPU_16BIT_DEC(SP.reg, 8); break;
			case 0x3C: CPU_8BIT_INC(AF.hi,4); break;
			case 0x3D: CPU_8BIT_DEC(AF.hi,4); break;
			case 0x3E: { Cpu::cycles = 8; uint8_t n = ReadMem(PC++); AF.hi = n; } break;
			case 0x3F: { Cpu::cycles = 4; if (getFlag(Carry)) setFlag(Carry, true); else setFlag(Carry);
									 setFlag(Half, true); setFlag(Nega, true); } break;

			case 0x40: CPU_REG_LOAD(BC.hi, BC.hi, 4); break;
			case 0x41: CPU_REG_LOAD(BC.hi, BC.lo, 4); break;
			case 0x42: CPU_REG_LOAD(BC.hi, DE.hi, 4); break;
			case 0x43: CPU_REG_LOAD(BC.hi, DE.lo, 4); break;
			case 0x44: CPU_REG_LOAD(BC.hi, HL.hi, 4); break;
			case 0x45: CPU_REG_LOAD(BC.hi, HL.lo, 4); break;
			case 0x46: CPU_REG_LOAD_ROM(BC.hi, HL.reg); break;
			case 0x47: CPU_REG_LOAD(BC.hi, AF.hi, 4); break;
			case 0x48: CPU_REG_LOAD(BC.lo, BC.hi, 4); break;
			case 0x49: CPU_REG_LOAD(BC.lo, BC.lo, 4); break;
			case 0x4A: CPU_REG_LOAD(BC.lo, DE.hi, 4); break;
			case 0x4B: CPU_REG_LOAD(BC.lo, DE.lo, 4); break;
			case 0x4C: CPU_REG_LOAD(BC.lo, HL.hi, 4); break;
			case 0x4D: CPU_REG_LOAD(BC.lo, HL.lo, 4); break;
			case 0x4E: CPU_REG_LOAD_ROM(BC.lo, HL.reg); break;
			case 0x4F: CPU_REG_LOAD(BC.lo, AF.hi, 4); break;

			case 0x50: CPU_REG_LOAD(DE.hi, BC.hi, 4); break;
			case 0x51: CPU_REG_LOAD(DE.hi, BC.lo, 4); break;
			case 0x52: CPU_REG_LOAD(DE.hi, DE.hi, 4); break;
			case 0x53: CPU_REG_LOAD(DE.hi, DE.lo, 4); break;
			case 0x54: CPU_REG_LOAD(DE.hi, HL.hi, 4); break;
			case 0x55: CPU_REG_LOAD(DE.hi, HL.lo, 4); break;
			case 0x56: CPU_REG_LOAD_ROM(DE.hi, HL.reg); break;
			case 0x57: CPU_REG_LOAD(DE.hi, AF.hi, 4); break;
			case 0x58: CPU_REG_LOAD(DE.lo, BC.hi, 4); break;
			case 0x59: CPU_REG_LOAD(DE.lo, BC.lo, 4); break;
			case 0x5A: CPU_REG_LOAD(DE.lo, DE.hi, 4); break;
			case 0x5B: CPU_REG_LOAD(DE.lo, DE.lo, 4); break;
			case 0x5C: CPU_REG_LOAD(DE.lo, HL.hi, 4); break;
			case 0x5D: CPU_REG_LOAD(DE.lo, HL.lo, 4); break;
			case 0x5E: CPU_REG_LOAD_ROM(DE.lo, HL.reg); break;
			case 0x5F: CPU_REG_LOAD(DE.lo, AF.hi, 4); break;

			case 0x60: CPU_REG_LOAD(HL.hi, BC.hi, 4); break;
			case 0x61: CPU_REG_LOAD(HL.hi, BC.lo, 4); break;
			case 0x62: CPU_REG_LOAD(HL.hi, DE.hi, 4); break;
			case 0x63: CPU_REG_LOAD(HL.hi, DE.lo, 4); break;
			case 0x64: CPU_REG_LOAD(HL.hi, HL.hi, 4); break;
			case 0x65: CPU_REG_LOAD(HL.hi, HL.lo, 4); break;
			case 0x66: CPU_REG_LOAD_ROM(HL.hi, HL.reg); break;
			case 0x67: CPU_REG_LOAD(HL.hi, AF.hi, 4); break;
			case 0x68: CPU_REG_LOAD(HL.lo, BC.hi, 4); break;
			case 0x69: CPU_REG_LOAD(HL.lo, BC.lo, 4); break;
			case 0x6A: CPU_REG_LOAD(HL.lo, DE.hi, 4); break;
			case 0x6B: CPU_REG_LOAD(HL.lo, DE.lo, 4); break;
			case 0x6C: CPU_REG_LOAD(HL.lo, HL.hi, 4); break;
			case 0x6D: CPU_REG_LOAD(HL.lo, HL.lo, 4); break;
			case 0x6E: CPU_REG_LOAD_ROM(HL.lo, HL.reg); break;
			case 0x6F: CPU_REG_LOAD(HL.lo, AF.hi, 4); break;

			case 0x70: { writeMem(HL.reg, BC.hi); Cpu::cycles = 8; } break;
			case 0x71: { writeMem(HL.reg, BC.lo); Cpu::cycles = 8; } break;
			case 0x72: { writeMem(HL.reg, DE.hi); Cpu::cycles = 8; } break;
			case 0x73: { writeMem(HL.reg, DE.lo); Cpu::cycles = 8; } break;
			case 0x74: { writeMem(HL.reg, HL.hi); Cpu::cycles = 8; } break;
			case 0x75: { writeMem(HL.reg, HL.lo); Cpu::cycles = 8; } break;
			case 0x76: { Cpu::cycles = 4; halt = true; } break;
			case 0x77: { writeMem(HL.reg, AF.hi); Cpu::cycles = 8; } break;
			case 0x78: CPU_REG_LOAD(AF.hi, BC.hi, 4); break;
			case 0x79: CPU_REG_LOAD(AF.hi, BC.lo, 4); break;
			case 0x7A: CPU_REG_LOAD(AF.hi, DE.hi, 4); break;
			case 0x7B: CPU_REG_LOAD(AF.hi, DE.lo, 4); break;
			case 0x7C: CPU_REG_LOAD(AF.hi, HL.hi, 4); break;
			case 0x7D: CPU_REG_LOAD(AF.hi, HL.lo, 4); break;
			case 0x7E: CPU_REG_LOAD_ROM(AF.hi, HL.reg); break;
			case 0x7F: CPU_REG_LOAD(AF.hi, AF.hi, 4); break;

			case 0x80: CPU_8BIT_ADD(AF.hi, BC.hi, 4, false, false); break;
			case 0x81: CPU_8BIT_ADD(AF.hi, BC.lo, 4, false, false); break;
			case 0x82: CPU_8BIT_ADD(AF.hi, DE.hi, 4, false, false); break;
			case 0x83: CPU_8BIT_ADD(AF.hi, DE.lo, 4, false, false); break;
			case 0x84: CPU_8BIT_ADD(AF.hi, HL.hi, 4, false, false); break;
			case 0x85: CPU_8BIT_ADD(AF.hi, HL.lo, 4, false, false); break;
			case 0x86: CPU_8BIT_ADD(AF.hi, ReadMem(HL.reg), 8, false, false); break;
			case 0x87: CPU_8BIT_ADD(AF.hi, AF.hi, 4, false, false); break;
			case 0x88: CPU_8BIT_ADD(AF.hi, BC.hi, 4, false, true); break;
			case 0x89: CPU_8BIT_ADD(AF.hi, BC.lo, 4, false, true); break;
			case 0x8A: CPU_8BIT_ADD(AF.hi, DE.hi, 4, false, true); break;
			case 0x8B: CPU_8BIT_ADD(AF.hi, DE.lo, 4, false, true); break;
			case 0x8C: CPU_8BIT_ADD(AF.hi, HL.hi, 4, false, true); break;
			case 0x8D: CPU_8BIT_ADD(AF.hi, HL.lo, 4, false, true); break;
			case 0x8E: CPU_8BIT_ADD(AF.hi, ReadMem(HL.reg), 8, false, true); break;
			case 0x8F: CPU_8BIT_ADD(AF.hi, AF.hi, 4, false, true); break;

			case 0x90: CPU_8BIT_SUB(AF.hi, BC.hi, 4, false, false); break;
			case 0x91: CPU_8BIT_SUB(AF.hi, BC.lo, 4, false, false); break;
			case 0x92: CPU_8BIT_SUB(AF.hi, DE.hi, 4, false, false); break;
			case 0x93: CPU_8BIT_SUB(AF.hi, DE.lo, 4, false, false); break;
			case 0x94: CPU_8BIT_SUB(AF.hi, HL.hi, 4, false, false); break;
			case 0x95: CPU_8BIT_SUB(AF.hi, HL.lo, 4, false, false); break;
			case 0x96: CPU_8BIT_SUB(AF.hi, ReadMem(HL.reg), 8, false, false); break;
			case 0x97: CPU_8BIT_SUB(AF.hi, AF.hi, 4, false, false); break;
			case 0x98: CPU_8BIT_SUB(AF.hi, BC.hi, 4, false, true); break;
			case 0x99: CPU_8BIT_SUB(AF.hi, BC.lo, 4, false, true); break;
			case 0x9A: CPU_8BIT_SUB(AF.hi, DE.hi, 4, false, true); break;
			case 0x9B: CPU_8BIT_SUB(AF.hi, DE.lo, 4, false, true); break;
			case 0x9C: CPU_8BIT_SUB(AF.hi, HL.hi, 4, false, true); break;
			case 0x9D: CPU_8BIT_SUB(AF.hi, HL.lo, 4, false, true); break;
			case 0x9E: CPU_8BIT_SUB(AF.hi, ReadMem(HL.reg), 8, false, true); break;
			case 0x9F: CPU_8BIT_SUB(AF.hi, AF.hi, 4, false, true); break;

			case 0xA0: CPU_8BIT_AND(AF.hi, BC.hi, 4, false); break;
			case 0xA1: CPU_8BIT_AND(AF.hi, BC.lo, 4, false); break;
			case 0xA2: CPU_8BIT_AND(AF.hi, DE.hi, 4, false); break;
			case 0xA3: CPU_8BIT_AND(AF.hi, DE.lo, 4, false); break;
			case 0xA4: CPU_8BIT_AND(AF.hi, HL.hi, 4, false); break;
			case 0xA5: CPU_8BIT_AND(AF.hi, HL.lo, 4, false); break;
			case 0xA6: CPU_8BIT_AND(AF.hi, ReadMem(HL.reg),8, false); break;
			case 0xA7: CPU_8BIT_AND(AF.hi, AF.hi, 4, false); break;
			case 0xA8: CPU_8BIT_XOR(AF.hi, BC.hi, 4, false); break;
			case 0xA9: CPU_8BIT_XOR(AF.hi, BC.lo, 4, false); break;
			case 0xAA: CPU_8BIT_XOR(AF.hi, DE.hi, 4, false); break;
			case 0xAB: CPU_8BIT_XOR(AF.hi, DE.lo, 4, false); break;
			case 0xAC: CPU_8BIT_XOR(AF.hi, HL.hi, 4, false); break;
			case 0xAD: CPU_8BIT_XOR(AF.hi, HL.lo, 4, false); break;
			case 0xAE: CPU_8BIT_XOR(AF.hi, ReadMem(HL.reg), 8, false); break;
			case 0xAF: CPU_8BIT_XOR(AF.hi, AF.hi, 4, false); break;

			case 0xB0: CPU_8BIT_OR(AF.hi, BC.hi, 4, false); break;
			case 0xB1: CPU_8BIT_OR(AF.hi, BC.lo, 4, false); break;
			case 0xB2: CPU_8BIT_OR(AF.hi, DE.hi, 4, false); break;
			case 0xB3: CPU_8BIT_OR(AF.hi, DE.lo, 4, false); break;
			case 0xB4: CPU_8BIT_OR(AF.hi, HL.hi, 4, false); break;
			case 0xB5: CPU_8BIT_OR(AF.hi, HL.lo, 4, false); break;
			case 0xB6: CPU_8BIT_OR(AF.hi, ReadMem(HL.reg), 8, false); break;
			case 0xB7: CPU_8BIT_OR(AF.hi, AF.hi, 4, false); break;
			case 0xB8: CPU_8BIT_COMPARE(AF.hi, BC.hi, 4, false); break;
			case 0xB9: CPU_8BIT_COMPARE(AF.hi, BC.lo, 4, false); break;
			case 0xBA: CPU_8BIT_COMPARE(AF.hi, DE.hi, 4, false); break;
			case 0xBB: CPU_8BIT_COMPARE(AF.hi, DE.lo, 4, false); break;
			case 0xBC: CPU_8BIT_COMPARE(AF.hi, HL.hi, 4, false); break;
			case 0xBD: CPU_8BIT_COMPARE(AF.hi, HL.lo, 4, false); break;
			case 0xBE: CPU_8BIT_COMPARE(AF.hi, ReadMem(HL.reg), 8, false); break;
			case 0xBF: CPU_8BIT_COMPARE(AF.hi, AF.hi, 4, false); break;

			case 0xC0: CPU_RETURN(true, Zero, false); break;
			case 0xC1: { BC.reg = pop_stack(); Cpu::cycles = 12; } break;
			case 0xC2: CPU_JUMP(true, Zero, false); break;
			case 0xC3: CPU_JUMP(false, (FLAG)0, false); break;
			case 0xC4: CPU_CALL(true, Zero, false);break;
			case 0xC5: push_stack(BC.reg); Cpu::cycles = 16;break;
			case 0xC6: CPU_8BIT_ADD(AF.hi, 0,8,true,false); break;
			case 0xC7: CPU_RST(0x00); break;
			case 0xC8: CPU_RETURN(true, Zero, true); break;
			case 0xC9: CPU_RETURN(false, (FLAG)0, false); break;
			case 0xCA: CPU_JUMP(true, Zero, true); break;
			case 0xCB: ExecuteExtendedOpcode(); break;
			case 0xCC: CPU_CALL(true, Zero, true);break;
			case 0xCD: CPU_CALL(false, (FLAG)0, false); break;
			case 0xCE: CPU_8BIT_ADD(AF.hi, 0,8,true,true); break;
			case 0xCF: CPU_RST(0x08); break;

			case 0xD0: CPU_RETURN(true, Carry, false); break;
			case 0xD1: { DE.reg = pop_stack(); Cpu::cycles = 12; } break;
			case 0xD2: CPU_JUMP(true, Carry, false); break;
			case 0xD3: break;
			case 0xD4: CPU_CALL(true, Carry, false);break;
			case 0xD5: push_stack(DE.reg); Cpu::cycles = 16;break;
			case 0xD6: CPU_8BIT_SUB(AF.hi, 0,8,true,false); break;
			case 0xD7: CPU_RST(0x10); break;
			case 0xD8: CPU_RETURN(true, Carry, true); break;
			case 0xD9: { PC = pop_stack(); bus->masterIntr = true; Cpu::cycles = 8; } break;
			case 0xDA: CPU_JUMP(true, Carry, true); break;
			case 0xDB: break;
			case 0xDC: CPU_CALL(true, Carry, true); break;
			case 0xDE: CPU_8BIT_SUB(AF.hi, 0,8,true,true); break;
			case 0xDF: CPU_RST(0x18); break;

			case 0xE0: { uint8_t n = ReadMem(PC++); uint16_t address = 0xFF00 + n; writeMem(address, AF.hi); Cpu::cycles = 12; } break;
			case 0xE1: { HL.reg = pop_stack(); Cpu::cycles = 12; } break;
			case 0xE2: { writeMem((0xFF00+BC.lo), AF.hi); Cpu::cycles = 8; } break;
			case 0xE3: break;
			case 0xE4: break;
			case 0xE5: { push_stack(HL.reg); Cpu::cycles = 16; } break;
			case 0xE6: CPU_8BIT_AND(AF.hi, 0,8, true); break;
			case 0xE7: CPU_RST(0x20); break;
			case 0xE8: break;
			case 0xE9: { Cpu::cycles = 4; PC = HL.reg; } break;
			case 0xEA: { Cpu::cycles = 16; uint8_t lo = ReadMem(PC++); uint8_t hi = ReadMem(PC++); uint16_t nn = make_16_bit(lo, hi); writeMem(nn, AF.hi); } break;
			case 0xEB: break;
			case 0xEC: break;
			case 0xED: break;
			case 0xEE: CPU_8BIT_XOR(AF.hi, 0,8, true); break;
			case 0xEF: CPU_RST(0x28); break;

			case 0xF0: { uint8_t n = ReadMem(PC++); uint16_t address = 0xFF00 + n; AF.hi = ReadMem(address); Cpu::cycles = 12; } break;
			case 0xF1: AF.reg = pop_stack(); AF.reg &= 0xFFF0; Cpu::cycles = 12;break;
			case 0xF2: CPU_REG_LOAD_ROM(AF.hi, (0xFF00+BC.lo)); break;
			case 0xF3: { PendingIntDisable = true; Cpu::cycles = 4; } break;
			case 0xF4: break;
			case 0xF5: { push_stack(AF.reg); Cpu::cycles = 16; } break;
			case 0xF6: CPU_8BIT_OR(AF.hi, 0,8, true); break;
			case 0xF7: CPU_RST(0x30); break;
			case 0xF8: { int8_t n = ReadMem(PC++); setFlag(Zero, true); setFlag(Nega, true); uint16_t value = (SP.reg + n) & 0xFFFF; HL.reg = value; unsigned int v = SP.reg + n;\
									 if((int)n > 0xFFFF) setFlag(Carry); else setFlag(Carry, true); if((SP.reg & 0xF) + (n & 0xF) > 0xF) setFlag(Half);\
									 else setFlag(Half, true); } break;

			case 0xF9: { SP.reg = HL.reg; Cpu::cycles = 8; } break;
			case 0xFA: { Cpu::cycles = 16; uint8_t lo = ReadMem(PC++); uint8_t hi = ReadMem(PC++); uint16_t nn = make_16_bit(lo, hi); uint8_t n = ReadMem(nn); AF.hi = n; } break; 
			case 0xFB: { PendingIntEnable = true; Cpu::cycles = 4; } break;
			case 0xFC: break;
			case 0xFD: break;
			case 0xFE: CPU_8BIT_COMPARE(AF.hi, 0,8, true); break;
			case 0xFF: CPU_RST(0x38); break;
		}
	}
	else 
		Cpu::cycles = 4;
	if (PendingIntDisable)
	{
		if (ReadMem(PC - 1) != 0xF3)
		{
			PendingIntDisable = false;
			bus->masterIntr = false;
		}
	}
	if (PendingIntEnable)
	{
		if (ReadMem(PC - 1) != 0xFB)
		{
			PendingIntEnable = false;
			bus->masterIntr = true;
		}
	}
	return Cpu::cycles;
}

void Cpu::ExecuteExtendedOpcode()
{
	uint8_t opcode = ReadMem(PC++);

	switch (opcode) {
		case 0x00: CPU_RLC(BC.hi); break;
		case 0x01: CPU_RLC(BC.lo); break;
		case 0x02: CPU_RLC(DE.hi); break;
		case 0x03: CPU_RLC(DE.lo); break;
		case 0x04: CPU_RLC(HL.hi); break;
		case 0x05: CPU_RLC(HL.lo); break;
		case 0x06: CPU_RLC_MEMORY(HL.reg); break;
		case 0x07: CPU_RLC(AF.hi); break;
		case 0x08: CPU_RRC(BC.hi); break;
		case 0x09: CPU_RRC(BC.lo); break;
		case 0x0A: CPU_RRC(DE.hi); break;
		case 0x0B: CPU_RRC(DE.lo); break;
		case 0x0C: CPU_RRC(HL.hi); break;
		case 0x0D: CPU_RRC(HL.lo); break;
		case 0x0E: CPU_RRC_MEMORY(HL.reg); break;
		case 0x0F: CPU_RRC(AF.hi); break;

		case 0x10: CPU_RL(BC.hi); break;
		case 0x11: CPU_RL(BC.lo); break;
		case 0x12: CPU_RL(DE.hi); break;
		case 0x13: CPU_RL(DE.lo); break;
		case 0x14: CPU_RL(HL.hi); break;
		case 0x15: CPU_RL(HL.lo); break;
		case 0x16: CPU_RL_MEMORY(HL.reg); break;
		case 0x17: CPU_RL(AF.hi); break;
		case 0x18: CPU_RR(BC.hi); break;
		case 0x19: CPU_RR(BC.lo); break;
		case 0x1A: CPU_RR(DE.hi); break;
		case 0x1B: CPU_RR(DE.lo); break;
		case 0x1C: CPU_RR(HL.hi); break;
		case 0x1D: CPU_RR(HL.lo); break;
		case 0x1E: CPU_RR_MEMORY(HL.reg); break;
		case 0x1F: CPU_RR(AF.hi); break;

		case 0x20: CPU_SLA(BC.hi); break;
		case 0x21: CPU_SLA(BC.lo); break;
		case 0x22: CPU_SLA(DE.hi); break;
		case 0x23: CPU_SLA(DE.lo); break;
		case 0x24: CPU_SLA(HL.hi); break;
		case 0x25: CPU_SLA(HL.lo); break;
		case 0x26: CPU_SLA_MEMORY(HL.reg); break;
		case 0x27: CPU_SLA(AF.hi); break;
		case 0x28: CPU_SRA(BC.hi); break;
		case 0x29: CPU_SRA(BC.lo); break;
		case 0x2A: CPU_SRA(DE.hi); break;
		case 0x2B: CPU_SRA(DE.lo); break;
		case 0x2C: CPU_SRA(HL.hi); break;
		case 0x2D: CPU_SRA(HL.lo); break;
		case 0x2E: CPU_SRA_MEMORY(HL.reg); break;
		case 0x2F: CPU_SRA(AF.hi); break;

		case 0x30: CPU_SWAP_NIBBLES(BC.hi); break;
		case 0x31: CPU_SWAP_NIBBLES(BC.lo); break;
		case 0x32: CPU_SWAP_NIBBLES(DE.hi); break;
		case 0x33: CPU_SWAP_NIBBLES(DE.lo); break;
		case 0x34: CPU_SWAP_NIBBLES(HL.hi); break;
		case 0x35: CPU_SWAP_NIBBLES(HL.lo); break;
		case 0x36: CPU_SWAP_NIB_MEM(HL.reg); break;
		case 0x37: CPU_SWAP_NIBBLES(AF.hi); break;
		case 0x38: CPU_SRL(BC.hi); break;
		case 0x39: CPU_SRL(BC.lo); break;
		case 0x3A: CPU_SRL(DE.hi); break;
		case 0x3B: CPU_SRL(DE.lo); break;
		case 0x3C: CPU_SRL(HL.hi); break;
		case 0x3D: CPU_SRL(HL.lo); break;
		case 0x3E: CPU_SRL_MEMORY(HL.reg); break;
		case 0x3F: CPU_SRL(AF.hi); break;

		case 0x40: CPU_TEST_BIT(BC.hi, 0 , 8); break;
		case 0x41: CPU_TEST_BIT(BC.lo, 0 , 8); break;
		case 0x42: CPU_TEST_BIT(DE.hi, 0 , 8); break;
		case 0x43: CPU_TEST_BIT(DE.lo, 0 , 8); break;
		case 0x44: CPU_TEST_BIT(HL.hi, 0 , 8); break;
		case 0x45: CPU_TEST_BIT(HL.lo, 0 , 8); break;
		case 0x46: CPU_TEST_BIT(ReadMem(HL.reg), 0 , 16); break;
		case 0x47: CPU_TEST_BIT(AF.hi, 0 , 8); break;
		case 0x48: CPU_TEST_BIT(BC.hi, 1 , 8); break;
		case 0x49: CPU_TEST_BIT(BC.lo, 1 , 8); break;
		case 0x4A: CPU_TEST_BIT(DE.hi, 1 , 8); break;
		case 0x4B: CPU_TEST_BIT(DE.lo, 1 , 8); break;
		case 0x4C: CPU_TEST_BIT(HL.hi, 1 , 8); break;
		case 0x4D: CPU_TEST_BIT(HL.lo, 1 , 8); break;
		case 0x4E: CPU_TEST_BIT(ReadMem(HL.reg), 1 , 16); break;
		case 0x4F: CPU_TEST_BIT(AF.hi, 1 , 8); break;

		case 0x50: CPU_TEST_BIT(BC.hi, 2 , 8); break;
		case 0x51: CPU_TEST_BIT(BC.lo, 2 , 8); break;
		case 0x52: CPU_TEST_BIT(DE.hi, 2 , 8); break;
		case 0x53: CPU_TEST_BIT(DE.lo, 2 , 8); break;
		case 0x54: CPU_TEST_BIT(HL.hi, 2 , 8); break;
		case 0x55: CPU_TEST_BIT(HL.lo, 2 , 8); break;
		case 0x56: CPU_TEST_BIT(ReadMem(HL.reg), 2 , 16); break;
		case 0x57: CPU_TEST_BIT(AF.hi, 2 , 8); break;
		case 0x58: CPU_TEST_BIT(BC.hi, 3 , 8); break;
		case 0x59: CPU_TEST_BIT(BC.lo, 3 , 8); break;
		case 0x5A: CPU_TEST_BIT(DE.hi, 3 , 8); break;
		case 0x5B: CPU_TEST_BIT(DE.lo, 3 , 8); break;
		case 0x5C: CPU_TEST_BIT(HL.hi, 3 , 8); break;
		case 0x5D: CPU_TEST_BIT(HL.lo, 3 , 8); break;
		case 0x5E: CPU_TEST_BIT(ReadMem(HL.reg), 3 , 16); break;
		case 0x5F: CPU_TEST_BIT(AF.hi, 3 , 8); break;

		case 0x60: CPU_TEST_BIT(BC.hi, 4 , 8); break;
		case 0x61: CPU_TEST_BIT(BC.lo, 4 , 8); break;
		case 0x62: CPU_TEST_BIT(DE.hi, 4 , 8); break;
		case 0x63: CPU_TEST_BIT(DE.lo, 4 , 8); break;
		case 0x64: CPU_TEST_BIT(HL.hi, 4 , 8); break;
		case 0x65: CPU_TEST_BIT(HL.lo, 4 , 8); break;
		case 0x66: CPU_TEST_BIT(ReadMem(HL.reg), 4 , 16); break;
		case 0x67: CPU_TEST_BIT(AF.hi, 4 , 8); break;
		case 0x68: CPU_TEST_BIT(BC.hi, 5 , 8); break;
		case 0x69: CPU_TEST_BIT(BC.lo, 5 , 8); break;
		case 0x6A: CPU_TEST_BIT(DE.hi, 5 , 8); break;
		case 0x6B: CPU_TEST_BIT(DE.lo, 5 , 8); break;
		case 0x6C: CPU_TEST_BIT(HL.hi, 5 , 8); break;
		case 0x6D: CPU_TEST_BIT(HL.lo, 5 , 8); break;
		case 0x6E: CPU_TEST_BIT(ReadMem(HL.reg), 5 , 16); break;
		case 0x6F: CPU_TEST_BIT(AF.hi, 5 , 8); break;

		case 0x70: CPU_TEST_BIT(BC.hi, 6 , 8); break;
		case 0x71: CPU_TEST_BIT(BC.lo, 6 , 8); break;
		case 0x72: CPU_TEST_BIT(DE.hi, 6 , 8); break;
		case 0x73: CPU_TEST_BIT(DE.lo, 6 , 8); break;
		case 0x74: CPU_TEST_BIT(HL.hi, 6 , 8); break;
		case 0x75: CPU_TEST_BIT(HL.lo, 6 , 8); break;
		case 0x76: CPU_TEST_BIT(ReadMem(HL.reg), 6 , 16); break;
		case 0x77: CPU_TEST_BIT(AF.hi, 6 , 8); break;
		case 0x78: CPU_TEST_BIT(BC.hi, 7 , 8); break;
		case 0x79: CPU_TEST_BIT(BC.lo, 7 , 8); break;
		case 0x7A: CPU_TEST_BIT(DE.hi, 7 , 8); break;
		case 0x7B: CPU_TEST_BIT(DE.lo, 7 , 8); break;
		case 0x7C: CPU_TEST_BIT(HL.hi, 7 , 8); break;
		case 0x7D: CPU_TEST_BIT(HL.lo, 7 , 8); break;
		case 0x7E: CPU_TEST_BIT(ReadMem(HL.reg), 7 , 16); break;
		case 0x7F: CPU_TEST_BIT(AF.hi, 7 , 8); break;

		case 0x80: CPU_RESET_BIT(BC.hi, 0); break;
		case 0x81: CPU_RESET_BIT(BC.lo, 0); break;
		case 0x82: CPU_RESET_BIT(DE.hi, 0); break;
		case 0x83: CPU_RESET_BIT(DE.lo, 0); break;
		case 0x84: CPU_RESET_BIT(HL.hi, 0); break;
		case 0x85: CPU_RESET_BIT(HL.lo, 0); break;
		case 0x86: CPU_RESET_BIT_MEMORY(HL.reg, 0); break;
		case 0x87: CPU_RESET_BIT(AF.hi, 0); break;
		case 0x88: CPU_RESET_BIT(BC.hi, 1); break;
		case 0x89: CPU_RESET_BIT(BC.lo, 1); break;
		case 0x8A: CPU_RESET_BIT(DE.hi, 1); break;
		case 0x8B: CPU_RESET_BIT(DE.lo, 1); break;
		case 0x8C: CPU_RESET_BIT(HL.hi, 1); break;
		case 0x8D: CPU_RESET_BIT(HL.lo, 1); break;
		case 0x8E: CPU_RESET_BIT_MEMORY(HL.reg, 1); break;
		case 0x8F: CPU_RESET_BIT(AF.hi, 1); break;

		case 0x90: CPU_RESET_BIT(BC.hi, 2); break;
		case 0x91: CPU_RESET_BIT(BC.lo, 2); break;
		case 0x92: CPU_RESET_BIT(DE.hi, 2); break;
		case 0x93: CPU_RESET_BIT(DE.lo, 2); break;
		case 0x94: CPU_RESET_BIT(HL.hi, 2); break;
		case 0x95: CPU_RESET_BIT(HL.lo, 2); break;
		case 0x96: CPU_RESET_BIT_MEMORY(HL.reg, 2); break;
		case 0x97: CPU_RESET_BIT(AF.hi, 2); break;
		case 0x98: CPU_RESET_BIT(BC.hi, 3); break;
		case 0x99: CPU_RESET_BIT(BC.lo, 3); break;
		case 0x9A: CPU_RESET_BIT(DE.hi, 3); break;
		case 0x9B: CPU_RESET_BIT(DE.lo, 3); break;
		case 0x9C: CPU_RESET_BIT(HL.hi, 3); break;
		case 0x9D: CPU_RESET_BIT(HL.lo, 3); break;
		case 0x9E: CPU_RESET_BIT_MEMORY(HL.reg, 3); break;
		case 0x9F: CPU_RESET_BIT(AF.hi, 3); break;

		case 0xA0: CPU_RESET_BIT(BC.hi, 4); break;
		case 0xA1: CPU_RESET_BIT(BC.lo, 4); break;
		case 0xA2: CPU_RESET_BIT(DE.hi, 4); break;
		case 0xA3: CPU_RESET_BIT(DE.lo, 4); break;
		case 0xA4: CPU_RESET_BIT(HL.hi, 4); break;
		case 0xA5: CPU_RESET_BIT(HL.lo, 4); break;
		case 0xA6: CPU_RESET_BIT_MEMORY(HL.reg, 4); break;
		case 0xA7: CPU_RESET_BIT(AF.hi, 4); break;
		case 0xA8: CPU_RESET_BIT(BC.hi, 5); break;
		case 0xA9: CPU_RESET_BIT(BC.lo, 5); break;
		case 0xAA: CPU_RESET_BIT(DE.hi, 5); break;
		case 0xAB: CPU_RESET_BIT(DE.lo, 5); break;
		case 0xAC: CPU_RESET_BIT(HL.hi, 5); break;
		case 0xAD: CPU_RESET_BIT(HL.lo, 5); break;
		case 0xAE: CPU_RESET_BIT_MEMORY(HL.reg, 5); break;
		case 0xAF: CPU_RESET_BIT(AF.hi, 5); break;

		case 0xB0: CPU_RESET_BIT(BC.hi, 6); break;
		case 0xB1: CPU_RESET_BIT(BC.lo, 6); break;
		case 0xB2: CPU_RESET_BIT(DE.hi, 6); break;
		case 0xB3: CPU_RESET_BIT(DE.lo, 6); break;
		case 0xB4: CPU_RESET_BIT(HL.hi, 6); break;
		case 0xB5: CPU_RESET_BIT(HL.lo, 6); break;
		case 0xB6: CPU_RESET_BIT_MEMORY(HL.reg, 6); break;
		case 0xB7: CPU_RESET_BIT(AF.hi, 6); break;
		case 0xB8: CPU_RESET_BIT(BC.hi, 7); break;
		case 0xB9: CPU_RESET_BIT(BC.lo, 7); break;
		case 0xBA: CPU_RESET_BIT(DE.hi, 7); break;
		case 0xBB: CPU_RESET_BIT(DE.lo, 7); break;
		case 0xBC: CPU_RESET_BIT(HL.hi, 7); break;
		case 0xBD: CPU_RESET_BIT(HL.lo, 7); break;
		case 0xBE: CPU_RESET_BIT_MEMORY(HL.reg, 7); break;
		case 0xBF: CPU_RESET_BIT(AF.hi, 7); break;

		case 0xC0: CPU_SET_BIT(BC.hi, 0); break;
		case 0xC1: CPU_SET_BIT(BC.lo, 0); break;
		case 0xC2: CPU_SET_BIT(DE.hi, 0); break;
		case 0xC3: CPU_SET_BIT(DE.lo, 0); break;
		case 0xC4: CPU_SET_BIT(HL.hi, 0); break;
		case 0xC5: CPU_SET_BIT(HL.lo, 0); break;
		case 0xC6: CPU_SET_BIT_MEMORY(HL.reg, 0); break;
		case 0xC7: CPU_SET_BIT(AF.hi, 0); break;
		case 0xC8: CPU_SET_BIT(BC.hi, 1); break;
		case 0xC9: CPU_SET_BIT(BC.lo, 1); break;
		case 0xCA: CPU_SET_BIT(DE.hi, 1); break;
		case 0xCB: CPU_SET_BIT(DE.lo, 1); break;
		case 0xCC: CPU_SET_BIT(HL.hi, 1); break;
		case 0xCD: CPU_SET_BIT(HL.lo, 1); break;
		case 0xCE: CPU_SET_BIT_MEMORY(HL.reg, 1); break;
		case 0xCF: CPU_SET_BIT(AF.hi, 1); break;

		case 0xD0: CPU_SET_BIT(BC.hi, 2); break;
		case 0xD1: CPU_SET_BIT(BC.lo, 2); break;
		case 0xD2: CPU_SET_BIT(DE.hi, 2); break;
		case 0xD3: CPU_SET_BIT(DE.lo, 2); break;
		case 0xD4: CPU_SET_BIT(HL.hi, 2); break;
		case 0xD5: CPU_SET_BIT(HL.lo, 2); break;
		case 0xD6: CPU_SET_BIT_MEMORY(HL.reg, 2); break;
		case 0xD7: CPU_SET_BIT(AF.hi, 2); break;
		case 0xD8: CPU_SET_BIT(BC.hi, 3); break;
		case 0xD9: CPU_SET_BIT(BC.lo, 3); break;
		case 0xDA: CPU_SET_BIT(DE.hi, 3); break;
		case 0xDB: CPU_SET_BIT(DE.lo, 3); break;
		case 0xDC: CPU_SET_BIT(HL.hi, 3); break;
		case 0xDD: CPU_SET_BIT(HL.lo, 3); break;
		case 0xDE: CPU_SET_BIT_MEMORY(HL.reg, 3); break;
		case 0xDF: CPU_SET_BIT(AF.hi, 3); break;

		case 0xE0: CPU_SET_BIT(BC.hi, 4); break;
		case 0xE1: CPU_SET_BIT(BC.lo, 4); break;
		case 0xE2: CPU_SET_BIT(DE.hi, 4); break;
		case 0xE3: CPU_SET_BIT(DE.lo, 4); break;
		case 0xE4: CPU_SET_BIT(HL.hi, 4); break;
		case 0xE5: CPU_SET_BIT(HL.lo, 4); break;
		case 0xE6: CPU_SET_BIT_MEMORY(HL.reg, 4); break;
		case 0xE7: CPU_SET_BIT(AF.hi, 4); break;
		case 0xE8: CPU_SET_BIT(BC.hi, 5); break;
		case 0xE9: CPU_SET_BIT(BC.lo, 5); break;
		case 0xEA: CPU_SET_BIT(DE.hi, 5); break;
		case 0xEB: CPU_SET_BIT(DE.lo, 5); break;
		case 0xEC: CPU_SET_BIT(HL.hi, 5); break;
		case 0xED: CPU_SET_BIT(HL.lo, 5); break;
		case 0xEE: CPU_SET_BIT_MEMORY(HL.reg, 5); break;
		case 0xEF: CPU_SET_BIT(AF.hi, 5); break;

		case 0xF0: CPU_SET_BIT(BC.hi, 6); break;
		case 0xF1: CPU_SET_BIT(BC.lo, 6); break;
		case 0xF2: CPU_SET_BIT(DE.hi, 6); break;
		case 0xF3: CPU_SET_BIT(DE.lo, 6); break;
		case 0xF4: CPU_SET_BIT(HL.hi, 6); break;
		case 0xF5: CPU_SET_BIT(HL.lo, 6); break;
		case 0xF6: CPU_SET_BIT_MEMORY(HL.reg, 6); break;
		case 0xF7: CPU_SET_BIT(AF.hi, 6); break;
		case 0xF8: CPU_SET_BIT(BC.hi, 7); break;
		case 0xF9: CPU_SET_BIT(BC.lo, 7); break;
		case 0xFA: CPU_SET_BIT(DE.hi, 7); break;
		case 0xFB: CPU_SET_BIT(DE.lo, 7); break;
		case 0xFC: CPU_SET_BIT(HL.hi, 7); break;
		case 0xFD: CPU_SET_BIT(HL.lo, 7); break;
		case 0xFE: CPU_SET_BIT_MEMORY(HL.reg, 7); break;
		case 0xFF: CPU_SET_BIT(AF.hi, 7); break;
	}
}

void Cpu::intr(uint8_t id)
{
	halt = false;

	bus->masterIntr = false;
	uint8_t n = ReadMem(0xFF0F);
	n = BitReset(n, id);
	bus->mem[0xFF0F] =  n;
	push_stack(PC);
	
	switch (id) {
		case 0: PC = 0x40; break;
		case 1: PC = 0x48; break;
		case 2: PC = 0x50; break;
		case 4: PC = 0x60; break;
	}

}
