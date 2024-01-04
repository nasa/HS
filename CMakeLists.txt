project(CFS_HS C)

set(APP_SRC_FILES
  fsw/src/hs_monitors.c
  fsw/src/hs_utils.c
  fsw/src/hs_sysmon.c
  fsw/src/hs_app.c
  fsw/src/hs_cmds.c
)

if (CFE_EDS_ENABLED_BUILD)
  list(APPEND APP_SRC_FILES
    fsw/src/hs_eds_dispatch.c
  )
else()
  list(APPEND APP_SRC_FILES
    fsw/src/hs_dispatch.c
  )
endif()

# Create the app module
add_cfe_app(hs ${APP_SRC_FILES})

# This permits direct access to public headers in the fsw/inc directory
target_include_directories(hs PUBLIC fsw/inc)
add_cfe_app_dependency(hs iodriver)

set(APP_TABLE_FILES
  fsw/tables/hs_mat.c
  fsw/tables/hs_xct.c
  fsw/tables/hs_amt.c
  fsw/tables/hs_emt.c
)

add_cfe_tables(hs ${APP_TABLE_FILES})

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if(ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif()
