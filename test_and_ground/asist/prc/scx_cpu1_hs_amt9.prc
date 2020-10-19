PROC scx_cpu1_hs_amt9
;*******************************************************************************
;  Test Name:  hs_amt9
;  Test Level: Build Verification
;  Test Type:  Functional
;
;  Test Description
;	The purpose of this procedure is to generate an Application Monitoring
;	Table that contains the maximum number of entries.
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
;       06/23/09	W. Moleski	Initial release
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
local apid = 0xf72
local HSAppName = "HS"                  
local amtTblName = HSAppName & "." & HS_AMT_TABLENAME
local hostCPU = "CPU3"

write ";*********************************************************************"
write ";  Define the Application Monitoring Table "
write ";*********************************************************************"
; Set each entry's Application Name
SCX_CPU1_HS_AMT[1].AppName = "CFE_ES"
SCX_CPU1_HS_AMT[2].AppName = "CFE_EVS"
SCX_CPU1_HS_AMT[3].AppName = "CFE_TIME"
SCX_CPU1_HS_AMT[4].AppName = "CFE_TBL"
SCX_CPU1_HS_AMT[5].AppName = "CFE_SB"
SCX_CPU1_HS_AMT[6].AppName = "HS"
SCX_CPU1_HS_AMT[7].AppName = "TST_HS"
SCX_CPU1_HS_AMT[8].AppName = "SC"
SCX_CPU1_HS_AMT[9].AppName = "TST_SC"
SCX_CPU1_HS_AMT[10].AppName = "MM"
SCX_CPU1_HS_AMT[11].AppName = "TST_MM"
SCX_CPU1_HS_AMT[12].AppName = "MD"
SCX_CPU1_HS_AMT[13].AppName = "TST_MD"
SCX_CPU1_HS_AMT[14].AppName = "CS"
SCX_CPU1_HS_AMT[15].AppName = "TST_CS"
SCX_CPU1_HS_AMT[16].AppName = "HK"
SCX_CPU1_HS_AMT[17].AppName = "TST_HK"
SCX_CPU1_HS_AMT[18].AppName = "FM"
SCX_CPU1_HS_AMT[19].AppName = "TST_FM"
SCX_CPU1_HS_AMT[20].AppName = "LC"
SCX_CPU1_HS_AMT[21].AppName = "TST_LC"
SCX_CPU1_HS_AMT[22].AppName = "SCH"
SCX_CPU1_HS_AMT[23].AppName = "TST_SCH"
SCX_CPU1_HS_AMT[24].AppName = "TST_ES"
SCX_CPU1_HS_AMT[25].AppName = "TST_EVS"
SCX_CPU1_HS_AMT[26].AppName = "TST_TIME"
SCX_CPU1_HS_AMT[27].AppName = "TST_TBL"
SCX_CPU1_HS_AMT[28].AppName = "TST_TBL2"
SCX_CPU1_HS_AMT[29].AppName = "TST_SB"
SCX_CPU1_HS_AMT[30].AppName = "TST_ES2"
SCX_CPU1_HS_AMT[31].AppName = "TST_ES3"
SCX_CPU1_HS_AMT[32].AppName = "TST_ES4"

;; Set the common fields in each entry
for index = 1 to HS_MAX_MONITORED_APPS do
  SCX_CPU1_HS_AMT[index].NullTerm = 0
  SCX_CPU1_HS_AMT[index].CycleCnt = 10
  SCX_CPU1_HS_AMT[index].ActionType = HS_AMT_ACT_EVENT
enddo

;; Restore procedure logging
%liv (log_procedure) = logging

local endmnemonic = "SCX_CPU1_HS_AMT[" & HS_MAX_MONITORED_APPS & "].ActionType"

;; Create the Table Load file
s create_tbl_file_from_cvt (hostCPU,apid,"App Monitoring Table Load 9","hs_def_amt9",amtTblName,"SCX_CPU1_HS_AMT[1].AppName",endmnemonic)

write ";*********************************************************************"
write ";  End procedure SCX_CPU1_hs_amt9 "
write ";*********************************************************************"
ENDPROC
