/************************************************************************
** File: hs_emt.c 
**
** NASA Docket No. GSC-16,151-1, and identified as "Core Flight Software System (CFS)
** Health and Safety Application Version 2"
** 
** Copyright © 2007-2014 United States Government as represented by the
** Administrator of the National Aeronautics and Space Administration. All Rights
** Reserved. 
** 
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
** http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License. 
**
** Purpose:
**  The CFS Health and Safety (HS) Event Monitor Table Definition
**
** Notes:
**
**
*************************************************************************/


/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include "hs_tbl.h"
#include "hs_tbldefs.h"
#include "cfe_tbl_filedef.h"


static CFE_TBL_FileDef_t CFE_TBL_FileDef __attribute__((__used__)) =
{
    "HS_Default_EventMon_Tbl", HS_APP_NAME ".EventMon_Tbl", "HS EventMon Table",
    "hs_emt.tbl", (sizeof(HS_EMTEntry_t) * HS_MAX_MONITORED_EVENTS)
};



HS_EMTEntry_t      HS_Default_EventMon_Tbl[HS_MAX_MONITORED_EVENTS] =
{
/*          AppName                    NullTerm EventID        ActionType */
                                                
/*   0 */ { "CFE_ES",                  0,       10,            HS_EMT_ACT_NOACT },
/*   1 */ { "CFE_EVS",                 0,       10,            HS_EMT_ACT_NOACT },
/*   2 */ { "CFE_TIME",                0,       10,            HS_EMT_ACT_NOACT },
/*   3 */ { "CFE_TBL",                 0,       10,            HS_EMT_ACT_NOACT },
/*   4 */ { "CFE_SB",                  0,       10,            HS_EMT_ACT_NOACT },
/*   5 */ { "",                        0,       10,            HS_EMT_ACT_NOACT },
/*   6 */ { "",                        0,       10,            HS_EMT_ACT_NOACT },
/*   7 */ { "",                        0,       10,            HS_EMT_ACT_NOACT },
/*   8 */ { "",                        0,       10,            HS_EMT_ACT_NOACT },
/*   9 */ { "",                        0,       10,            HS_EMT_ACT_NOACT },
/*  10 */ { "",                        0,       10,            HS_EMT_ACT_NOACT },
/*  11 */ { "",                        0,       10,            HS_EMT_ACT_NOACT },
/*  12 */ { "",                        0,       10,            HS_EMT_ACT_NOACT },
/*  13 */ { "",                        0,       10,            HS_EMT_ACT_NOACT },
/*  14 */ { "",                        0,       10,            HS_EMT_ACT_NOACT },
/*  15 */ { "",                        0,       10,            HS_EMT_ACT_NOACT },
                                 
};                               
                                 
/************************/       
/*  End of File Comment */       
/************************/       
