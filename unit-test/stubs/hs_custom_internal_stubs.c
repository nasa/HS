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
 *
 * Auto-Generated stub implementations for functions defined in hs_custom_internal header
 */

#include "hs_custom_internal.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for HS_CustomCleanup()
 * ----------------------------------------------------
 */
void HS_CustomCleanup(void)
{

    UT_GenStub_Execute(HS_CustomCleanup, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_CustomCommands()
 * ----------------------------------------------------
 */
int32 HS_CustomCommands(const CFE_SB_Buffer_t *BufPtr)
{
    UT_GenStub_SetupReturnBuffer(HS_CustomCommands, int32);

    UT_GenStub_AddParam(HS_CustomCommands, const CFE_SB_Buffer_t *, BufPtr);

    UT_GenStub_Execute(HS_CustomCommands, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_CustomCommands, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_CustomGetUtil()
 * ----------------------------------------------------
 */
int32 HS_CustomGetUtil(void)
{
    UT_GenStub_SetupReturnBuffer(HS_CustomGetUtil, int32);

    UT_GenStub_Execute(HS_CustomGetUtil, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_CustomGetUtil, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_CustomInit()
 * ----------------------------------------------------
 */
int32 HS_CustomInit(void)
{
    UT_GenStub_SetupReturnBuffer(HS_CustomInit, int32);

    UT_GenStub_Execute(HS_CustomInit, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_CustomInit, int32);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_CustomMonitorUtilization()
 * ----------------------------------------------------
 */
void HS_CustomMonitorUtilization(void)
{

    UT_GenStub_Execute(HS_CustomMonitorUtilization, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_IdleTask()
 * ----------------------------------------------------
 */
void HS_IdleTask(void)
{

    UT_GenStub_Execute(HS_IdleTask, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_MarkIdleCallback()
 * ----------------------------------------------------
 */
void HS_MarkIdleCallback(void)
{

    UT_GenStub_Execute(HS_MarkIdleCallback, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_SetUtilDiagCmd()
 * ----------------------------------------------------
 */
void HS_SetUtilDiagCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_GenStub_AddParam(HS_SetUtilDiagCmd, const CFE_SB_Buffer_t *, BufPtr);

    UT_GenStub_Execute(HS_SetUtilDiagCmd, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_SetUtilParamsCmd()
 * ----------------------------------------------------
 */
void HS_SetUtilParamsCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_GenStub_AddParam(HS_SetUtilParamsCmd, const CFE_SB_Buffer_t *, BufPtr);

    UT_GenStub_Execute(HS_SetUtilParamsCmd, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_UtilDiagReport()
 * ----------------------------------------------------
 */
void HS_UtilDiagReport(void)
{

    UT_GenStub_Execute(HS_UtilDiagReport, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_UtilizationIncrement()
 * ----------------------------------------------------
 */
void HS_UtilizationIncrement(void)
{

    UT_GenStub_Execute(HS_UtilizationIncrement, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_UtilizationMark()
 * ----------------------------------------------------
 */
void HS_UtilizationMark(void)
{

    UT_GenStub_Execute(HS_UtilizationMark, Basic, NULL);
}
