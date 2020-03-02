/*
 * FastOutHeader.cpp
 *
 *  Created on: Jun 29, 2019
 *      Author: oleh
 */

#include <iostream>
// boost::is_pointer is dependency for boost::tokenizer
#include <boost/type_traits.hpp>
#include <boost/tokenizer.hpp>

#include <Utils.h>
#include <DataGroup.h>
#include <DataSet.h>
#include <IntegralData.h>

const static std::string SIGNATURE_ID("HPLogic_Fast_Binary_Format_Data_File");

DataGroup* DataGroup::spInst = nullptr;
DataGroup::DataGroup(std::ifstream& anInFile)
 : inFile(anInFile)
 , verMajor(0)
 , verMinor(0)
 , timeCrossCorrID(0)
 , stateCrossCorrID(0)
 , numDataSets(0)
{
	spInst = this;
}

DataGroup::~DataGroup() {
}

bool DataGroup::process() {
	std::string signature;

	getline(inFile, signature);

	//TODO: for debug std::cout << "+++ Signature: " << signature << std::endl;

	if(signature != SIGNATURE_ID)
		return false;

	std::string line;
	getline(inFile, line);

    ::sscanf(line.data(), "%d %d", &verMajor, &verMinor) ;
    //TODO: for debug std::cout << "+++ Version: Major: " << verMajor << ", Minor: " << verMinor << std::endl;

    /* the correlation bits */
	getline(inFile, line);
        printBits(line);

    /* the cross correlation id's */
	getline(inFile, line);
    ::sscanf(line.data(), "%d %d\n", &timeCrossCorrID, &stateCrossCorrID ) ;

    //TODO: for debug std::cout << "+++    Time cross correlation id = " << timeCrossCorrID << std::endl;
    //TODO: for debug std::cout << "+++    State cross correlation id = " << stateCrossCorrID << std::endl;

	getline(inFile, line);
    numDataSets = ::atoi(line.data());

    //TODO: for debug std::cout << "+++ DataSets: "  << numDataSets << std::endl;
    // std::flush(std::cout);

    //TODO: for debug     std::cout << "+++ line |" << line << "|" << std::endl;

    for (auto i = 0 ; i < numDataSets ; i++ ) {
        //TODO: for debug std::cout << "+++ DataSet #" << i+1 << std::endl;
        auto dataSet = std::make_shared<DataSet>(inFile);
        dataSet->process();
        dataSets.push_back(dataSet);
    }

	return true;
}

std::shared_ptr<SharedObject> DataGroup::isObject(long id) {
	auto itr = sharedObjMap.find(id);
	if(itr != sharedObjMap.end())
	{
		//TODO: for debug printf("+++ DataGroup::isObject id: %ld. obj: %p \n", id, itr->second);

		return itr->second;
	}

	//TODO: for debug printf("+++ DataGroup::isObject id: %ld. obj: nil \n", id);

	return std::shared_ptr<SharedObject>();
}

bool DataGroup::addObject(std::shared_ptr<SharedObject> anObj) {
	auto itr = sharedObjMap.find(anObj->getId());
	if(itr == sharedObjMap.end())
	{
		//TODO: for debug printf("+++ DataGroup::addObject id: %ld. obj: %p \n", anObj->getId(), anObj);
		sharedObjMap[anObj->getId()] = anObj;
		return true;
	}

	return false;
}

void DataGroup::printBits(const std::string& line)
{
	int bit = 0;
        auto list = Utils::tokenizeInt(line);
	const auto num = list.front();
	list.pop_front();
        (void)bit;
	(void)num;

        //TODO: for debug std::cout << "+++ Correlation Bits: " << line << std::endl;
        for (auto& bit: list) {
             switch ( bit ) {
                 case 0:
        	     //TODO: for debug std::cout << "+++ Time ";
                     break ;
                 case 1:
        	      //TODO: for debug std::cout << "+++ State ";
                      break ;
                 default:
        	      //TODO: for debug std::cout << "+++ unknown " << bit;
                      break ;
             }
        }
        //TODO: for debug std::cout << std::endl;
}
