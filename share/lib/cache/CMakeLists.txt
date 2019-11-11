install (FILES
  maybe_start_to_fill.xpnet
  done_with_slot.xpnet
  fill.xpnet
  get_slot_for_id.xpnet
  init.xpnet
  start_to_fill.xpnet
  type.xpnet
  DESTINATION ${PNETC_LIB_DIR}/cache
  )

add_subdirectory (demo)
