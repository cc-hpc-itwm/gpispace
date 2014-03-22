#ifndef FHG_COM_MESSAGE_HPP
#define FHG_COM_MESSAGE_HPP 1

#include <fhgcom/header.hpp>

#include <vector>

namespace fhg
{
  namespace com
  {
    struct message_t
    {
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

      void resize (const std::size_t n)
      {
        data.resize (n);
        header.length = n;
      }

      void resize ()
      {
        data.resize (header.length);
      }

      template <typename Iterator>
      void assign (Iterator begin, Iterator end)
      {
        data.assign(begin, end);
        header.length = data.size ();
      }

      const char * buf () const { return &data[0]; }
      std::size_t size () const { return data.size(); }

      p2p::header_t header;
      std::vector<char> data;
    };
  }
}

#endif
