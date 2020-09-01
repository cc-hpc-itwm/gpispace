namespace fhg
{
  namespace iml
{
  namespace rif
  {
    template<typename... String>
      void started_process_promise::set_result (String... messages)
    {
      return set_result (std::vector<std::string> {messages...});
    }
  }
}
}
