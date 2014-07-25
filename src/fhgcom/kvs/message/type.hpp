#ifndef FHG_COM_KVS_MESSAGE_HPP
#define FHG_COM_KVS_MESSAGE_HPP 1

#include <fhgcom/kvs/message/del.hpp>
#include <fhgcom/kvs/message/error.hpp>
#include <fhgcom/kvs/message/get.hpp>
#include <fhgcom/kvs/message/list.hpp>
#include <fhgcom/kvs/message/put.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/variant.hpp>

namespace fhg
{
  namespace com
  {
    namespace kvs
    {
      namespace message
      {
        typedef boost::variant< put
                              , msg_get
                              , del
                              , error
                              , list
                              > type;
      }
    }
  }
}

#endif
