/*
 * Label.cpp
 *
 *  Created on: Jun 29, 2019
 *      Author: oleh
 */
#include <string>
#include <iostream>
#include <Label.h>
#include <vector>

#include <Utils.h>
#include <DataGroup.h>
#include <IntegralData.h>

Label::Label(std::ifstream& anInFile)
 : inFile(anInFile)
 , id(0)
 , integralData(0)
 , initVal(true) {
}

Label::~Label() {
}

bool Label::process() {
	std::string line;
	SharedObject* sharedObject = nullptr;

	std::getline(inFile, line);
	id = ::atol(line.data());
    std::cout << "+++  Label ID: " << id << std::endl;

    if (( sharedObject = DataGroup::instance()->isObject(id) )) {
        std::cout << "+++    already seen this LabelEntry object" << std::endl;
        return false;
    }

    /* label name */
	std::getline(inFile, line);
	auto toks = Utils::tokenizeStr(line);

    name = toks.back();
    auto pos = std::string::npos;
    while((pos = name.find("`")) != std::string::npos)
    		name.erase(pos);

    std::cout << "+++    Label Name: " << name << std::endl;

    /* label data */
    if(!readLabelData())
    	return false;

    /* label attributes */
    Utils::readAttributes(inFile, "  Label");

	return true;
}

bool Label::readLabelData() {
	std::string line;

	std::getline(inFile, line);

    if ( line == "NoData" ) {
    	Utils::readAttributes(inFile, "  NoData" ) ;
    } else if  ( line == "States") {
    	integralData = IntegralData::read(inFile, 1) ;
    	std::getline(inFile, line);
    	Utils::readAttributes(inFile, "  States" ) ;
    } else if  ( line == "StateCount" ) {
    	integralData = IntegralData::read(inFile,1) ;
//        fscanf( fp, "\n" ) ;
    	Utils::readAttributes(inFile, "  StateCount" ) ;
    } else if  ( line == "Glitch" ) {
    	integralData = IntegralData::read(inFile, 1);
//        fscanf( fp, "\n" ) ;
    	integralData = IntegralData::read(inFile, 1);
//        fscanf( fp, "\n" ) ;
    	Utils::readAttributes(inFile, "  Glitch" ) ;
    } else if  ( line == "Analog" ) {
    	integralData = IntegralData::read(inFile, 1) ;
//        fscanf( fp, "\n" ) ;
////      readVerticalHeader( fp, infoLevel ) ;
//        fscanf( fp, "%s\n", tmpstr ) ;
    	Utils::readAttributes(inFile, "  Analog" ) ;
    } else if  ( line == "TextLines" ) {
    	integralData = IntegralData::read(inFile, 1) ;
//        fscanf( fp, "\n" ) ;
    	Utils::readAttributes(inFile, "  StateCount" ) ;
    } else {
        std::cerr << "Unknown LabelData object: " << line << std::endl;
        return false;
    }

    std::getline(inFile, line);

	return true;
}

bool Label::getBits(int aRecIdx, std::vector<bool>& aBitVec) {
	integralData->extractBits(aRecIdx, aBitVec);
	if(!initVal && storedBitVec == aBitVec)
		return false;
	if(initVal)
		initVal = false;
	storedBitVec = aBitVec;
	return true;
}










