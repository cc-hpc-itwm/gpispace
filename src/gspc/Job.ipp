namespace gspc
{
  template<typename Archive>
    void Job::serialize (Archive& ar, unsigned int)
  {
    ar & task;
  }
}
