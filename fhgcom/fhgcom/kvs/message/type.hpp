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
#include <fhgcom/kvs/message/error.hpp>
#include <fhgcom/kvs/message/save.hpp>
#include <fhgcom/kvs/message/load.hpp>
#include <fhgcom/kvs/message/list.hpp>
#include <fhgcom/kvs/message/req_list.hpp>
#include <fhgcom/kvs/message/clear.hpp>
#include <fhgcom/kvs/message/atomic.hpp>
#include <fhgcom/kvs/message/term.hpp>
#include <fhgcom/kvs/message/ping.hpp>

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
                              , msg_save
                              , msg_load
                              , req_list
                              , list
                              , clear
                              , msg_inc
                              , msg_term
                              , msg_ping
                              > type;
      }
    }
  }
}

#endif
