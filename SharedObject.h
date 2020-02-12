/*
 * SharedObject.h
 *
 *  Created on: Jun 30, 2019
 *      Author: oleh
 */

#ifndef SHAREDOBJECT_H_
#define SHAREDOBJECT_H_

class SharedObject {
public:
	SharedObject(long anId)
	: id(anId) {}
	virtual ~SharedObject() {}

	long getId()
	{return id;}

private:
	long id;
};

#endif /* SHAREDOBJECT_H_ */
