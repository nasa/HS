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
 *   Specification for the CFS Health and Safety (HS) routines that
 *   handle command processing
 */
#ifndef HS_DISPATCH_H
#define HS_DISPATCH_H

/*************************************************************************
 * Includes
 ************************************************************************/
#include "cfe.h"

/*************************************************************************
 * Exported Functions
 ************************************************************************/

/**
 * \brief Process a command pipe message
 *
 *  \par Description
 *       Processes a single software bus command pipe message. Checks
 *       the message and command IDs and calls the appropriate routine
 *       to handle the message.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] BufPtr Pointer to Software Bus buffer
 */
void HS_AppPipe(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Verify message length
 *
 *  \par Description
 *       Checks if the actual length of a software bus message matches
 *       the expected length and sends an error event if a mismatch
 *       occurs
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] MsgPtr         Message Pointer
 *  \param[in] ExpectedLength Expected length
 *
 *  \return Boolean message length matches response
 *  \retval true  Length matches expected
 *  \retval false Length does not match expected
 *
 *  \sa #HS_CMD_LEN_ERR_EID
 */
bool HS_VerifyMsgLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);

/**
 * \brief Housekeeping dispatcher
 *
 * Verifies and processes the received command
 */
void HS_SendHkVerifyDispatch(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Noop command dispatcher
 *
 * Verifies and processes the received command
 *
 *  \sa #HS_NOOP_CC
 */
void HS_NoopVerifyDispatch(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Reset counters dispatcher
 *
 * Verifies and processes the received command
 *
 *  \sa #HS_RESET_CC
 */
void HS_ResetVerifyDispatch(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Enable application monitor dispatcher
 *
 * Verifies and processes the received command
 *
 *  \sa #HS_ENABLE_APP_MON_CC
 */
void HS_EnableAppMonVerifyDispatch(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Disable application monitor dispatcher
 *
 * Verifies and processes the received command
 *
 *  \sa #HS_DISABLE_APP_MON_CC
 */
void HS_DisableAppMonVerifyDispatch(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Enable critical events monitor dispatcher
 *
 * Verifies and processes the received command
 *
 *  \sa #HS_ENABLE_EVENT_MON_CC
 */
void HS_EnableEventMonVerifyDispatch(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Disable critical events monitor dispatcher
 *
 * Verifies and processes the received command
 *
 *  \sa #HS_DISABLE_EVENT_MON_CC
 */
void HS_DisableEventMonVerifyDispatch(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Enable aliveness indicator dispatcher
 *
 * Verifies and processes the received command
 *
 *  \sa #HS_ENABLE_ALIVENESS_CC
 */
void HS_EnableAlivenessVerifyDispatch(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Disable aliveness indicator dispatcher
 *
 * Verifies and processes the received command
 *
 *  \sa #HS_DISABLE_ALIVENESS_CC
 */
void HS_DisableAlivenessVerifyDispatch(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Enable CPU Hogging indicator dispatcher
 *
 * Verifies and processes the received command
 *
 *  \sa #HS_ENABLE_CPU_HOG_CC
 */
void HS_EnableCpuHogVerifyDispatch(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Disable CPU Hogging indicator dispatcher
 *
 * Verifies and processes the received command
 *
 *  \sa #HS_DISABLE_CPU_HOG_CC
 */
void HS_DisableCpuHogVerifyDispatch(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Reset resets performed dispatcher
 *
 * Verifies and processes the received command
 *
 *  \sa #HS_SET_MAX_RESETS_CC
 */
void HS_ResetResetsPerformedVerifyDispatch(const CFE_SB_Buffer_t *BufPtr);

/**
 * \brief Set max resets dispatcher
 *
 * Verifies and processes the received command
 *
 *  \sa #HS_RESET_RESETS_PERFORMED_CC
 */
void HS_SetMaxResetsVerifyDispatch(const CFE_SB_Buffer_t *BufPtr);

#endif
