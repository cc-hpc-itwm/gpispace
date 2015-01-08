# bernd.loerwald@itwm.fraunhofer.de

ADD_ASCIIDOC (share_example_workerlist-asciidoc-html
  INPUT README
  OUTPUT README.html
  BACKEND xhtml11
)

add_subdirectory (test)
