/*
 * AxisX.h
 *
 *  Created on: Jul 6, 2019
 *      Author: oleh
 */

#ifndef AXISX_H_
#define AXISX_H_

#include <iostream>
#include <fstream>
#include <vector>

#include <SharedObject.h>

class AxisX : public SharedObject {
public:
	AxisX()
	: SharedObject(0) {}
	virtual ~AxisX() {}

	static AxisX* read(std::ifstream& inFile);
	virtual int64_t getTime(int aRecIdx)
	{return 0;}

	virtual int getTrig()
	{return 0;}
	virtual int getSamples()
	{return 0;}

private:
	static AxisX* readAbscissaData();
	static AxisX* readPeriodic2();
};

#endif /* AXISX_H_ */
