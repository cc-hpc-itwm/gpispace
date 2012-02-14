#include <iostream>
#include <stdint.h>

#include <we/we.hpp>
#include <we/mgmt/layer.hpp>
#include <we/mgmt/type/requirement.hpp>
#include <list>

typedef uint64_t id_type;

typedef we::mgmt::layer<id_type, we::activity_t> layer_t;
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
    std::cout << "submitted id = " << id << std::endl;
    layer.print_statistics (std::cerr);
    layer.failed (id, enc);
  }

  bool cancel (const id_type & id, const std::string &)
  {
    std::cout << "cancelling id = " << id << std::endl;
    layer.cancelled (id);
    return true;
  }

  bool finished (const id_type & id, const std::string &)
  {
    std::cout << "finished id = " << id << std::endl;
    return false;
  }

  bool failed (const id_type & id, const std::string &)
  {
    std::cout << "failed id = " << id << std::endl;
    return false;
  }

  bool cancelled (const id_type & id)
  {
    std::cout << "cancelled id = " << id << std::endl;
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
      , we::transition_t::mod_type ("m", "f")
      );
    we::activity_t act (mod_call);
    layer.submit (generate_id(), layer_t::policy::codec::encode(act));

    sleep (1);
    layer.print_statistics (std::cerr);
  }

  {
    we::transition_t expr
      ( "expression"
      , we::transition_t::expr_type ("${out} := 3L")
      );
    we::activity_t act (expr);
    layer.submit (generate_id(), layer_t::policy::codec::encode(act));

    sleep (1);
    layer.print_statistics (std::cerr);
  }

  return EXIT_SUCCESS;
}
