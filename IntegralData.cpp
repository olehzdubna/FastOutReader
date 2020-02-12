/*
 * IntegralData.cpp
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#include <iostream>
#include <fstream>
#include <string>
#include <list>

#include <Utils.h>
#include <IntegralData.h>
#include <BitPackedData.h>
#include <StringData.h>
#include <BitBlockData.h>
#include <IntegralArray.h>
#include <PagedIntegralData.h>

/*
//  Read integral data from a file
//  See 'IntegralData' section of online help for HPLogic Fast Binary Data
//  File Format under the File Out tool (6.0)
*/
IntegralData* IntegralData::read(std::ifstream& inFile, int inegralTypeline) {

    int numbits ;
    IntegralData* integralData = nullptr;

    std::string line;

	std::getline(inFile, line);

	auto items = Utils::tokenizeStr(line);
	auto sectionName = items.front();
	items.pop_front();

    std::cout << "+++  IntegralData::read " << line << ", section: " << sectionName << std::endl;

    if (sectionName == "BitPackedData") {
        integralData = BitPackedData::read(inFile);
    } else if  (sectionName == "StringData") {
    	integralData = StringData::read(inFile);
    } else if  (sectionName == "BitBlockData") {
    	integralData = BitBlockData::read(inFile);
    } else if  ( sectionName == "IntegralArray") {
    	bool isSigned = false;
        if ((numbits = Utils::extractIntType (items.front(), isSigned)) > 0) {
            std::cout << "+++  IntegralData::read Array numbits " << numbits << ", sign " << isSigned << std::endl;
            integralData = IntegralArray::read(inFile, numbits, isSigned, inegralTypeline) ;
        } else {
    		std::cerr << "Invalid IntegralArray object" << std::endl;
        }
    } else if  (sectionName == "PagedIntegralData") {
    	bool isSigned = false;
    	if((numbits = Utils::extractIntType (items.front(), isSigned)) > 0) {
            std::cout << "+++  IntegralData::read Paged numbits " << numbits << ", sign " << isSigned << std::endl;
            integralData = PagedIntegralData::read(inFile, numbits, isSigned);
    	} else {
    		std::cerr << "Invalid PagedIntegralData object" << std::endl;
    	}
    } else {
        std::cerr << "Unknown IntegralData object (%s)" << std::endl;
    }

    return integralData ;
}

