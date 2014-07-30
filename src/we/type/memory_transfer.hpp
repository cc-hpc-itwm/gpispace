// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_MEMORY_TRANSFER_HPP
#define WE_TYPE_MEMORY_TRANSFER_HPP

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
        void serialize (Archive&, const unsigned int)
      {}
    };
  }
}

namespace boost
{
  namespace serialization
  {
    template<class Archive>
      inline void save_construct_data
        (Archive& ar, we::type::memory_transfer const* mt, const unsigned int)
    {
      ar << BOOST_SERIALIZATION_NVP (mt->global());
      ar << BOOST_SERIALIZATION_NVP (mt->local());
      ar << BOOST_SERIALIZATION_NVP (mt->not_modified_in_module_call());
    }

    template<class Archive>
      inline void load_construct_data
        (Archive& ar, we::type::memory_transfer* mt, const unsigned int)
    {
      std::string global;
      std::string local;
      boost::optional<bool> not_modified_in_module_call;
      ar >> BOOST_SERIALIZATION_NVP (global);
      ar >> BOOST_SERIALIZATION_NVP (local);
      ar >> BOOST_SERIALIZATION_NVP (not_modified_in_module_call);
      ::new (mt) we::type::memory_transfer
          (global, local, not_modified_in_module_call);
    }
  }
}

#endif
