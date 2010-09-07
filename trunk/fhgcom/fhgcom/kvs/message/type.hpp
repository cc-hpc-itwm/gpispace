#ifndef FHG_COM_KVS_MESSAGE_HPP
#define FHG_COM_KVS_MESSAGE_HPP 1

#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>

#include <fhgcom/kvs/message/put.hpp>
#include <fhgcom/kvs/message/get.hpp>
#include <fhgcom/kvs/message/del.hpp>
#include <fhgcom/kvs/message/value.hpp>
#include <fhgcom/kvs/message/error.hpp>
#include <fhgcom/kvs/message/save.hpp>
#include <fhgcom/kvs/message/load.hpp>

namespace fhg
{
  namespace com
  {
    namespace kvs
    {
      namespace message
      {

        typedef boost::variant< put
                              , get
                              , del
                              , value
                              , error
                              , msg_save
                              , msg_load
                              > type;
      }
    }
  }
}

#endif
