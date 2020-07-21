namespace gspc
{
  namespace remote_interface
  {
    template<typename Archive>
      void ID::serialize (Archive& ar, unsigned int)
    {
      ar & id;
    }
  }
}
