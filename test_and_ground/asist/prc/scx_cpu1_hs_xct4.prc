PROC scx_cpu1_hs_xct4
;*******************************************************************************
;  Test Name:  hs_xct4
;  Test Level: Build Verification
;  Test Type:  Functional
;
;  Test Description
;	The purpose of this procedure is to generate an Execution Counter Table
;	that contains invalid data.
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
local apid = 0xf75
local HSAppName = "HS"                  
local xctTblName = HSAppName & "." & HS_XCT_TABLENAME
local hostCPU = "CPU3"

write ";*********************************************************************"
write ";  Define the Application Monitoring Table "
write ";*********************************************************************"
; Entry 1
SCX_CPU1_HS_XCT[1].ResourceName = "CFE_ES"
SCX_CPU1_HS_XCT[1].NullTerm = 0
SCX_CPU1_HS_XCT[1].ResourceType = HS_XCT_TYPE_APP_MAIN

;; Entry 2
SCX_CPU1_HS_XCT[2].ResourceName = "CFE_EVS"
SCX_CPU1_HS_XCT[2].NullTerm = 0
SCX_CPU1_HS_XCT[2].ResourceType = HS_XCT_TYPE_APP_MAIN

;; Entry 3
SCX_CPU1_HS_XCT[3].ResourceName = "CFE_TIME"
SCX_CPU1_HS_XCT[3].NullTerm = 5
SCX_CPU1_HS_XCT[3].ResourceType = HS_XCT_TYPE_APP_MAIN

;; Entry 4
SCX_CPU1_HS_XCT[4].ResourceName = "CFE_TBL"
SCX_CPU1_HS_XCT[4].NullTerm = 0
SCX_CPU1_HS_XCT[4].ResourceType = HS_XCT_TYPE_APP_MAIN

;; Entry 5
SCX_CPU1_HS_XCT[5].ResourceName = "CFE_SB"
SCX_CPU1_HS_XCT[5].NullTerm = 0
SCX_CPU1_HS_XCT[5].ResourceType = HS_XCT_TYPE_ISR + 1

for index = 6 to HS_MAX_EXEC_CNT_SLOTS do
  SCX_CPU1_HS_XCT[index].ResourceName = ""
  SCX_CPU1_HS_XCT[index].NullTerm = 0
  SCX_CPU1_HS_XCT[index].ResourceType = HS_XCT_TYPE_NOTYPE
enddo

;; Restore procedure logging
%liv (log_procedure) = logging

local endmnemonic = "SCX_CPU1_HS_XCT[" & HS_MAX_EXEC_CNT_SLOTS & "].ResourceType"

;; Create the Table Load file
s create_tbl_file_from_cvt (hostCPU,apid,"Execution Counter Table Invalid Load","hs_def_xct4",xctTblName,"SCX_CPU1_HS_XCT[1].ResourceName",endmnemonic)

write ";*********************************************************************"
write ";  End procedure SCX_CPU1_hs_xct4 "
write ";*********************************************************************"
ENDPROC
