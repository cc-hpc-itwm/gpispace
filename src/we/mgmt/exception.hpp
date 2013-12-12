// {petry,rahn}@itwm.fhg.de

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

      struct no_such_mapping : public std::runtime_error
      {
      public:
        typedef std::string external_id_type;

        no_such_mapping ( const std::string& msg
                        , const external_id_type& ext_id
                        )
          : std::runtime_error (msg)
          , _id (ext_id)
        {}
        ~no_such_mapping () throw ()
        {}

        const external_id_type& id() const { return _id; }

      private:
        const external_id_type _id;
      };
    }
  }
}

#endif
