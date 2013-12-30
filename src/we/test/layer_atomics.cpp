#include <iostream>
#include <stdint.h>

#include <we/mgmt/layer.hpp>
#include <we/type/schedule_data.hpp>

#include <we/type/module_call.hpp>
#include <we/type/expression.hpp>
#include <we/type/transition.hpp>
#include <we/mgmt/type/activity.hpp>

#include <boost/lexical_cast.hpp>

#include <list>
#include <string>

typedef std::string id_type;

typedef we::mgmt::layer layer_t;

static inline id_type generate_id ()
{
  static uint64_t _cnt (0);

  const id_type id (boost::lexical_cast<id_type> (_cnt));

  ++_cnt;

  return id;
}

template <typename L>
struct daemon_t
{
  daemon_t ()
    : layer (this, &generate_id)
  {}

  void submit ( const id_type & id
              , const we::mgmt::type::activity_t & act
              , const we::type::schedule_data&
              , const we::mgmt::layer::id_type& parent_id
              )
  {
    layer.finished (id, act);
  }

  bool cancel (const id_type &, const std::string &)
  {
    return false;
  }

  bool finished (const id_type &, we::mgmt::type::activity_t const&)
  {
    return false;
  }

  bool failed( const id_type & id
             , const int error_code
             , const std::string & reason
             )
  {
    return false;
  }

  bool canceled (const id_type &)
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
    we::type::transition_t mod_call
      ( "module call"
      , we::type::module_call_t ("m", "f")
      , condition::type ("true")
      , false
      , we::type::property::type()
      );
    we::mgmt::type::activity_t act (mod_call);
    layer.submit (generate_id(), act);

    sleep (1);
  }

  {
    we::type::transition_t expr
      ( "expression"
      , we::type::expression_t ("${out} := 3L")
      , condition::type ("true")
      , true
      , we::type::property::type()
      );
    we::mgmt::type::activity_t act (expr);
    layer.submit (generate_id(), act);

    sleep (1);
  }

  return EXIT_SUCCESS;
}
