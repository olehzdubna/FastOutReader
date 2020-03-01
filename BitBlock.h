/*
 * BitBlock.h
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#ifndef BITBLOCK_H_
#define BITBLOCK_H_

#include <IntegralData.h>
#include <memory>

class BitBlock: public IntegralData {
public:
	BitBlock(long anId);
	static std::shared_ptr<BitBlock> read(std::ifstream& inFile);
	const char* getRecord(int anIdx) const;
private:
	std::shared_ptr<char> buffer;
	int   numSamples;
	int   numBytesPerSample;
};

#endif /* BITBLOCK_H_ */
