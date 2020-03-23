namespace fhg
{
  namespace vmem
  {
    template<typename Archive>
      void netdev_id::serialize (Archive& ar, unsigned int)
    {
      ar & value;
    }
  }
}
