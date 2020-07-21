namespace gspc
{
  namespace resource
  {
    template<typename Archive>
      void ID::serialize (Archive& ar, unsigned int)
    {
      ar & remote_interface;
      ar & id;
    }
  }
}
