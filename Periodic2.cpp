#include <Periodic2.h>
#include <Utils.h>

std::shared_ptr<Periodic2> Periodic2::read(std::ifstream& inFile) {
    auto periodic = Periodic::read(inFile);
    // Read fractinal component of period
    auto periodic2 = std::make_shared<Periodic2>();
    periodic2->update(periodic->getSamples(), periodic->getTrig(), 
                     periodic->getFirstTime(), periodic->getIncrementTime());

    std::string line;
    std::getline(inFile, line);
    ::scanf(line.data(), "%d %d\n", &periodic2->m_numerator, &periodic2->m_denominator);

    Utils::readAttributes(inFile, "Periodic2");
    return periodic2;
}
