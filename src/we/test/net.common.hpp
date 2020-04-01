namespace
{
  void unexpected_workflow_response
    (pnet::type::value::value_type const&, pnet::type::value::value_type const&)
  {
    throw std::logic_error ("got unexpected workflow_response");
  }

  we::type::property::type no_properties()
  {
    return {};
  }
}
