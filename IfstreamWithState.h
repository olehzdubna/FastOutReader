#ifndef __IFSTREAMWITHSTATE_H__
#define __IFSTREAMWITHSTATE_H__

#include <string>
#include <fstream>

class IfstreamWithState : public std::ifstream
{
    public:
        IfstreamWithState(const std::string& dir,
                          const std::string& file);
	const std::string& getDirectoryPrefix() const
	{
	    return m_directoryPrefix;
	}
    private:
       std::string m_directoryPrefix;
};

#endif
