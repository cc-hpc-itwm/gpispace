#ifndef FHG_COM_KVS_MESSAGE_SHOW_HPP
#define FHG_COM_KVS_MESSAGE_SHOW_HPP 1

#include <stdexcept>
#include <fhgcom/kvs/message/type.hpp>
#include <boost/variant/static_visitor.hpp>

namespace fhg
{
  namespace com
  {
    namespace kvs
    {
      namespace message
      {
        namespace visitor
        {
          class show : public boost::static_visitor<>
          {
          public:
            show (std::ostream & os)
              : stream_(os)
            {}

            void operator() (put const & m)
            {
              stream_ << "PUT (.to be filled.)";
            }

            void operator() (msg_get const & m)
            {
              stream_ << "GET " << m.key();
            }

            void operator() (del const & m)
            {
              stream_ << "DEL " << m.key();
            }

            void operator() (error const & m)
            {
              if (m.ec() != error::KVS_ENOERROR)
              {
                stream_ << "ERROR " << error::code_to_string (m.ec()) << ": " << m.what();
              }
              else
              {
                stream_ << "OK";
              }
            }

            void operator() (msg_save const & m)
            {
              stream_ << "SAVE " << m.file();
            }

            void operator() (msg_load const & m)
            {
              stream_ << "LOAD " << m.file();
            }

            void operator() (list const & m)
            {
              stream_ << "LIST";
            }

            template <typename T>
            void operator() (T const &) const
            {
              throw std::runtime_error ("STRANGE: show: got unsupported message type");
            }
          private:
            std::ostream & stream_;
          };
        }

        inline std::ostream & operator << (std::ostream & os, type const & msg)
        {
          visitor::show v (os);
          boost::apply_visitor (v, msg);
          return os;
        }
      }
    }
  }
}

#endif
