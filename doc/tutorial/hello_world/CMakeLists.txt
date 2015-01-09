# mirko.rahn@itwm.fraunhofer.de

ADD_ASCIIDOC (tutorial_hello_world-asciidoc-html
  INPUT README
  OUTPUT README.html
  BACKEND xhtml11
)

add_subdirectory (test)
