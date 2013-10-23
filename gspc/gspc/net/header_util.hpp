#ifndef GSPC_NET_HEADER_UTIL_HPP
#define GSPC_NET_HEADER_UTIL_HPP

#include <gspc/net/frame.hpp>
#include <boost/lexical_cast.hpp>

namespace gspc
{
  namespace net
  {
    namespace header
    {
      template <typename T>
      T get (frame const &f, frame::key_type const & key, T const &dflt)
      {
        frame::header_value v = f.get_header (key);
        if (v)
        {
          try
          {
            return boost::lexical_cast<T>(*v);
          }
          catch (boost::bad_lexical_cast const &)
          {
            return dflt;
          }
        }

        return dflt;
      }

      template <typename T>
      void set (frame &f, frame::key_type const &key, T const &val)
      {
        f.set_header (key, boost::lexical_cast<frame::value_type>(val));
      }

      class item_t
      {
      public:
        typedef boost::optional<frame::value_type> optional_value;

        explicit
        item_t (frame::key_type const &key)
          : m_key (key)
        {}

        item_t ( frame::key_type const &key
               , frame::value_type const &val
               )
          : m_key (key)
          , m_val (val)
        {}

        item_t ( frame::key_type const &key
               , frame const & f
               )
          : m_key (key)
          , m_val (f.get_header (key))
        {}

        void apply_to (frame & f) const
        {
          f.set_header (m_key, *m_val);
        }

        void get (frame const &f)
        {
          m_val = f.get_header (m_key);
        }

        frame::key_type key () const { return m_key; }
        optional_value  value () const { return m_val; }
      private:
        frame::key_type m_key;
        optional_value  m_val;
      };

#define MK_HEADER(name, key)                    \
      class name : public item_t                \
      {                                         \
      public:                                   \
        name ()                                 \
          : item_t (key)                        \
        {}                                      \
        explicit                                \
          name (frame::value_type const & val)  \
          : item_t (key, val)                   \
        {}                                      \
        explicit                                \
          name (frame const & f)                \
          : item_t (key, f)                     \
        {}                                      \
      }

      MK_HEADER (destination, "destination");
      MK_HEADER (receipt, "receipt");
      MK_HEADER (receipt_id, "receipt-id");
      MK_HEADER (message_id, "message-id");
      MK_HEADER (correlation_id, "correlation-id");
      MK_HEADER (content_length, "content-length");
      MK_HEADER (content_type, "content-type");
      MK_HEADER (id, "id");
      MK_HEADER (transaction, "transaction");
      MK_HEADER (heart_beat, "heart-beat");
      MK_HEADER (version, "version");
      MK_HEADER (login, "login");
      MK_HEADER (passcode, "passcode");

#undef MK_HEADER
    }
  }
}

#endif
