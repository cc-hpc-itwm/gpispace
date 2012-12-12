#include <iostream>
#include <stdint.h>

#include <we/we.hpp>
#include <we/mgmt/layer.hpp>
#include <we/mgmt/type/requirement.hpp>

#include <we/type/module_call.hpp>
#include <we/type/expression.hpp>

#include <list>

typedef uint64_t id_type;

typedef we::mgmt::layer<id_type> layer_t;

typedef std::list<we::mgmt::requirement_t<std::string> > requirement_list_t;

static inline id_type generate_id ()
{
  static id_type id(0);
  return ++id;
}

template <typename L>
struct daemon_t
{
  daemon_t ()
    : layer (this, &generate_id)
  {}

  void submit (const id_type & id, const std::string &enc, requirement_list_t req_list = requirement_list_t())
  {
    layer.print_statistics (std::cerr);
    layer.finished (id, enc);
  }

  bool cancel (const id_type &, const std::string &)
  {
    return false;
  }

  bool finished (const id_type &, const std::string &)
  {
    return false;
  }

  bool failed( const id_type & id
             , const std::string & result
             , const int error_code
             , const std::string & reason
             )
  {
    return false;
  }

  bool cancelled (const id_type &)
  {
    return false;
  }

  L layer;
};

int main ()
{
  daemon_t<layer_t> daemon;
  layer_t & layer = daemon.layer;

  {
    we::transition_t mod_call
      ( "module call"
      , we::type::module_call_t ("m", "f")
      );
    we::activity_t act (mod_call);
    layer.submit (generate_id(), layer_t::policy::codec::encode(act));

    sleep (1);
    layer.print_statistics (std::cerr);
  }

  {
    we::transition_t expr ( "expression"
                          , we::type::expression_t ("${out} := 3L")
                          );
    we::activity_t act (expr);
    layer.submit (generate_id(), layer_t::policy::codec::encode(act));

    sleep (1);
    layer.print_statistics (std::cerr);
  }

  return EXIT_SUCCESS;
}
