/*
 * Extractor.h
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#ifndef EXTRACTOR_H_
#define EXTRACTOR_H_

#include <vector>
#include <list>

class Reorderer;
class Extractor {
public:
	Extractor();
	~Extractor();
	static Extractor* read(std::ifstream& inFile);
	void extractBits(const uint8_t* aRec, std::vector<bool>& aBitVec);
	void deleteReorder();

private:
	void extractByteBits(uint8_t aByte, uint8_t aMask, std::vector<bool>& aBitVec);

private:
	  unsigned int width;
	  unsigned int widthInBytes;
	  unsigned int inverted;
	  std::vector<uint8_t> mask;
	  int reorderDirection;
	  Reorderer* reorder;
	  std::list<int> maskIdx;
};

#endif /* EXTRACTOR_H_ */
