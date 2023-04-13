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
  hs_tbldefs.h
  hs_tbl.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(HS_CFGFILE_SRC_hs_mission_cfg "hs_eds_designparameters.h")
  #set(HS_CFGFILE_SRC_hs_tbldefs "hs_eds_designparameters.h")
  set(HS_CFGFILE_SRC_hs_msgdefs "hs_eds_cc.h")
  set(HS_CFGFILE_SRC_hs_msg "hs_eds_typedefs.h")
  set(HS_CFGFILE_SRC_hs_tbl "hs_eds_typedefs.h")

else(CFE_EDS_ENABLED_BUILD)

  # In a non-EDS build, the user supplies these files, plus a couple more
  # There are defaults for all in the "config" dir
  list (APPEND HS_MISSION_CONFIG_FILE_LIST
    hs_msg_structs.h
    hs_table_structs.h
  )

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(HS_CFGFILE ${HS_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${HS_CFGFILE}" NAME_WE)
  if (DEFINED HS_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE "${HS_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE "${CMAKE_CURRENT_LIST_DIR}/config/default_${HS_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${HS_CFGFILE}"
    FALLBACK_FILE       ${DEFAULT_SOURCE}
  )
endforeach()
