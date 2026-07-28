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
 *   Specification for the CFS Health and Safety (HS) command and telemetry
 *   message constant definitions.
 *
 *  For HS this is only the function/command code definitions
 */
#ifndef DEFAULT_HS_MSGDEFS_H
#define DEFAULT_HS_MSGDEFS_H

#include "hs_fcncodes.h"

/**
 * Macro to set single enable bit in the standard TLM data structure
 *
 * Each entry is a single bit so it needs to be set via a bitmask
 * The left-most bit should be the first entry (index 0)
 */
#define HS_SET_TLM_ENABLE_BITMASK(arr, p)    \
    do                                       \
    {                                        \
        arr[(p) / 8] |= (0x80 >> ((p) % 8)); \
    } while (0)

#endif
