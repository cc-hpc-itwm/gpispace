#ifndef SEDA_STAGE_ALREADY_REGISTERED_HPP
#define SEDA_STAGE_ALREADY_REGISTERED_HPP 1

#include <string>
#include <seda/SedaException.hpp>

namespace seda 
{
    class StageAlreadyRegistered : public SedaException {
    public:
        StageAlreadyRegistered(const std::string& stageName) :
            SedaException("stage already registred: "+stageName),
            _stageName(stageName) {}
        virtual ~StageAlreadyRegistered() throw() { }
        
        virtual const std::string& stageName() const { return _stageName; }
    private:
        std::string _stageName;
    };
}

#endif // ! SEDA_STAGE_ALREADY_REGISTERED_HPP
