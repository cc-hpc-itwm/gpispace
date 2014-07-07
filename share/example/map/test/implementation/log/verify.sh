# mirko.rahn@itwm.fraunhofer.de

test "$(pnetget -i "${RESULT}" -p done)" = "[]"             || die $EX_ERR_RES
