PROC scx_cpu1_hs_amt13
;*******************************************************************************
;  Test Name:  hs_amt13
;  Test Level: Build Verification
;  Test Type:  Functional
;
;  Test Description
;	This procedure generates a Critical Application Table image that
;	contains multiple entries for the HS application. Each entry specifies a
;	different action.
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
;       01/14/11	W. Moleski	Initial release
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
local apid = 0xf72
local HSAppName = "HS"
local amtTblName = HSAppName & "." & HS_AMT_TABLENAME
local hostCPU = "CPU3"

write ";*********************************************************************"
write ";  Define the Application Monitoring Table "
write ";*********************************************************************"
; Entry 1
SCX_CPU1_HS_AMT[1].AppName = "CFE_ES"
SCX_CPU1_HS_AMT[1].NullTerm = 0
SCX_CPU1_HS_AMT[1].CycleCnt = 10
SCX_CPU1_HS_AMT[1].ActionType = HS_AMT_ACT_EVENT

;; Entry 2
SCX_CPU1_HS_AMT[2].AppName = "CFE_EVS"
SCX_CPU1_HS_AMT[2].NullTerm = 0
SCX_CPU1_HS_AMT[2].CycleCnt = 10
SCX_CPU1_HS_AMT[2].ActionType = HS_AMT_ACT_EVENT

;; Entry 3
SCX_CPU1_HS_AMT[3].AppName = "CFE_TIME"
SCX_CPU1_HS_AMT[3].NullTerm = 0
SCX_CPU1_HS_AMT[3].CycleCnt = 10
SCX_CPU1_HS_AMT[3].ActionType = HS_AMT_ACT_EVENT

;; Entry 4
SCX_CPU1_HS_AMT[4].AppName = "CFE_TBL"
SCX_CPU1_HS_AMT[4].NullTerm = 0
SCX_CPU1_HS_AMT[4].CycleCnt = 2
SCX_CPU1_HS_AMT[4].ActionType = HS_AMT_ACT_EVENT

;; Entry 5
SCX_CPU1_HS_AMT[5].AppName = "CFE_SB"
SCX_CPU1_HS_AMT[5].NullTerm = 0
SCX_CPU1_HS_AMT[5].CycleCnt = 10
SCX_CPU1_HS_AMT[5].ActionType = HS_AMT_ACT_EVENT

;; Entry 6
SCX_CPU1_HS_AMT[6].AppName = HSAppName
SCX_CPU1_HS_AMT[6].NullTerm = 0
SCX_CPU1_HS_AMT[6].CycleCnt = 10
SCX_CPU1_HS_AMT[6].ActionType = HS_AMT_ACT_PROC_RESET

;; Entry 7
SCX_CPU1_HS_AMT[7].AppName = HSAppName
SCX_CPU1_HS_AMT[7].NullTerm = 0
SCX_CPU1_HS_AMT[7].CycleCnt = 10
SCX_CPU1_HS_AMT[7].ActionType = HS_AMT_ACT_APP_RESTART

;; Entry 8
SCX_CPU1_HS_AMT[8].AppName = HSAppName
SCX_CPU1_HS_AMT[8].NullTerm = 0
SCX_CPU1_HS_AMT[8].CycleCnt = 10
SCX_CPU1_HS_AMT[8].ActionType = HS_AMT_ACT_EVENT

;; Entry 9
SCX_CPU1_HS_AMT[9].AppName = HSAppName
SCX_CPU1_HS_AMT[9].NullTerm = 0
SCX_CPU1_HS_AMT[9].CycleCnt = 10
SCX_CPU1_HS_AMT[9].ActionType = HS_AMT_ACT_LAST_NONMSG+1

;; Make the rest of the entries empty
for index = 10 to HS_MAX_MONITORED_APPS do
  SCX_CPU1_HS_AMT[index].AppName = ""
  SCX_CPU1_HS_AMT[index].NullTerm = 0
  SCX_CPU1_HS_AMT[index].CycleCnt = 0
  SCX_CPU1_HS_AMT[index].ActionType = HS_AMT_ACT_NOACT
enddo

;; Restore procedure logging
%liv (log_procedure) = logging

local endmnemonic = "SCX_CPU1_HS_AMT[" & HS_MAX_MONITORED_APPS & "].ActionType"

;; Create the Table Load file
s create_tbl_file_from_cvt (hostCPU,apid,"App Monitoring Table Load 13","hs_def_amt13",amtTblName,"SCX_CPU1_HS_AMT[1].AppName",endmnemonic)

write ";*********************************************************************"
write ";  End procedure SCX_CPU1_hs_amt13 "
write ";*********************************************************************"
ENDPROC
