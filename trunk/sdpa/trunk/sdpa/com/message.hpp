#ifndef SDPA_COM_MESSAGE_HPP
#define SDPA_COM_MESSAGE_HPP 1

#include <string>

namespace sdpa
{
  namespace com
  {
    template < typename NameType
             , typename MessageIdType
             , typename DataType
             >
    class basic_message_t
    {
    public:
      typedef NameType name_type;
      typedef MessageIdType message_id_type;
      typedef DataType data_type;

      basic_message_t ( name_type const & from
                      , name_type const & to
                      , message_id_type const & message_id
                      , data_type const & data
                      );

      name_type const & from () const;
      name_type const & to () const;
      data_type const & data () const;
      message_id_type const & id () const;
    private:
      name_type m_from_;
      name_type m_to_;
      message_id_type m_id_;
      data_type m_data_;
    };

    typedef basic_message_t< std::string
                           , std::string
                           , std::string
                           > message_t;
  }
}

#include "message.tcc"

#endif
