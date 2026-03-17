#include <gspc/util/ostream/modifier.hpp>

#include <ostream>
#include <sstream>



    namespace gspc::util::ostream
    {
      std::ostream& operator<< (std::ostream& os, const modifier& m)
      {
        return m (os);
      }
      std::string modifier::string() const
      {
        std::ostringstream oss;
        oss << *this;
        return oss.str();
      }
    }
