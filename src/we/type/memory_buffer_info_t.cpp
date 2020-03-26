#include <we/expr/parse/parser.hpp>
#include <we/type/memory_buffer_info_t.hpp>

namespace we
{
  namespace type
  {
    //! For deserialization only.
    memory_buffer_info_t::memory_buffer_info_t() = default;

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
        (_size.ast().eval_all (context));
    }

    unsigned long memory_buffer_info_t::alignment
       (expr::eval::context const& input) const
     {
       expr::eval::context context (input);
       auto const alignment 
         ( boost::get<unsigned long>
             (_alignment.ast().eval_all (context))
         );
    
       //note: equivalent to std::has_single_bit (c++ 20)
       auto const is_power_of_2 =
         [] (unsigned long n)
         {
           return n != 0 && (n & (n - 1)) == 0;
         };
       
       if (!is_power_of_2 (alignment))
       {
         throw std::runtime_error 
           ("Invalid alignment expression. The alignment should be a power of 2!");
       }
      
       return alignment;
     }
  }
}
