#include "StdAfx.h"
#include "fastrand.h"
#include <chrono>

// This RNG implementation is based on work by Colin Green which was in turn based on the Xorshift PRNGs of George Marsaglia.
// The period of the RNG is 2^128-1. It is (reportedly) up to 8x faster than C# Random depending on the function used (NextUInt being the fastest I believe)
// My own testing shows that for generating 100000 integers, in the worst case this RNG is 25% faster than the previous C#Random implementation and over 60% faster
// than the implementation in PhatSDK and Util. Best case it is 50% and 75% faster respectively.
// I have made all functions that involve providing a random number from within a range inclusive of both the minimum and maximum values for the sake of uniformity.
// ~scruples


// Globally available instance of FastRand random number generator
FastRand FastRNG = FastRand();

FastRand::FastRand()
{
	// If given no seed, initialise using chrono::system_clock
	Init(std::chrono::system_clock::now().time_since_epoch().count());
}

// Initialise the generator using a given seed. Allows a change of seed to be made to a current generator
FastRand::FastRand(int seed)
{
	Init(seed);
}

// Initialise the generator internal values. Only 1 of the 4 values needs to be changed for the seed to impact the generator. y,z,w are set to internal constants
void FastRand::Init(int seed)
{
	x = (unsigned int)seed;
	y = Y;
	z = Z;
	w = W;
}

// Generates a random int between 0 and INT_MAX (inclusive of INT_MAX)
int FastRand::Next()
{
	unsigned int t = (x ^ (x << 11));
	x = y; y = z; z = w;
	return (int)(0x7FFFFFFF & (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8))));
}

// Generates a random int between 0 and max (inclusive of max)
int FastRand::Next(int max)
{
		if (max<0)
			throw std::invalid_argument("Argument must be greater than 0.");

		unsigned int t = (x ^ (x << 11));
		x = y; y = z; z = w;
		return (int)((UNIT_INT*(int)(0x7FFFFFFF & (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)))))*(max+1));
}

// Generates a random int between min and max (inclusive of max). Min can be negative.
int FastRand::Next(int min, int max)
{
	if (min>max)
		throw std::invalid_argument("Min argument is larger than max argument.");

	unsigned int t = (x ^ (x << 11));
	x = y; y = z; z = w;

	int range = max - min + 1;
	//explicit casting used to maximize performance
	return min + (int)((UNIT_INT*(double)(int)(0x7FFFFFFF & (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)))))*(double)range);
}

// Generates a random double between 0.0 and 1.0 (inclusive of 1.0)
double FastRand::NextDouble()
{
	unsigned int t = (x ^ (x << 11));
	x = y; y = z; z = w;
	//explicit casting used to maximize performance
	return (UNIT_INT*(int)(0x7FFFFFFF & (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)))));
}

// Generates a random double between 0.0 and max (inclusive of max)
double FastRand::NextDouble(double max)
{
	return (NextDouble()*max);
}

// Generates a random double between min and max (inclusive of max)
double FastRand::NextDouble(double min, double max)
{
	if (min>max)
		throw std::invalid_argument("Min argument is larger than max argument.");
	double range = max - min;
	return min + (NextDouble()*range);
}
 
// Generates a random unsigned int between 0 and UINT_MAX (inclusive of UINT_MAX)
unsigned int FastRand::NextUInt()
{
	unsigned int t = (x ^ (x << 11));
	x = y; y = z; z = w;
	return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
}

// Generates a random unsigned int between min and max (inclusive of max)
unsigned int FastRand::NextUInt(unsigned int min, unsigned int max)
{
	if (min>max)
		throw std::invalid_argument("Min argument is larger than max argument.");
	unsigned int range = max - min + 1;

	unsigned int t = (x ^ (x << 11));
	x = y; y = z; z = w;
	return min + (((w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)))/UINT_MAX)*range);
}
