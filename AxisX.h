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

#include <memory>
#include <SharedObject.h>

class AxisX : public SharedObject {
public:
	AxisX()
	: SharedObject(0) {}
	virtual ~AxisX() {}

	static std::shared_ptr<AxisX> read(std::ifstream& inFile);
	virtual int64_t getTime(int aRecIdx)
	{return 0;}

	virtual int getTrig()
	{return samples;}
	virtual int getSamples()
	{return trig;}

private:
	static std::shared_ptr<AxisX> readAbscissaData(std::ifstream& inFile);
private:
         int samples{0};
	 int trig{0};
};

#endif /* AXISX_H_ */
