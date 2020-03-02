/*
 * BitBlock.cpp
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#include <cstring>
#include <iostream>

#include <BitBlock.h>
#include <Utils.h>
#include <DataGroup.h>
#include <IfstreamWithState.h>

BitBlock::BitBlock(long anId)
 : IntegralData (anId)
 , numSamples(0)
 , numBytesPerSample() {
}

/*
//  Read a BitBlock object from the given file.
//  See 'IntegralData->BitBlock' or 'IntegralData->PagedBitBlock' sections
//  of the online help for Fast Binary Data File Format under the File Out
//  tool (6.4 or 6.5)
*/
std::shared_ptr<BitBlock> BitBlock::read(std::ifstream& inFile) {

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

	std::shared_ptr<BitBlock> bitBlock;
	if (auto bitBlockPtr = static_cast<BitBlock*>(DataGroup::instance()->isObject(id).get())) {
		//TODO: for debug std::cout << " +++ already seen this LabelEntry object" << std::endl;
		return std::shared_ptr<BitBlock>(bitBlockPtr);
	}
	std::cout << std::endl;

	/* is this data in this file or in separate file? */
	std::getline(inFile, line);
	//TODO: for debug std::cout << "+++  Label::readBitBlock " << line << std::endl;

	::sscanf(line.data(), "%d `%[^`]`", &useFile, filename ) ;
        
	const auto& inFileWithPrefix = static_cast<const IfstreamWithState&>(inFile);
	if ( useFile ) {
		//TODO: for debug std::cout << "+++      Data is in file " <<  filename << "." << std::endl;
		dataFile.open(filename, std::ios::binary);
		if(!dataFile.is_open() || dataFile.fail()) {
		  char *basename = strrchr( filename, '/' ) ;
		  if (basename) {
		          std::string basenameWithPrefix(inFileWithPrefix.getDirectoryPrefix() + "/" + 
			                                 std::string(basename+1));
			  dataFile.open(basenameWithPrefix.c_str(), std::ios::binary);
			  if(!dataFile.is_open() || dataFile.fail()) {
		             /* full name did not work, try just base name */
			     dataFile.open(basename + 1, std::ios::binary);
			     if(!dataFile.is_open() || dataFile.fail()) {
				  std::cerr << "   Can't open file " << (basename + 1) << std::endl;
				  return nullptr;
		              }
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

	bitBlock = std::make_shared<BitBlock>(id);

	std::getline(dataFile, line);
	::sscanf(line.data(), "%d %d\n", &bitBlock->numSamples, &bitBlock->numBytesPerSample ) ;

	//TODO: for debug std::cout << "+++        Number samples: " << bitBlock->numSamples << "  Bytes Per Sample: " << bitBlock->numBytesPerSample << std::endl;

	/* raw bytes */
	bitBlock->buffer = std::shared_ptr<char>(new char[bitBlock->numSamples * bitBlock->numBytesPerSample],
	                                         std::default_delete<char[]>());

	dataFile.read(bitBlock->buffer.get(), bitBlock->numBytesPerSample * bitBlock->numSamples);

	std::getline(inFile, line);

	DataGroup::instance()->addObject(bitBlock);

	if ( useFile ) {
	  dataFile.close();
	}

	return bitBlock;
}

const char* BitBlock::getRecord(int anIdx) const {
	return &buffer.get()[anIdx * numBytesPerSample];
}


