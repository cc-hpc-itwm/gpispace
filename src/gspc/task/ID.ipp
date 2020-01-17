namespace gspc
{
  namespace task
  {
    template<typename Archive>
      void ID::serialize (Archive& ar, unsigned int)
    {
      ar & id;
    }
  }
}
