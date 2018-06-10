#include "StdAfx.h"
#include "PhatSDK.h"
#include "RandomRange.h"
#include "fastrand.h"
#include "easylogging++.h"

void testRandomValueGenerator()
{
	int testRolls = 10000;
	std::map<int, int> valueDistribution;
	for (int i = 0; i < testRolls; i++)
	{
		//int test = (int)floor(getRandomDouble(0.0, 1.0, eRandomFormula::favorMid, 2, 0) * 10);
		int test = getRandomNumber(1, 10, eRandomFormula::favorSpecificValue, 3, 0, 4);
		//int test = getRandomNumber(0, 10, eRandomFormula::favorMid, 2);
		if (valueDistribution.find(test) != valueDistribution.end())
			valueDistribution[test]++;
		else
			valueDistribution.emplace(test, 1);
	}

	for each(auto entry in valueDistribution)
	{
		SERVER_ERROR << "RNG Distribution - Value:" << entry.first << " Amount:" << entry.second << " Percent:" << (entry.second * 100.0 / testRolls);
	}
}

int getRandomNumberWithFavoredValue(int minInclusive, int maxInclusive, double favorValue, double favorStrength)
{
	if (favorStrength < DBL_EPSILON)
	{
		// favourStrength is zero so were just return a uniform random number
		int dReturn = floor((rand() / (long double)RAND_MAX) * (maxInclusive - minInclusive + 1)) + minInclusive;

		return min(max(minInclusive, dReturn), maxInclusive);
	}


	// Okay, so here goes nothing!
	// 1/a^abs(x-m) is a function that grows exponentially towards m either side of it on the x-axis
	// where a = 1 + favorStrength^2/(maxInclusive-minInclusive)
	// we want to randomly sample a random number in the area of this function to get a biased value
	// between n and m (n<m), the area is a^(n-m)/log(a)
	// between m and n (n>m), the area is (2-a^m-n)/log(a) where m is the favorValue

	int numValues = (maxInclusive - minInclusive) + 1;
	double a = 1 + pow(favorStrength, 2) / numValues;
	double logA = log(a);

	// we assume that each point has a width of 1 (+-0.5)
	// so we pick a random number between the area left of (minInclusive-0.5) so:
	double minArea = pow(a, (minInclusive - 0.5) - favorValue);
	// and the area left of (maxInclusive+0.5)
	double totArea = (2 - pow(a, favorValue - (maxInclusive + 0.5))) - minArea;
	// here goes...
	double r = FastRNG.NextDouble() * totArea + minArea;

	int iReturn = 0;

	// the area at n=m is a^(n-m)/log(a) = a^0/log(a) = 1/log(a)
	if (r < 1)
	{
		// Now we just have to reaarange the equation for n
		// so n such that a^(n-m)/log(a) = r our randomly picked area
		iReturn = round(log(r) / logA + favorValue);
	}
	else
	{
		// similarly,
		// n such that (2 - a^(m-n))/log(a) = r
		iReturn = round(favorValue - log(2 - r) / logA);
	}

	// the equations chosen are simply what was here before, but calculated with a bit more elegance...
	return min(max(minInclusive, iReturn), maxInclusive);
}

int getRandomNumberExclusive(int maxExclusive, eRandomFormula formula, double favorStrength, double favorModifier, double favorSpecificValue)
{
	return getRandomNumber(0, maxExclusive - 1, formula, favorSpecificValue, favorStrength, favorModifier);
}

int getRandomNumber(int minInclusive, int maxInclusive, eRandomFormula formula, double favorStrength, double favorModifier, double favorSpecificValue)
{
	int numbersAmount = maxInclusive - minInclusive;

	switch (formula)
	{
	case eRandomFormula::favorSpecificValue:
	{
		favorSpecificValue = favorSpecificValue + (numbersAmount * favorModifier);
		favorSpecificValue = min(favorSpecificValue, maxInclusive);
		favorSpecificValue = max(favorSpecificValue, minInclusive);
		return getRandomNumberWithFavoredValue(minInclusive, maxInclusive, favorSpecificValue, favorStrength);
	}
	case eRandomFormula::favorLow:
	{
		favorSpecificValue = minInclusive + (numbersAmount * favorModifier);
		favorSpecificValue = min(favorSpecificValue, maxInclusive);
		favorSpecificValue = max(favorSpecificValue, minInclusive);
		return getRandomNumberWithFavoredValue(minInclusive, maxInclusive, favorSpecificValue, favorStrength);
	}
	case eRandomFormula::favorMid:
	{
		int midValue = (int)round(((double)(maxInclusive - minInclusive) / 2)) + minInclusive;
		favorSpecificValue = midValue + (numbersAmount * favorModifier);
		favorSpecificValue = min(favorSpecificValue, maxInclusive);
		favorSpecificValue = max(favorSpecificValue, minInclusive);
		return getRandomNumberWithFavoredValue(minInclusive, maxInclusive, favorSpecificValue, favorStrength);
	}
	case eRandomFormula::favorHigh:
	{
		favorSpecificValue = maxInclusive - (numbersAmount * favorModifier);
		favorSpecificValue = min(favorSpecificValue, maxInclusive);
		favorSpecificValue = max(favorSpecificValue, minInclusive);
		return getRandomNumberWithFavoredValue(minInclusive, maxInclusive, favorSpecificValue, favorStrength);
	}
	default:
	case eRandomFormula::equalDistribution:
	{
		return FastRNG.Next(minInclusive, maxInclusive);
	}
	}
}

double getRandomDouble(double maxInclusive, eRandomFormula formula, double favorStrength, double favorModifier, double favorSpecificValue)
{
	return getRandomDouble(0, maxInclusive, formula, favorStrength, favorModifier, favorSpecificValue);
}

double getRandomDouble(double minInclusive, double maxInclusive, eRandomFormula formula, double favorStrength, double favorModifier, double favorSpecificValue)
{
	if (formula == eRandomFormula::equalDistribution) { return FastRNG.NextDouble(minInclusive, maxInclusive); }

	double decimalPlaces = 1000;
	int minInt = (int)round(minInclusive * decimalPlaces);
	int maxInt = (int)round(maxInclusive * decimalPlaces);

	int favorSpecificValueInt = (int)round(favorSpecificValue * decimalPlaces);

	int randomInt = getRandomNumber(minInt, maxInt, formula, favorStrength, favorModifier, favorSpecificValueInt);
	double returnValue = randomInt / decimalPlaces;

	returnValue = min(returnValue, maxInclusive);
	returnValue = max(returnValue, minInclusive);

	return returnValue;
}
