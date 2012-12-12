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
#include <boost/unordered_map.hpp>
#include <boost/serialization/access.hpp>

namespace we
{
  namespace mgmt {

    struct activity_information_t
    {
      enum status_t
      {
        UNDEFINED = -1
      , PENDING
      , RUNNING
      , FINISHED
      , FAILED
      , CANCELLED
      , SUSPENDED
      };

      std::string name;
      status_t status;
      int level;

      typedef boost::unordered_map<std::string, std::string> data_t;
      data_t data;
    };

    struct basic_layer
    {
      typedef std::string id_type;
      typedef std::string result_type;
      typedef std::string reason_type;
      typedef std::string encoded_type;

      virtual void submit(const id_type & id, const encoded_type & ) = 0;
      virtual bool cancel(const id_type & id, const reason_type & reason) = 0;

      virtual bool finished(const id_type & id, const result_type & result) = 0;
      virtual bool failed( const id_type & id
                         , const result_type & result
                         , const int error_code
                         , const std::string & reason
                         ) = 0;
      virtual bool cancelled(const id_type & id) = 0;

      virtual bool fill_in_info (const id_type & id, activity_information_t & info) const = 0;

      virtual ~basic_layer() {}

      friend class boost::serialization::access;
      template <class Archive>
      void serialize(Archive&, const unsigned int ){}
    };
  }
}

#endif
