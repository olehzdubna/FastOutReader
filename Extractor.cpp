/*
 * Extractor.cpp
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#include <iostream>
#include <string>
#include <list>

#include <Utils.h>
#include <Extractor.h>
#include <Reorderer.h>

Extractor::Extractor() {
}

Extractor::~Extractor() {

}

/*
//  Read an Extractor object from the given file.
//  See 'IntegralData->Label Extractor' section fo the online help
//  for Fast Binary Data File Format under the File Out tool (6.8)
*/
Extractor* Extractor::read(std::ifstream& inFile) {
	std::string line;

	int haveReorderObject;
	int mask;

	Extractor* extractor = new Extractor();
	/* get the width in bit and bytes of the mask and whether it needs to
	   be inverted or not */
	std::getline(inFile, line);
	//TODO: for debug std::cout << "+++ Label::readExtractorInfo, line: " << line << std::endl;

	auto items = Utils::tokenizeStr(line);
	extractor->widthInBytes = ::atoi(items.front().data());
	items.pop_front();
	extractor->width = ::atoi(items.front().data());
	items.pop_front();
	extractor->inverted = ::atoi(items.front().data());

//	fscanf( fp, "%u %u %d ", &e->widthInBytes, &e->width, &e->inverted ) ;

	//TODO: for debug std::cout << "+++      width=" << extractor->width << "  bytes=" << extractor->widthInBytes << "  inverted=" << extractor->inverted << std::endl;

	/* get the label mask */
    for (int idx = 0 ; idx < extractor->widthInBytes ; idx++ ) {
    	items.pop_front();

      ::sscanf(items.front().data(), "%x", &mask);
      extractor->mask.push_back(mask);
      if(mask > 0)
    	  extractor->maskIdx.push_front(idx);
    }

    //TODO: for debug std::cout << "+++      mask[";
    //TODO: for debug for(auto msk: extractor->mask)
    //TODO: for debug 	std::cout << std::hex << (int)msk << std::dec << " ";
    //TODO: for debug std::cout << "]" << std::endl;

    //TODO: for debug std::cout << "+++      mask idx[";
    //TODO: for debug for(auto mId: extractor->maskIdx)
    //TODO: for debug	std::cout << (int)mId << " ";
    //TODO: for debug std::cout << "]" << std::endl;

	/* a reserved field for future use */
//    fscanf( fp, "%d\n", &tmp ) ;

	/* get the reorder direction - may or may not actually be used */
	std::getline(inFile, line);
	extractor->reorderDirection = ::atoi(line.data());
	//TODO: for debug std::cout << "+++ reorderDirection " << extractor->reorderDirection << std::endl;

	/* get the reorder structure flag */
	std::getline(inFile, line);
	haveReorderObject = ::atoi(line.data());
	//TODO: for debug std::cout << "+++ haveReorderObject " << haveReorderObject << std::endl;

	if ( haveReorderObject ) {
		extractor->reorder = Reorderer::read(inFile);
	} else {
		extractor->reorder = nullptr ;   /* make sure pointer is cleared */
		std::getline(inFile, line);
	}

	return extractor;
}

void Extractor::extractBits(const uint8_t* aRec, std::vector<bool>& aBitVec) {
	std::vector<bool> bits;
//	std::cout << "+++ Extractor::extractBits " << (void*)aRec << " \n";
//
//	for(int i=0; i<widthInBytes; i++) {
//		char buf[80];
//		sprintf(buf, "0x%2.2x", aRec[i]);
//		std::cout << buf << " ";
//	}
//	std::cout << std::endl;
//	for(int i=0; i<widthInBytes; i++) {
//		char buf[80];
//		sprintf(buf, "0x%2.2x", mask[i]);
//		std::cout << buf << " ";
//	}
//	std::cout << std::endl;
	for(auto idx: maskIdx) {
		extractByteBits(aRec[idx], mask[idx], aBitVec);
	}
//	std::cout << std::endl;
}

void Extractor::extractByteBits(uint8_t aByte, uint8_t aMask, std::vector<bool>& aBitVec) {
	for(int i=0; i<8; i++) {
		if(aMask & 0x01) {
			aBitVec.push_back(aByte & 0x01);
		}
		aMask = (aMask >> 1);
		aByte = (aByte >> 1);
	}
}

void Extractor::deleteReorder() {
	delete reorder;
}
