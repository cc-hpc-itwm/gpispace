namespace gspc
{
  namespace job
  {
    namespace finish_reason
    {
      template<typename Archive>
        void Cancelled::serialize (Archive& ar, unsigned int)
      {
        ar & reason;
      }
    }
  }
}
