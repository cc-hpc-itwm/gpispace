#include <iostream>
#include <we/we.hpp>
#include <we/mgmt/bits/descriptor.hpp>
#include <kdm/simple_generator.hpp>
#include <boost/bind.hpp>

typedef we::mgmt::detail::descriptor< we::activity_t
                                    , unsigned int
                                    , unsigned int
                                    > descriptor_t;


int main()
{
  we::transition_t simple_trans (kdm::kdm<we::activity_t>::generate());
  we::activity_t act ( simple_trans );
  act.add_input
    ( we::input_t::value_type
      ( we::token_t ( "config_file"
                    , literal::STRING
                    , std::string("test.conf")
                    )
      , simple_trans.input_port_by_name ("config_file")
      )
    );
  act.inject_input();

  descriptor_t d0 (0, act);
  d0.came_from_external_as (0);
  std::cout << d0 << std::endl;

  descriptor_t d1;
  try
  {
    d1 = d0.extract (1);
    d1.sent_to_external_as (1);
    std::cout << d0 << std::endl;
    std::cout << d1 << std::endl;
  }
  catch (std::exception const &ex)
  {
    std::cerr << "extraction not possible: " << ex.what() << std::endl;
  }

  try
  {
    d0.inject (d1);
    std::cout << d0 << std::endl;
  }
  catch (std::exception const &ex)
  {
    std::cerr << "injection not possible: " << ex.what() << std::endl;
  }

  return EXIT_SUCCESS;
}
