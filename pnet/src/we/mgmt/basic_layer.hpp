/*
 * =====================================================================================
 *
 *       Filename:  basic_layer.hpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  03/03/2010 03:32:41 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_BASIC_LAYER_HPP
#define WE_MGMT_BASIC_LAYER_HPP 1

#include <string>
#include <boost/serialization/access.hpp>

#include <we/type/user_data.hpp>

namespace we
{
  namespace mgmt {

    struct basic_layer
    {
      typedef std::string id_type;
      typedef std::string result_type;
      typedef std::string reason_type;
      typedef std::string encoded_type;

      virtual void submit(const id_type & id, const encoded_type &, we::type::user_data const &) = 0;
      virtual bool cancel(const id_type & id, const reason_type & reason) = 0;

      virtual bool finished(const id_type & id, const result_type & result) = 0;
      virtual bool failed( const id_type & id
                         , const result_type & result
                         , const int error_code
                         , const std::string & reason
                         ) = 0;
      virtual bool cancelled(const id_type & id) = 0;

      virtual ~basic_layer() {}

      friend class boost::serialization::access;
      template <class Archive>
      void serialize(Archive&, const unsigned int ){}
    };
  }
}

#endif
