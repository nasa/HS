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
 * Auto-Generated stub implementations for functions defined in hs_cmds header
 */

#include "hs_cmds.h"
#include "utgenstub.h"

/*
 * ----------------------------------------------------
 * Generated stub function for HS_AcquirePointers()
 * ----------------------------------------------------
 */
void HS_AcquirePointers(void)
{

    UT_GenStub_Execute(HS_AcquirePointers, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_AppMonStatusRefresh()
 * ----------------------------------------------------
 */
void HS_AppMonStatusRefresh(void)
{

    UT_GenStub_Execute(HS_AppMonStatusRefresh, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_DisableAlivenessCmd()
 * ----------------------------------------------------
 */
CFE_Status_t HS_DisableAlivenessCmd(const HS_DisableAlivenessCmd_t *BufPtr)
{
    UT_GenStub_SetupReturnBuffer(HS_DisableAlivenessCmd, CFE_Status_t);

    UT_GenStub_AddParam(HS_DisableAlivenessCmd, const HS_DisableAlivenessCmd_t *, BufPtr);

    UT_GenStub_Execute(HS_DisableAlivenessCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_DisableAlivenessCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_DisableAppMonCmd()
 * ----------------------------------------------------
 */
CFE_Status_t HS_DisableAppMonCmd(const HS_DisableAppMonCmd_t *BufPtr)
{
    UT_GenStub_SetupReturnBuffer(HS_DisableAppMonCmd, CFE_Status_t);

    UT_GenStub_AddParam(HS_DisableAppMonCmd, const HS_DisableAppMonCmd_t *, BufPtr);

    UT_GenStub_Execute(HS_DisableAppMonCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_DisableAppMonCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_DisableCpuHogCmd()
 * ----------------------------------------------------
 */
CFE_Status_t HS_DisableCpuHogCmd(const HS_DisableCpuHogCmd_t *BufPtr)
{
    UT_GenStub_SetupReturnBuffer(HS_DisableCpuHogCmd, CFE_Status_t);

    UT_GenStub_AddParam(HS_DisableCpuHogCmd, const HS_DisableCpuHogCmd_t *, BufPtr);

    UT_GenStub_Execute(HS_DisableCpuHogCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_DisableCpuHogCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_DisableEventMonCmd()
 * ----------------------------------------------------
 */
CFE_Status_t HS_DisableEventMonCmd(const HS_DisableEventMonCmd_t *BufPtr)
{
    UT_GenStub_SetupReturnBuffer(HS_DisableEventMonCmd, CFE_Status_t);

    UT_GenStub_AddParam(HS_DisableEventMonCmd, const HS_DisableEventMonCmd_t *, BufPtr);

    UT_GenStub_Execute(HS_DisableEventMonCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_DisableEventMonCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_EnableAlivenessCmd()
 * ----------------------------------------------------
 */
CFE_Status_t HS_EnableAlivenessCmd(const HS_EnableAlivenessCmd_t *BufPtr)
{
    UT_GenStub_SetupReturnBuffer(HS_EnableAlivenessCmd, CFE_Status_t);

    UT_GenStub_AddParam(HS_EnableAlivenessCmd, const HS_EnableAlivenessCmd_t *, BufPtr);

    UT_GenStub_Execute(HS_EnableAlivenessCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_EnableAlivenessCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_EnableAppMonCmd()
 * ----------------------------------------------------
 */
CFE_Status_t HS_EnableAppMonCmd(const HS_EnableAppMonCmd_t *BufPtr)
{
    UT_GenStub_SetupReturnBuffer(HS_EnableAppMonCmd, CFE_Status_t);

    UT_GenStub_AddParam(HS_EnableAppMonCmd, const HS_EnableAppMonCmd_t *, BufPtr);

    UT_GenStub_Execute(HS_EnableAppMonCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_EnableAppMonCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_EnableCpuHogCmd()
 * ----------------------------------------------------
 */
CFE_Status_t HS_EnableCpuHogCmd(const HS_EnableCpuHogCmd_t *BufPtr)
{
    UT_GenStub_SetupReturnBuffer(HS_EnableCpuHogCmd, CFE_Status_t);

    UT_GenStub_AddParam(HS_EnableCpuHogCmd, const HS_EnableCpuHogCmd_t *, BufPtr);

    UT_GenStub_Execute(HS_EnableCpuHogCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_EnableCpuHogCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_EnableEventMonCmd()
 * ----------------------------------------------------
 */
CFE_Status_t HS_EnableEventMonCmd(const HS_EnableEventMonCmd_t *BufPtr)
{
    UT_GenStub_SetupReturnBuffer(HS_EnableEventMonCmd, CFE_Status_t);

    UT_GenStub_AddParam(HS_EnableEventMonCmd, const HS_EnableEventMonCmd_t *, BufPtr);

    UT_GenStub_Execute(HS_EnableEventMonCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_EnableEventMonCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_MsgActsStatusRefresh()
 * ----------------------------------------------------
 */
void HS_MsgActsStatusRefresh(void)
{

    UT_GenStub_Execute(HS_MsgActsStatusRefresh, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_NoopCmd()
 * ----------------------------------------------------
 */
CFE_Status_t HS_NoopCmd(const HS_NoopCmd_t *BufPtr)
{
    UT_GenStub_SetupReturnBuffer(HS_NoopCmd, CFE_Status_t);

    UT_GenStub_AddParam(HS_NoopCmd, const HS_NoopCmd_t *, BufPtr);

    UT_GenStub_Execute(HS_NoopCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_NoopCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_ResetCmd()
 * ----------------------------------------------------
 */
CFE_Status_t HS_ResetCmd(const HS_ResetCmd_t *BufPtr)
{
    UT_GenStub_SetupReturnBuffer(HS_ResetCmd, CFE_Status_t);

    UT_GenStub_AddParam(HS_ResetCmd, const HS_ResetCmd_t *, BufPtr);

    UT_GenStub_Execute(HS_ResetCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_ResetCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_ResetCounters()
 * ----------------------------------------------------
 */
void HS_ResetCounters(void)
{

    UT_GenStub_Execute(HS_ResetCounters, Basic, NULL);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_ResetResetsPerformedCmd()
 * ----------------------------------------------------
 */
CFE_Status_t HS_ResetResetsPerformedCmd(const HS_ResetResetsPerformedCmd_t *BufPtr)
{
    UT_GenStub_SetupReturnBuffer(HS_ResetResetsPerformedCmd, CFE_Status_t);

    UT_GenStub_AddParam(HS_ResetResetsPerformedCmd, const HS_ResetResetsPerformedCmd_t *, BufPtr);

    UT_GenStub_Execute(HS_ResetResetsPerformedCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_ResetResetsPerformedCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_SendHkCmd()
 * ----------------------------------------------------
 */
CFE_Status_t HS_SendHkCmd(const HS_SendHkCmd_t *BufPtr)
{
    UT_GenStub_SetupReturnBuffer(HS_SendHkCmd, CFE_Status_t);

    UT_GenStub_AddParam(HS_SendHkCmd, const HS_SendHkCmd_t *, BufPtr);

    UT_GenStub_Execute(HS_SendHkCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_SendHkCmd, CFE_Status_t);
}

/*
 * ----------------------------------------------------
 * Generated stub function for HS_SetMaxResetsCmd()
 * ----------------------------------------------------
 */
CFE_Status_t HS_SetMaxResetsCmd(const HS_SetMaxResetsCmd_t *BufPtr)
{
    UT_GenStub_SetupReturnBuffer(HS_SetMaxResetsCmd, CFE_Status_t);

    UT_GenStub_AddParam(HS_SetMaxResetsCmd, const HS_SetMaxResetsCmd_t *, BufPtr);

    UT_GenStub_Execute(HS_SetMaxResetsCmd, Basic, NULL);

    return UT_GenStub_GetReturnValue(HS_SetMaxResetsCmd, CFE_Status_t);
}
