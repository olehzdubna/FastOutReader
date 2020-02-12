/*
 * Periodic.cpp
 *
 *  Created on: Jul 6, 2019
 *      Author: oleh
 */

#include <iostream>

#include <Utils.h>
#include <Periodic.h>

Periodic::Periodic()
 : AxisX()
 , samples(0)
 , trig(0)
 , firstTime(0)
 , incrementTime(0) {
}

Periodic::~Periodic() {
}

/*
//  Read periodic information from a file
//  See 'Vertical Header->Abscissa Data Type->Perodic' section of online
//  help for HPLogic Fast Binary Data File Format under the File Out tool (8.2)
*/
Periodic* Periodic::read(std::ifstream& inFile) {
	std::string line;

	std::cout << "+++  Periodic Data" << std::endl;

	auto periodic = new Periodic();

	/* number of samples and trigger position */
	std::getline(inFile, line);
	::sscanf(line.data(), "%d %d\n", &periodic->samples, &periodic->trig ) ;

	std::cout << "+++    samples = " << periodic->samples << "   trigger = " << periodic->trig << std::endl;

	/* Time of first sample sample */
	/* Time between samples */
	std::getline(inFile, line);
	::sscanf(line.data(), "%d %d\n", &periodic->firstTime, &periodic->incrementTime ) ;
	std::cout << "+++    origin = " << periodic->firstTime << " ps,   increment = " << periodic->incrementTime << std::endl;

	Utils::readAttributes(inFile, "Periodic");

	return periodic;
}
