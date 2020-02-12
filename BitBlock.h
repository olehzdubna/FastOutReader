/*
 * BitBlock.h
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#ifndef BITBLOCK_H_
#define BITBLOCK_H_

#include <IntegralData.h>

class BitBlock: public IntegralData {
public:
	BitBlock(long anId);
	~BitBlock();

	static BitBlock* read(std::ifstream& inFile);

	const char* getRecord(int anIdx) const;

private:
	char* buffer;
	int   numSamples;
	int   numBytesPerSample;
};

#endif /* BITBLOCK_H_ */
