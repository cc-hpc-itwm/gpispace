include (fhg_executable_macros)

fhg_add_runtime_executable (
  we-exec.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 we-dev # for the client only
                 ${FhgLog_LIBRARY}
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_DL_LIBS}
)

fhg_add_runtime_executable (
  we-eval.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 we-dev # for the client only
                 ${FhgLog_LIBRARY}
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_THREAD_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
                 ${CMAKE_DL_LIBS}
)

fhg_add_runtime_executable (
  pnetget.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (
  pnetput.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (
  pnet2dot.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (
  pneti.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (
  pnetsc.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (
  pnetlt.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

fhg_add_runtime_executable (
  pnetc.cpp
  LINK_LIBRARIES fhg-revision
                 pnet
                 ${Boost_FILESYSTEM_LIBRARY}
                 ${Boost_PROGRAM_OPTIONS_LIBRARY}
                 ${Boost_SERIALIZATION_LIBRARY}
                 ${Boost_SYSTEM_LIBRARY}
)

add_executable (iheapsort iheapsort.c)
target_link_libraries (iheapsort mmgr)

add_executable (trie_vs_smap trie_vs_smap.c)
target_link_libraries (trie_vs_smap mmgr)

add_executable (perftmmgr perftmmgr.c)
target_link_libraries (perftmmgr mmgr)
