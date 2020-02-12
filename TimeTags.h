/*
 * TimerTags.h
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#ifndef TIMETAGS_H_
#define TIMETAGS_H_

#include <iostream>

#include <IntegralData.h>
#include <AxisX.h>

class TimeTags: public AxisX {
public:
	TimeTags();
	~TimeTags();

	static TimeTags* read(std::ifstream& inFile, int pagedFlag);
	int64_t getTime(int aRecIdx);

	int getSamples()
	{return samples;}
	int getTrig()
	{return trig;}

private:
	int samples;
	int trig;
	IntegralData* integralData;
};

#endif /* TIMETAGS_H_ */
