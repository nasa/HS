/*************************************************************************
** File: hs_test_utils.h"
**
** NASA Docket No. GSC-18,476-1, and identified as "Core Flight System
** (cFS) Health and Safety (HS) Application version 2.3.2”
**
** Copyright © 2020 United States Government as represented by the
** Administrator of the National Aeronautics and Space Administration.
** All Rights Reserved.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
** http://www.apache.org/licenses/LICENSE-2.0
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
** Purpose:
**   This file contains the function prototypes and global variables for
**   the unit test utilities for the HS application.
**
*************************************************************************/

/*
 * Includes
 */

#include "hs_tbldefs.h"
#include "hs_app.h"
#include "hs_verify.h"
#include "hs_tbl.h"
#include "hs_msgdefs.h"
#include "hs_version.h"
#include "hs_events.h"
#include "hs_utils.h"
#include "hs_cmds.h"
#include "hs_custom.h"
#include "hs_monitors.h"
#include "hs_msg.h"
#include "utstubs.h"
#include <time.h>

extern HS_AppData_t HS_AppData;

/*
 * Global context structures
 */
typedef struct
{
    uint16      EventID;
    uint16      EventType;
    const char *Spec;
} __attribute__((packed)) CFE_EVS_SendEvent_context_t;

typedef struct
{
    const char *Spec;
} __attribute__((packed)) CFE_ES_WriteToSysLog_context_t;

int32 UT_Utils_stub_reporter_hook(void *UserObj, int32 StubRetcode, uint32 CallCount, const UT_StubContext_t *Context);

/*
 * Function Definitions
 */

void HS_Test_InitMsg(CFE_SB_MsgId_t TestMsgId, CFE_MSG_FcnCode_t FcnCode, size_t MsgSize);
void HS_Test_Setup(void);
void HS_Test_TearDown(void);

/************************/
/*  End of File Comment */
/************************/
