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

#include <stdexcept>

namespace we { namespace mgmt { namespace exception {
  template <typename Id>
  struct activity_not_found : std::runtime_error
  {
    typedef Id id_type;

    activity_not_found (std::string const& msg, id_type const& id_)
      : std::runtime_error(msg)
      , id(id_)
    {}

    virtual ~activity_not_found() throw() {}

    const id_type id;
  };

      struct validation_error : public std::runtime_error
      {
        validation_error (const std::string & msg)
          : std::runtime_error (msg)
        {}

        ~validation_error () throw ()
        {}
      };

      template <typename ExternalId>
      struct already_there : public std::runtime_error
      {
        already_there (const std::string & msg, ExternalId const & ext_id)
          : std::runtime_error (msg)
          , id (ext_id)
        { }

        ~already_there () throw ()
        { }

        const ExternalId id;
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
}}}

#endif
