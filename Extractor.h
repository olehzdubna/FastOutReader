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
#include <memory>

class Reorderer;
class Extractor {
public:
	Extractor();
	~Extractor();
	static std::shared_ptr<Extractor> read(std::ifstream& inFile);
	void extractBits(const uint8_t* aRec, std::vector<bool>& aBitVec);
private:
	void extractByteBits(uint8_t aByte, uint8_t aMask, std::vector<bool>& aBitVec);
private:
	  unsigned int width;
	  unsigned int widthInBytes;
	  unsigned int inverted;
	  std::vector<uint8_t> mask;
	  int reorderDirection;
	  std::shared_ptr<Reorderer> reorder;
	  std::list<int> maskIdx;
};

#endif /* EXTRACTOR_H_ */
