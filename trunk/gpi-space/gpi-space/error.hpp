#ifndef GPI_SPACE_ERROR_HPP
#define GPI_SPACE_ERROR_HPP 1

namespace gpi
{
  namespace error
  {
    struct succes
    {
      static const int value = 0;
    };

    struct general
    {
      static const int value = 1;
    };

    struct timeout
    {
      static const int value = 42;
    };

    struct code_t
    {
    public:
      template <typename T>
      code_t ()
        : code (T::value)
      {}

      const int code;
    };

    namespace detail
    {
      template <>
      std::string message<0> ()
      {
        return "success";
      }

      template <int I>
      std::string message ()
      {
        return "unknown";
      }
    }

    std::string message (code_t const & ec)
    {
      return detail::message<ec.code> ();
    }
  }
}

#endif
