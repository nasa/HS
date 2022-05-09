##############################################################################
## File: README.txt
##
## Purpose: CFS HS application unit test instructions, results, and coverage
##############################################################################

-------------------------
HS Unit Test Instructions
-------------------------
This unit test was run in a virtual machine running Ubuntu 18.04 and uses the
ut-assert stubs and default hooks for the cFE, OSAL and PSP.

To run the unit test enter the following commands at the command line prompt in
the top-level cFS directory (after ensuring that HS is listed as a target). Note
that in order to successfully compile the unit tests the "-Werror" compilation
flag must be disabled.

make distclean
make SIMULATION=native ENABLE_UNIT_TESTS=true prep
make
make test
make lcov

HS 2.4.0 Unit Test Results:

Tests Executed:    208
Assert Pass Count: 1338
Assert Fail Count: 0

==========================================================================
hs_app.c - Line Coverage:     100.0%
           Function Coverage: 100.0%
           Branch Coverage:   100.0%

==========================================================================
hs_cmds.c - Line Coverage:     100.0%
            Function Coverage: 100.0%
            Branch Coverage:   100.0%

==========================================================================
hs_custom.c - Line Coverage:     100.0%
              Function Coverage: 100.0%
              Branch Coverage:    94.6%

Branch coverage gap is caused by several conditions in hs_custom.c which
rely on platform configuration parameters.  Certain branches cannot be
covered with the default configuration parameters.
==========================================================================
hs_monitors.c - Line Coverage:     100.0%
                Function Coverage: 100.0%
                Branch Coverage:    98.8%

==========================================================================
hs_utils.c - Line Coverage:     100.0%
             Function Coverage: 100.0%
             Branch Coverage:   100.0%

==========================================================================
