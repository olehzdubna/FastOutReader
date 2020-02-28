/*
 * AxisX.cpp
 *
 *  Created on: Jul 6, 2019
 *      Author: oleh
 */
#include <iostream>

#include <AxisX.h>
#include <TimeTags.h>
#include <Periodic.h>

std::shared_ptr<AxisX> AxisX::read(std::ifstream& inFile)
{
/*
//  Read X axis information from a file
//  See section 'Vertical Header->Abscissa Data Type' of online help for
//  HPLogic Fast Binary Data File Format under the File Out tool (8.0)
*/
	std::string line;
	std::getline(inFile, line);

	//TODO: for debug std::cout << "+++ DataSet::readXaxis " << line << std::endl;

    if (line == "AbscissaData") {
        return readAbscissaData();
    } else if  (line == "Periodic") {
        return Periodic::read(inFile);
    } else if  (line == "Periodic2") {
        return readPeriodic2();
    } else if  (line == "TimeTags") {
        return TimeTags::read(inFile, 0);
    } else if  (line == "PagedTimeTags") {
        return TimeTags::read(inFile, 1);
    }

    std::cerr << "Unknown Abscissa Data Type " << line << std::endl;
    return std::shared_ptr<AxisX>();
}

std::shared_ptr<AxisX> AxisX::readAbscissaData() {
	return std::shared_ptr<AxisX>();
}

std::shared_ptr<AxisX> AxisX::readPeriodic2() {
	return std::shared_ptr<AxisX>();
}
