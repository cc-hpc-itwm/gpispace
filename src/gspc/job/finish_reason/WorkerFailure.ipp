namespace gspc
{
  namespace job
  {
    namespace finish_reason
    {
      template<typename Archive>
        void WorkerFailure::serialize (Archive& ar, unsigned int)
      {
        ar & exception;
      }
    }
  }
}
