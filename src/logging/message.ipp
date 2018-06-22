namespace fhg
{
  namespace logging
  {
    template<typename Archive>
      void message::serialize (Archive& ar, unsigned int)
    {
      ar & _content;
      ar & _category;
    }
  }
}
