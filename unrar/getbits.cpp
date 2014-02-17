//	11/1/05 gmilow - Modified 

#include "stdafx.h"
#include "rar.hpp"

BitInput::BitInput()
{
  InBuf=new byte[MAX_SIZE];
}


BitInput::~BitInput()
{
  delete[] InBuf;
}


void BitInput::faddbits(int Bits)
{
  addbits(Bits);
}


unsigned int BitInput::fgetbits()
{
  return(getbits());
}
