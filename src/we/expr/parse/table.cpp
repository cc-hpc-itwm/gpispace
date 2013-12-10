
#include <we/expr/token/type.hpp>
#include <we/expr/parse/action.hpp>
#include <we/expr/exception.hpp>

#include <iostream>
#include <iomanip>

#include <stdlib.h>

#include <boost/optional.hpp>

int main (int argc, char** argv)
{
  const std::size_t w (20);

  std::cout << "/* " << std::setw(w) <<  "." << "     ";

  for (int a (expr::token::_token_begin); a != expr::token::_token_end; ++a)
    {
      std::cout << std::setw(w) << (expr::token::type)a << ",";
    }

  std::cout << std::setw(w) << "*/" << std::endl;

  for (int a (expr::token::_token_begin); a != expr::token::_token_end; ++a)
    {
      std::cout << "/* " << std::setw(w) << (expr::token::type)a << " */ {";

      for (int b (expr::token::_token_begin); b != expr::token::_token_end; ++b)
        {
          std::cout << std::setw(w)
                    << expr::parse::action::action ( (expr::token::type)a
                                                   , (expr::token::type)b
                                                   )
                    << ",";
        }

      std::cout << "}" << std::endl;
    }

  return EXIT_SUCCESS;
}
