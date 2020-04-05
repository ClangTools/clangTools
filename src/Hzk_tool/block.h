#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <cmath>
#include <string>
#include <cstring>
#include <cstdio>
#include <vector>

typedef unsigned char byte;

#ifndef _BIT_WRITE_
#define _BV(n) (0x01U << (n))
#define _LV(n) (~_BV(n))
#define bitRead(x, n) ((x) & _BV(n))
#define bitSet(x, n) ((x) |= _BV(n))
#define bitClear(x, n) ((x) &= _LV(n))
#define bitWrite(x, n, b) ((b)? bitSet((x),(n)) : bitClear((x), (n)))
#endif

static const char REVERSE[] =
{	
	0x00, 0x08, 0x04, 0x0c, 
	0x02, 0x0a, 0x06, 0x0e, 
	0x01, 0x09, 0x05, 0x0d, 
	0x03, 0x0b, 0x07, 0x0f,
};

static const unsigned char c_on[] =
{ 0xe2, 0x96, 0xa0, 0x20 };
static const unsigned char c_off[] =
{ 0xe2, 0x96, 0xa1, 0x20 };

class Block
{
public:

	Block(char *p, int length, int byte_in_row,int col_count);
	virtual ~Block();

	bool isSquare();

	void flipInRow();
	void flipInCol();
	void flipInDiag();
	void flipInByte();

	enum Direction
	{
		BIT_IN_COL_NEGA, 
		BIT_IN_COL_POSI, 
		BIT_IN_ROW_NEGA, 
		BIT_IN_ROW_POSI,
	};

	enum Rotation
	{
		R0, R90, R180, R270
	};

	void move(bool recycle = false);
	void setMoveDirection(Direction d);
	void rotate(Rotation r);
	void opposite();

	static byte flipByte(byte c);
	static std::string byteStringPure(unsigned char c);
	static std::string byteString(unsigned char c);

	std::string getVarString(int var_per_row = 8);
	std::string getPatternString();
	std::vector<std::vector<unsigned char>> getPattern();

    int _byte_in_row;
private:

	byte * const _p;
	const int _length;
	const int _row_count;
	const int _col_count;

	void reverseArrayInBit(byte *destination, byte *source, int length);

	void moveBitInColNega(bool recycle);
	void moveBitInColPosi(bool recycle);
	void moveBitInRowNega(bool recycle);
	void moveBitInRowPosi(bool recycle);

	void (Block::*_move)(bool);
};

#endif
