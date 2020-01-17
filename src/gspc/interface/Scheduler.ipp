namespace gspc
{
  namespace interface
  {
    template<typename Derived>
        Scheduler::Scheduler (Derived* derived)
      : _comm_server_for_worker (derived)
    {}
  }
}
