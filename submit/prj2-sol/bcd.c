#include "bcd.h"
#include <math.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/** Return BCD encoding of binary (which has normal binary representation).
 *
 *  Examples: binary_to_bcd(0xc) => 0x12;
 *            binary_to_bcd(0xff) => 0x255
 *
 *  If error is not NULL, sets *error to OVERFLOW_ERR if binary is too
 *  big for the Bcd type, otherwise *error is unchanged.
 *
 *
 */
Binary
get_bcd_digit(Bcd bcd, int digitIndex)
{
//	unsigned long temp = 0xf <<(4*(digitIndex-1));
//	temp &= bcd;
//	return temp >>(4*(digitIndex-1));
	return ((bcd>>(BCD_BITS*(digitIndex-1)))&0xf);
}

Bcd
set_bcd_digit(Bcd bcd, int digitIndex, Binary digit)
{
	bcd &= (~(0xf << ((digitIndex-1)*BCD_BITS)));
	bcd |= (digit << ((digitIndex-1)*BCD_BITS));
	return bcd;
}
Bcd
binary_to_bcd(Binary value, BcdError *error)
{
  int bcdDigits = 0;
  Binary temp = value;
  while(temp != 0)
  {
	  temp /= 10;
	  bcdDigits++;
  }
  if(error != NULL)
  {
	  if(bcdDigits > MAX_BCD_DIGITS)
		  *error = OVERFLOW_ERR;
  }
  Bcd bcd = 0;
  for(int i = 0; i < bcdDigits; i++)
  {
	bcd |= ((value%10) << (i*BCD_BITS));
	value /= 10;
  }
  return bcd;
}

/** Return binary encoding of BCD value bcd.
 *
 *  Examples: bcd_to_binary(0x12) => 0xc;
 *            bcd_to_binary(0x255) => 0xff
 *
 *  If error is not NULL, sets *error to BAD_VALUE_ERR if bcd contains
 *  a bad BCD digit.
 *  Cannot overflow since Binary can represent larger values than Bcd
 */
Binary
bcd_to_binary(Bcd bcd, BcdError *error)
{
	Binary num = 0x0;
	int nybbles = (((int)log2(bcd)+1)/4);
	printf("nybbles: %d\n", nybbles);
	for(int i = nybbles; i >= 1; i--)
	{
		Binary digit = get_bcd_digit(bcd, i);
		printf("get_digit: %d\n", digit);
		if(digit >= 10)
			*error = BAD_VALUE_ERR;
		num *= 10;
		num += digit;
//		printf("%x\n", num);

	}
	printf("result: %d\n", num);
  	return num;
}

/** Return BCD encoding of decimal number corresponding to string s.
 *  Behavior undefined on overflow or if s contains a non-digit
 *  character.  Sets *p to point to first non-digit char in s.
 *  Rougly equivalent to strtol().
 *
 *  If error is not NULL, sets *error to OVERFLOW_ERR if binary is too
 *  big for the Bcd type, otherwise *error is unchanged.
 */
Bcd
str_to_bcd(const char *s, const char **p, BcdError *error)
{
  int nybbles = 0;
  while(s[nybbles] != '\0') nybbles++;
  if(error != NULL)
  {
	  if(nybbles > MAX_BCD_DIGITS)
		  *error = OVERFLOW_ERR;
  }
  *p = s+1;

  Bcd bcd = 0;
  for(int i = 0; *s != '\0'; s++)
  {
	long temp = (long)(s - '0');
	bcd |= ((temp & 0xf) << (i*BCD_BITS));
	i++;
  }
  return bcd;
}

/** Convert bcd to a NUL-terminated string in buf[] without any
 *  non-significant leading zeros.  Never write more than bufSize
 *  characters into buf.  The return value is the number of characters
 *  written (excluding the NUL character used to terminate strings).
 *
 *  If error is not NULL, sets *error to BAD_VALUE_ERR is bcd contains
 *  a BCD digit which is greater than 9, OVERFLOW_ERR if bufSize bytes
 *  is less than BCD_BUF_SIZE, otherwise *error is unchanged.
 */
int
bcd_to_str(Bcd bcd, char buf[], size_t bufSize, BcdError *error)
{
  if(bufSize >= BCD_BUF_SIZE)
	  *error = OVERFLOW_ERR;
  int nybbles = (((int)log2(bcd)+1)/4);

  for(int i = nybbles; i >= 1; i--)
  {
	if(get_bcd_digit(bcd, i) >= 10)
		*error = BAD_VALUE_ERR;
  }
  snprintf(buf, bufSize, "%x", (int)bcd);

  return nybbles;
}

/** Return the BCD representation of the sum of BCD int's x and y.
 *
 *  If error is not NULL, sets *error to to BAD_VALUE_ERR is x or y
 *  contains a BCD digit which is greater than 9, OVERFLOW_ERR on
 *  overflow, otherwise *error is unchanged.
 */
Bcd
bcd_add(Bcd x, Bcd y, BcdError *error)
{
  int nybbles = (((int)log2(x)+1)/4);
  Bcd bcd = 0;
  int carry = 0;
  for(int i = 1; i <= nybbles; i++)
  {
	  int xDig = get_bcd_digit(x, i);
	  int yDig = get_bcd_digit(y, i);
	  if(error != NULL)
		  if(xDig > 9 || yDig > 9){
			  *error = BAD_VALUE_ERR;
		  }
	  int sum = xDig + yDig + carry;
	  Binary temp = sum%10;
	  bcd = set_bcd_digit(bcd, i, temp);
	  carry = sum / 10;
  }
  if(nybbles > MAX_BCD_DIGITS || (nybbles == MAX_BCD_DIGITS && carry > 0))
	  *error = OVERFLOW_ERR;
  
  return bcd;
}

static Bcd
bcd_multipy_digit(Bcd multiplicand, unsigned bcdDigit, BcdError *error)
{
	int nybbles = (((int)log2(bcd)+1)/4);

	int carry = 0;
	for(int i = 1; i <= nybbles; i++)
	{
		int dig = get_bcd_digit(x, i);
		if(error != NULL)
		  if(dig > 9){
			  *error = BAD_VALUE_ERR;
		  }
		dig *= pow(10, i-1); 
		int product = dig * bcdDigit + carry;
		bcd = set_bcd_digit(bcd, i, product%10)
		carry = product/10;
	}
	return bcd;
}

/** Return the BCD representation of the product of BCD int's x and y.
 *
 * If error is not NULL, sets *error to to BAD_VALUE_ERR is x or y
 * contains a BCD digit which is greater than 9, OVERFLOW_ERR on
 * overflow, otherwise *error is unchanged.
 */
Bcd
bcd_multiply(Bcd x, Bcd y, BcdError *error)
{
	int nybbles = (((int)log2(y)+1)/4);
	Bcd sum = 0;

	for(int i = 1; i <= nybbles; i++)
	{
		int digY = get_bcd_digit(y, i);
		Bcd product = bcd_multiply_digit(x, digY, *error);
		product = product << ((i-1)*4);
		sum = bcd_add(product, sum);
	}

  return sum;
}
