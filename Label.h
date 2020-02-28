/*
 * Label.h
 *
 *  Created on: Jun 29, 2019
 *      Author: oleh
 */

#ifndef LABEL_H_
#define LABEL_H_

#include <fstream>
#include <string>
#include <vector>
#include <memory>

class IntegralData;
class Label {
public:
	Label(std::ifstream& anInFile);
	virtual ~Label();

	bool process();

	bool getBits(int aRecIdx, std::vector<bool>& aBitVec);
	const std::string& getName()
	{return name;}
	long getId()
	{return id;}
    void clear()
    {storedBitVec.clear();}

private:
	bool readLabelData();

private:
	std::ifstream& inFile;
    long id ;
    std::string name;
    std::shared_ptr<IntegralData> integralData;
    std::vector<bool> storedBitVec;
    bool initVal;
};

#endif /* LABEL_H_ */
