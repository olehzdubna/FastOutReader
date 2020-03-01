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
#include <thread>
#include <chrono>

#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>

#include <fstapi.h>

#include <DataGroup.h>
#include <DataSet.h>
#include <Label.h>
#include <IfstreamWithState.h>

#include "terminal.hpp"
#include "style.hpp"
#include "image.hpp"
#include "reflow.hpp"
#include "components/text.hpp"
#include "components/stacklayout.hpp"
#include "components/flowlayout.hpp"
#include "components/progress.hpp"
#include "components/maxwidth.hpp"

using namespace std::chrono_literals;

int main(int argc, char** argv) {

        std::string inFileName, outFileName, gtkwavePath;
        boost::program_options::options_description od("Usage: fastread");
	boost::program_options::variables_map vm;
	try 
	{
           od.add_options()
	   ("help,h", "Show usage")
	   ("input,i", boost::program_options::value<std::string>(&inFileName), "In filename")
	   ("output,o", boost::program_options::value<std::string>(&outFileName), "Out filename")
           ("with-gtkwave,g", boost::program_options::value<std::string>(&gtkwavePath), "Feed output to gtkwave located elsewhere");

           boost::program_options::store(boost::program_options::parse_command_line(argc, argv, od), vm);
	   boost::program_options::notify(vm);
           if(vm.empty() || vm.count("help"))
	   {
               std::cout << od << std::endl;
	       return 0;
	   }
	   if(inFileName.empty() || outFileName.empty())
	   {
	       throw std::runtime_error("Both input and output filepaths are required"); 
	   }
        }
	catch(const std::exception& e)
	{
	   std::cerr << "Failed to parse command-line arguments, reason: " << e.what() << std::endl;
	   std::cout << od << std::endl;
           return -3;
	}

        boost::filesystem::path filepath(inFileName);
	const auto& dirPrefix = filepath.parent_path().native();
        auto inFile = std::make_unique<IfstreamWithState>(dirPrefix, inFileName);
	if(!inFile->is_open() || inFile->fail()) {
		std::cerr << "Cannot open input file " << inFileName << std::endl;
		return -2;
	}

	auto dataGroup = std::make_unique<DataGroup>(*inFile.get());
	dataGroup->process();

	std::cout << "*** DONE Reading ***" << std::endl;

	fstHandle aliasHandle = 0;

	typedef struct {
		std::string name;
		int width;
		fstHandle handle;
		std::shared_ptr<Label> lbl;
	} LabelVar;

	std::vector<LabelVar> vars;
	char v[32];

	auto dataSet = dataGroup->getDataSets().front();
	//TODO: for debug std::cout << "+++  start time: " << dataSet->getTime(dataSet->getStartSample()) << ", trig time: " << dataSet->getTime(dataSet->getTrig()) << std::endl;

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
        
	std::atomic_int sampleCount(dataSet->getLastSample() * vars.size());
	double progressNormalizationExponent = std::floor(std::log10(sampleCount.load()));
	std::atomic_ullong progressNormalizationFactor(std::pow(10,progressNormalizationExponent));
        std::atomic_int progressIdx(0);
	std::thread progressThread([&progressIdx, &sampleCount, &progressNormalizationFactor]()
	{
            const auto& renderToTerm = [](auto const& vt, unsigned const w, rxterm::Component const& c) {
                return vt.flip(c.render(w).toString());
             };
              
	     //TODO: Need to get actual terminal width, for now hard-coding to 80 
	     const std::size_t terminalWidth = 80;
	     //TODO: Possibly need to make delay configurable
             const auto updateDelay = 200ms;

             // construct 3-segment progress bar
             const auto& progressBar = [](auto x, auto y, auto z) -> rxterm::FlowLayout<> 
	     {
                 return {
                            rxterm::Text("Processing samples..."),
                            rxterm::FlowLayout<>{
                            rxterm::MaxWidth(20, rxterm::Progress(x)),
                            rxterm::MaxWidth(20, rxterm::Progress(y)),
                            rxterm::MaxWidth(20, rxterm::Progress(z))
                        }
                 };
             };

             const double normFactor = static_cast<double>(progressNormalizationFactor.load()); 
             rxterm::VirtualTerminal vt;
	     while(progressIdx.load() < sampleCount.load())
	     {
	        vt = renderToTerm(vt, terminalWidth, progressBar(
		                                                 (1 / normFactor) * progressIdx, 
		                                                 (2 / normFactor) * progressIdx, 
							         (3 / normFactor) * progressIdx));
		std::this_thread::sleep_for(updateDelay);
	     }
	});

	int timIdx = dataSet->getStartSample();
//	for(int timIdx = dataSet->getStartSample(); timIdx < 100; timIdx++)
	while(timIdx < dataSet->getLastSample())
	{
		//std::cout << "time " << dataSet->getTime(timIdx) - startTime << std::endl;
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
		        progressIdx.store(progressIdx.load()+1);
		}
		progressIdx.store(progressIdx.load()+1);
		++timIdx;
	}
        progressThread.join();

	::fstWriterClose(ctx);

	std::string commandStr = (!gtkwavePath.empty() ? gtkwavePath : "gtkwave");
        commandStr += " " + outFileName;  
	if(std::system(commandStr.c_str()) != 0)
	{
            std::cerr << "***WARNING*** Could not resolve gtkwave path = " << commandStr 
	              << ", defaulting to binary built from repository" << std::endl; 
            commandStr = "./gtkwave3 " + outFileName;
            if(std::system(commandStr.c_str()) != 0)
	    {
                std::cerr << "***ERROR*** Failed to start " << commandStr << std::endl;
	        return -4;
	    }
	}
	return 0;
}
