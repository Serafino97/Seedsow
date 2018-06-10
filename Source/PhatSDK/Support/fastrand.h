#pragma once 

class FastRand {
private:
	const double UNIT_INT = 1.0 / (double)INT_MAX;
	const double UNIT_UINT = 1.0 / (double)UINT_MAX;
	const unsigned int Y = 842502087, Z = 3579807591, W = 273326509;
	unsigned int w, x, y, z;
public:
	FastRand();
	FastRand(int seed);
	void Init(int seed);
	int Next();
	int Next(int max);
	int Next(int min, int max);
	double NextDouble();
	double NextDouble(double max);
	double NextDouble(double min, double max);
	unsigned int NextUInt();
	unsigned int NextUInt(unsigned int min, unsigned int max);

};

extern FastRand FastRNG;