// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/parse/parser.hpp>

#include <we/expr/parse/util/get_names.hpp>

#include <iostream>
#include <string>

int
main (int argc, char ** argv)
{
  const std::string input
    ("${x.a} * ${y.a} - ${z.coord.phi} + ${x.b} / sin (${t} + pi)");

  expr::parse::parser parser (input);

  expr::parse::util::name_set_t names
    (expr::parse::util::get_names (parser.front()));

  typedef expr::parse::util::name_set_t name_set_t;

  for ( name_set_t::const_iterator name (names.begin())
      ; name != names.end()
      ; ++name
      )
    {
      std::cout << expr::parse::util::write_key_vec (*name) << std::endl;
    }

  return EXIT_SUCCESS;
}
