#ifndef __EXTRA_H__
#define __EXTRA_H__

template <typename T>
bool TestBit(T data, int pos)
{
	T mask = 1 << pos;
	return (data & mask) ? true : false;
}

template <typename T>
T BitSet(T data, int pos)
{
	T mask = 1 << pos;
	data |= mask;
	return data;
}

template <typename T>
T BitReset(T data, int pos)
{
	T mask = 1 << pos;
	data &= ~mask;
	return data;
}

template <typename T>
T BitGetVal(T data, int pos)
{
	T mask = 1 << pos;
	return (data & mask) ? 1 : 0;
}

#endif
