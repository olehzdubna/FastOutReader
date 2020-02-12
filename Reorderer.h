/*
 * Reorderer.h
 *
 *  Created on: Jul 4, 2019
 *      Author: oleh
 */

#ifndef REORDERER_H_
#define REORDERER_H_

class Reorderer {
public:
	Reorderer();
	~Reorderer();

	static Reorderer* read(std::ifstream& inFile);

private:
	int endian16;
	int endian32;
	int endian64;
	int endian128;
	int width ;
	int* input2outputMap32;
	int* output2inputMap32;
};

#endif /* REORDERER_H_ */
