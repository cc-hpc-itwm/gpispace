namespace gspc
{
  template<typename Archive>
    void Resource::serialize (Archive& ar, unsigned int)
  {
    ar & resource_class;
  }
}
