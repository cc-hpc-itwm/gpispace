/*
 * =====================================================================================
 *
 *       Filename:  exception.hpp
 *
 *    Description:  exception definitions
 *
 *        Version:  1.0
 *        Created:  03/15/2010 01:31:17 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_LAYER_EXCEPTION_HPP
#define WE_MGMT_LAYER_EXCEPTION_HPP 1

#include <we/type/id.hpp>

#include <stdexcept>

namespace we
{
  namespace mgmt
  {
    namespace exception
    {
      class activity_not_found : std::runtime_error
      {
      public:
        activity_not_found ( const std::string& msg
                           , const petri_net::activity_id_type& id
                           )
          : std::runtime_error (msg)
          , _id (id)
        {}
        virtual ~activity_not_found() throw() {}
        const petri_net::activity_id_type& id() const { return _id; }

      private:
        const petri_net::activity_id_type _id;
      };

      struct validation_error : public std::runtime_error
      {
        validation_error (const std::string & msg)
          : std::runtime_error (msg)
        {}

        ~validation_error () throw ()
        {}
      };

      struct already_there : public std::runtime_error
      {
      public:
        typedef std::string external_id_type;

        already_there ( const std::string& msg
                      , const external_id_type& ext_id
                      )
          : std::runtime_error (msg)
          , _id (ext_id)
        {}
        ~already_there () throw ()
        {}

        const external_id_type& id() const { return _id; }

      private:
        const external_id_type _id;
      };

      template <typename IdType>
      struct no_such_mapping : public std::runtime_error
      {
        no_such_mapping (const std::string & msg, IdType const & an_id)
          : std::runtime_error (msg)
          , id (an_id)
        {}

        ~no_such_mapping () throw ()
        {}

        const IdType id;
      };
    }
  }
}

#endif
