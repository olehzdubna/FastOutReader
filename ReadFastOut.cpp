/*
 * ReadFastOut.cpp
 *
 *  Created on: Jun 29, 2019
 *      Author: oleh
 */
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <string>

#include <boost/program_options.hpp>

#include <fstapi.h>

#include <DataGroup.h>
#include <DataSet.h>
#include <Label.h>

int main(int argc, char** argv) {

        std::string inFileName, outFileName;
        boost::program_options::options_description od("Usage: fastread");
	try 
	{
           od.add_options()
	   ("help,h", "Show usage")
	   ("input,i", boost::program_options::value(&inFileName),"In filename")
	   ("output,o", boost::program_options::value(&outFileName), "Out filename");
	   
	   boost::program_options::variables_map vm;
           boost::program_options::store(boost::program_options::parse_command_line(argc, argv, od), vm);
           if(vm.empty() || vm.count("help"))
	   {
               std::cout << od << std::endl;
	       return 0;
	   }
        }
	catch(const std::exception& e)
	{
	   std::cerr << "Failed to parse command-line arguments, reason: " << e.what() << std::endl;
           return -3;
	}

	std::ifstream inFile;
	inFile.open(inFileName);
	if(!inFile.is_open() || inFile.fail()) {
		std::cerr << "Cannot open input file " << inFileName << std::endl;
		return -2;
	}

	auto dataGroup = new DataGroup(inFile);
	dataGroup->process();

	std::cout << "*** DONE Reading ***" << std::endl;

	fstHandle aliasHandle = 0;

	typedef struct {
		std::string name;
		int width;
		fstHandle handle;
		Label* lbl;
	} LabelVar;

	std::vector<LabelVar> vars;
	char v[32];

	auto dataSet = dataGroup->getDataSets().front();
	std::cout << "+++  start time: " << dataSet->getTime(dataSet->getStartSample()) << ", trig time: " << dataSet->getTime(dataSet->getTrig()) << std::endl;

	auto ctx = ::fstWriterCreate(outFileName.c_str(), 1);
	fstWriterSetPackType(ctx, FST_WR_PT_LZ4);
	fstWriterSetRepackOnClose(ctx, 0);
	fstWriterSetParallelMode(ctx, 0);

	auto startTime = dataSet->getTime(dataSet->getStartSample());
//	fstWriterSetTimezero(ctx, startTime);
	fstWriterSetTimezero(ctx, 0);
	fstWriterSetTimescale(ctx, -12);
	fstWriterSetDate(ctx, "01012020");
	fstWriterSetVersion(ctx, "1.0");

	fstWriterSetScope(ctx, FST_ST_VCD_MODULE, "LA", nullptr);

	std::vector<bool> bitVec;
	LabelVar var;
	for(auto lbl: dataSet->getLabels()) {
		bitVec.clear();
		var.name = lbl->getName();
		lbl->getBits(0, bitVec);
		var.width = bitVec.size();
		std::list<char> bitChrs;
		for(auto bit: bitVec) {
			bitChrs.push_front(bit ? '1':'0');
		}

		var.handle = fstWriterCreateVar(ctx, FST_VT_VCD_WIRE, FST_VD_IMPLICIT, var.width, var.name.data(), aliasHandle);

		int i = bitVec.size();
		for(auto bit: bitVec) {
			v[--i] = bit ? '1':'0';
		}

		var.lbl = lbl;
		vars.push_back(var);
		lbl->clear();
	}

	fstWriterSetUpscope(ctx);

//	for(int timIdx = dataSet->getStartSample(); timIdx < 100; timIdx++)
	for(int timIdx = dataSet->getStartSample(); timIdx < dataSet->getLastSample(); timIdx++)
	{
//		std::cout << "time " << dataSet->getTime(timIdx) - startTime << std::endl;
		fstWriterEmitTimeChange(ctx, dataSet->getTime(timIdx) - startTime);

		for(auto& var: vars) {
			bitVec.clear();
			if(var.lbl->getBits(timIdx, bitVec)) {
//
//				std::cout << "Lbl " << var.name << ", bits ";
//
				int i = bitVec.size();
				for(auto bit: bitVec) {
					v[--i] = bit ? '1':'0';
//					std::cout << v[i] << " ";
				}
				fstWriterEmitValueChange(ctx, var.handle, &v[0]);
//				std::cout << std::endl;
			}
		}
	}

	::fstWriterClose(ctx);

	delete dataGroup;

	return 0;
}


