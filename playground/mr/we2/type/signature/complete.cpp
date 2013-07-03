// mirko.rahn@itwm.fraunhofer.de

#include <we2/type/signature/complete.hpp>

#include <we2/type/value/name.hpp>

#include <set>
#include <iostream>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        std::set<std::string> init_typenames_complete()
        {
          std::set<std::string> tn;

          tn.insert (value::CONTROL());
          tn.insert (value::BOOL());
          tn.insert (value::INT());
          tn.insert (value::LONG());
          tn.insert (value::UINT());
          tn.insert (value::ULONG());
          tn.insert (value::FLOAT());
          tn.insert (value::DOUBLE());
          tn.insert (value::CHAR());
          tn.insert (value::STRING());
          tn.insert (value::LIST());
          tn.insert (value::SET());
          tn.insert (value::MAP());

          return tn;
        }

        bool is_complete (const std::string& tname)
        {
          static std::set<std::string> tn (init_typenames_complete());

          return tn.find (tname) != tn.end();
        }
      }

      complete::complete (const std::string& tname)
        : _tname (tname)
      {}
      const std::string& complete::tname() const
      {
        return _tname;
      }
      std::ostream& operator<< (std::ostream& os, const complete& c)
      {
        os << c.tname();
        if (not is_complete (c.tname()))
          {
            os << "::type";
          }
        return os;
      }
    }
  }
}
