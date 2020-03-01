/*
 * BitBlockData.cpp
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#include <iostream>

#include <BitBlockData.h>
#include <DataGroup.h>

BitBlockData::BitBlockData(long anId)
 : IntegralData(anId)
 , extractor(nullptr)
{
}

BitBlockData::~BitBlockData() {
}

/*
//  Read bitblock data object from this file
//  See 'IntegralData->BitBlockData' section of online help for HPLogic
//  Fast Binary Data File Format under the File Out tool (6.3)
*/
std::shared_ptr<BitBlockData> BitBlockData::read(std::ifstream& inFile) {

	std::string line;
	long id;

	//TODO: for debug std::cout << "+++      BitBlockData:" << std::endl;

	std::getline(inFile, line);
	/* integral data id */
	id = ::atol(line.data());

	//TODO: for debug std::cout << "+++      Integral ID: " << id << std::endl;

	std::shared_ptr<BitBlockData> bitBlockData;
	if (auto bitBlockDataPtr = static_cast<BitBlockData*>(DataGroup::instance()->isObject(id).get())) {
		//TODO: for debug std::cout << "+++    already seen this LabelEntry object" << std::endl;
		return std::shared_ptr<BitBlockData>(bitBlockDataPtr);
	}

	bitBlockData = std::make_shared<BitBlockData>(id);

	/* extractor information for this label */
	bitBlockData->extractor = Extractor::read(inFile);

	/* stuckone and stuckzero */
	std::getline(inFile, line);
	/* sscanf( tmpstrp, "%s %s\n", tmpstr, tmpstr ) ; */

	/* bitblock */
	bitBlockData->bitBlock = BitBlock::read(inFile);

//    if ( infoLevel > 99 ) {
//      outputBitBlockData( id, &extractor ) ;
//    }

	/* some memory cleanup... */
//    free( extractor.mask );
	return bitBlockData;
}

void BitBlockData::extractBits(int aRecIdx, std::vector<bool>& aBitVec) {
	auto record = bitBlock->getRecord(aRecIdx);
	extractor->extractBits((const uint8_t*)record, aBitVec);
}


