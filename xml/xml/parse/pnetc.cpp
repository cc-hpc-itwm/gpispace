// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/parser.hpp>

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <we/we.hpp>

#include <fhg/revision.hpp>

#include <xml/parse/headerlist.hpp>
#include <xml/parse/headergen.hpp>

// ************************************************************************* //

namespace po = boost::program_options;

namespace {
  template<typename Stream>
  class wrapping_word_stream
  {
  private:
    const std::size_t _max_len;
    mutable std::size_t _len;
    Stream& _stream;
  public:
    wrapping_word_stream (Stream& stream, const std::size_t max = 75)
      : _max_len (max)
      , _len (0)
      , _stream (stream)
    {}

    void put (const std::string& w) const
    {
      if (_len + w.size() > _max_len)
        {
          _stream << " \\"; newl(); append (" ");
        }
      else if (_len > 0)
        {
          append (" ");
        }

      append (w);
    }

    void append (const std::string& s) const
    {
      _stream << s;

      _len += s.size();
    }

    void newl () const
    {
      _stream << std::endl;

      _len = 0;
    }
  };

  class quote
  {
  private:
    std::string _quoted;

  public:
    quote (const std::string& s) : _quoted ()
    {
      std::string::const_iterator pos (s.begin());
      const std::string::const_iterator end (s.end());

      while (pos != end)
        {
          switch (*pos)
            {
            case ' ': _quoted += "\\ "; break;
            case '$': _quoted += "$$"; break;
            default: _quoted += *pos; break;
            }

          ++pos;
        }
    };

    operator const std::string& () const { return _quoted; }
  };


  std::string quote_for_list (const std::string& s)
  {
    std::string quoted;
    std::string::const_iterator pos (s.begin());
    const std::string::const_iterator end (s.end());

    while (pos != end)
      {
        switch (*pos)
          {
          case ' ': quoted += "\\ "; break;
          default: quoted += *pos; break;
          }

        ++pos;
      }

    return quoted;
  }

  std::string quote_for_list (const boost::filesystem::path& p)
  {
    return quote_for_list (p.string());
  }

  template<typename Stream>
  void write_dependencies ( const xml::parse::state::type& state
                          , const std::string& input
                          , Stream& stream
                          )
  {
    wrapping_word_stream<Stream> wrapping_stream (stream);

    if (state.dependencies_target().size() > 0)
      {
        BOOST_FOREACH ( const std::string& target
                      , state.dependencies_target()
                      )
          {
            wrapping_stream.put (target);
          }
      }

    if (state.dependencies_target_quoted().size() > 0)
      {
        BOOST_FOREACH ( const std::string& target
                      , state.dependencies_target_quoted()
                      )
          {
            wrapping_stream.put (quote (target));
          }
      }

    if (  (state.dependencies_target().size() == 0)
       && (state.dependencies_target_quoted().size() == 0)
       )
      {
        wrapping_stream.put (input);
      }

    wrapping_stream.append (":");

    BOOST_FOREACH ( const boost::filesystem::path& path
                  , state.dependencies()
                  )
      {
        const std::string& dep (path.string());

        if (dep != input)
          {
            wrapping_stream.put (dep);
          }
      }

    wrapping_stream.newl();

    if (state.dependencies_add_phony_targets())
      {
        BOOST_FOREACH ( const boost::filesystem::path& path
                      , state.dependencies()
                      )
          {
            const std::string& dep (path.string());

            if (dep != input)
              {
                wrapping_stream.newl();
                wrapping_stream.append (dep);
                wrapping_stream.append(":");
                wrapping_stream.newl();
              }
          }
      }
  }
} // anonymous namespace

namespace xml
{
  namespace parse
  {
    //! \todo move stuff to anonymous namespace

    // ********************************************************************* //

    type::function_type
    frontend (state::type & state, const std::string & input)
    {
      type::function_type f (just_parse (state, input));

      if (state.dump_xml_file().size() > 0)
        {
          const std::string& file (state.dump_xml_file());
          std::ofstream stream (file.c_str());

          if (!stream.good())
            {
              throw error::could_not_open_file (file);
            }

          fhg::util::xml::xmlstream s (stream);

          xml::parse::type::dump::dump (s, f, state);
        }

      // set all the collected requirements to the top level function
      f.requirements = state.requirements();

      f.specialize (state);
      f.resolve (state, f.forbidden_below());
      f.type_check (state);
      f.sanity_check (state);

      if (state.path_to_cpp().size() > 0)
        {
          type::fun_info_map m;

          type::find_module_calls (state, f, m);

          type::mk_wrapper (state, m);
          type::mk_makefile (state, m);

          includes::descrs_type descrs;

          includes::mks (descrs);
          includes::we_header_gen (state, descrs);

          type::struct_to_cpp (state, f);
        }

      if (state.dump_dependenciesD())
        {
          state.dump_dependencies() = input + ".d";
        }

      if (state.dump_dependencies().size() > 0)
        {
          const std::string& file (state.dump_dependencies());
          std::ofstream stream (file.c_str());

          if (!stream.good())
            {
              throw error::could_not_open_file (file);
            }

          write_dependencies (state, input, stream);
        }

      if (state.list_dependencies().size() > 0)
        {
          const std::string& file (state.list_dependencies());
          std::ofstream stream (file.c_str());

          if (not stream)
            {
              throw error::could_not_open_file (file);
            }

          stream << quote_for_list (input) << std::endl;

          BOOST_FOREACH (const boost::filesystem::path& p, state.dependencies())
            {
              stream << quote_for_list(p) << std::endl;
            }
        }

      return f;
    }
  } // namespace parse
} // namespace xml

int
main (int argc, char ** argv)
{
  std::string input ("/dev/stdin");
  std::string output ("/dev/stdout");
  bool xml (false);

  po::options_description desc("General");

  desc.add_options()
    ( "help,h", "this message")
    ( "version,V", "print version information")
    ( "input,i"
    , po::value<std::string>(&input)->default_value(input)
    , "input file name, - for stdin, first positional parameter"
    )
    ( "output,o"
    , po::value<std::string>(&output)->default_value(output)
    , "output file name, - for stdout, second positional parameter"
    )
    ( "xml,x"
    , po::value<bool>(&xml)->default_value(xml)->implicit_value(true)
    , "write xml instead of text format"
    )
    ;

  xml::parse::state::type state;

  state.add_options (desc);

  po::positional_options_description p;
  p.add("input", 1).add("output",2);

  po::variables_map vm;

  try
  {
    po::store( po::command_line_parser(argc, argv)
             . options(desc).positional(p)
             . extra_parser (xml::parse::state::reg_M)
             . run()
             , vm
             );
    po::notify(vm);
  }
  catch (std::exception const & ex)
  {
    std::cerr << "invalid argument: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count("help"))
    {
      std::cout << argv[0] << ": the petri net compiler" << std::endl;

      std::cout << desc << std::endl;

      return EXIT_SUCCESS;
    }

  if (vm.count("version"))
    {
      std::cout << fhg::project_info();

      return EXIT_SUCCESS;
    }

  if (input == "-")
    {
      input = "/dev/stdin";
    }

  if (output == "-")
    {
      output = "/dev/stdout";
    }

  try
  {
    xml::parse::type::function_type f (xml::parse::frontend (state, input));

    we::transition_t trans (f.synthesize (state));

    we::type::optimize::optimize (trans, state.options_optimize());

    // WORK HERE: The xml dump from the transition
#if 0
    {
      std::ofstream stream ("/dev/stderr");
      fhg::util::xml::xmlstream s (stream);

      we::type::dump::dump (s, trans);
    }
#endif

    const we::activity_t act (trans);


    std::ofstream out (output.c_str());

    out << ( xml
           ? we::util::xml_codec::encode (act)
           : we::util::text_codec::encode (act)
           )
      ;
  }
  catch (std::exception const & ex)
  {
    std::cerr << "pnetc: failed: " << ex.what() << std::endl;

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
