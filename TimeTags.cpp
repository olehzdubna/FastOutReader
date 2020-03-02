/*
 * TimerTags.cpp
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */


#include <TimeTags.h>

#ifdef Linux
    #include <byteswap.h>
#endif

#ifdef FreeBSD
    #include <sys/endian.h>
#endif

#include <string>
#include <iostream>

#include <Utils.h>

TimeTags::TimeTags()
 : AxisX()
 , samples(0)
 , trig(0) {
}

TimeTags::~TimeTags() {
}

/*
//  Read time tag information from a file
//  See 'Vertical Header->Abscissa Data Type->Time Tags' section of online
//  help for HPLogic Fast Binary Data File Format under the File Out tool (8.3)
*/
std::shared_ptr<TimeTags> TimeTags::read(std::ifstream& inFile, int pagedFlag) {

	std::string line;
	int haveData = 1 ;

	//TODO: for debug std::cout << "+++  Time Tagged Data    " << pagedFlag << std::endl;

	auto timeTags = std::make_shared<TimeTags>();

	std::getline(inFile, line);
	/* number of samples and trigger position */
	::sscanf(line.data(), "%d %d\n", &timeTags->samples, &timeTags->trig ) ;

	//TODO: for debug std::cout << "+++    samples = " << timeTags->samples << "   trigger = " << timeTags->trig << std::endl;

	if ( pagedFlag ) {
		std::getline(inFile, line);
		//TODO: for debug std::cout << "+++  TimeTags::read " << line << std::endl;
		haveData = ::atoi(line.data());
	}

	if ( haveData ) {
		timeTags->integralData =  IntegralData::read(inFile, 1);
		std::getline(inFile, line);
		std::getline(inFile, line);
	}

	Utils::readAttributes(inFile, "TimeTags" );

	return timeTags;
}

int64_t TimeTags::getTime(int aRecIdx) {
	int64_t tim = 0;

	std::vector<uint8_t> byteVec;
	integralData->extractBytes(aRecIdx, byteVec);

	if(byteVec.size() == sizeof(tim))
		tim = 
#ifdef Linux 
		        bswap_64
#endif
#ifdef FreeBSD
                        bswap64
#endif
		          (*reinterpret_cast<int64_t*>(byteVec.data()));

	return tim;
}


