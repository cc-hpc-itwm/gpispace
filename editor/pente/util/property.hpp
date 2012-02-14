// mirko.rahn@itwm.fhg.de

#ifndef _FHG_PNETE_UTIL_PROPERTY_HPP
#define _FHG_PNETE_UTIL_PROPERTY_HPP 1

namespace fhg
{
  namespace pnete
  {
    namespace util
    {
      namespace property
      {
        class setter
        {
        private:
          ::we::type::property::path_type _path;

        public:
          explicit setter (const ::we::type::property::key_type& key)
            : _path (::we::type::property::util::split ("fhg.pnete." + key))
          {}

          template<typename T>
          void set (::we::type::property::type* prop, T x)
          {
            if (prop)
              {
                prop->set
                  ( _path.begin()
                  , _path.end()
                  , boost::lexical_cast< ::we::type::property::value_type> (x)
                  );
              }
          }
        };
      }
    }
  }
}

#endif
