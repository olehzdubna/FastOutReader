/*
 * BitBlock.cpp
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#include <iostream>

#include <BitBlock.h>
#include <Utils.h>
#include <DataGroup.h>

BitBlock::BitBlock(long anId)
 : IntegralData (anId)
 , buffer(nullptr)
 , numSamples(0)
 , numBytesPerSample() {
}

BitBlock::~BitBlock() {
	delete [] buffer;
}

/*
//  Read a BitBlock object from the given file.
//  See 'IntegralData->BitBlock' or 'IntegralData->PagedBitBlock' sections
//  of the online help for Fast Binary Data File Format under the File Out
//  tool (6.4 or 6.5)
*/
BitBlock* BitBlock::read(std::ifstream& inFile) {

	std::string line;
	int paged = 0;
	long id;
	int useFile;
	char filename[BUFSIZ];

	std::ifstream dataFile;

	std::getline(inFile, line);

	if (line == "BitBlock") {
		paged = 0 ;
	} else if  (line == "PagedBitBlock") {
		paged = 1 ;
	} else {
		std::cerr << "Unknown BitBlock object " << line << std::endl;
		return nullptr;
	}

	/* first two ints reserved */
	std::getline(inFile, line);
	std::getline(inFile, line);

	std::getline(inFile, line);
	/* BitBlock id */
	id = ::atol(line.data());

	//TODO: for debug std::cout << "+++      BitBlock ID: " << id;

	/* id == 0 means that there is no data - reason unknown!!! */
	if ( id == 0 ) {
	  std::cout << "   Data missing!" << std::endl;
	  return nullptr;
	}

	BitBlock* bitBlock = nullptr;
	if ((bitBlock = static_cast<BitBlock*>(DataGroup::instance()->isObject(id))) ) {
		//TODO: for debug  std::cout << "    already seen this LabelEntry object" << std::endl;
		return bitBlock;
	}
	std::cout << std::endl;

	/* is this data in this file or in separate file? */
	std::getline(inFile, line);
	//TODO: for debug std::cout << "+++  Label::readBitBlock " << line << std::endl;

	::sscanf(line.data(), "%d `%[^`]`", &useFile, filename ) ;

	if ( useFile ) {
		//TODO: for debug std::cout << "+++      Data is in file " <<  filename << "." << std::endl;
		dataFile.open(filename);
		if(!dataFile.is_open()) {
		  /* full name did not work, try just base name */
		  char *basename = strrchr( filename, '/' ) ;
		  if (basename) {
			  dataFile.open(basename+1);
			  if(!dataFile.is_open()) {
				  std::cerr << "   Can't open file " << basename+1 << std::endl;
				  return nullptr;
			  }
		  }
		}
		Utils::readAuxDataFileHeader(dataFile) ;
	} else {
	  std::swap(dataFile, inFile);
	}

	if ( paged ) {
	  /* next two ints are reserved for paging information... */
		std::getline(dataFile, line);
	}

	bitBlock = new BitBlock(id);

	std::getline(dataFile, line);
	::sscanf(line.data(), "%d %d\n", &bitBlock->numSamples, &bitBlock->numBytesPerSample ) ;

	//TODO: for debug std::cout << "+++        Number samples: " << bitBlock->numSamples << "  Bytes Per Sample: " << bitBlock->numBytesPerSample << std::endl;

	/* raw bytes */
	bitBlock->buffer = new char[bitBlock->numSamples * bitBlock->numBytesPerSample];

	dataFile.read(bitBlock->buffer, bitBlock->numBytesPerSample * bitBlock->numSamples);

	std::getline(inFile, line);

	DataGroup::instance()->addObject(bitBlock);

	if ( useFile ) {
	  dataFile.close();
	}

	return bitBlock;
}

const char* BitBlock::getRecord(int anIdx) const {
	return &buffer[anIdx * numBytesPerSample];
}


