include (doc)

ADD_DOC (user-doc-html
  INPUT user
  OUTPUT user.html
  BACKEND xhtml11
)

ADD_DOC (user-doc-docbook
  INPUT user
  OUTPUT user.docbook
  BACKEND docbook
)
