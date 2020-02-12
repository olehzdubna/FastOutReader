/*
 * FastOutHeader.h
 *
 *  Created on: Jun 29, 2019
 *      Author: oleh
 */

#ifndef DATAGROUP_H_
#define DATAGROUP_H_

#include <fstream>
#include <list>
#include <string>
#include <map>
#include <vector>
#include <boost/tokenizer.hpp>

#include <SharedObject.h>

class DataSet;
class IntegralData;
class DataGroup {
public:
	DataGroup(std::ifstream& anInFile);
	virtual ~DataGroup();
	static DataGroup*  instance()
	{return spInst;}
	bool process();
	const std::vector<DataSet*>& getDataSets()
	{return dataSets;}

	SharedObject* isObject(long id);
	bool addObject(SharedObject* anObj);

private:
	static DataGroup* spInst;
	std::ifstream& inFile;
	int verMajor;
	int verMinor;
	int timeCrossCorrID;
	int stateCrossCorrID;
	int numDataSets;
	std::map<long, SharedObject*> sharedObjMap;
	std::vector<DataSet*> dataSets;
};

#endif /* DATAGROUP_H_ */
