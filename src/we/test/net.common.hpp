namespace
{
  void unexpected_workflow_response
    (pnet::type::value::value_type const&, pnet::type::value::value_type const&)
  {
    throw std::logic_error ("got unexpected workflow_response");
  }

  std::mt19937& random_engine()
  {
    static std::mt19937 _ {std::random_device{}()};

    return _;
  }

  we::type::property::type no_properties()
  {
    return {};
  }
}