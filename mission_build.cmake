###########################################################
#
# HS App mission build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# Add stand alone documentation
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/docs/dox_src ${MISSION_BINARY_DIR}/docs/hs-usersguide)

# The list of header files that control the HS configuration
set(HS_MISSION_CONFIG_FILE_LIST
  hs_mission_cfg.h
  hs_msgdefs.h
  hs_msg.h
  hs_msg_structs.h
  hs_table_structs.h
  hs_tbldefs.h
  hs_tbl.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(HS_CFGFILE ${HS_MISSION_CONFIG_FILE_LIST})
  set(DEFAULT_SOURCE "${CMAKE_CURRENT_LIST_DIR}/config/default_${HS_CFGFILE}")
  generate_config_includefile(
    FILE_NAME           "${HS_CFGFILE}"
    FALLBACK_FILE       ${DEFAULT_SOURCE}
  )
endforeach()
