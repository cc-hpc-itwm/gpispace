// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/parse/parser.hpp>

#include <sstream>

namespace expr
{
  namespace parse
  {
    std::string parse_result (const std::string& input)
    {
      std::ostringstream oss;

      try
        {
          oss << parser (input) << std::endl;
        }
      catch (const exception::parse::exception& e)
        {
          std::string::const_iterator pos (input.begin());

          std::string white;

          for (unsigned int k (0); k < e.eaten; ++k, ++pos)
            {
              oss << *pos;

              if (*pos == '\n')
                {
                  white.clear();
                }
              else
                {
                  white += " ";
                }
            }
          oss << std::endl << white << "^" << std::endl;

          oss << e.what() << std::endl;
        }
      catch (const exception::eval::type_error& e)
        {
          oss << e.what() << std::endl;
        }
      catch (const exception::eval::divide_by_zero& e)
        {
          oss << e.what() << std::endl;
        }

      return oss.str();
    }
  }
}
