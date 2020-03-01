/*
 * BitPackedData.cpp
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#include <iostream>
#include <fstream>

#include <DataGroup.h>
#include <BitPackedData.h>

BitPackedData::BitPackedData(long anId)
 : IntegralData(anId) {
}

BitPackedData::~BitPackedData() {
}

/*
//  Read bit packed data from a file
//  See 'IntegralData->BitPackedData' section of online help for HPLogic
//  Fast Binary Data File Format under the File Out tool (6.2)
*/
std::shared_ptr<BitPackedData> BitPackedData::read(std::ifstream& inFile) {
	std::string line;

	long id = 0;
	char *buffer ;
	int numSamples, bytesPerSample ;
        (void)buffer;
        (void)numSamples;
	(void)bytesPerSample;

	//TODO: for debug std::cout << "+++      BitPackedData:" << std::endl;

	std::getline(inFile, line);

	/* integral data id */
	id = ::atol(line.data());

	//TODO: for debug std::cout << "+++      Integral ID: " << id << std::endl;

	std::shared_ptr<BitPackedData> bitPackedData;
	if (auto bitPackedDataPtr = static_cast<BitPackedData*>(DataGroup::instance()->isObject(id).get())) {
		//TODO: for debug std::cout << "+++    already seen this LabelEntry object" << std::endl;
		return std::shared_ptr<BitPackedData>(bitPackedDataPtr);
	}

	bitPackedData = std::make_shared<BitPackedData>(id);
	std::getline(inFile, line);
	::sscanf(line.data(), "%d %d %d\n", &bitPackedData->start, &bitPackedData->width, &bitPackedData->inverted ) ;

	//TODO: for debug std::cout << "+++      start=" << bitPackedData->start << "   width=" << bitPackedData->width << "    inverted=" << bitPackedData->inverted << std::endl;;

	/* stuckone and stuckzero */
	std::getline(inFile, line);
	/* sscanf( tmpstrp, "%s %s\n", tmpstr, tmpstr ) ; */

	std::getline(inFile, line);
	/* datablock */
	::sscanf(line.data(), "%d %d\n", &bitPackedData->bytesPerLine, &bitPackedData->integralTypesPerLine ) ;
	//TODO: for debug std::cout << "+++      bytesPerLine=" << bitPackedData->bytesPerLine << "   integralTypesPerLine=" << bitPackedData->integralTypesPerLine << std::endl;

//	    GlobalSuppressDataOutput = 1 ;
	bitPackedData->integralData2 = IntegralData::read(inFile, bitPackedData->integralTypesPerLine) ;
//	    GlobalSuppressDataOutput = 0 ;
	//
	//    fscanf( fp, "\n" ) ;
	//
	//    if ( infoLevel > 99 ) {
	//      /* output the data for this label */
	//      outputPackedData( id2, width, start, inverted ) ;
	//    }

   return bitPackedData;
}
