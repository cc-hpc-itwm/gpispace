// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_BITS_TRANSITION_OPTIMIZE_OPTION_HPP
#define WE_TYPE_BITS_TRANSITION_OPTIMIZE_OPTION_HPP 1

#include <we/type/property.hpp>

#include <boost/program_options.hpp>

#include <fhg/util/read_bool.hpp>

namespace we { namespace type {
    namespace optimize
    {
      // ******************************************************************* //

      namespace options
      {
        namespace po = boost::program_options;
        namespace property = we::type::property;

        struct type
        {
        private:
          bool _not;
          bool _simple_pipe_elimination;

          std::string _Onot;
          std::string _Osimple_pipe_elimination;

        public:
          type (void)
            : _not (false)
            , _simple_pipe_elimination (true)

            , _Onot ("Onot")
            , _Osimple_pipe_elimination ("Osimple-pipe-elimination")
          {}

          // *************************************************************** //

          bool property ( const property::path_type & path
                        , const property::value_type & value
                        )
          {
            if (path.size() != 1)
              {
                return false;
              }

#define GET_PROP(x)                                                       \
            else if (path.size() == 1 && path[0] == _O ## x)              \
              {                                                           \
                _ ## x = fhg::util::read_bool (value); return true;       \
              }

            GET_PROP(simple_pipe_elimination)

#undef GET_PROP
            else
              {
                return false;
              }
          }

          // *************************************************************** //

#define ACCESS(x)                                         \
        bool x (void) const { return (!_not) && _ ## x; } \
        bool & x (void) { return _ ## x; }

        ACCESS(simple_pipe_elimination)
#undef ACCESS

          // *************************************************************** //

          void add_options (po::options_description & desc)
          {
#define VAL(x) po::value<bool>(&_ ## x)->default_value (_ ## x)

            desc.add_options ()
              ( _Onot.c_str()
              , VAL(not)
              , "disable all optimizations"
              )
              ( _Osimple_pipe_elimination.c_str()
              , VAL(simple_pipe_elimination)
              , "eliminate simple pipeline transitions"
              )
              ;
#undef VAL
          }
        };
      }
    }
  }
}

#endif
