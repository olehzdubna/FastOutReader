/*
 * IntegralArray.cpp
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#include <iostream>

#include <DataGroup.h>
#include <IntegralArray.h>

IntegralArray::IntegralArray(long anId)
 : IntegralData(anId){
}

IntegralArray::~IntegralArray() {
}

/*
//  Read an Integral Array from a file
//  See 'IntegralData->IntegralArray' section of online help for HPLogic
//  Fast Binary Data File Format under the File Out tool (6.1)
//
*/
IntegralArray* IntegralArray::read(std::ifstream& inFile,int numbits, bool sign, int ipl) {

	std::string line;
	long id ;
	int len ;

	char *buffer ;
	int count, maxsize;
	int column = 0;
	int size = numbits / 8 ;

	std::cout << "+++      IntegralArray with " << numbits << " bits and sign " << sign << std::endl;

	std::getline(inFile, line);
	/* integral data id */
	id = ::atol(line.data());

	std::cout << "+++      Integral ID: " << id << std::endl;

	IntegralArray* integralArray = nullptr;
	if ((integralArray = static_cast<IntegralArray*>(DataGroup::instance()->isObject(id))) ) {
		std::cout << "+++    already seen this LabelEntry object" << std::endl;
		return integralArray;
	}

	integralArray = new IntegralArray(id);

	std::getline(inFile, line);
	  /* length */
	len = ::atol(line.data());

	std::cout << "+++        Length: " << len << "  Size: " << size*len << std::endl;

	/* stuckone and stuckzero */
	std::getline(inFile, line);
	/* sscanf( tmpstr, "%s %s\n", x, y ) ; */

	/* raw bytes */
	buffer = new char[size * len];

	inFile.read(buffer, size * len) ;

	len = len / ipl ;    /* adjust len/size to account for integralsPerLine */
	size = size * ipl ;  /* used by BitPackedData objects...                */

	DataGroup::instance()->addObject(integralArray);

//    if ( infoLevel > 99 ) {
//        outputIntegralData( id ) ;
//    }

	return integralArray;
}
