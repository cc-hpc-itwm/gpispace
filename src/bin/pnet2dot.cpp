// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/Activity.hpp>
#include <we/type/Transition.hpp>
#include <we/type/id.hpp>
#include <we/type/net.hpp>
#include <we/type/signature/show.hpp>
#include <we/type/value/show.hpp>

#include <fhg/project_info.hpp>
#include <fhg/util/cctype.hpp>
#include <fhg/util/indenter.hpp>
#include <fhg/util/starts_with.hpp>
#include <util-generic/print_exception.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
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
    static std::string const plugin ("tan");
    static std::string const node ("white");
    static std::string const put_token ("lightblue");
    static std::string const tp_many ("black:invis:black");
    static std::string const number_of_tokens ("orange");
  }

  namespace style
  {
    static std::string const association ("dotted");
    static std::string const read_connection ("dashed");
  }

  using id_type = unsigned long;

  static const std::string endl ("\\n");
  static const std::string arrow (" -> ");

  std::string parens ( std::string const& s
                     , std::string open
                     , std::string close
                     )
  {
    return open + s + close;
  }

  std::string brackets (std::string const& s)
  {
    return " " + parens (s, "[", "]");
  }

  std::string props (std::string const& s)
  {
    return parens (s, ":: ", " ::");
  }

  std::string dquote (std::string const& s)
  {
    return parens (s, "\"", "\"");
  }

  std::string lines (char const& b, std::string const& s)
  {
    std::string l;

    std::string::const_iterator pos (s.begin());
    const std::string::const_iterator end (s.end());

    while (pos != end)
    {
      if (*pos == b)
      {
        ++pos;

        while (pos != end && (fhg::util::isspace (*pos) || *pos == b))
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

  std::string quote (char const& c)
  {
    switch (c)
    {
    case '{': return "\\{";
    case '}': return "\\}";
    case '>': return "\\>";
    case '<': return "\\<";
    case '"': return "\\\"";
    case '|': return "\\|";
    case '\n': return "\\n";
    default: return std::string (1, c);
    }
  }

  std::string quote (std::string const& s)
  {
    std::string q;

    for (char pos : s)
    {
      q += quote (pos);
    }

    return lines (';', q);
  }

  std::string keyval (std::string const& key, std::string const& val)
  {
    return key + " = " + dquote (val);
  }

  std::string name (id_type const& id, std::string const& _name)
  {
    std::ostringstream s;

    s << "n" << id << "_" << _name;

    return s.str();
  }

  std::string bgcolor (std::string const& color)
  {
    return keyval ("bgcolor", color);
  }

  std::string node ( std::string const& shape
                   , std::string const& label
                   , std::string const& fillcolor = color::node
                   )
  {
    return brackets ( keyval ("shape", shape)
                    + ", "
                    + keyval ("label", label)
                    + ", "
                    + keyval ("style", "filled")
                    + ", "
                    + keyval ("fillcolor", fillcolor)
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
    bool full {false};
    std::list<std::function <bool (we::type::Transition const&)>> filter{};
    bool show_token {true};
    bool show_signature {true};
    bool show_priority {false};
    bool show_virtual {true};
    bool show_tunnel_connection {true};

    bool should_be_expanded (we::type::Transition const& x) const
    {
      for (std::function <bool (we::type::Transition const&)> f : filter)
      {
        if (f (x))
        {
          return false;
        }
      }

      return true;
    }
  };

  std::string with_signature ( std::string const& name
                             , pnet::type::signature::signature_type const& sig
                             , options const& opts
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
    ( we::type::Transition const&
    , id_type&
    , options const&
    , fhg::util::indenter&
    , std::optional<we::type::TokensOnPorts> input
    , std::optional<we::type::TokensOnPorts> output
    );

  class visit_transition : public ::boost::static_visitor<std::string>
  {
  private:
    id_type& id;
    fhg::util::indenter& _indent;
    options const& opts;

  public:
    visit_transition
      (id_type& _id, fhg::util::indenter& indent, options const& _opts)
      : id (_id)
      , _indent (indent)
      , opts (_opts)
    {}

    std::string operator() (we::type::Expression const& expr) const
    {
      std::ostringstream s;

      s << _indent
        << name (id, "expression")
        << node (shape::expression, quote (::boost::lexical_cast<std::string> (expr)));

      return s.str();
    }

    std::string operator() (we::type::ModuleCall const& mod_call) const
    {
      std::ostringstream s;

      s << _indent
        << name (id, "modcall")
        << node (shape::modcall, ::boost::lexical_cast<std::string> (mod_call));

      return s.str();
    }

    std::string operator() (we::type::MultiModuleCall const& mod_calls) const
    {
      std::ostringstream s;

      for (auto const& mod_call : mod_calls)
      {
        s << _indent
          << name (id, "modcall")
          << node (shape::modcall, ::boost::lexical_cast<std::string> (mod_call.second));
      }

      return s.str();
    }

    std::string operator() (we::type::net_type const& net) const
    {
      std::ostringstream s;

      const id_type id_net (id);

      s << _indent << "subgraph cluster_net_" << id_net << " {";

      for (auto const& [place_id, place] : net.places())
      {
        auto const number_of_tokens_place_id
          {net.number_of_tokens_place (place_id)};

        if (number_of_tokens_place_id)
        {
          s << _indent << "subgraph cluster_place_" << place_id << " {"
            << keyval ("style", "invis")
            ;
        }

        std::ostringstream token;

        if (opts.show_token)
        {
          for (auto const& [_ignore, t] : net.get_token (place_id))
          {
            token << endl << pnet::type::value::show (t);
          }
        }

        std::ostringstream virt;

        if (opts.show_virtual && place.property().is_true ({"virtual"}))
        {
            virt << endl << props ("virtual");
        }

        s << fhg::util::deeper (_indent)
          << name ( id_net
                  , "place_" + ::boost::lexical_cast<std::string> (place_id)
                  )
          << node
             ( shape::place
             , with_signature (place.name(), place.signature(), opts)
             + quote (token.str())
             + virt.str()
             , place.is_marked_for_put_token() ? color::put_token
               : place.name()[0] == '#' ? color::number_of_tokens
               : color::node
             );

        if (number_of_tokens_place_id)
        {
          s << fhg::util::deeper (_indent)
            << name ( id_net
                    , "place_" + ::boost::lexical_cast<std::string> (place_id)
                    )
            << arrow
            << name ( id_net
                    , "place_" + ::boost::lexical_cast<std::string> (*number_of_tokens_place_id)
                    )
            << association()
            << "}"
            ;
        }
      }

      for (auto const& it : net.transitions())
      {
        we::transition_id_type const& trans_id (it.first);
        we::type::Transition const& trans (it.second);
        const id_type id_trans (++id);

        ++_indent;
        s << to_dot (trans, id, opts, _indent, std::nullopt, std::nullopt);
        --_indent;

        if (net.port_to_place().find (trans_id) != net.port_to_place().end())
        {
          for (auto const& port_to_place : net.port_to_place().at (trans_id))
          {
            s << fhg::util::deeper (_indent)
              << name ( id_trans
                      , "port_" + ::boost::lexical_cast<std::string> (port_to_place.first)
                      )
              << arrow
              << name ( id_net
                      , "place_" + ::boost::lexical_cast<std::string> (port_to_place.second._place_id)
                      );
          }
        }

        auto const& tid_with_tp_many (net.port_many_to_place().find (trans_id));
        if (tid_with_tp_many != net.port_many_to_place().end())
        {
          for (auto const& port_to_place : tid_with_tp_many->second)
          {
            s << fhg::util::deeper (_indent)
              << name ( id_trans
                      , "port_" + ::boost::lexical_cast<std::string> (port_to_place.first)
                      )
              << arrow
              << name ( id_net
                      , "place_" + ::boost::lexical_cast<std::string> (port_to_place.second._place_id)
                      )
              << brackets (keyval ("color", color::tp_many));
          }
        }

        if (net.place_to_port().find (trans_id) !=  net.place_to_port().end())
        {
          for (auto const& place_to_port : net.place_to_port().at (trans_id))
          {
            s << fhg::util::deeper (_indent)
              << name ( id_net
                      , "place_" + ::boost::lexical_cast<std::string> (place_to_port.first)
                      )
              << arrow
              << name ( id_trans
                      , "port_" + ::boost::lexical_cast<std::string> (place_to_port.second._port_id)
                      )
              << (  net.place_to_transition_read().find
                    ( we::type::net_type::adj_pt_type::value_type
                     (place_to_port.first, trans_id)
                    )
                 != net.place_to_transition_read().end()
                 ? brackets (keyval ("style", style::read_connection))
                 : ""
                 );

            if (place_to_port.second._property.get ({"pnetc", "tunnel"}))
            {
              s << association();
            }
          }
        }
      }

      s << fhg::util::deeper (_indent) << bgcolor (color::internal)
        << _indent << "} /* " << "cluster_net_" << id_net << " */";

      return s.str();
    }
  };

  std::string to_dot
    ( we::type::Transition const& t
    , id_type& id
    , options const& opts
    , fhg::util::indenter& indent
    , std::optional<we::type::TokensOnPorts> input
    , std::optional<we::type::TokensOnPorts> output
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

    std::ostringstream cond;

    if (t.condition())
    {
      std::ostringstream oss;
      expr::parse::parser const& p (t.condition()->ast());

      std::copy
        ( p.begin()
        , p.end()
        , std::ostream_iterator<expr::parse::parser::nd_t> (oss, ";\n")
        );

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

    for (we::type::Transition::PortByID::value_type const& p : t.ports_input())
    {
      std::ostringstream token;

      if (opts.show_token && input)
      {
        for (auto const& vp : *input)
        {
          if (vp._port_id == p.first)
          {
            token << endl << pnet::type::value::show (vp._token);
          }
        }
      }

      s << fhg::util::deeper (indent)
        << name (id_trans, "port_" + ::boost::lexical_cast<std::string> (p.first))
        << node ( shape::port_in
                , with_signature (p.second.name(), p.second.signature(), opts)
                + quote (token.str())
                );
    }
    for (we::type::Transition::PortByID::value_type const& p : t.ports_output())
    {
      std::ostringstream token;

      if (opts.show_token && output)
      {
        for (auto const& vp : *output)
        {
          if (vp._port_id == p.first)
          {
            token << endl << pnet::type::value::show (vp._token);
          }
        }
      }

      s << fhg::util::deeper (indent)
        << name (id_trans, "port_" + ::boost::lexical_cast<std::string> (p.first))
        << node ( shape::port_out
                , with_signature (p.second.name(), p.second.signature(), opts)
                + quote (token.str())
                );
    }
    for (we::type::Transition::PortByID::value_type const& p : t.ports_tunnel())
    {
      s << fhg::util::deeper (indent)
        << name (id_trans, "port_" + ::boost::lexical_cast<std::string> (p.first))
        << node ( shape::port_tunnel
                , with_signature (p.second.name(), p.second.signature(), opts)
                );
    }

    if (opts.should_be_expanded (t))
    {
      ++indent;
      s << ::boost::apply_visitor (visit_transition (id, indent, opts), t.data());
      --indent;

      for (we::type::Transition::PortByID::value_type const& p : t.ports_input())
      {
        if (p.second.associated_place())
        {
          s << fhg::util::deeper (indent)
            << name (id_trans, "port_" + ::boost::lexical_cast<std::string> (p.first))
            << arrow
            << name (id_trans
                    , "place_"
                    + ::boost::lexical_cast<std::string> (*p.second.associated_place())
                    )
            << association();
        }
      }
      for (we::type::Transition::PortByID::value_type const& p : t.ports_output())
      {
        if (p.second.associated_place())
        {
          s << fhg::util::deeper (indent)
            << name (id_trans, "port_" + ::boost::lexical_cast<std::string> (p.first))
            << arrow
            << name (id_trans
                    , "place_"
                    + ::boost::lexical_cast<std::string> (*p.second.associated_place())
                    )
            << association();
        }
      }
      for (we::type::Transition::PortByID::value_type const& p : t.ports_tunnel())
      {
        if (p.second.associated_place())
        {
          s << fhg::util::deeper (indent)
            << name (id_trans, "port_" + ::boost::lexical_cast<std::string> (p.first))
            << arrow
            << name (id_trans
                    , "place_"
                    + ::boost::lexical_cast<std::string> (*p.second.associated_place())
                    )
            << association();
        }
      }
    }

    s << fhg::util::deeper (indent)
      << bgcolor ( t.expression() ? ( !!t.prop().get ({"gspc","we","plugin"})
                                    ? color::plugin
                                    : color::expression
                                    )
                 : t.module_call() ? color::modcall
                 : color::external
                 )
      << indent
      << "} /* " << "cluster_" << id_trans << " == " << t.name() << " */";

    return s.str();
  }

  void to_dot ( std::ostream& os
              , we::type::Activity const& activity
              , options const& options
              )
  {
    id_type id (0);
    fhg::util::indenter indent (1);

    os << "digraph \"" << activity.name() << "\" {"
       << "\n" << "compound=true"
       << "\n" << "rankdir=LR"
       << to_dot ( activity.transition()
                 , id
                 , options
                 , indent
                 , activity.input()
                 , activity.output()
                 )
       << "\n" << "} /* " << activity.name() << " */" << "\n";
  }
}

int main (int argc, char** argv)
try
{
  std::string input;
  std::string output;

  using Strings = std::vector<std::string>;

  Strings not_starts_with;
  Strings not_ends_with;

  options options;

  ::boost::program_options::options_description desc ("General");
  ::boost::program_options::options_description show ("Show");
  ::boost::program_options::options_description expand ("Expand");

#define BOOLVAL(x) \
  ::boost::program_options::value<bool> (&x)->default_value (x)->implicit_value (true)

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
    , ::boost::program_options::value<Strings> (&not_starts_with)
    , "do not expand transitions that start with a certain prefix"
    )
    ( "not-ends-with"
    , ::boost::program_options::value<Strings> (&not_ends_with)
    , "do not expand transitions that end with a certain suffix"
    );

  desc.add_options()
    ( "help,h", "this message")
    ( "version,V", "print version information")
    ( "input,i"
    , ::boost::program_options::value<std::string> (&input)->default_value ("-")
    , "input file name, - for stdin, first positional parameter"
    )
    ( "output,o"
    , ::boost::program_options::value<std::string> (&output)->default_value ("-")
    , "output file name, - for stdout, second positional parameter"
    );

  desc.add (show).add (expand);

#undef BOOLVAL

  ::boost::program_options::positional_options_description p;
  p.add ("input", 1).add ("output", 2);

  ::boost::program_options::variables_map vm;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser (argc, argv)
    . options (desc).positional (p).run()
    , vm
    );
  ::boost::program_options::notify (vm);

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

  for (std::string const& nsw : not_starts_with)
  {
    options.filter.push_back
      ( [&nsw] (we::type::Transition const& t)
      {
        return fhg::util::starts_with (nsw, t.name());
      }
      );
  }
  for (std::string const& s : not_ends_with)
  {
    options.filter.push_back
      ( [&s] (we::type::Transition const& t)
      {
        return fhg::util::ends_with (s, t.name());
      }
      );
  }

  we::type::Activity const act
    ( input == "-"
    ? we::type::Activity (std::cin)
    : we::type::Activity (::boost::filesystem::path (input))
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
catch (...)
{
  std::cerr << "EX: " << fhg::util::current_exception_printer() << '\n';

  return EXIT_FAILURE;
}
