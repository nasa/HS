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
 *   Utility functions for the cFS Health and Safety (HS) application.
 */
#ifndef HS_UTILS_H
#define HS_UTILS_H

/*************************************************************************
 * Includes
 *************************************************************************/
#include "cfe.h"

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
 *  \sa #HS_LEN_ERR_EID
 */
bool HS_VerifyMsgLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);

/**
 * \brief Verify AMT Action Type
 *
 *  \par Description
 *       Checks if the specified value is a valid AMT Action Type.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] ActionType Action type to validate
 *
 *  \return Boolean action valid response
 *  \retval true  Action type valid
 *  \retval false Action type not valid
 */
bool HS_AMTActionIsValid(uint16 ActionType);

/**
 * \brief Verify EMT Action Type
 *
 *  \par Description
 *       Checks if the specified value is a valid EMT Action Type.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param[in] ActionType Action type to validate
 *
 *  \return Boolean action valid response
 *  \retval true  Action type valid
 *  \retval false Action type not valid
 */
bool HS_EMTActionIsValid(uint16 ActionType);

#endif
