# bernd.loerwald@itwm.fraunhofer.de

ADD_ASCIIDOC (tutorial_sequence-asciidoc-html
  INPUT README
  OUTPUT README.html
  BACKEND xhtml11
)

add_subdirectory (test)
