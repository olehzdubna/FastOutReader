#include <IfstreamWithState.h>

IfstreamWithState::IfstreamWithState(const std::string& dir,
                  const std::string& file) :
                        std::ifstream(file),
                        m_directoryPrefix(dir)
{
    // Do nothing
}
