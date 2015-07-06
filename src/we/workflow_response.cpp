#include <we/workflow_response.hpp>

#include <we/type/value/peek_or_die.hpp>
#include <we/type/value/serialize.hpp>

#include <rpc/client.hpp>

#include <algorithm>

namespace we
{
  void workflow_response ( pnet::type::value::value_type const& rpc_server
                         , pnet::type::value::value_type const& value
                         )
  {
    fhg::rpc::remote_endpoint remote_client
      ( pnet::type::value::peek_or_die<std::string> (rpc_server, {"address"})
      , pnet::type::value::peek_or_die<unsigned int> (rpc_server, {"port"})
      );
    fhg::rpc::sync_remote_function<void (pnet::type::value::value_type)>
      (remote_client, "set_result") (value);
  }

  namespace
  {
    class is_field : public boost::static_visitor<bool>
    {
    public:
      is_field (std::string const& name, std::string const& type)
        : _name (name)
        , _type (type)
      {}
      bool operator() (std::pair<std::string, std::string> const& field) const
      {
        return field.first == _name && field.second == _type;
      }
      template<typename T>
        bool operator() (T const&) const
      {
        return false;
      }

    private:
      std::string const _name;
      std::string const _type;
    };

    class struct_has_field : public boost::static_visitor<bool>
    {
    public:
      struct_has_field (std::string const& name, std::string const& type)
        : _name (name)
        , _type (type)
      {}
      bool operator() (std::pair< std::string
                                , pnet::type::signature::structure_type
                                > const& fields
                      ) const
      {
        return std::any_of
          ( fields.second.begin(), fields.second.end()
          , [this] (pnet::type::signature::field_type const& field)
            {
              return boost::apply_visitor (is_field (_name, _type), field);
            }
          );
      }

    private:
      std::string const _name;
      std::string const _type;
    };

    class has_field : public boost::static_visitor<bool>
    {
    public:
      has_field (std::string const& name, std::string const& type)
        : _name (name)
        , _type (type)
      {}
      bool operator() (std::string const&) const
      {
        return false;
      }
      bool operator() (pnet::type::signature::structured_type const& s) const
      {
        return boost::apply_visitor (struct_has_field (_name, _type), s);
      }

    private:
      std::string const _name;
      std::string const _type;
    };
  }

  bool is_rpc_server_description
    (pnet::type::signature::signature_type const& signature)
  {
    return boost::apply_visitor (has_field ("address", "string"), signature)
      && boost::apply_visitor (has_field ("port", "unsigned int"), signature)
      ;
  }
}
