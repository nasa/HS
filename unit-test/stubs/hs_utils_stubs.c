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

#include "hs_utils.h"

/* UT includes */
#include "uttest.h"
#include "utstubs.h"

bool HS_VerifyMsgLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    UT_Stub_RegisterContext(UT_KEY(HS_VerifyMsgLength), MsgPtr);
    UT_Stub_RegisterContextGenericArg(UT_KEY(HS_VerifyMsgLength), ExpectedLength);
    return UT_DEFAULT_IMPL(HS_VerifyMsgLength);
}

bool HS_AMTActionIsValid(uint16 ActionType)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(HS_AMTActionIsValid), ActionType);
    return UT_DEFAULT_IMPL(HS_AMTActionIsValid);
}

bool HS_EMTActionIsValid(uint16 ActionType)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(HS_EMTActionIsValid), ActionType);
    return UT_DEFAULT_IMPL(HS_EMTActionIsValid);
}
