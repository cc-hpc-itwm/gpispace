ADD_ASCIIDOC (user-asciidoc-html
  INPUT user/gpispace.txt
  OUTPUT user.html
  BACKEND xhtml11
  )

add_subdirectory (tutorial)
