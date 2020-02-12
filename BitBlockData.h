/*
 * BitBlockData.h
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#ifndef BITBLOCKDATA_H_
#define BITBLOCKDATA_H_

#include <Extractor.h>
#include <BitBlock.h>
#include <IntegralData.h>

class BitBlockData: public IntegralData {
public:
	BitBlockData(long anId);
	virtual ~BitBlockData();

	static BitBlockData* read(std::ifstream& inFile);
	void extractBits(int aRecIdx, std::vector<bool>& aBitVec);

private:
	Extractor* extractor;
	BitBlock* bitBlock;
};

#endif /* BITBLOCKDATA_H_ */
