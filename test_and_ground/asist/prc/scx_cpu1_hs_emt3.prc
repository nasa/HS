PROC scx_cpu1_hs_emt3
;*******************************************************************************
;  Test Name:  hs_emt3
;  Test Level: Build Verification
;  Test Type:  Functional
;
;  Test Description
;	The purpose of this procedure is to generate an Event Monitoring Table 
;	that contains an entry for the HS_NOOP event with an action to cause a
;	cFE Processor Reset
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
; Entry 1
SCX_CPU1_HS_EMT[1].AppName = "CFE_ES"
SCX_CPU1_HS_EMT[1].NullTerm = 0
SCX_CPU1_HS_EMT[1].EventID = 10
SCX_CPU1_HS_EMT[1].ActionType = HS_EMT_ACT_NOACT

;; Entry 2
SCX_CPU1_HS_EMT[2].AppName = "CFE_EVS"
SCX_CPU1_HS_EMT[2].NullTerm = 0
SCX_CPU1_HS_EMT[2].EventID = 10
SCX_CPU1_HS_EMT[2].ActionType = HS_EMT_ACT_NOACT

;; Entry 3
SCX_CPU1_HS_EMT[3].AppName = "CFE_TIME"
SCX_CPU1_HS_EMT[3].NullTerm = 0
SCX_CPU1_HS_EMT[3].EventID = 10
SCX_CPU1_HS_EMT[3].ActionType = HS_EMT_ACT_NOACT

;; Entry 4
SCX_CPU1_HS_EMT[4].AppName = "CFE_TBL"
SCX_CPU1_HS_EMT[4].NullTerm = 0
SCX_CPU1_HS_EMT[4].EventID = 10
SCX_CPU1_HS_EMT[4].ActionType = HS_EMT_ACT_NOACT

;; Entry 5
SCX_CPU1_HS_EMT[5].AppName = "CFE_SB"
SCX_CPU1_HS_EMT[5].NullTerm = 0
SCX_CPU1_HS_EMT[5].EventID = 10
SCX_CPU1_HS_EMT[5].ActionType = HS_EMT_ACT_NOACT

;; Entry 6
SCX_CPU1_HS_EMT[6].AppName = "HS"
SCX_CPU1_HS_EMT[6].NullTerm = 0
SCX_CPU1_HS_EMT[6].EventID = 23
SCX_CPU1_HS_EMT[6].ActionType = HS_EMT_ACT_PROC_RESET

for index = 7 to HS_MAX_MONITORED_EVENTS do
  SCX_CPU1_HS_EMT[index].AppName = ""
  SCX_CPU1_HS_EMT[index].NullTerm = 0
  SCX_CPU1_HS_EMT[index].EventID = 0
  SCX_CPU1_HS_EMT[index].ActionType = HS_EMT_ACT_NOACT
enddo

;; Restore procedure logging
%liv (log_procedure) = logging

local endmnemonic = "SCX_CPU1_HS_EMT[" & HS_MAX_MONITORED_EVENTS & "].ActionType"

;; Create the Table Load file
s create_tbl_file_from_cvt (hostCPU,apid,"Event Monitoring Table Load 3","hs_def_emt3",emtTblName,"SCX_CPU1_HS_EMT[1].AppName",endmnemonic)

write ";*********************************************************************"
write ";  End procedure SCX_CPU1_hs_emt3 "
write ";*********************************************************************"
ENDPROC
