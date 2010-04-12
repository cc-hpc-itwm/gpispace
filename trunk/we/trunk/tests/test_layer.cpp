#include <iostream>
#include <sstream>

#include <stdint.h>
#include <we/type/transition.hpp>
#include <we/mgmt/layer.hpp>
#include "test_layer.hpp"

using namespace we::mgmt;
using namespace test;

// this is ugly
typedef petri_net::net<we::mgmt::detail::place_t
                     , we::type::transition_t<we::mgmt::detail::place_t, we::mgmt::detail::edge_t, we::mgmt::detail::token_t>
                     , we::mgmt::detail::edge_t
                     , we::mgmt::detail::token_t> pnet_t;

typedef uint64_t id_type;
//  typedef unsigned long id_type;
//  typedef unsigned int id_type;
//  typedef int id_type;
//  typedef std::string id_type;

typedef we::mgmt::layer<basic_layer<id_type>, pnet_t> layer_t;
typedef sdpa_daemon<layer_t> daemon_type;

static void test_execute(unsigned int id, const std::string & a)
{
  std::cerr << "execute: " << id << " " << a << std::endl;
}

inline std::ostream & operator << (std::ostream & s, const pnet_t & n)
{
  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
  {
    s << "[" << n.get_place (*p) << ":";

    typedef boost::unordered_map<we::mgmt::detail::token_t, size_t> token_cnt_t;
    token_cnt_t token;
    for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
    {
      token[*tp]++;
    }

    for (token_cnt_t::const_iterator t (token.begin()); t != token.end(); ++t)
    {
      if (t->second > 1)
      {
        s << " " << t->second << "x " << t->first;
      }
      else
      {
        s << " " << t->first;
      }
    }
    s << "]";
  }

  return s;
}

int main ()
{
  // instantiate layer
  daemon_type daemon;
  daemon_type::layer_type & mgmt_layer = daemon.layer();

  std::vector<id_type> ids;
  mgmt_layer.sig_execute.connect ( &test_execute );
  for (std::size_t i (0); i < 1; ++i)
  {
	daemon_type::id_type id = daemon.gen_id();
	ids.push_back(id);

	mgmt_layer.submit(id, "");
  }

  for (std::vector<id_type>::const_iterator id (ids.begin()); id != ids.end(); ++id)
  {
	mgmt_layer.suspend(*id);
    sleep(3);
	mgmt_layer.resume(*id);
//	mgmt_layer.failed(*id, "");
//	mgmt_layer.finished(*id, "");
	mgmt_layer.cancel(*id, "");
    sleep(1);
	mgmt_layer.suspend(*id);
  }

  sleep(1); // not nice, but we cannot wait for a network to finish right now

  return EXIT_SUCCESS;
}
