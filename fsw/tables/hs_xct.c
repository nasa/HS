/************************************************************************
 * NASA Docket No. GSC-19,200-1, and identified as "cFS Draco"
 *
 * Copyright (c) 2023 United States Government as represented by the
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
 *  The CFS Health and Safety (HS) Execution Counters Table Definition
 */

/************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"
#include "hs_mission_cfg.h"
#include "hs_tbl.h"
#include "hs_tbldefs.h"
#include "cfe_tbl_filedef.h"

HS_XCTEntry_t HS_ExeCount_Tbl[HS_MAX_EXEC_CNT_SLOTS] = {
    /*          ResourceName               NullTerm ResourceType              */

    /*   0 */ { "CFE_ES",   0, HS_XCTResType_APP_MAIN },
    /*   1 */
    { "CFE_EVS",  0, HS_XCTResType_APP_MAIN },
    /*   2 */
    { "CFE_TIME", 0, HS_XCTResType_APP_MAIN },
    /*   3 */
    { "CFE_TBL",  0, HS_XCTResType_APP_MAIN },
    /*   4 */
    { "CFE_SB",   0, HS_XCTResType_APP_MAIN },
    /*   5 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*   6 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*   7 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*   8 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*   9 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  10 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  11 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  12 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  13 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  14 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  15 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  16 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  17 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  18 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  19 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  20 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  21 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  22 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  23 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  24 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  25 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  26 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  27 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  28 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  29 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  30 */
    { "",         0, HS_XCTResType_NOTYPE   },
    /*  31 */
    { "",         0, HS_XCTResType_NOTYPE   },
};

CFE_TBL_FILEDEF(HS_ExeCount_Tbl, HS.ExeCount_Tbl, HS ExeCount Table, hs_xct.tbl)
