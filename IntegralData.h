/*
 * IntegralData.h
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#ifndef INTEGRALDATA_H_
#define INTEGRALDATA_H_

#include <fstream>
#include <vector>
#include <SharedObject.h>

class IntegralData : public SharedObject {
public:
	IntegralData(long anId)
	: SharedObject(anId)
	{}
	virtual ~IntegralData()
	{}

	static std::shared_ptr<IntegralData> read(std::ifstream& inFile, int inegralDataperline);

	virtual void extractBits(int aRecIdx, std::vector<bool>& aBitVec) {}
	virtual void extractBytes(int aRecIdx, std::vector<uint8_t>& aByteVec) {}

};



#endif /* INTEGRALDATA_H_ */
