// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_BITS_TRANSITION_OPTIMIZE_OPTION_HPP
#define WE_TYPE_BITS_TRANSITION_OPTIMIZE_OPTION_HPP 1

#include <boost/program_options.hpp>

#include <fhg/util/read_bool.hpp>

namespace we { namespace type {
    namespace optimize
    {
      // ******************************************************************* //

      namespace options
      {
        namespace po = boost::program_options;

        struct type
        {
        private:
          bool _not;
          bool _simple_pipe_elimination;
          bool _merge_expressions;

          std::string _Onot;
          std::string _Osimple_pipe_elimination;
          std::string _Omerge_expressions;

        public:
          type (void)
            : _not (false)
            , _simple_pipe_elimination (true)
            , _merge_expressions (true)

            , _Onot ("Onot")
            , _Osimple_pipe_elimination ("Osimple-pipe-elimination")
            , _Omerge_expressions ("Omerge-expressions")
          {}

          // *************************************************************** //

#define ACCESS(x)                                         \
        bool x (void) const { return (!_not) && _ ## x; } \
        bool & x (void) { return _ ## x; }

        ACCESS(simple_pipe_elimination)
        ACCESS(merge_expressions)
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
              ( _Omerge_expressions.c_str()
              , VAL(merge_expressions)
              , "merge consecutive expression transitions"
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
