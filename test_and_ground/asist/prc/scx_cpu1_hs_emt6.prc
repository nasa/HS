PROC scx_cpu1_hs_emt6
;*******************************************************************************
;  Test Name:  hs_emt6
;  Test Level: Build Verification
;  Test Type:  Functional
;
;  Test Description
;	The purpose of this procedure is to generate an Event Monitoring Table
;	that contains the maximum entries as defined by the 
;	HS_MAX_MONITORED_EVENTS configuration parameter.
;
;  Requirements Tested
;       None
;
;  Prerequisite Conditions
;	None
;
;  Assumptions and Constraints
;	None.
;
;  Change History
;
;	Date		Name		Description
;       06/24/09	W. Moleski	Initial release
;       01/13/11        W. Moleski      Added variables for app and table names
;                                       and dynamically created the endmnemonic
;
;  Arguments
;	None.
;
;  Procedures Called
;	Name			 Description
;       create_tbl_file_from_cvt Procedure that creates a load file from
;                                the specified arguments and cvt
;
;  Expected Test Results and Analysis
;
;**********************************************************************
local logging = %liv (log_procedure)
%liv (log_procedure) = FALSE

#include "hs_platform_cfg.h"
#include "hs_tbldefs.h"

write ";*********************************************************************"
write ";  define local variables "
write ";*********************************************************************"
;; CPU1 is the default
local apid = 0xf73
local HSAppName = "HS"                  
local emtTblName = HSAppName & "." & HS_EMT_TABLENAME
local hostCPU = "CPU3"

write ";*********************************************************************"
write ";  Define the Application Monitoring Table "
write ";*********************************************************************"
; Set the AppNames for each Entry
SCX_CPU1_HS_EMT[1].AppName = "CFE_ES"
SCX_CPU1_HS_EMT[1].EventID = 3

SCX_CPU1_HS_EMT[2].AppName = "CFE_EVS"
SCX_CPU1_HS_EMT[2].EventID = 0

SCX_CPU1_HS_EMT[3].AppName = "CFE_TIME"
SCX_CPU1_HS_EMT[3].EventID = 4

SCX_CPU1_HS_EMT[4].AppName = "CFE_TBL"
SCX_CPU1_HS_EMT[4].EventID = 10

SCX_CPU1_HS_EMT[5].AppName = "CFE_SB"
SCX_CPU1_HS_EMT[5].EventID = 35

SCX_CPU1_HS_EMT[6].AppName = "HS"
SCX_CPU1_HS_EMT[6].EventID = 23

SCX_CPU1_HS_EMT[7].AppName = "TST_HS"
SCX_CPU1_HS_EMT[7].EventID = 2

SCX_CPU1_HS_EMT[8].AppName = "CS"
SCX_CPU1_HS_EMT[8].EventID = 2

SCX_CPU1_HS_EMT[9].AppName = "TST_CS"
SCX_CPU1_HS_EMT[9].EventID = 2

SCX_CPU1_HS_EMT[10].AppName = "SC"
SCX_CPU1_HS_EMT[10].EventID = 52

SCX_CPU1_HS_EMT[11].AppName = "TST_SC"
SCX_CPU1_HS_EMT[11].EventID = 2

SCX_CPU1_HS_EMT[12].AppName = "MM"
SCX_CPU1_HS_EMT[12].EventID = 2

SCX_CPU1_HS_EMT[13].AppName = "TST_MM"
SCX_CPU1_HS_EMT[13].EventID = 2

SCX_CPU1_HS_EMT[14].AppName = "MD"
SCX_CPU1_HS_EMT[14].EventID = 10

SCX_CPU1_HS_EMT[15].AppName = "TST_MD"
SCX_CPU1_HS_EMT[15].EventID = 2

SCX_CPU1_HS_EMT[16].AppName = "HK"
SCX_CPU1_HS_EMT[16].EventID = 4

for index = 1 to HS_MAX_MONITORED_EVENTS do
  SCX_CPU1_HS_EMT[index].NullTerm = 0
  SCX_CPU1_HS_EMT[index].ActionType = HS_EMT_ACT_NOACT
enddo

;; Restore procedure logging
%liv (log_procedure) = logging

local endmnemonic = "SCX_CPU1_HS_EMT[" & HS_MAX_MONITORED_EVENTS & "].ActionType"

;; Create the Table Load file
s create_tbl_file_from_cvt (hostCPU,apid,"Event Monitoring Table Load 6","hs_def_emt6",emtTblName,"SCX_CPU1_HS_EMT[1].AppName",endmnemonic)

write ";*********************************************************************"
write ";  End procedure SCX_CPU1_hs_emt6 "
write ";*********************************************************************"
ENDPROC
