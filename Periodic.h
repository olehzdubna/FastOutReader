/*
 * Periodic.h
 *
 *  Created on: Jul 6, 2019
 *      Author: oleh
 */

#ifndef PERIODIC_H_
#define PERIODIC_H_

#include <fstream>
#include <vector>

#include <IntegralData.h>
#include <AxisX.h>

class Periodic: public AxisX {
public:
	Periodic();
	~Periodic();

	static std::shared_ptr<Periodic> read(std::ifstream& inFile);
	void extractBytes(int aRecIdx, std::vector<uint8_t>& aByteVec){}

	int getSamples()
	{return samples;}
	int getTrig()
	{return trig;}
        int getFirstTime()
	{return firstTime;}
        int getIncrementTime()
	{return incrementTime;}

        void update(int asamples, int atrig, int origin, int increment)
	{
            samples = asamples;
	    trig = atrig;
	    firstTime = origin;
            incrementTime = increment;
	}
private:
	int samples;
	int trig;
	int firstTime;
	int incrementTime;
};

#endif /* PERIODIC_H_ */
