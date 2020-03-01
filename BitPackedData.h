/*
 * BitPackedData.h
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#ifndef BITPACKEDDATA_H_
#define BITPACKEDDATA_H_

#include <IntegralData.h>

class BitPackedData: public IntegralData {
public:
	BitPackedData(long anId);
	~BitPackedData();
	static std::shared_ptr<BitPackedData> read(std::ifstream& inFile);
private:
	int start;
	int width;
	int inverted;
	int bytesPerLine;
	int integralTypesPerLine;
	std::shared_ptr<IntegralData> integralData2;
};

#endif /* BITPACKEDDATA_H_ */
