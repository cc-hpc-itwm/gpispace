#include <gspc/we/exception.hpp>
#include <gspc/we/expr/eval/context.hpp>
#include <gspc/we/expr/parse/parser.hpp>
#include <gspc/we/expr/type/Context.hpp>
#include <gspc/we/expr/type/Type.hpp>
#include <gspc/we/type/value/show.hpp>

#include <gspc/util/functor_visitor.hpp>
#include <gspc/util/ostream/modifier.hpp>
#include <gspc/util/print_container.hpp>
#include <gspc/util/print_exception.hpp>

#include <readline/history.h>
#include <readline/readline.h>

#include <iostream>
#include <string>

namespace
{
  bool read_line (std::string& s)
  {
    char* line (readline ("> "));

    if (!line)
    {
      return false;
    }

    s = std::string (line);

    if (*line)
    {
      add_history (line);
    }

    free (line);

    return true;
  }

  struct print_type_context : gspc::util::ostream::modifier
  {
    print_type_context (gspc::we::expr::type::Context const& context)
      : _root (context.at ({}).value())
    {}
    gspc::we::expr::Type _root;

    std::ostream& operator() (std::ostream& os) const
    {
      return gspc::util::visit<decltype (os)>
        ( _root
        , [&] (gspc::we::expr::type::Struct const& _struct) -> decltype (os)
          {
            return os
              << gspc::util::print_container
                 ( "", "\n", "", _struct._fields
                 , [] (auto& s, auto const& field) -> decltype (s)
                   {
                     return s << field._name << " :: " << field._type;
                   }
                 )
              ;
          }
        , [&] (auto const& type) -> decltype (os)
          {
            return os << type;
          }
        );
    }
  };
}

int main()
try
{
  std::cout << "clear context: #" << '\n'
            << "list state: ?" << std::endl
    ;

  gspc::we::expr::eval::context eval_context;
  gspc::we::expr::type::Context type_context;
  std::string input;

  while (read_line (input))
  {
    switch (input[0])
    {
    case '?':
      std::cout << "eval context (delete with #):\n"
                << eval_context
                << "type context (delete with #):\n"
                << print_type_context (type_context)
                << std::endl
        ;
      break;
    case '#':
      eval_context = gspc::we::expr::eval::context();
      type_context = gspc::we::expr::type::Context();
      std::cout << "context deleted" << std::endl;

      break;
    default:
      try
      {
        gspc::we::expr::parse::parser const parser
          (gspc::we::expr::parse::parser::DisableConstantFolding{}, input);

        std::cout << "type: "
                  << parser.type_check_all (type_context)
                  << '\n'
                  << "value: "
                  << gspc::pnet::type::value::show (parser.eval_all (eval_context))
                  << std::endl
          ;
      }
      catch (gspc::we::expr::exception::parse::exception const& e)
      {
        std::cout << input
                  << std::string (e.eaten, ' ') << "^\n"
                  << "EXCEPTION: " << e.what()
                  << std::endl
          ;
      }
      catch (...)
      {
        std::cout << "EXCEPTION: "
                  << gspc::util::current_exception_printer()
                  << std::endl
          ;
      }
    }
  }

  std::cout << '\n';

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cerr << "EXCEPTION: " << gspc::util::current_exception_printer() << '\n';

  return EXIT_FAILURE;
}
