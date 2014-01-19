// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/parse/parser.hpp>
#include <we/type/activity.hpp>
#include <we/type/id.hpp>
#include <we/type/net.hpp>
#include <we/type/signature/show.hpp>
#include <we/type/transition.hpp>
#include <we/type/value/show.hpp>

#include <fhg/revision.hpp>
#include <fhg/util/starts_with.hpp>
#include <fhg/util/indenter.hpp>
#include <fhg/util/read_bool.hpp>

#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/map.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

namespace
{
  namespace shape
  {
    static std::string const condition ("record");
    static std::string const port_in ("house");
    static std::string const port_out ("invhouse");
    static std::string const port_tunnel ("ellipse");
    static std::string const expression ("none");
    static std::string const modcall ("box");
    static std::string const place ("ellipse");
  }

  namespace color
  {
    static std::string const internal ("white");
    static std::string const external ("dimgray");
    static std::string const modcall ("yellow");
    static std::string const expression ("white");
    static std::string const node ("white");
    static std::string const subnet_internal ("grey");
  }

  namespace style
  {
    static std::string const association ("dotted");
    static std::string const read_connection ("dashed");
  }

  typedef unsigned long id_type;

  static const std::string endl ("\\n");
  static const std::string arrow (" -> ");

  std::string parens ( const std::string& s
                     , const std::string open
                     , const std::string close
                     )
  {
    return open + s + close;
  }

  std::string brackets (const std::string& s)
  {
    return " " + parens (s, "[", "]");
  }

  std::string props (const std::string& s)
  {
    return parens (s, ":: ", " ::");
  }

  std::string dquote (const std::string& s)
  {
    return parens (s, "\"", "\"");
  }

  std::string lines (const char& b, const std::string& s)
  {
    std::string l;

    std::string::const_iterator pos (s.begin());
    const std::string::const_iterator end (s.end());

    while (pos != end)
    {
      if (*pos == b)
      {
        ++pos;

        while (pos != end && (isspace (*pos) || *pos == b))
        {
          ++pos;
        }

        l += endl;
      }
      else
      {
        l += *pos;

        ++pos;
      }
    }

    return l;
  }

  std::string quote (const char& c)
  {
    switch (c)
    {
    case '{': return "\\{";
    case '}': return "\\}";
    case '>': return "\\>";
    case '<': return "\\<";
    case '"': return "\\\"";
    case '|': return "\\|";
    default: return std::string (1, c);
    }
  }

  std::string quote (const std::string& s)
  {
    std::string q;

    for (std::string::const_iterator pos (s.begin()); pos != s.end(); ++pos)
    {
      q += quote (*pos);
    }

    return lines (';', q);
  }

  std::string keyval (const std::string& key, const std::string& val)
  {
    return key + " = " + dquote (val);
  }

  std::string name (const id_type& id, const std::string& _name)
  {
    std::ostringstream s;

    s << "n" << id << "_" << _name;

    return s.str();
  }

  std::string bgcolor (const std::string& color)
  {
    return keyval ("bgcolor", color);
  }

  std::string node (const std::string& shape, const std::string& label)
  {
    return brackets ( keyval ("shape", shape)
                    + ", "
                    + keyval ("label", label)
                    + ", "
                    + keyval ("style", "filled")
                    + ", "
                    + keyval ("fillcolor", color::node)
                    );
  }

  std::string association()
  {
    return brackets ( keyval ("style", style::association)
                    + ", "
                    + keyval ("dir", "none")
                    );
  }

  class options
  {
  public:
    bool full;
    std::list<boost::function <bool (we::type::transition_t const&)> >
      filter;
    bool show_token;
    bool show_signature;
    bool show_priority;
    bool show_intext;
    bool show_virtual;
    bool show_tunnel_connection;

    bool should_be_expanded (const we::type::transition_t& x) const
    {
      BOOST_FOREACH
        (boost::function <bool (we::type::transition_t const&)> f, filter)
      {
        if (f (x))
        {
          return false;
        }
      }

      return true;
    }

    options()
      : full (false)
      , filter()
      , show_token (true)
      , show_signature (true)
      , show_priority (false)
      , show_intext (false)
      , show_virtual (false)
      , show_tunnel_connection (true)
    {}
  };

  std::string with_signature ( const std::string& name
                             , const pnet::type::signature::signature_type& sig
                             , const options& opts
                             )
  {
    std::ostringstream s;

    s << name;

    if (opts.show_signature)
    {
      s << endl << pnet::type::signature::show (sig);
    }

    return s.str();
  }

  std::string to_dot
    ( const we::type::transition_t&
    , id_type&
    , const options&
    , fhg::util::indenter&
    , boost::optional<we::type::activity_t::input_t>
    , boost::optional<we::type::activity_t::output_t>
    );

  class visit_transition : public boost::static_visitor<std::string>
  {
  private:
    id_type& id;
    fhg::util::indenter& _indent;
    const options& opts;

  public:
    visit_transition
      (id_type& _id, fhg::util::indenter& indent, const options& _opts)
      : id (_id)
      , _indent (indent)
      , opts (_opts)
    {}

    std::string operator() (const we::type::expression_t& expr) const
    {
      std::ostringstream s;

      s << _indent
        << name (id, "expression")
        << node (shape::expression, quote (boost::lexical_cast<std::string> (expr)));

      return s.str();
    }

    std::string operator() (const we::type::module_call_t& mod_call) const
    {
      std::ostringstream s;

      s << _indent
        << name (id, "modcall")
        << node (shape::modcall, boost::lexical_cast<std::string> (mod_call));

      return s.str();
    }

    std::string operator() (const we::type::net_type& net) const
    {
      std::ostringstream s;

      const id_type id_net (id);

      s << _indent << "subgraph cluster_net_" << id_net << " {";

      typedef std::pair<we::place_id_type, place::type> ip_type;

      BOOST_FOREACH (const ip_type& ip, net.places())
      {
        const we::place_id_type& place_id (ip.first);
        const place::type& place (ip.second);

        std::ostringstream token;

        if (opts.show_token)
        {
          BOOST_FOREACH ( const pnet::type::value::value_type& t
                        , net.get_token (place_id)
                        )
          {
            token << endl << pnet::type::value::show (t);
          }
        }

        std::ostringstream virt;

        if ( opts.show_virtual
           && fhg::util::read_bool
              (place.property().get ("virtual").get_value_or ("false"))
           )
        {
            virt << endl << props ("virtual");
        }

        s << fhg::util::deeper (_indent)
          << name ( id_net
                  , "place_" + boost::lexical_cast<std::string> (place_id)
                  )
          << node
             ( shape::place
             , with_signature (place.name(), place.signature(), opts)
             + quote (token.str())
             + virt.str()
             );
      }

      typedef std::pair<we::transition_id_type,we::type::transition_t> it_type;

      BOOST_FOREACH (const it_type& it, net.transitions())
      {
        const we::transition_id_type& trans_id (it.first);
        const we::type::transition_t& trans (it.second);
        const id_type id_trans (++id);

        ++_indent;
        s << to_dot (trans, id, opts, _indent, boost::none, boost::none);
        --_indent;

        if (net.port_to_place().find (trans_id) != net.port_to_place().end())
        {
          BOOST_FOREACH
            ( we::type::net_type::port_to_place_with_info_type::value_type
              const& port_to_place
            , net.port_to_place().at (trans_id)
            )
          {
            s << fhg::util::deeper (_indent)
              << name ( id_trans
                      , "port_" + boost::lexical_cast<std::string> (port_to_place.get_left())
                      )
              << arrow
              << name ( id_net
                      , "place_" + boost::lexical_cast<std::string> (port_to_place.get_right())
                      );
          }
        }

        if (net.place_to_port().find (trans_id) !=  net.place_to_port().end())
        {
          BOOST_FOREACH
            ( we::type::net_type::place_to_port_with_info_type::value_type
              const& place_to_port
            , net.place_to_port().at (trans_id)
            )
          {
            s << fhg::util::deeper (_indent)
              << name ( id_net
                      , "place_" + boost::lexical_cast<std::string> (place_to_port.get_left())
                      )
              << arrow
              << name ( id_trans
                      , "port_" + boost::lexical_cast<std::string> (place_to_port.get_right())
                      )
              << (  net.place_to_transition_read().find
                    ( we::type::net_type::adj_pt_type::value_type
                     (place_to_port.get_left(), trans_id)
                    )
                 != net.place_to_transition_read().end()
                 ? brackets (keyval ("style", style::read_connection))
                 : ""
                 );
          }
        }
      }

      s << fhg::util::deeper (_indent) << bgcolor (color::internal)
        << _indent << "} /* " << "cluster_net_" << id_net << " */";

      return s.str();
    }
  };

  std::string to_dot
    ( const we::type::transition_t& t
    , id_type& id
    , const options& opts
    , fhg::util::indenter& indent
    , boost::optional<we::type::activity_t::input_t> input
    , boost::optional<we::type::activity_t::output_t> output
    )
  {
    std::ostringstream s;

    const id_type id_trans (id);

    s << indent << "subgraph cluster_" << id_trans << " {";

    std::ostringstream priority;

    if (opts.show_priority)
    {
      priority << "| priority: " << t.priority();
    }

    std::ostringstream intext;

    if (opts.show_intext)
    {
      intext << "|" << (t.is_internal() ? "internal" : "external");
    }

    std::ostringstream cond;

    if (t.condition())
    {
      std::ostringstream oss;
      oss << t.condition()->ast();

      cond << "|" << lines ('&', quote (oss.str()));
    }

    s << fhg::util::deeper (indent)
      << name (id_trans, "condition")
      << node ( shape::condition
              , t.name()
              + cond.str()
              + intext.str()
              + priority.str()
              );

    BOOST_FOREACH ( we::type::transition_t::port_map_t::value_type const& p
                  , t.ports_input()
                  )
    {
      std::ostringstream token;

      if (opts.show_token && input)
      {
        BOOST_FOREACH ( we::type::activity_t::input_t::value_type const& vp
                      , *input
                      )
        {
          if (vp.second == p.first)
          {
            token << endl << pnet::type::value::show (vp.first);
          }
        }
      }

      s << fhg::util::deeper (indent)
        << name (id_trans, "port_" + boost::lexical_cast<std::string> (p.first))
        << node ( shape::port_in
                , with_signature (p.second.name(), p.second.signature(), opts)
                + quote (token.str())
                );
    }
    BOOST_FOREACH ( we::type::transition_t::port_map_t::value_type const& p
                  , t.ports_output()
                  )
    {
      std::ostringstream token;

      if (opts.show_token && output)
      {
        BOOST_FOREACH ( we::type::activity_t::output_t::value_type const& vp
                      , *output
                      )
        {
          if (vp.second == p.first)
          {
            token << endl << pnet::type::value::show (vp.first);
          }
        }
      }

      s << fhg::util::deeper (indent)
        << name (id_trans, "port_" + boost::lexical_cast<std::string> (p.first))
        << node ( shape::port_out
                , with_signature (p.second.name(), p.second.signature(), opts)
                + quote (token.str())
                );
    }
    BOOST_FOREACH ( we::type::transition_t::port_map_t::value_type const& p
                  , t.ports_tunnel()
                  )
    {
      s << fhg::util::deeper (indent)
        << name (id_trans, "port_" + boost::lexical_cast<std::string> (p.first))
        << node ( shape::port_tunnel
                , with_signature (p.second.name(), p.second.signature(), opts)
                );
    }

    if (opts.should_be_expanded (t))
    {
      ++indent;
      s << boost::apply_visitor (visit_transition (id, indent, opts), t.data());
      --indent;

      BOOST_FOREACH ( we::type::transition_t::port_map_t::value_type const& p
                    , t.ports_input()
                    )
      {
        if (p.second.associated_place())
        {
          s << fhg::util::deeper (indent)
            << name (id_trans, "port_" + boost::lexical_cast<std::string> (p.first))
            << arrow
            << name (id_trans
                    , "place_"
                    + boost::lexical_cast<std::string> (*p.second.associated_place())
                    )
            << association();
        }
      }
      BOOST_FOREACH ( we::type::transition_t::port_map_t::value_type const& p
                    , t.ports_output()
                    )
      {
        if (p.second.associated_place())
        {
          s << fhg::util::deeper (indent)
            << name (id_trans, "port_" + boost::lexical_cast<std::string> (p.first))
            << arrow
            << name (id_trans
                    , "place_"
                    + boost::lexical_cast<std::string> (*p.second.associated_place())
                    )
            << association();
        }
      }
      BOOST_FOREACH ( we::type::transition_t::port_map_t::value_type const& p
                    , t.ports_tunnel()
                    )
      {
        if (p.second.associated_place())
        {
          s << fhg::util::deeper (indent)
            << name (id_trans, "port_" + boost::lexical_cast<std::string> (p.first))
            << arrow
            << name (id_trans
                    , "place_"
                    + boost::lexical_cast<std::string> (*p.second.associated_place())
                    )
            << association();
        }
      }
    }

    s << fhg::util::deeper (indent)
      << bgcolor ( t.expression() ? color::expression
                 : t.module_call() ? color::modcall
                 : t.is_internal() ? color::subnet_internal
                 : color::external
                 )
      << indent
      << "} /* " << "cluster_" << id_trans << " == " << t.name() << " */";

    return s.str();
  }

  void to_dot ( std::ostream& os
              , we::type::activity_t const& activity
              , options const& options
              )
  {
    id_type id (0);
    fhg::util::indenter indent (1);

    os << "digraph \"" << activity.transition().name() << "\" {"
       << "\n" << "compound=true"
       << "\n" << "rankdir=LR"
       << to_dot ( activity.transition(), id, options, indent
                 , activity.input()
                 , activity.output()
                 )
       << "\n" << "} /* " << activity.transition().name() << " */" << "\n";
  }
}

int main (int argc, char** argv)
try
{
  std::string input;
  std::string output;

  typedef std::vector<std::string> vec_type;

  vec_type not_starts_with;
  vec_type not_ends_with;

  options options;

  boost::program_options::options_description desc ("General");
  boost::program_options::options_description show ("Show");
  boost::program_options::options_description expand ("Expand");

#define BOOLVAL(x) \
  boost::program_options::value<bool> (&x)->default_value (x)->implicit_value (true)

  show.add_options()
    ( "full-signatures"
    , BOOLVAL (options.full)
    , "whether or not to show full signatures"
    )
    ( "token"
    , BOOLVAL (options.show_token)
    , "whether or not to show the tokens on a place"
    )
    ( "signature"
    , BOOLVAL (options.show_signature)
    , "whether or not to show the place and port signatures"
    )
    ( "priority"
    , BOOLVAL (options.show_priority)
    , "whether or not to show the transition priority"
    )
    ( "intext"
    , BOOLVAL (options.show_intext)
    , "whether or not to show the transition internal/external flag"
    )
    ( "virtual"
    , BOOLVAL (options.show_virtual)
    , "whether or not to show the virtual flag"
    )
    ( "tunnel-connection"
    , BOOLVAL (options.show_tunnel_connection)
    , "whether or not to show the tunnel connections"
    );

  expand.add_options()
    ( "not-starts-with"
    , boost::program_options::value<vec_type> (&not_starts_with)
    , "do not expand transitions that start with a certain prefix"
    )
    ( "not-ends-with"
    , boost::program_options::value<vec_type> (&not_ends_with)
    , "do not expand transitions that end with a certain suffix"
    );

  desc.add_options()
    ( "help,h", "this message")
    ( "version,V", "print version information")
    ( "input,i"
    , boost::program_options::value<std::string> (&input)->default_value ("-")
    , "input file name, - for stdin, first positional parameter"
    )
    ( "output,o"
    , boost::program_options::value<std::string> (&output)->default_value ("-")
    , "output file name, - for stdout, second positional parameter"
    );

  desc.add (show).add (expand);

#undef BOOLVAL

  boost::program_options::positional_options_description p;
  p.add ("input", 1).add ("output", 2);

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser(argc, argv)
    . options (desc).positional (p).run()
    , vm
    );
  boost::program_options::notify (vm);

  if (vm.count ("help"))
  {
    std::cout << argv[0] << ": convert to graphviz format" << std::endl;
    std::cout << desc << std::endl;

    return EXIT_SUCCESS;
  }

  if (vm.count ("version"))
  {
    std::cout << fhg::project_info ("pnet2dot");

    return EXIT_SUCCESS;
  }

  BOOST_FOREACH (std::string const& p, not_starts_with)
  {
    options.filter.push_back
      ( boost::bind ( &fhg::util::starts_with, p
                    , boost::bind (&we::type::transition_t::name, _1)
                    )
      );
  }
  BOOST_FOREACH (std::string const& s, not_ends_with)
  {
    options.filter.push_back
      ( boost::bind ( &fhg::util::ends_with, s
                    , boost::bind (&we::type::transition_t::name, _1)
                    )
      );
  }

  we::type::activity_t const act
    ( input == "-"
    ? we::type::activity_t (std::cin)
    : we::type::activity_t (boost::filesystem::path (input))
    );

  if (output == "-")
  {
    to_dot (std::cout, act, options);
  }
  else
  {
    std::ofstream ostream (output.c_str());

    if (!ostream)
    {
      throw std::runtime_error ("failed to open " + output + " for writing");
    }

    to_dot (ostream, act, options);
  }

  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  std::cerr << e.what() << std::endl;

  return EXIT_FAILURE;
}
