#ifndef __PERIODIC2_H__
#define __PERIODIC2_H__

#include <Periodic.h>

class Periodic2 : public Periodic {
    public:
        Periodic2()=default;
	~Periodic2()=default;
	
	static std::shared_ptr<Periodic2> read(std::ifstream& inFile);

	int getNumerator() const {
	    return m_numerator;
	}
	int getDenominator() const {
	    return m_denominator;
	}
    private:
        int m_numerator{0};
	int m_denominator{1};
};

#endif
