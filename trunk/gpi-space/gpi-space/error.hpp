#ifndef GPI_SPACE_ERROR_HPP
#define GPI_SPACE_ERROR_HPP 1

namespace gpi
{
  namespace error
  {
    namespace errc
    {
      enum errc_t
        {
          success = 0,
          unknown,
          timeout
        };
    }

    class code_t
    {
    public:
      explicit code_t (int val)
        : m_val (val)
      {}

      virtual ~code_t () {}
      virtual const char * name() const = 0;
      int value () const { return m_val; }
    private:
      int m_val;
    };

    struct success : public code_t
    {
      success ()
        : code_t (errc::success)
      {}

      const char * name () const { return "success"; }
    };

    struct unknown : public code_t
    {
      unknown ()
        : code_t (errc::unknown)
      {}

      const char * name () const { return "success"; }
    };

    struct timeout : public code_t
    {
      timeout ()
        : code_t (errc::timeout)
      {}

      const char * name () const { return "timeout"; }
    };
  }
}

#endif
