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
          bool _simplify_expression_sequences;

          std::string _Onot;
          std::string _Osimple_pipe_elimination;
          std::string _Omerge_expressions;
          std::string _Osimplify_expression_sequences;

        public:
          type (void)
            : _not (false)
            , _simple_pipe_elimination (true)
            , _merge_expressions (true)
            , _simplify_expression_sequences (true)

            , _Onot ("Onot")
            , _Osimple_pipe_elimination ("Osimple-pipe-elimination")
            , _Omerge_expressions ("Omerge-expressions")
            , _Osimplify_expression_sequences ("Osimplify-expression-sequences")
          {}

          // *************************************************************** //

#define ACCESS(x)                                         \
        bool x (void) const { return (!_not) && _ ## x; } \
        bool & x (void) { return _ ## x; }

        ACCESS(simple_pipe_elimination)
        ACCESS(merge_expressions)
        ACCESS(simplify_expression_sequences)
#undef ACCESS

          // *************************************************************** //

          void add_options (po::options_description & desc)
          {
#define VAL(x) po::value<bool>(&_ ## x)->default_value (_ ## x) \
                                       ->implicit_value(true)

            po::options_description optimize ("Optimization");

            optimize.add_options ()
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
              ( _Osimplify_expression_sequences.c_str()
              , VAL(simplify_expression_sequences)
              , "simplify expression sequences, e.g. dead code elimination"
              )
              ;

            desc.add (optimize);
#undef VAL
          }
        };
      }
    }
  }
}

#endif
