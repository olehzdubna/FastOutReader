/*
 * DataSet.cpp
 *
 *  Created on: Jun 29, 2019
 *      Author: oleh
 */

#include <sys/endian.h>
#include <iostream>

#include <Utils.h>
#include <DataSet.h>
#include <Label.h>
#include <TimeTags.h>
#include <Periodic.h>

#define ES_TIME_CORR_SECTION    "Read error in TIME_CORR_INFO section"
#define ES_STATE_CORR_SECTION   "Read error in STATE_CORR_INFO section"
#define ID_STATE_CORR "StateCorrelationInfo"
#define ID_TIME_CORR "TimeCorrelationInfo"

DataSet::DataSet(std::ifstream& anInFile)
 : inFile(anInFile)
 , numLabelEntries(0)
 , id1(0)
 , id3(0)
 , beginTime(0)
 , endTime(0)
 , startSample(0)
 , lastSample(0)
 , timeLine(nullptr)
{
}

DataSet::~DataSet() {
	delete timeLine;
}

bool DataSet::process() {
	std::string line;

	std::getline(inFile, line);
	numLabelEntries = ::atoi(line.data());
    //TODO: for debug std::cout << "+++  Number of LabelEntries: " << numLabelEntries << std::endl;

    if (numLabelEntries > 100)
    	numLabelEntries = 100;

    for(int i = 0 ; i < numLabelEntries ; i++) {

    	//TODO: for debug std::cout << "+++  Label -- " << i+1 << std::endl;

    	if(!readLabel())
    		return false;
    }

    /* x-axis data */
    timeLine = AxisX::read(inFile);

    /* time correlation info */
    readTimeCorrelationInfo();

    /* state correlation info */
    readStateCorrelationInfo();

	std::getline(inFile, line);
    /* origin path */
	char tmpstr[100];
    ::sscanf(line.data(), "`%[^`]`\n", tmpstr ) ;
    //TODO: for debug std::cout << "+++  Origin: " << tmpstr << std::endl;

	std::getline(inFile, line);
    /* dataset id, etc... */
    ::sscanf(line.data(), "%d %d\n", &id1, &id3);
    //TODO: for debug std::cout << "+++  DataSet ID: " << id1 << std::endl;
    //TODO: for debug std::cout << "+++  Run ID    : " << id3 << std::endl;

    /* Begin and End time */
	std::getline(inFile, line);
    ::sscanf(line.data(), "%d\n", &beginTime);
	std::getline(inFile, line);
    ::sscanf(line.data(), "%d\n", &endTime);
    //TODO: for debug std::cout << "+++  Begin Time: " << beginTime << std::endl;
    //TODO: for debug std::cout << "+++  End   Time: " << endTime << std::endl;

    /* start sample */
	std::getline(inFile, line);
    startSample = atoi(line.data());
    /* last sample */
	std::getline(inFile, line);
    lastSample = atoi(line.data());

    //TODO: for debug std::cout << "+++  Start sample: " << startSample << std::endl;
    //TODO: for debug std::cout << "+++   Last sample: " << lastSample << std::endl;

    return true;
}

bool DataSet::readLabel() {

	auto label = new Label(inFile);
	label->process();

	labels.push_back(label);

	return true;
}

bool DataSet::readTimeCorrelationInfo() {
/*
//  Read time correlation information from a file
//  See 'Vertical Header->Time Correlation Info' section of online help for
//  HPLogic Fast Binary Data File Format under the File Out tool (9.0)
*/
	std::string line;
	int type ;
	int source ;

	std::getline(inFile, line);
	if (line != ID_TIME_CORR) {
		std::cerr << ES_TIME_CORR_SECTION << std::endl;
		return false;
	}

	/* time correlation type, source, and offset */
	std::getline(inFile, line);
	char strLine[100];
	::sscanf(line.data(), "%d %d %s\n", &type, &source, strLine);
	//TODO: for debug std::cout << "+++  Time Correlation type: " << type << "  source: " << source << "  Offset: " << strLine <<std::endl;

	return true;
}

bool DataSet::readStateCorrelationInfo() {
/*
//  Read state correlation information from a file
//  See 'Vertical Header->State Correlation Info' section of online help for
//  HPLogic Fast Data File Format under the File Out tool (10.0)
*/
	std::string line;

	std::getline(inFile, line);
	if (line != ID_STATE_CORR) {
		std::cerr << ES_STATE_CORR_SECTION << std::endl;
		return false;
	}

	/* state correlation offset */
	std::getline(inFile, line);
	//TODO: for debug std::cout << "+++  State Correlation Offset: " << line << std::endl;

	return true;
}

int64_t DataSet::getTime(int aRecIdx) {
	return timeLine->getTime(aRecIdx);
}


