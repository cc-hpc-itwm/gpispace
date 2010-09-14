// message implementation header file -*- C++ -*-

namespace sdpa
{
  namespace com
  {
    template < typename NameType
             , typename MessageIdType
             , typename DataType
             >
    basic_message_t<NameType, MessageIdType, DataType>
    ::basic_message_t ( NameType const & from
                      , NameType const & to
                      , MessageIdType const & message_id
                      , DataType const & data
                      )
      : m_from_(from)
      , m_to_(to)
      , m_id_(message_id)
      , m_data_(data)
    {}

    template < typename NameType
             , typename MessageIdType
             , typename DataType
             >
    NameType const &
    basic_message_t<NameType, MessageIdType, DataType>
    ::from () const
    {
      return m_from_;
    }

    template < typename NameType
             , typename MessageIdType
             , typename DataType
             >
    NameType const &
    basic_message_t<NameType, MessageIdType, DataType>
    ::to () const
    {
      return m_to_;
    }

    template < typename NameType
             , typename MessageIdType
             , typename DataType
             >
    DataType const &
    basic_message_t<NameType, MessageIdType, DataType>
    ::data () const
    {
      return m_data_;
    }

    template < typename NameType
             , typename MessageIdType
             , typename DataType
             >
    MessageIdType const &
    basic_message_t<NameType, MessageIdType, DataType>
    ::id () const
    {
      return m_id_;
    }
  }
}
