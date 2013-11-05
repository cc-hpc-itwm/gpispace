include (doc)

ADD_ASCIIDOC (user-asciidoc-html
  INPUT user.txt
  OUTPUT user.html
  BACKEND xhtml11
)

ADD_ASCIIDOC (user-asciidoc-docbook
  INPUT user.txt
  OUTPUT user.docbook
  BACKEND docbook
)

ADD_A2X (user-a2x-html
  INPUT user.txt
  FORMAT xhtml
)

ADD_A2X (user-a2x-pdf
  INPUT user.txt
  FORMAT pdf
  OPTION --fop
)
