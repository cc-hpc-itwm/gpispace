// mirko.rahn@itwm.fraunhofer.de

#ifndef FHG_UTIL_REMOVE_PREFIX_HPP
#define FHG_UTIL_REMOVE_PREFIX_HPP

#include <string>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    class remove_prefix_failed : public std::runtime_error
    {
    public:
      remove_prefix_failed (const std::string word, const std::string prefix);
      virtual ~remove_prefix_failed() throw() { }

      const std::string& word() const { return _word; }
      const std::string& prefix() const { return _prefix; }
    private:
      const std::string _word;
      const std::string _prefix;
    };

    std::string remove_prefix ( const std::string& prefix
                              , const std::string& word
                              );
  }
}

#endif
