namespace gspc
{
  namespace job
  {
    template<typename Archive>
      void ID::serialize (Archive& ar, unsigned int)
    {
      ar & id;
      ar & task_id;
    }
  }
}
