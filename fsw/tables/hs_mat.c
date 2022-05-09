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
 *  The CFS Health and Safety (HS) Message Actions Table Definition
 */

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include "hs_tbl.h"
#include "hs_tbldefs.h"
#include "cfe_tbl_filedef.h"

static CFE_TBL_FileDef_t CFE_TBL_FileDef
    __attribute__((__used__)) = {"HS_Default_MsgActs_Tbl", HS_APP_NAME ".MsgActs_Tbl", "HS MsgActs Table", "hs_mat.tbl",
                                 (sizeof(HS_MATEntry_t) * HS_MAX_MSG_ACT_TYPES)};

HS_MATEntry_t HS_Default_MsgActs_Tbl[HS_MAX_MSG_ACT_TYPES] = {
    /*          EnableState               Cooldown   Message */

    /*   0 */ {HS_MAT_STATE_DISABLED,
               10,
               {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}},
    /*   1 */
    {HS_MAT_STATE_DISABLED,
     10,
     {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}},
    /*   2 */
    {HS_MAT_STATE_DISABLED,
     10,
     {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}},
    /*   3 */
    {HS_MAT_STATE_DISABLED,
     10,
     {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}},
    /*   4 */
    {HS_MAT_STATE_DISABLED,
     10,
     {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}},
    /*   5 */
    {HS_MAT_STATE_DISABLED,
     10,
     {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}},
    /*   6 */
    {HS_MAT_STATE_DISABLED,
     10,
     {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}},
    /*   7 */
    {HS_MAT_STATE_DISABLED,
     10,
     {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}},

};

/************************/
/*  End of File Comment */
/************************/
