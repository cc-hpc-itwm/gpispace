namespace gspc
{
  namespace job
  {
    namespace finish_reason
    {
      template<typename Archive>
        void Finished::serialize (Archive& ar, unsigned int)
      {
        ar & task_result;
      }
    }
  }
}
