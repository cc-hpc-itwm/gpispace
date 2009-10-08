#ifndef SEDA_STAGE_NOT_FOUND_HPP
#define SEDA_STAGE_NOT_FOUND_HPP 1

#include <string>
#include <seda/SedaException.hpp>

namespace seda 
{
    class StageNotFound : public SedaException {
    public:
        StageNotFound(const std::string& stage) :
            SedaException("stage could not be found: "+stage),
            _stageName(stage) {}
        virtual ~StageNotFound() throw() { }
        
        virtual const std::string& stageName() const { return _stageName; }
    private:
        std::string _stageName;
    };
}

#endif // ! SEDA_STAGE_NOT_FOUND_HPP
