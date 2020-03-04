#include <we/expr/parse/parser.hpp>
#include <we/type/memory_buffer_info_t.hpp>

namespace we
{
  namespace type
  {
    memory_buffer_info_t::memory_buffer_info_t() {}

    memory_buffer_info_t::memory_buffer_info_t
        (std::string const& size, std::string const& alignment)
      : _size (size)
      , _alignment (alignment)
    {}

    unsigned long memory_buffer_info_t::size
      (expr::eval::context const& input) const
    {
      expr::eval::context context (input);
      return boost::get<unsigned long>
        (expr::parse::parser (_size).eval_all (context));
    }

    unsigned long memory_buffer_info_t::alignment
       (expr::eval::context const& input) const
     {
       expr::eval::context context (input);
       return boost::get<unsigned long>
         (expr::parse::parser (_alignment).eval_all (context));
     }
  }
}
