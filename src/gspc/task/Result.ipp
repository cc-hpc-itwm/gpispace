namespace gspc
{
  namespace task
  {
    template<typename Archive>
      void Result::serialize (Archive& ar, unsigned int)
    {
      ar & outputs;
      ar & heureka_group;
    }
  }
}
