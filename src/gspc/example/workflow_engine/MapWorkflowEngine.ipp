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

  //! \note BEGIN required by activity_t
  template<typename Archive>
    void MapWorkflowEngine::serialize (Archive& ar, unsigned int)
  {
    ar & _workflow_state;
    ar & _processing_state;
  }
  //! \note END required by activity_t
}
