/*
 * Common.h
 *
 *  Created on: Jun 30, 2019
 *      Author: oleh
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <fstream>
#include <list>

class Utils {

	/* structures used by the bitblock data types */
	struct ReorderType {
	  int endian16, endian32, endian64, endian128 ;
	  int width ;
	  int * input2outputMap32 ;
	  int * output2inputMap32 ;
	} ;

	struct ExtractorType {
	  unsigned int width ;
	  unsigned int widthInBytes ;
	  unsigned int inverted ;
	  unsigned char * mask ;
	  int reorderDirection ;
	  struct ReorderType * reorder ;
	} ;

public:
	Utils(std::ifstream& anInFile);
	virtual ~Utils();

	static bool readAttributes(std::ifstream&, const char *who);

//	long readIntegralData(int ipl);
//	long readPagedIntegralData(int numbits, int sign);
//	long readBitPackedData();
//	void readStringData();
//	void readStringBlock(int length);
//	void readBitBlockData();
//	void readExtractorInfo(struct ExtractorType *e);
//	void readReorderObject(struct ReorderType *r);
//	long readBitBlock();
//	long readIntegralArray(int numbits, bool sign, int ipl);
//	bool readTimeTags(int pagedFlag);

	static void readAuxDataFileHeader(std::ifstream& dataFile);
	static std::list<int> tokenizeInt(const std::string& aStr);
	static std::list<std::string> tokenizeStr(const std::string& aStr);
	static int  extractIntType(const std::string& aTypeStr, bool& isSigned);

private:

private:
//TODO: unused	std::ifstream& inFile;
//TODO: unused	bool GlobalSuppressDataOutput;
};

#endif /* UTILS_H_ */
