#include <stdio.h>
#include <assert.h>

int main(int argc, const * argv[]) {

	unsigned int i = 1512 //10111101000
	printf("%d\n", getbit


}
static inline unsigned
get_bit(HammingWord word, int bitIndex)
{
  assert(bitIndex > 0);
  return ((1 << (bitIndex)) & word) != 0;
}

/** Return word with bit at bitIndex in word set to bitValue. */
static inline HammingWord
set_bit(HammingWord word, int bitIndex, unsigned bitValue)
{
  assert(bitIndex > 0);
  assert(bitValue == 0 || bitValue == 1);
  if(bitValue 
  word |= (bitValue << (bitIndex-1));
  return word;
}

/** Given a Hamming code with nParityBits, return 2**nParityBits - 1,
 *  i.e. the max # of bits in an encoded word (# data bits + # parity
 *  bits).
 */
static inline unsigned
get_n_encoded_bits(unsigned nParityBits)
{
  return ((1 << nParityBits) -1);
}

/** Return non-zero if bitIndex indexes a bit which will be used for a
 *  Hamming parity bit; i.e. the bit representation of bitIndex
 *  contains only a single 1.
 */
static inline int
is_parity_position(int bitIndex)
{
  assert(bitIndex > 0);
  if(bitIndex & (bitIndex-1) == 0)
	  return 1;
  return 0;
}

/** Return the parity over the data bits in word specified by the
 *  parity bit bitIndex.  The word contains a total of nBits bits.
 *  Equivalently, return parity over all data bits whose bit-index has
 *  a 1 in the same position as in bitIndex.
 */
static int
compute_parity(HammingWord word, int bitIndex, unsigned nBits)
{
  assert(bitIndex > 0);
  int parity = 0;
 
  for(int i = 1; i <= nBits; i++)
  {
	  if((i & bitIndex) == bitIndex)
	  {
		if(get_bit(word, i) == 1)
		 	 parity++;
	  }
  }	

  return parity%2;
}

/** Encode data using nParityBits Hamming code parity bits.
 *  Assumes data is within range of values which can be encoded using
 *  nParityBits.
 */
HammingWord
hamming_encode(HammingWord data, unsigned nParityBits)
{
  HammingWord encoded = 0;
  int nBits = get_n_encoded_bits(nParityBits), j = 1;

  while(j <= nBits - nParityBits)
  {
	  for(int i  = 1; i <= nBits; i++)
	  {
		  if(!is_parity_position(i))
		  {
			 int temp = get_bit(data, j);
			 encoded = set_bit(encoded, i, temp);
			 j++;
		  }
	  }

 }
  for(int i = 1; i <= nBits; i++)
  {
	  if(is_parity_position(i))
	  {
		  int temp = compute_parity(encoded, i, nBits);
		  encoded = set_bit(encoded, i, temp);
	  }
  }
  return encoded;
}

/** Decode encoded using nParityBits Hamming code parity bits.
 *  Set *hasError if an error was corrected.
 *  Assumes that data is within range of values which can be decoded
