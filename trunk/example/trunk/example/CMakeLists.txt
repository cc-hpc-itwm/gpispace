add_state_machine(example)


set(example_src example.cpp
  example_sm.h example_sm.cpp
  )

ADD_LIBRARY(example ${example_src})
