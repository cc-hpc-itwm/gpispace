// structured token, expression transition, mirko.rahn@itwm.fraunhofer.de

#include <we/net.hpp>
#include <we/function/cond_exp.hpp>
#include <we/type/token.hpp>
#include <we/expr/parse/parser.hpp>
#include <we/expr/eval/context.hpp>

#include <we/util/show.hpp>

#include "timer.hpp"

#include <string>

#include <boost/function.hpp>
#include <boost/program_options.hpp>
#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

// ************************************************************************* //

// WORK HERE: implement as variant
struct place_t
{
public:
  typedef std::string name_t;

private:
  name_t name;
  we::token::signature_t signature;

public:
  const name_t & get_name (void) const { return name; }
  const we::token::signature_t & get_signature (void) const { return signature; }

  place_t (const name_t & _name)
    : name (_name)
    , signature ()
  {}

  place_t ( const name_t & _name
          , const we::token::type_name_t & _type_name
          )
    : name (_name)
  {
    signature[""] = _type_name;
  }

  place_t ( const name_t & _name
          , const we::token::signature_t & _signature
          )
    : name (_name)
    , signature (_signature)
  {}
};

static std::ostream & operator << (std::ostream & s, const place_t & p)
{
  return s << p.get_name();
}

static bool operator == (const place_t & a, const place_t & b)
{
  return a.get_name() == b.get_name();
}

static inline std::size_t hash_value (const place_t & p)
{
  boost::hash<place_t::name_t> h;

  return h(p.get_name());
}

// ************************************************************************* //

typedef unsigned int transition_cnt_t;
typedef std::pair<transition_cnt_t,std::string> transition_t;
typedef unsigned int edge_cnt_t;
typedef std::pair<edge_cnt_t,std::string> edge_t;

static edge_cnt_t e (0);
static transition_cnt_t t (0);

static edge_t mk_edge (const std::string & descr)
{
  return edge_t (e++, descr);
}

static transition_t mk_trans (const std::string & descr)
{
  return transition_t (t++, descr);
}

typedef petri_net::net<place_t, transition_t, edge_t, we::token::type> pnet_t;

// ************************************************************************* //

static const place_t::name_t & place_name ( const pnet_t & net
                                          , const petri_net::pid_t & pid
                                          )
{
  return net.get_place(pid).get_name();
}

static const we::token::signature_t &
place_signature ( const pnet_t & net
                , const petri_net::pid_t & pid
                )
{
  return net.get_place(pid).get_signature();
}

// ************************************************************************* //

using std::cout;
using std::endl;

static void marking (const pnet_t & n)
{
  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      cout << "{" << n.get_place (*p) << ":";

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        cout << " " << *tp;

      cout << "}";
    }
  cout << endl;
}

// ************************************************************************* //

static Function::Condition::Expression<we::token::type>
mk_cond (const pnet_t & net, const std::string & exp)
{
  return Function::Condition::Expression<we::token::type>
    ( exp
    , boost::bind (&place_name, boost::ref (net), _1)
    );
}

// ************************************************************************* //

class prefix_not_unique : public std::runtime_error
{
public:
  prefix_not_unique (const std::string & pref) 
    : std::runtime_error ("prefix not unique: " + pref)
  {}
};

class no_value_given : public std::runtime_error
{
public:
  no_value_given (const std::string & pref) 
    : std::runtime_error ("no_value_given for: " + pref)
  {}
};

class type_error : public std::runtime_error
{
public:
  type_error ( const std::string & field
             , const std::string & required
             , const std::string & given
             )
    : std::runtime_error ( "type error: " + field 
                         + " requires value of type " + required 
                         + ", given value of type " + given
                         ) {};
};

typedef expr::eval::context<std::string> context_t;

// construct token from context, use information from signature

static we::token::type mk_token ( const std::string & pref
                                , const we::token::signature_t & signature
                                , const context_t & context
                                )
{
  if (signature.size() == 0)
    {
      try
        {
          const expr::variant::type & v (context.value (pref));

          const std::string req ("control");
          const std::string has
            (boost::apply_visitor (expr::variant::visitor_type_name(), v));

          if (has == req)
            return we::token::type (v);
          else
            throw type_error (pref, req, has);
        }
      catch (expr::exception::eval::missing_binding<std::string> &)
        {
          throw no_value_given (pref);
        }
    }
  else if (signature.size() == 1)
    {
      try
        {
          const expr::variant::type & v (context.value (pref));

          const std::string req ((*(signature.begin())).second);
          const std::string has
            (boost::apply_visitor (expr::variant::visitor_type_name(), v));

          if (has == req)
            return we::token::type (v);
          else
            throw type_error (pref, req, has);
        }
      catch (expr::exception::eval::missing_binding<std::string> &)
        {
          throw no_value_given (pref);
        }
    }
  else
    {
      we::token::type::map_t m;

      for ( we::token::signature_t::const_iterator sig (signature.begin())
          ; sig != signature.end()
          ; ++sig
          )
        {
          const std::string field (pref + "." + sig->first);

          try
            {
              const expr::variant::type & v (context.value (field));

              const std::string req (sig->second);
              const std::string has
                (boost::apply_visitor (expr::variant::visitor_type_name(), v));

              if (has == req)
                m[sig->first] = v;
              else
                throw type_error (field, req, has);
            }
          catch (expr::exception::eval::missing_binding<std::string> &)
            {
              throw no_value_given (field);
            }
        }

      return we::token::type (m);
    }
}

class Transition
{
private:
  const std::string name;
  const std::string expression;
  const expr::parse::parser<std::string> parser;
  expr::eval::context<std::string> context;
  typedef boost::function<std::string (const pid_t &)> translate_t;
  typedef boost::function<we::token::signature_t (const pid_t &)> sig_t;
  translate_t translate;
  sig_t signature;

public:
  explicit Transition ( const std::string & _name
                      , const std::string & _expression
                      , const translate_t & _translate
                      , const sig_t & _signature
                      )
    : name (_name)
    , expression (_expression)
    , parser (expression)
    , context ()
    , translate (_translate)
    , signature (_signature)
  {}

  pnet_t::output_t operator () ( const pnet_t::input_t & input
                               , const pnet_t::output_descr_t & output_descr
                               )
  {
    for ( pnet_t::input_t::const_iterator top (input.begin())
        ; top != input.end()
        ; ++top
        )
      {
        const we::token::type 
          token (Function::Transition::get_token<we::token::type> (*top));

        const petri_net::pid_t
          pid (Function::Transition::get_pid<we::token::type> (*top));

        token.bind (translate (pid), context);
      }

    parser.eval_all (context);

    pnet_t::output_t output;

    for ( pnet_t::output_descr_t::const_iterator out (output_descr.begin())
        ; out != output_descr.end()
        ; ++out
        )
      {
        const petri_net::pid_t pid (out->first);

        typedef 
          Function::Transition::Traits<we::token::type>::token_on_place_t top_t;

        output.push_back (top_t (mk_token ( translate (pid)
                                          , signature (pid)
                                          , context
                                          )
                                , pid
                                )
                         );
      }

    return output;
  }
};

static petri_net::tid_t mk_transition ( pnet_t & net
                                      , const std::string & name
                                      , const std::string & expression
                                      , const std::string & condition
                                      )
{
  return net.add_transition 
    ( mk_trans (name)
    , Transition ( name, expression
                 , boost::bind (&place_name, boost::ref(net), _1)
                 , boost::bind (&place_signature, boost::ref(net), _1)
                 )
    , mk_cond (net, condition)
    );
}

static petri_net::tid_t mk_transition ( pnet_t & net
                                      , const std::string & name
                                      , const std::string & expression
                                      )
{
  return net.add_transition 
    ( mk_trans (name)
    , Transition ( name, expression
                 , boost::bind (&place_name, boost::ref(net), _1)
                 , boost::bind (&place_signature, boost::ref(net), _1)
                 )
    );
}

// ************************************************************************* //

using petri_net::connection_t;
using petri_net::PT;
using petri_net::PT_READ;
using petri_net::TP;

namespace po = boost::program_options;

int
main (int argc, char ** argv)
{
  long NUM_SLICES (3);
  long MAX_DEPTH (4);
  long CAP_IN_PROGRESS (0);
  bool PRINT_MARKING (true);

  po::options_description desc("options");

  desc.add_options()
    ("help", "this message")
    ("slices", po::value<long>(&NUM_SLICES)->default_value(3), "num slices")
    ("depth", po::value<long>(&MAX_DEPTH)->default_value(4), "max depth")
    ("cap", po::value<long>(&CAP_IN_PROGRESS)->default_value(0), "capacity in place 'in_progress'")
    ("print", po::value<bool>(&PRINT_MARKING)->default_value(true), "print after each fire")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
    {
      cout << desc << "\n";
      return EXIT_SUCCESS;
    }

  pnet_t net;

  petri_net::pid_t pid_NUM_SLICES (net.add_place (place_t("NUM_SLICES","long")));
  petri_net::pid_t pid_MAX_DEPTH (net.add_place (place_t("MAX_DEPTH","long")));
  petri_net::pid_t pid_splitted (net.add_place (place_t("splitted","long")));
  petri_net::pid_t pid_slice_in (net.add_place (place_t("slice_in","long")));
  petri_net::pid_t pid_slice_out (net.add_place (place_t("slice_out","long")));
  petri_net::pid_t pid_joined (net.add_place (place_t("joined","long")));

  we::token::signature_t sig;

  sig["slice"] = "long";
  sig["depth"] = "long";

  petri_net::pid_t pid_slice_depth (net.add_place (place_t("slice_depth", sig)));

  petri_net::pid_t pid_done (net.add_place (place_t("done"))); // control
  petri_net::pid_t pid_in_progress (net.add_place (place_t("in_progress"))); // control

  petri_net::tid_t tid_split
    ( mk_transition 
      ( net
      , "split"
      , "${slice_in}    := ${splitted};     \
         ${splitted}    := ${splitted} + 1; \
         ${in_progress} := [];              "
      , "${splitted} < ${NUM_SLICES}"
      )
    );
  petri_net::tid_t tid_tag
    ( mk_transition
      ( net
      , "tag"
      , "${slice_depth.slice} := ${slice_in}; ${slice_depth.depth} := 0"
      )
    );
  petri_net::tid_t tid_work
    ( mk_transition
      ( net
      , "work"
      , "${slice_depth.slice} := ${slice_depth.slice};   \
         ${slice_depth.depth} := ${slice_depth.depth} + 1"
      , "${slice_depth.depth} < ${MAX_DEPTH}"
      )
    );

  petri_net::tid_t tid_untag
    ( mk_transition
      ( net
      , "untag"
      , "${slice_out} := ${slice_depth.slice}"
      , "${slice_depth.depth} >= ${MAX_DEPTH}"
      )
    );

  petri_net::pid_t tid_join
    ( mk_transition
      ( net
      , "join"
      , "${joined} := ${joined} + 1"
      )
    );

  petri_net::tid_t tid_finalize 
    ( mk_transition
      ( net
      , "finalize"
      , "${done} := []"
      , "${joined} == ${NUM_SLICES} & ${splitted} == ${NUM_SLICES}"
      )
    );

  net.add_edge ( mk_edge ("get splitted")
               , connection_t (PT, tid_split, pid_splitted)
               );
  net.add_edge ( mk_edge ("read NUM_SLICES")
               , connection_t (PT_READ, tid_split, pid_NUM_SLICES)
               );
  net.add_edge ( mk_edge ("put slice_in")
               , connection_t (TP, tid_split, pid_slice_in)
               );
  net.add_edge ( mk_edge ("put splitted")
               , connection_t (TP, tid_split, pid_splitted)
               );
  net.add_edge ( mk_edge ("put in_progress")
               , connection_t (TP, tid_split, pid_in_progress)
               );

  net.add_edge ( mk_edge ("get slice_in")
               , connection_t (PT, tid_tag, pid_slice_in)
               );
  net.add_edge ( mk_edge ("put slice_depth")
               , connection_t (TP, tid_tag, pid_slice_depth)
               );

  net.add_edge ( mk_edge ("get slice_depth")
               , connection_t (PT, tid_work, pid_slice_depth)
               );
  net.add_edge ( mk_edge ("read MAX_DEPTH")
               , connection_t (PT_READ, tid_work, pid_MAX_DEPTH)
               );
  net.add_edge ( mk_edge ("put slice_depth")
               , connection_t (TP, tid_work, pid_slice_depth)
               );

  net.add_edge ( mk_edge ("get slice_depth")
               , connection_t (PT, tid_untag, pid_slice_depth)
               );
  net.add_edge ( mk_edge ("read MAX_DEPTH")
               , connection_t (PT_READ, tid_untag, pid_MAX_DEPTH)
               );
  net.add_edge ( mk_edge ("put slice_out")
               , connection_t (TP, tid_untag, pid_slice_out)
               );

  net.add_edge ( mk_edge ("get slice_out")
               , connection_t (PT, tid_join, pid_slice_out)
               );
  net.add_edge ( mk_edge ("get joined")
               , connection_t (PT, tid_join, pid_joined)
               );
  net.add_edge ( mk_edge ("put joined")
               , connection_t (TP, tid_join, pid_joined)
               );
  net.add_edge ( mk_edge ("get in_progress")
               , connection_t (PT, tid_join, pid_in_progress)
               );

  net.add_edge ( mk_edge ("get splitted")
               , connection_t (PT, tid_finalize, pid_splitted)
               );
  net.add_edge ( mk_edge ("get joined")
               , connection_t (PT, tid_finalize, pid_joined)
               );
  net.add_edge ( mk_edge ("read NUM_SLICES")
               , connection_t (PT_READ, tid_finalize, pid_NUM_SLICES)
               );
  net.add_edge ( mk_edge ("put done")
               , connection_t (TP, tid_finalize, pid_done)
               );

  if (CAP_IN_PROGRESS > 0)
    net.set_capacity (pid_in_progress, CAP_IN_PROGRESS);

  net.put_token (pid_splitted, we::token::type(expr::variant::type(0L)));
  net.put_token (pid_joined, we::token::type(expr::variant::type(0L)));
  net.put_token (pid_NUM_SLICES, we::token::type(expr::variant::type(NUM_SLICES)));
  net.put_token (pid_MAX_DEPTH, we::token::type(expr::variant::type(MAX_DEPTH)));

  marking (net);

  {
    boost::mt19937 engine;

    Timer_t timer ("fire", NUM_SLICES * MAX_DEPTH + 4 * NUM_SLICES + 1);

    while (!net.enabled_transitions().empty())
      {
        net.fire_random(engine);

        if (PRINT_MARKING)
          marking (net);
      }
  }

  marking (net);

  return EXIT_SUCCESS;
}
