/*
 * PagedIntegralData.h
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#ifndef PAGEDINTEGRALDATA_H_
#define PAGEDINTEGRALDATA_H_

#include <IntegralData.h>

class PagedIntegralData : public IntegralData {
public:
	PagedIntegralData(long anId);
	~PagedIntegralData();

	static std::shared_ptr<PagedIntegralData> read(std::ifstream& inFile, int numbits, int sign);
	void extractBytes(int aRecIdx, std::vector<uint8_t>& aByteVec);

private:
    uint8_t* buffer ;
    uint32_t numSamples;
    uint32_t numBytesPerSample;
};

#endif /* PAGEDINTEGRALDATA_H_ */
