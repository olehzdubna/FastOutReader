/*
 * StringData.cpp
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#include <iostream>
#include <DataGroup.h>
#include <StringData.h>

StringData::StringData(long anId)
 : IntegralData(anId)
 , numChars(0) {
}

StringData::~StringData() {
}

/*
//  Read string data from a file
//  See 'IntegralData->Strings' section of online help for HPLogic Fast
//  Binary Data File Format under the File Out tool (6.7)
*/
StringData* StringData::read(std::ifstream& inFile) {
	std::string line;
    long id ;
    int length ;

    std::cout << "+++      StringData: " << std::endl;

	std::getline(inFile, line);
    /* integral data id */
    id = ::atol(line.data());

    std::cout << "+++      Integral ID: " << id << std::endl;

    StringData* stringData = nullptr;
    if ((stringData = static_cast<StringData*>(DataGroup::instance()->isObject(id))) ) {
        std::cout << "+++    already seen this LabelEntry object" << std::endl;
        return stringData;
    }

	stringData = new StringData(id);

	std::getline(inFile, line);
    length = ::atoi(line.data());

    std::cout << "+++      length=" << length << std::endl;

    /* stringblock */
    stringData->readStringBlock(inFile, length);

    return stringData;
}

void StringData::readStringBlock(std::ifstream& inFile, int length)
{
/*
//  Read a string block from a file
//  See 'IntegralData->Strings->StringBlock' section of online help for
//  HPLogic Fast Binary Data File Format under the File Out tool (6.7.2)
*/
    for (int i = 0; i < length; i++)
    {
      inFile.read((char*)&numChars, sizeof(numChars));
      buffer.reserve(numChars);

      /* raw bytes */
      inFile.read(buffer.data(), numChars) ;
    }
}
