/************************************************************************
 * NASA Docket No. GSC-18,920-1, and identified as “Core Flight
 * System (cFS) Health & Safety (HS) Application version 2.4.1”
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
 *   Health and Safety (HS) application custom component.
 *   This custom code is designed to work without modification; however the
 *   nature of specific platforms may make it desirable to modify this code
 *   to work in a more efficient way, or to provide greater functionality.
 */

/*************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include "cfe_psp.h"
#include "osapi.h"
#include "hs_app.h"
#include "hs_cmds.h"
#include "hs_msg.h"
#include "hs_utils.h"
#include "hs_custom.h"
#include "hs_custom_internal.h"
#include "hs_dispatch.h"
#include "hs_events.h"
#include "hs_monitors.h"
#include "hs_perfids.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Report Utilization Diagnostics Information                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_ReportDiagVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_ReportDiagCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_ReportDiagCmd(BufPtr);
    }
}

void HS_SetUtilParamsVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_SetUtilParamsCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_SetUtilParamsCmd(BufPtr);
    }
}

void HS_SetUtilDiagVerifyDispatch(const CFE_SB_Buffer_t *BufPtr)
{
    size_t ExpectedLength = sizeof(HS_SetUtilDiagCmd_t);

    /*
    ** Verify message packet length
    */
    if (HS_VerifyMsgLength(&BufPtr->Msg, ExpectedLength))
    {
        HS_SetUtilDiagCmd(BufPtr);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Processes any additional commands                               */
/*                                                                 */
/* NOTE: For complete prolog information, see 'hs_custom.h'        */
/*                                                                 */
/* This function MUST return CFE_SUCCESS if any command is found   */
/* and must NOT return CFE_SUCCESS if no command was found         */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 HS_CustomCommands(const CFE_SB_Buffer_t *BufPtr)
{
    int32             Status      = CFE_SUCCESS;
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&BufPtr->Msg, &CommandCode);

    switch (CommandCode)
    {
        case HS_REPORT_DIAG_CC:
            HS_ReportDiagVerifyDispatch(BufPtr);
            break;

        case HS_SET_UTIL_PARAMS_CC:
            HS_SetUtilParamsVerifyDispatch(BufPtr);
            break;

        case HS_SET_UTIL_DIAG_CC:
            HS_SetUtilDiagVerifyDispatch(BufPtr);
            break;

        default:
            Status = CFE_STATUS_BAD_COMMAND_CODE;
            break;

    } /* end CommandCode switch */

    return Status;
}
