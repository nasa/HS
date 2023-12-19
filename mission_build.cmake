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
  hs_fcncodes.h
  hs_interface_cfg.h
  hs_mission_cfg.h
  hs_msgdefs.h
  hs_msg.h
  hs_msgstruct.h
  hs_tbldefs.h
  hs_tbl.h
  hs_tblstruct.h
  hs_topicids.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, these files come generated from the EDS tool
  set(HS_CFGFILE_SRC_hs_interface_cfg "hs_eds_designparameters.h")
  set(HS_CFGFILE_SRC_hs_fcncodes      "hs_eds_cc.h")
  set(HS_CFGFILE_SRC_hs_msgstruct     "hs_eds_typedefs.h")
  set(HS_CFGFILE_SRC_hs_tblstruct     "hs_eds_typedefs.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(HS_CFGFILE ${HS_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${HS_CFGFILE}" NAME_WE)
  if (DEFINED HS_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${HS_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${HS_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${HS_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
