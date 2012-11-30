// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/require.hpp>

#include <boost/foreach.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      requirements_type::requirements_type()
        : _map (0)
      { }

      void requirements_type::set ( const require_key_type & key
                                  , const bool & mandatory
                                  )
      {
        map_type::iterator pos (_map.find (key));

        if (pos != _map.end())
        {
          pos->second |= mandatory;
        }
        else
        {
          _map[key] = mandatory;
        }
      }

      requirements_type::const_iterator requirements_type::begin () const
      {
        return _map.begin();
      }
      requirements_type::const_iterator requirements_type::end () const
      {
        return _map.end();
      }

      void requirements_type::join (const requirements_type& reqs)
      {
        BOOST_FOREACH (const map_type::value_type& req, reqs)
        {
          set (req.first, req.second);
        }
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const requirements_type & cs
                  )
        {
          for ( requirements_type::const_iterator cap (cs.begin())
              ; cap != cs.end()
              ; ++cap
              )
          {
            s.open ("require");
            s.attr ("key", cap->first);
            s.attr ("mandatory", cap->second);
            s.close ();
          }
        }
      }
    }
  }
}
