/*
 * IntegralArray.h
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#ifndef INTEGRALARRAY_H_
#define INTEGRALARRAY_H_

#include <IntegralData.h>

class IntegralArray: public IntegralData {
public:
	IntegralArray(long anId);
	~IntegralArray();

	static std::shared_ptr<IntegralArray> read(std::ifstream& inFile, int numbits, bool sign, int ipl);
};

#endif /* INTEGRALARRAY_H_ */
