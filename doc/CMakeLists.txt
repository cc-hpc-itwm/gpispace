add_subdirectory (tutorial)

find_package (ASCIIDOC)
asciidoc (NAME gspc-monitor.adoc
  SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/gspc-monitor/gspc-monitor.adoc"
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/gspc-monitor.html"
  INSTALL
  INSTALL_DESTINATION share/gspc
)
