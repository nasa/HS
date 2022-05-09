/************************************************************************
 * NASA Docket No. GSC-18,920-1, and identified as “Core Flight
 * System (cFS) Health & Safety (HS) Application version 2.4.0”
 *
 * Copyright (c) 2021 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *   This file contains the function prototypes and global variables for
 *   the unit test utilities for the HS application.
 */
#ifndef HS_TEST_UTILS_H
#define HS_TEST_UTILS_H

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
    uint16 EventID;
    uint16 EventType;
    char   Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
} CFE_EVS_SendEvent_context_t;

typedef struct
{
    char Spec[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];
} CFE_ES_WriteToSysLog_context_t;

extern CFE_EVS_SendEvent_context_t    context_CFE_EVS_SendEvent[];
extern CFE_ES_WriteToSysLog_context_t context_CFE_ES_WriteToSysLog;

/* Command buffer typedef for any handler */
typedef union
{
    CFE_SB_Buffer_t       Buf;
    HS_NoArgsCmd_t        NoArgsCmd;
    HS_SetMaxResetsCmd_t  SetMaxResetsCmd;
    HS_SetUtilParamsCmd_t SetUtilParamsCmd;
    HS_SetUtilDiagCmd_t   SetUtilDiagCmd;
} UT_CmdBuf_t;

extern UT_CmdBuf_t UT_CmdBuf;

/* Unit test ids */
#define HS_UT_PIPEID_1 CFE_SB_PIPEID_C(CFE_RESOURCEID_WRAP(1))

/*
 * Function Definitions
 */

void HS_Test_Setup(void);
void HS_Test_TearDown(void);

#endif
