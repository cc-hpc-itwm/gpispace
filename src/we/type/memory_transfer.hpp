#pragma once

#include <boost/serialization/nvp.hpp>

#include <string>
#include <unordered_map>

namespace we
{
  namespace type
  {
    struct memory_transfer
    {
    public:
      //! \note serialization only
      memory_transfer() = default;

      memory_transfer
        ( std::string const& global
        , std::string const& local
        , boost::optional<bool> const& not_modified_in_module_call
        )
        : _global (global)
        , _local (local)
        , _not_modified_in_module_call (not_modified_in_module_call)
      {}
      std::string const& global() const
      {
        return _global;
      }
      std::string const& local() const
      {
        return _local;
      }
      boost::optional<bool> const& not_modified_in_module_call() const
      {
        return _not_modified_in_module_call;
      }
    private:
      std::string _global;
      std::string _local;
      boost::optional<bool> _not_modified_in_module_call;

      friend class boost::serialization::access;
      template<class Archive>
        void serialize (Archive& ar, const unsigned int)
      {
        ar & _global;
        ar & _local;
        ar & _not_modified_in_module_call;
      }
    };
  }
}
