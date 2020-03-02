/*
 * PagedIntegralData.cpp
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#include <cstring>
#include <iostream>

#include <Utils.h>
#include <DataGroup.h>
#include <PagedIntegralData.h>
#include <IfstreamWithState.h>

PagedIntegralData::PagedIntegralData(long anId)
 : IntegralData(anId)
 , numSamples(0)
 , numBytesPerSample(0) {
}

std::shared_ptr<PagedIntegralData> PagedIntegralData::read(std::ifstream& inFile, int numbits, int sign) {
	/*
	//  Read Paged Integral Data from a file
	//  See 'IntegralData->PagedIntegralData' section of online help for HPLogic
	//  Fast Binary Data File Format under the File Out tool (6.6)
	*/
	    std::string line;
	    long id ;
	    int useFile ;
	    char filename[BUFSIZ] ;
	    std::ifstream dataFile;

	    //TODO: for debug std::cout << "+++      PagedIntegralData with " << numbits << " bits and sign " << sign << std::endl;

		std::getline(inFile, line);
	    /* integral data id */
	    id = ::atol(line.data());

	    //TODO: for debug std::cout << "+++      Integral ID: " << id << std::endl;

	    /* id == 0 means that there is no data - reason unknown!!! */
	    if ( id == 0 ) {
	    	//TODO: for debug std::cout << "+++  Data missing! " << std::endl;
	    	return nullptr;
	    }

	    std::shared_ptr<PagedIntegralData> pagedIntegralData;

	    if (auto pagedIntegralDataPtr = static_cast<PagedIntegralData*>(DataGroup::instance()->isObject(id).get())) {
	        //TODO: for debug std::cout << "+++    already seen this LabelEntry object" << std::endl;
	        return std::shared_ptr<PagedIntegralData>(pagedIntegralDataPtr);
	    }

	    pagedIntegralData = std::make_shared<PagedIntegralData>(id);

	    /* is this data in this file or in separate file? */
		std::getline(inFile, line);
	    ::sscanf(line.data(), "%d `%[^`]`", &useFile, filename ) ;
    
           const auto& inFileWithPrefix = static_cast<const IfstreamWithState&>(inFile);

	    if ( useFile ) {
	        //TODO: for debug std::cout << "+++      Data is in file " <<  filename << "." << std::endl;
	        dataFile.open(filename);
	        if(!dataFile.is_open()) {
	          char *basename = strrchr( filename, '/' ) ;
	          if (basename) {
		         std::string basenameWithPrefix(inFileWithPrefix.getDirectoryPrefix() + "/" +
                                                        std::string(basename+1));
                          dataFile.open(basenameWithPrefix.c_str());
			  if(!dataFile.is_open()) {
	                      /* full name did not work, try just base name */
	        	      dataFile.open(basename+1);
	        	      if(!dataFile.is_open()) {
	        		  std::cerr << "   Can't open file " << filename << std::endl;
	              	          return std::shared_ptr<PagedIntegralData>();
	        	      }
			  }
	          }
	        }
	        Utils::readAuxDataFileHeader(dataFile) ;
	    } else {
	      std::swap(dataFile, inFile);
	    }

	    /* next two ints are reserved for paging information... */

	    std::getline(dataFile, line);
	    std::getline(dataFile, line);
	    ::sscanf(line.data(), "%d %d\n", &pagedIntegralData->numSamples, &pagedIntegralData->numBytesPerSample) ;

	    /* raw bytes */
	    pagedIntegralData->buffer = std::shared_ptr<uint8_t>(new uint8_t[pagedIntegralData->numSamples * pagedIntegralData->numBytesPerSample],
	                                                         std::default_delete<uint8_t>());
	    dataFile.read((char*)pagedIntegralData->buffer.get(), pagedIntegralData->numBytesPerSample * pagedIntegralData->numSamples);

	    DataGroup::instance()->addObject(pagedIntegralData);

	    if ( useFile ) {
	      dataFile.close();
	    }

	    return pagedIntegralData;
}

void PagedIntegralData::extractBytes(int aRecIdx, std::vector<uint8_t>& aByteVec) {
	if(aRecIdx >= numSamples)
		return;

	auto bytes = &buffer.get()[aRecIdx*numBytesPerSample];
	for(int i=0; i<numBytesPerSample; i++)
		aByteVec.push_back(bytes[i]);

}

