/*
 * StringData.h
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#ifndef STRINGDATA_H_
#define STRINGDATA_H_

#include <vector>
#include <IntegralData.h>

class StringData: public IntegralData {
public:
	StringData(long id);
	~StringData();
	static StringData* read(std::ifstream& inFile);
private:
	void readStringBlock(std::ifstream& inFile, int length);
private:
    unsigned int numChars ;
    std::vector<char> buffer;
};

#endif /* STRINGDATA_H_ */
