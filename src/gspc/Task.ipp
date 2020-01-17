namespace gspc
{
  template<typename Archive>
    void Task::serialize (Archive& ar, unsigned int)
  {
    ar & id;
    ar & resource_class;
    ar & inputs;
    ar & so;
    ar & symbol;
  }
}
