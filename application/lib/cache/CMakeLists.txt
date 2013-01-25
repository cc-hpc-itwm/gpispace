set (xmls
  done_with_slot.xpnet
  fill.xpnet
  get_slot_for_id.xpnet
  init.xpnet
  start_to_fill.xpnet
  type.xpnet
)

install (FILES ${xmls}
  DESTINATION ${PNETC_LIB_DIR}/cache
  COMPONENT examples
  )

add_subdirectory (get)
add_subdirectory (demo)
