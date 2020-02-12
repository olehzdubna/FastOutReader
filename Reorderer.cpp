/*
 * Reorderer.cpp
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */
#include <string>
#include <iostream>
#include <fstream>

#include <Reorderer.h>

Reorderer::Reorderer() {
}

Reorderer::~Reorderer() {
}

/*
//  Read label bit reorder information from the file
//  See 'IntegralData->Label Extractor' section of online help for HPLogic
//  Fast Binary Data File Format under the File Out tool (6.8.4)
*/
Reorderer* Reorderer::read(std::ifstream& inFile) {
	std::string line;
    int endian16, endian32, endian64, endian128, width ;
    char buffer[256] ;
    /* longest input2outputMapL = malloc( width * sizeof( longest )) ; */
    /* longest output2inputMapL = malloc( width * sizeof( longest )) ; */
    /* longest is a type that deals with numbers greater than 32 bits  */

    Reorderer* reorderer = new Reorderer();

	std::getline(inFile, line);
    ::sscanf(line.data(), "Endian16: %d Endian32: %d Endian64: %d Endian128: %d Width: %d\n",
                       &endian16, &endian32, &endian64, &endian128, &width) ;

    std::cout << "+++      Bit reordering: Endian16:" << endian16 << "  32:" << endian32 << "  64:" << endian64 << "  128:" << endian128 << std::endl;

    reorderer->endian16 = endian16 ;
    reorderer->endian32 = endian32 ;
    reorderer->endian64 = endian64 ;
    reorderer->endian128 = endian128 ;
    reorderer->width = width ;
    reorderer->input2outputMap32 = 0;
    reorderer->output2inputMap32 = 0;

    if ( endian16 || endian32 || endian64 || endian128 ) {
      return nullptr;
    }

    /* not one the "standard" endian orderings, so a custom one... */

    if ( width <= 32 ) {
    	reorderer->input2outputMap32 = (int*)malloc( width * sizeof( int ) ) ;
    	reorderer->output2inputMap32 =  (int*)malloc( width * sizeof( int ) ) ;

      std::cout << "+++        Custom Reordering:" << std::endl;

      for (int i = 0 ; i < width ; i++ ) {
   		std::getline(inFile, line);
        ::sscanf(line.data(), "%d %d\n", &(reorderer->input2outputMap32[i]),
                                                &(reorderer->output2inputMap32[i]) ) ;

      ::printf("+++                           %-#10.*x  %-#10.*x\n",
                                        width/8, reorderer->input2outputMap32[i],
                                        width/8, reorderer->output2inputMap32[i]);
      }

      return reorderer;
    }


    /* custom, but greater than 32 bits wide ... */

    for (int i = 0 ; i < width ; i++ ) {
        std::getline(inFile, line);
//        fscanf( fp, "%s", buffer ) ;
//        /* input2outputMapL[i] = strtolongest( buffer, 0, 16 ) ; */
//        fscanf( fp, "%s\n", buffer ) ;
//        /* output2inputMapL[i] = strtolongest( buffer, 0, 16 ) ; */
     }

    return reorderer;
}
