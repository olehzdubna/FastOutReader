/*
 * DataSet.h
 *
 *  Created on: Jun 29, 2019
 *      Author: oleh
 */

#ifndef DATASET_H_
#define DATASET_H_

#include <fstream>
#include <vector>

#include <AxisX.h>

class Label;
class DataSet {
public:
	DataSet(std::ifstream& anInFile);
	virtual ~DataSet();
	bool process();
	const std::vector<Label*>& getLabels() const
	{return labels;}
	int64_t getTime(int aIdx);
	int getStartSample()
	{return startSample;}
	int getLastSample()
	{return lastSample;}
	int getTrig()
	{return timeLine->getTrig();}

private:
	bool readLabel();
	bool readTimeCorrelationInfo();
	bool readStateCorrelationInfo();

private:
	std::ifstream& inFile;
    int numLabelEntries;
    int id1;
	int id3;
    int beginTime;
	int endTime;
    int startSample;
    int lastSample;
    std::vector<Label*> labels;
    AxisX* timeLine;
};

#endif /* DATASET_H_ */
