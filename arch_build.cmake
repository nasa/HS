###########################################################
#
# HS App platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the HS configuration
set(HS_PLATFORM_CONFIG_FILE_LIST
  hs_internal_cfg.h
  hs_custom.h
  hs_msgids.h
  hs_platform_cfg.h
)

if (CFE_EDS_ENABLED_BUILD)

  # In an EDS-based build, msgids come generated from the EDS tool
  set(HS_CFGFILE_SRC_hs_msgids "${CMAKE_CURRENT_LIST_DIR}/config/hs_eds_msg_topicids.h")

endif(CFE_EDS_ENABLED_BUILD)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(HS_CFGFILE ${HS_PLATFORM_CONFIG_FILE_LIST})
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
