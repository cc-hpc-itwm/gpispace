#ifndef FHG_COM_MESSAGE_HPP
#define FHG_COM_MESSAGE_HPP 1

namespace fhg
{
  namespace com
  {
    struct message_t
    {
      message_t ()
        : data_(0)
        , length_(0)
      {}

      explicit
      message_t (std::size_t len)
        : data_(0)
        , length_(len)
      {
        assert (len > 0);
        data_ = new char [len];
      }

      message_t (char ** data, std::size_t len)
        : data_(*data)
        , length_(len)
      {
        *data = 0;
      }

      message_t (const message_t & other)
        : data_ (0)
        , length_(0)
      {
        *this = other;
      }

      ~message_t ()
      {
        if (data_)
        {
          delete [] data_; data_ = 0;
        }
        length_ = 0;
      }

      message_t & operator= (const message_t & rhs)
      {
        if (this != &rhs)
        {
          this->~message_t ();
          if (rhs.data_)
          {
            length_ = rhs.length_;
            data_ = new char [length_];
            memcpy (data_, rhs.data_, length_);
          }
        }
        return *this;
      }

      std::size_t length () const { return length_; }
      const char * data () const { return data_; }
      char * data () { return data_; }
      void clear () { this->~message_t(); }
    private:
      char * data_;
      std::size_t length_;
    };
  }
}
#endif
