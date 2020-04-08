namespace gspc
{
  template<typename Archive>
    void MapInput::serialize (Archive& ar, unsigned int)
  {
    ar & N;
    ar & i;
    ar & o;
  }

  template<typename Archive>
    void MapWorkflowEngine::WorkflowState::serialize
      (Archive& ar, unsigned int /* version */)
  {
    ar & implementation;
    ar & N;
    ar & i;
  }
}
