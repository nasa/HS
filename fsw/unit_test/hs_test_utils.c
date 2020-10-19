/*************************************************************************
** File: hs_test_utils.c 
**
** NASA Docket No. GSC-18,476-1, and identified as "Core Flight System 
** (cFS) Health and Safety (HS) Application version 2.3.2” 
**
** Copyright © 2020 United States Government as represented by the 
** Administrator of the National Aeronautics and Space Administration.  
** All Rights Reserved. 
** 
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
** http://www.apache.org/licenses/LICENSE-2.0 
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License. 
**
** Purpose: 
**   This file contains unit test utilities for the HS application.
**
*************************************************************************/

/*
 * Includes
 */

#include "hs_test_utils.h"
#include "hs_app.h"

extern HS_AppData_t     HS_AppData;

/*
 * Function Definitions
 */

void HS_Test_Setup(void)
{
    /* initialize test environment to default state for every test */

    CFE_PSP_MemSet(&HS_AppData, 0, sizeof(HS_AppData_t));
    
    Ut_CFE_EVS_Reset();
    Ut_CFE_FS_Reset();
    Ut_CFE_TIME_Reset();
    Ut_CFE_TBL_Reset();
    Ut_CFE_SB_Reset();
    Ut_CFE_ES_Reset();
    Ut_OSAPI_Reset();
    Ut_OSFILEAPI_Reset();
} /* end HS_Test_Setup */

void HS_Test_TearDown(void)
{
    /* cleanup test environment */
} /* end HS_Test_TearDown */


/************************/
/*  End of File Comment */
/************************/
