# mirko.rahn@itwm.fraunhofer.de

fhg_add_application_test (tutorial_n_of_m_seq
  SCRIPT seq
  RESOURCE_LOCK tutorial_n_of_m
)

fhg_add_application_test (tutorial_n_of_m_par
  SCRIPT par
  RESOURCE_LOCK tutorial_n_of_m
)
