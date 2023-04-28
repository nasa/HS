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
 *   Utility functions for the CFS Health and Safety (HS) application.
 */

/************************************************************************
** Includes
*************************************************************************/
#include "hs_app.h"
#include "hs_utils.h"
#include "hs_monitors.h"
#include "hs_msgids.h"
#include "hs_eventids.h"
#include "hs_version.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify AMT Action Type                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool HS_AMTActionIsValid(uint16 ActionType)
{
    bool IsValid = true;

    if (ActionType < HS_AMT_ACT_NOACT)
    {
        IsValid = false;
    }
    else if (ActionType > (HS_AMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES))
    {
        /* HS allows for HS_AMT_ACT_LAST_NONMSG actions by default and
           HS_MAX_MSG_ACT_TYPES message actions defined in the Message
           Action Table. */
        IsValid = false;
    }

    return IsValid;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify EMT Action Type                                          */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool HS_EMTActionIsValid(uint16 ActionType)
{
    bool IsValid = true;

    if (ActionType < HS_EMT_ACT_NOACT)
    {
        IsValid = false;
    }
    else if (ActionType > (HS_EMT_ACT_LAST_NONMSG + HS_MAX_MSG_ACT_TYPES))
    {
        /* HS allows for HS_EMT_ACT_LAST_NONMSG actions by default and
           HS_MAX_MSG_ACT_TYPES message actions defined in the Message
           Action Table. */
        IsValid = false;
    }

    return IsValid;
}
