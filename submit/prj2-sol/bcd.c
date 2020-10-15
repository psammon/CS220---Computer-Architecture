#include "bcd.h"
#include <math.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
	return ((bcd>>(4*(digitIndex-1)))&0xf);
}

Bcd
set_bcd_digit(Bcd bcd, int digitIndex, Binary digit)
{
	bcd &= (~(0x00f << ((digitIndex-1)*4)));
	bcd |= (digit << ((digitIndex-1)*4));
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
	Binary num = 0;
	Binary temp = bcd;
	int bcdDigits = 0;
	while(temp != 0)
	{
		temp /= 16;
		bcdDigits++;
	}
	for(int i = bcdDigits; i >= 1; i--)
	{
		Binary digit = get_bcd_digit(bcd, i);
		if(error != NULL && digit > 9){
			*error = BAD_VALUE_ERR;
		}
		num *= 10;
		num += digit;

	}
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
 // printf("bcd: %s\n", s);

  int bcdDigits = strlen(s);
  if(error != NULL && bcdDigits > MAX_BCD_DIGITS)
	  *error = OVERFLOW_ERR;

  Bcd bcd = 0;
  int digitIndex = 1;
 // while(*s != '\0')
  for(int i = 0; i < bcdDigits; i++)
  {
	if(error != NULL && !(isdigit(s[i]))){
		*p = &s[i];
		*error = BAD_VALUE_ERR;
		break;
	}
	long d = (long)s[i] - '0';
	bcd = set_bcd_digit(bcd, digitIndex, d);
	digitIndex++;
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

  Binary temp = bcd;
  int bcdDigits = 0;
  while(temp != 0)
  {
	temp /= 16;
	bcdDigits++;
  }

  for(int i = 1; i >= bcdDigits; i++)
  {
	if(error != NULL && (get_bcd_digit(bcd, i) > 9)){
		*error = BAD_VALUE_ERR;
	}
  }
  snprintf(buf, bufSize, "%x", (int)bcd);

  return bcdDigits;
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
  Binary temp = x;
  int bcdDigits = 0;
  while(temp != 0)
  {
	temp /= 16;
	bcdDigits++;
  }
  if(error != NULL && bcdDigits > MAX_BCD_DIGITS)
	  *error = OVERFLOW_ERR;

  Bcd bcd = 0;
  int carry = 0;
  for(int i = 1; i <= bcdDigits; i++)
  {
	  int xDig = get_bcd_digit(x, i);
	  int yDig = get_bcd_digit(y, i);
	  if(error != NULL &&(xDig > 9 || yDig > 9))
			  *error = BAD_VALUE_ERR;

	  int sum = xDig + yDig + carry;
	  Binary temp = sum%10;
	  bcd = set_bcd_digit(bcd, i, temp);
	  carry = sum / 10;
  }
  if(error != NULL && bcdDigits == (MAX_BCD_DIGITS && carry > 0))
	  *error = OVERFLOW_ERR;
  
  return bcd;
}

static Bcd
bcd_multiply_digit(Bcd multiplicand, unsigned bcdDigit, BcdError *error)
{
	Binary temp = multiplicand;
  	int bcdDigits = 0;
  	while(temp != 0)
  	{
		temp /= 16;
		bcdDigits++;
 	 }


	int carry = 0;
	for(int i = 1; i <= bcdDigits; i++)
	{
		int dig = get_bcd_digit(multiplicand, i);
		if(error != NULL && dig > 9)
			  *error = BAD_VALUE_ERR;

		dig *= pow(10, i-1); 
		int product = dig * bcdDigit + carry;
		multiplicand = set_bcd_digit(multiplicand, i, product%10);
		carry = product/10;
	}

	return multiplicand;
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
	Binary temp = x;
  	int xDigits = 0;
  	while(temp != 0)
  	{
		temp /= 16;
		xDigits++;
 	}

	temp = y;
  	int yDigits = 0;
  	while(temp != 0)
  	{
		temp /= 16;
		yDigits++;
 	}

	Bcd sum = 0;
	for(int i = 1; i <= xDigits; i++)
	{
		int digY = get_bcd_digit(y, i);
		Bcd product = bcd_multiply_digit(x, digY, error);
		product *= ((i-1)*16);
		sum = bcd_add(product, sum, error);
	}

  return sum;
}
