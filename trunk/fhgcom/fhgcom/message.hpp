#ifndef FHG_COM_MESSAGE_HPP
#define FHG_COM_MESSAGE_HPP 1

#include <vector>

#include <fhgcom/header.hpp>

namespace fhg
{
  namespace com
  {
    struct message_t
    {
      typedef p2p::header_t header_t;

      message_t ()
        : data()
      {}

      explicit
      message_t (std::size_t len)
        : data(len)
      {}

      message_t (const char * d, std::size_t len)
        : data(d, d+len)
      {}

      template <typename Iterator>
      message_t (Iterator begin, Iterator end)
        : data(begin, end)
      {}

      message_t (const message_t & other)
        : data(other.data)
      {}

      ~message_t ()
      {}

      message_t & operator= (const message_t & rhs)
      {
        if (this != &rhs)
        {
          header = rhs.header;
          data = rhs.data;
        }
        return *this;
      }

      void resize (const std::size_t n)
      {
        data.resize (n);
      }

      void resize ()
      {
        data.resize (header.length);
      }

      template <typename Iterator>
      void assign (Iterator begin, Iterator end)
      {
        data.assign(begin, end);
      }

      const char * buf () const { return &data[0]; }
      std::size_t size () const { return data.size(); }

      header_t header;
      std::vector<char> data;
    };
  }
}

#endif
