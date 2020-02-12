/*
 * Common.cpp
 *
 *  Created on: Jun 30, 2019
 *      Author: oleh
 */


#include <iostream>

#include <Utils.h>
#include <DataGroup.h>

#define ID_AUX_DATA_HEADER "HPLogic_Additional_Data_File"

Utils::Utils(std::ifstream& anInFile)
 : inFile(anInFile)
 , GlobalSuppressDataOutput(false)
{}

Utils::~Utils() {
}

bool Utils::readAttributes(std::ifstream& inFile, const char *who)
{
/*
//  Read the attributes bit set from a file
//  Formatted: "%d [%d]* \n"
//  See 'LabelEntry->Label Attribute Bitset' section of online help for HPLogic
//  Fast Binary Data File Format under the File Out tool (4.2)
*/
    int i ;
    int num ;
    int bitnum ;
	std::string line;

	std::getline(inFile, line);
	num = ::atoi(line.data());
    std::cout << "+++  " << who << "--Attributes: " << num << std::endl;

    return true;
}

//long Utils::readIntegralData(int ipl)
//{
///*
////  Read integral data from a file
////  See 'IntegralData' section of online help for HPLogic Fast Binary Data
////  File Format under the File Out tool (6.0)
//*/
//    int numbits ;
//    long id = 0 ;
//
//    std::string line;
//
//	std::getline(inFile, line);
//
////    std::cout << "+++  Utils::readIntegralData " << line << std::endl;
//
//	auto items = DataGroup::tokenizeStr(line);
//	auto sectionName = items.front();
//	items.pop_front();
//
//    if (sectionName == "BitPackedData") {
//        id = readBitPackedData();
//    } else if  (sectionName == "StringData") {
//        readStringData();
//    } else if  (sectionName == "BitBlockData") {
//        readBitBlockData() ;
//    } else if  ( sectionName == "IntegralArray") {
//    	bool isSigned = false;
//        if ((numbits = extractIntType (items.front(), isSigned)) > 0) {
////            std::cout << "+++  Utils::readIntegralData Array numbits " << numbits << ", sign " << isSigned << std::endl;
//            id = readIntegralArray(numbits, isSigned, ipl) ;
//        } else {
//    		std::cerr << "Invalid IntegralArray object" << std::endl;
//        }
//    } else if  (sectionName == "PagedIntegralData") {
//    	bool isSigned = false;
//    	if((numbits = extractIntType (items.front(), isSigned)) > 0) {
////            std::cout << "+++  Utils::readIntegralData Paged numbits " << numbits << ", sign " << isSigned << std::endl;
//            id = readPagedIntegralData(numbits, isSigned);
//    	} else {
//    		std::cerr << "Invalid PagedIntegralData object" << std::endl;
//    	}
//    } else {
//        std::cerr << "Unknown IntegralData object (%s)" << std::endl;
//    }
//
//    return id ;
//}
//
//long Utils::readPagedIntegralData(int numbits, int sign )
//{
///*
////  Read Paged Integral Data from a file
////  See 'IntegralData->PagedIntegralData' section of online help for HPLogic
////  Fast Binary Data File Format under the File Out tool (6.6)
//*/
//    std::string line;
//    long id ;
//    int useFile ;
//    char filename[BUFSIZ] ;
//    std::ifstream dataFile;
//    int tmp ;
//    char *buffer ;
//    unsigned int numSamples, numBytesPerSample ;
//
//    std::cout << "+++      PagedIntegralData with " << numbits << " bits and sign " << sign << std::endl;
//
//	std::getline(inFile, line);
//    /* integral data id */
//    id = ::atol(line.data());
//
//    std::cout << "+++      Integral ID: " << id << std::endl;
//
//    /* id == 0 means that there is no data - reason unknown!!! */
//    if ( id == 0 ) {
//    	std::cout << "+++  Data missing! " << std::endl;
//    	return id ;
//    }
//    if ( DataGroup::instance()->isSharedObject(id) ) {
//        std::cout << "+++    already seen this LabelEntry object" << std::endl;
//        return id;
//    }
//
//    /* is this data in this file or in separate file? */
//	std::getline(inFile, line);
//    ::sscanf(line.data(), "%d `%[^`]`", &useFile, filename ) ;
//
//    if ( useFile ) {
//        std::cout << "+++      Data is in file " <<  filename << "." << std::endl;
//        dataFile.open(filename);
//        if(!dataFile.is_open()) {
//          /* full name did not work, try just base name */
//          char *basename = strrchr( filename, '/' ) ;
//          if (basename) {
//        	  dataFile.open(basename+1);
//        	  if(!dataFile.is_open()) {
//        		  std::cerr << "   Can't open file " << filename << std::endl;
//              	  return id ;
//        	  }
//          }
//        }
//        readAuxDataFileHeader(dataFile) ;
//    } else {
//      std::swap(dataFile, inFile);
//    }
//
//    /* next two ints are reserved for paging information... */
//
//    std::getline(dataFile, line);
//    std::getline(dataFile, line);
//    ::sscanf(line.data(), "%d %d\n", &numSamples, &numBytesPerSample) ;
//
//    /* raw bytes */
//    buffer = new char[numSamples * numBytesPerSample];
//
//    dataFile.read(buffer, numBytesPerSample * numSamples);
//
//    DataGroup::instance()->setSharedObjectBuffer(id, buffer, numSamples, numBytesPerSample);
////
////    if ( infoLevel > 99 ) {
////      outputIntegralData( id ) ;
////    }
//
//    if ( useFile ) {
//      dataFile.close();
//    }
//
//    return id ;
//}
//

void Utils::readAuxDataFileHeader(std::ifstream & dataFile) {
/*
//  Read past the file comment section and data file header:
//      "HP16505A File Comment Start     "
//      "          ( zero or more        "
//      "            text lines )        "
//      "HP16505A File Comment End       "
//      "HPLogic_Additional_Data_File    "
//
//  See 'IntegralData->PagedBitBlock' or 'IntegralData->PagedIntegralData'
//  sections of online help for HPLogic Fast Binary Data File Format under
//  the File Out tool (6.5.2 or 6.6.3)
*/

    std::string line;

    do {
    	std::getline(dataFile, line);
    	if(dataFile.eof() || dataFile.fail())
    		return;
    	if(line == ID_AUX_DATA_HEADER)
    		return;
    } while(true);
}
//
//long Utils::readBitPackedData()
//{
///*
////  Read bit packed data from a file
////  See 'IntegralData->BitPackedData' section of online help for HPLogic
////  Fast Binary Data File Format under the File Out tool (6.2)
//*/
//    std::string line;
//    long id, id2 ;
//    int bytesPerLine, integralTypesPerLine ;
//    int start, width, inverted ;
//    char *buffer ;
//    int numSamples, bytesPerSample ;
//
//    std::cout << "+++      BitPackedData:" << std::endl;
//
//	std::getline(inFile, line);
//
//    /* integral data id */
//    id = ::atol(line.data());
//
//    std::cout << "+++      Integral ID: " << id << std::endl;
//
//    if ( DataGroup::instance()->isSharedObject(id) ) {
//        std::cout << "+++    already seen this LabelEntry object" << std::endl;
//        return false;
//    }
//
//	std::getline(inFile, line);
//	::sscanf(line.data(), "%d %d %d\n", &start, &width, &inverted ) ;
//
//    std::cout << "+++      start=" << start << "   width=" << width << "    inverted=" << inverted << std::endl;;
//
//    /* stuckone and stuckzero */
//	std::getline(inFile, line);
//    /* sscanf( tmpstrp, "%s %s\n", tmpstr, tmpstr ) ; */
//
//	std::getline(inFile, line);
//    /* datablock */
//    ::sscanf(line.data(), "%d %d\n", &bytesPerLine, &integralTypesPerLine ) ;
//    std::cout << "+++      bytesPerLine=" << bytesPerLine << "   integralTypesPerLine=" << integralTypesPerLine << std::endl;
//
//    GlobalSuppressDataOutput = 1 ;
//    id2 = readIntegralData(integralTypesPerLine ) ;
//    GlobalSuppressDataOutput = 0 ;
////
////    fscanf( fp, "\n" ) ;
////
////    if ( infoLevel > 99 ) {
////      /* output the data for this label */
////      outputPackedData( id2, width, start, inverted ) ;
////    }
//
//    return id ;
//}
//
//void Utils::readStringData()
//{
///*
////  Read string data from a file
////  See 'IntegralData->Strings' section of online help for HPLogic Fast
////  Binary Data File Format under the File Out tool (6.7)
//*/
//
//    std::string line;
//    long id ;
//    int length ;
//
//    std::cout << "+++      StringData: " << std::endl;
//
//	std::getline(inFile, line);
//    /* integral data id */
//    id = ::atol(line.data());
//
//    std::cout << "+++      Integral ID: " << id << std::endl;
//
//    if ( DataGroup::instance()->isSharedObject(id) ) {
//        std::cout << "+++    already seen this LabelEntry object" << std::endl;
//        return;
//    }
//
//	std::getline(inFile, line);
//    length = ::atoi(line.data());
//
//    std::cout << "+++      length=" << length << std::endl;
//
//    /* stringblock */
//    readStringBlock(length);
//}
//
//void Utils::readStringBlock(int length)
//{
///*
////  Read a string block from a file
////  See 'IntegralData->Strings->StringBlock' section of online help for
////  HPLogic Fast Binary Data File Format under the File Out tool (6.7.2)
//*/
//    unsigned int numChars ;
//    std::vector<char> buffer;
//
//    for (int i = 0; i < length; i++)
//    {
//      inFile.read((char*)&numChars, sizeof(numChars));
//      buffer.reserve(numChars);
//
//      /* raw bytes */
//      inFile.read(buffer.data(), numChars) ;
//    }
//}
//
//void Utils::readBitBlockData()
//{
///*
////  Read bitblock data object from this file
////  See 'IntegralData->BitBlockData' section of online help for HPLogic
////  Fast Binary Data File Format under the File Out tool (6.3)
//*/
//
//    std::string line;
//    long id;
//    struct ExtractorType extractor ;
//
//    std::cout << "+++      BitBlockData:" << std::endl;
//
//	std::getline(inFile, line);
//    /* integral data id */
//    id = ::atol(line.data());
//
//    std::cout << "+++      Integral ID: " << id << std::endl;
//
//    if ( DataGroup::instance()->isSharedObject(id) ) {
//        std::cout << "+++    already seen this LabelEntry object" << std::endl;
//        return;
//    }
//
//    /* extractor information for this label */
//    readExtractorInfo(&extractor) ;
//
//    /* stuckone and stuckzero */
//	std::getline(inFile, line);
//    /* sscanf( tmpstrp, "%s %s\n", tmpstr, tmpstr ) ; */
//
//    /* bitblock */
//    id = readBitBlock();
//
////    if ( infoLevel > 99 ) {
////      outputBitBlockData( id, &extractor ) ;
////    }
//
//    /* some memory cleanup... */
//
////    free( extractor.mask );
//
//    if ( extractor.reorder )
//        delete extractor.reorder;
//}
//
//void Utils::readExtractorInfo(struct ExtractorType *e)
//{
///*
////  Read an Extractor object from the given file.
////  See 'IntegralData->Label Extractor' section fo the online help
////  for Fast Binary Data File Format under the File Out tool (6.8)
//*/
//    std::string line;
//
//    int tmp ;
//    int haveReorderObject ;
//    int i ;
//    int mask ;
//
//
//    /* get the width in bit and bytes of the mask and whether it needs to
//       be inverted or not */
//	std::getline(inFile, line);
//	auto items = DataGroup::tokenizeInt(line);
//	e->widthInBytes = items.front();
//	items.pop_front();
//	e->width = items.front();
//	items.pop_front();
//	e->inverted = items.front();
//
////	fscanf( fp, "%u %u %d ", &e->widthInBytes, &e->width, &e->inverted ) ;
//
//	std::cout << "+++ Label::readExtractorInfo, line: " << line << std::endl;
//    std::cout << "+++      width=" << e->width << "  bytes=" << e->widthInBytes << "  inverted=" << e->inverted << std::endl;
//
//    /* get the label mask */
////    e->mask = (unsigned char *)malloc( e->widthInBytes ) ;
////    if ( infoLevel > 4 ) {
////        printf( "      mask:" ) ;
////    }
////    for ( i = 0 ; i < e->widthInBytes ; i++ ) {
////      fscanf( fp, "%x", &mask ) ;
////      e->mask[i] = mask ;
////      if ( infoLevel > 4 ) {
////        printf( " 0x%x", mask ) ;
////      }
////    }
////
////    if ( infoLevel > 4 ) {
////      printf( "\n" ) ;
////    }
//
//    /* a reserved field for future use */
////    fscanf( fp, "%d\n", &tmp ) ;
//
//    /* get the reorder direction - may or may not actually be used */
//	std::getline(inFile, line);
//	e->reorderDirection = ::atoi(line.data());
//	std::cout << "+++ reorderDirection " << e->reorderDirection << std::endl;
//
//    /* get the reorder structure flag */
//	std::getline(inFile, line);
//	haveReorderObject = ::atoi(line.data());
//	std::cout << "+++ haveReorderObject " << haveReorderObject << std::endl;
//
//    if ( haveReorderObject ) {
//      e->reorder = new struct ReorderType;
//      readReorderObject(e->reorder ) ;
//    } else {
//      e->reorder = nullptr ;   /* make sure pointer is cleared */
//      std::getline(inFile, line);
//    }
//}
//
//void Utils::readReorderObject(struct ReorderType *r)
//{
///*
////  Read label bit reorder information from the file
////  See 'IntegralData->Label Extractor' section of online help for HPLogic
////  Fast Binary Data File Format under the File Out tool (6.8.4)
//*/
//    std::string line;
//    int endian16, endian32, endian64, endian128, width ;
//    char buffer[256] ;
//    /* longest input2outputMapL = malloc( width * sizeof( longest )) ; */
//    /* longest output2inputMapL = malloc( width * sizeof( longest )) ; */
//    /* longest is a type that deals with numbers greater than 32 bits  */
//    int i ;
//
//	std::getline(inFile, line);
//    ::sscanf(line.data(), "Endian16: %d Endian32: %d Endian64: %d Endian128: %d Width: %d\n",
//                       &endian16, &endian32, &endian64, &endian128, &width) ;
//
//    std::cout << "+++      Bit reordering: Endian16:" << endian16 << "  32:" << endian32 << "  64:" << endian64 << "  128:" << endian128 << std::endl;
//
//    r->endian16 = endian16 ;
//    r->endian32 = endian32 ;
//    r->endian64 = endian64 ;
//    r->endian128 = endian128 ;
//    r->width = width ;
//    r->input2outputMap32 = 0;
//    r->output2inputMap32 = 0;
//
//    if ( endian16 || endian32 || endian64 || endian128 ) {
//      return;
//    }
//
//    /* not one the "standard" endian orderings, so a custom one... */
//
//    if ( width <= 32 ) {
//
//      r->input2outputMap32 = (int*)malloc( width * sizeof( int ) ) ;
//      r->output2inputMap32 =  (int*)malloc( width * sizeof( int ) ) ;
//
//      std::cout << "+++        Custom Reordering:" << std::endl;
//
//      for ( i = 0 ; i < width ; i++ ) {
//   		std::getline(inFile, line);
//        ::sscanf(line.data(), "%d %d\n", &(r->input2outputMap32[i]),
//                                                &(r->output2inputMap32[i]) ) ;
//
//      ::printf("+++                           %-#10.*x  %-#10.*x\n",
//                                        width/8, r->input2outputMap32[i],
//                                        width/8, r->output2inputMap32[i]);
//      }
//
//      return ;
//    }
//
//
//    /* custom, but greater than 32 bits wide ... */
//
//    for ( i = 0 ; i < width ; i++ ) {
//        std::getline(inFile, line);
////        fscanf( fp, "%s", buffer ) ;
////        /* input2outputMapL[i] = strtolongest( buffer, 0, 16 ) ; */
////        fscanf( fp, "%s\n", buffer ) ;
////        /* output2inputMapL[i] = strtolongest( buffer, 0, 16 ) ; */
//     }
//
//}
//
//long Utils::readBitBlock()
//{
///*
////  Read a BitBlock object from the given file.
////  See 'IntegralData->BitBlock' or 'IntegralData->PagedBitBlock' sections
////  of the online help for Fast Binary Data File Format under the File Out
////  tool (6.4 or 6.5)
//*/
//    std::string line;
//    int paged = 0 ;
//    int tmp ;
//    long id ;
//    int useFile ;
//    char filename[BUFSIZ] ;
//    int numSamples, numBytesPerSample ;
//    char * buffer ;
//    std::ifstream dataFile;
//
//	std::getline(inFile, line);
//
//    if (line == "BitBlock") {
//        paged = 0 ;
//    } else if  (line == "PagedBitBlock") {
//        paged = 1 ;
//    } else {
//        std::cerr << "Unknown BitBlock object " << line << std::endl;
//        return -1;;
//    }
//
//    /* first two ints reserved */
//	std::getline(inFile, line);
//	std::getline(inFile, line);
//
//	std::getline(inFile, line);
//    /* BitBlock id */
//    id = ::atol(line.data());
//
//    std::cout << "+++      BitBlock ID: " << id;
//
//    /* id == 0 means that there is no data - reason unknown!!! */
//    if ( id == 0 ) {
//      std::cout << "   Data missing!" << std::endl;
//      return id ;
//    }
//
//    if ( DataGroup::instance()->isSharedObject(id) ) {
//        std::cout << "    already seen this LabelEntry object" << std::endl;
//        return id;
//    }
//    std::cout << std::endl;
//
//    /* is this data in this file or in separate file? */
//	std::getline(inFile, line);
//    std::cout << "+++  Label::readBitBlock " << line << std::endl;
//
//    ::sscanf(line.data(), "%d `%[^`]`", &useFile, filename ) ;
//
//    if ( useFile ) {
//        std::cout << "+++      Data is in file " <<  filename << "." << std::endl;
//        dataFile.open(filename);
//        if(!dataFile.is_open()) {
//          /* full name did not work, try just base name */
//          char *basename = strrchr( filename, '/' ) ;
//          if (basename) {
//        	  dataFile.open(basename+1);
//        	  if(!dataFile.is_open()) {
//        		  std::cerr << "   Can't open file " << basename+1 << std::endl;
//              	  return id ;
//        	  }
//          }
//        }
//        readAuxDataFileHeader(dataFile) ;
//    } else {
//      std::swap(dataFile, inFile);
//    }
//
//    if ( paged ) {
//      /* next two ints are reserved for paging information... */
//    	std::getline(dataFile, line);
//    }
//
//	std::getline(dataFile, line);
//	::sscanf(line.data(), "%d %d\n", &numSamples, &numBytesPerSample ) ;
//
//	std::cout << "+++        Number samples: " << numSamples << "  Bytes Per Sample: " << numBytesPerSample << std::endl;
//
//    /* raw bytes */
//    buffer = new char[numSamples * numBytesPerSample];
//
//    dataFile.read(buffer, numBytesPerSample * numSamples);
//
//	std::getline(inFile, line);
//
//	DataGroup::instance()->setSharedObjectBuffer(id, buffer, numSamples, numBytesPerSample);
//
//    if ( useFile ) {
//      dataFile.close();
//    }
//
//    return id ;
//}
//
//long Utils::readIntegralArray(int numbits, bool sign, int ipl)
//{
///*
////  Read an Integral Array from a file
////  See 'IntegralData->IntegralArray' section of online help for HPLogic
////  Fast Binary Data File Format under the File Out tool (6.1)
////
////
//*/
//    std::string line;
//    long id ;
//    int len ;
//
//    char *buffer ;
//    int count, maxsize;
//    int column = 0;
//    int size = numbits / 8 ;
//
//    std::cout << "+++      IntegralArray with " << numbits << " bits and sign " << sign << std::endl;
//
//	std::getline(inFile, line);
//    /* integral data id */
//    id = ::atol(line.data());
//
//    std::cout << "+++      Integral ID: " << id << std::endl;
//
//    if ( DataGroup::instance()->isSharedObject(id) ) {
//        std::cout << "+++    already seen this LabelEntry object" << std::endl;
//        return false;
//    }
//
//	std::getline(inFile, line);
//      /* length */
//    len = ::atol(line.data());
//
//    std::cout << "+++        Length: " << len << "  Size: " << size*len << std::endl;
//
//    /* stuckone and stuckzero */
//	std::getline(inFile, line);
//    /* sscanf( tmpstr, "%s %s\n", x, y ) ; */
//
//    /* raw bytes */
//    buffer = new char[size * len];
//
//    inFile.read(buffer, size * len) ;
//
//    len = len / ipl ;    /* adjust len/size to account for integralsPerLine */
//    size = size * ipl ;  /* used by BitPackedData objects...                */
//
//    DataGroup::instance()->setSharedObjectBuffer( id, buffer, len, size ) ;
//
////    if ( infoLevel > 99 ) {
////        outputIntegralData( id ) ;
////    }
//
//    return id ;
//}
//
//
//bool Utils::readTimeTags(int pagedFlag )  {
///*
////  Read time tag information from a file
////  See 'Vertical Header->Abscissa Data Type->Time Tags' section of online
////  help for HPLogic Fast Binary Data File Format under the File Out tool (8.3)
//*/
//    std::string line;
//
//	int samples, trig ;
//	int haveData = 1 ;
//
//	std::cout << "+++  Time Tagged Data    " << pagedFlag << std::endl;
//
//	std::getline(inFile, line);
//	/* number of samples and trigger position */
//	::sscanf(line.data(), "%d %d\n", &samples, &trig ) ;
//
//	std::cout << "+++    samples = " << samples << "   trigger = " << trig << std::endl;
//
//	if ( pagedFlag ) {
//		std::getline(inFile, line);
//		std::cout << "+++  Utils::readTimeTags " << line << std::endl;
//		haveData = ::atoi(line.data());
//	}
//
//	if ( haveData ) {
//	  readIntegralData(1);
//	  std::getline(inFile, line);
//	  std::getline(inFile, line);
//	}
//
//	readAttributes(inFile, "TimeTags" );
//
//	return true;
//}

std::list<int> Utils::tokenizeInt(const std::string& aStr) {
	typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
	tokenizer tok{aStr};
	std::list<int> retList;

	for(auto& token: tok)
		retList.push_back(::atoi(token.data()));

	return retList;
}

std::list<std::string> Utils::tokenizeStr(const std::string& aStr) {
	typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
	boost::char_separator<char> sep("<>: ");
	tokenizer tok{aStr,sep};
	std::list<std::string> retList;

	for(auto& token: tok)
		retList.push_back(token);

	return retList;
}

int Utils::extractIntType(const std::string& aTypeStr, bool& isSigned) {
	static const char* TAG_UNSIGNED = "unsigned";
	static const int   TAG_UNSIGNED_LEN = ::strlen(TAG_UNSIGNED);
	static const char* TAG_SIGNED = "signed";
	static const int   TAG_SIGNED_LEN = ::strlen(TAG_SIGNED);

//	std::cout << "+++    Utils::extractIntType " << aTypeStr << std::endl;

	int bits = 0;
	auto pos = aTypeStr.find(TAG_UNSIGNED);
	if(pos != std::string::npos) {
		auto bitsStr = aTypeStr.substr(TAG_UNSIGNED_LEN);
		bits = ::atoi(bitsStr.data());
		isSigned = false;
	}
	else {
		pos = aTypeStr.find(TAG_SIGNED);
		if(pos != std::string::npos) {
			auto bitsStr = aTypeStr.substr(TAG_SIGNED_LEN);
			bits = ::atoi(bitsStr.data());
			isSigned = true;
		}
	}
	return bits;
}

