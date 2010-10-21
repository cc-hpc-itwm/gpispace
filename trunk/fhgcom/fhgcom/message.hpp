#ifndef FHG_COM_MESSAGE_HPP
#define FHG_COM_MESSAGE_HPP 1

#include <vector>

namespace fhg
{
  namespace com
  {
    struct message_t
    {
      message_t ()
        : data_()
      {}

      explicit
      message_t (std::size_t len)
        : data_(len)
      {}

      message_t (const char * d, std::size_t len)
        : data_(d, d+len)
      {}

      message_t (const message_t & other)
        : data_(other.data_)
      {}

      ~message_t ()
      {}

      message_t & operator= (const message_t & rhs)
      {
        if (this != &rhs)
        {
          data_ = rhs.data_;
        }
        return *this;
      }

      void resize (std::size_t n)
      {
        data_.resize (n);
      }

      std::vector<char> & data () { return data_; }
      const std::vector<char> & data () const { return data_; }

      const char * buf () const { return &data_[0]; }
      std::size_t size () const { return data_.size(); }
    private:
      std::vector<char> data_;
    };
  }
}
#endif
