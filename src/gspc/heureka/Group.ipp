namespace gspc
{
  namespace heureka
  {
    template<typename Archive>
      void Group::serialize (Archive& ar, unsigned int)
    {
      ar & id;
    }
  }
}
