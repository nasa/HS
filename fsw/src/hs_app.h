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
 *   Unit specification for the Core Flight System (CFS)
 *   Health and Safety (HS) Application.
 */
#ifndef HS_APP_H
#define HS_APP_H

/************************************************************************
 * Includes
 ************************************************************************/
#include "hs_msg.h"
#include "hs_tbl.h"
#include "hs_platform_cfg.h"
#include "hs_app_monitor.h"
#include "hs_event_monitor.h"
#include "hs_exec_monitor.h"
#include "hs_msg_action.h"
#include "cfe.h"
#include "cfe_msgids.h"

/************************************************************************
 * Macro Definitions
 ************************************************************************/

/**
 * \name HS Pipe Parameters
 * \{
 */
#define HS_CMD_PIPE_NAME    "HS_CMD_PIPE"
#define HS_EVENT_PIPE_NAME  "HS_EVENT_PIPE"
#define HS_WAKEUP_PIPE_NAME "HS_WAKEUP_PIPE"
/**\}*/

/**
 * \name HS CDS Buffer String
 * \{
 */
#define HS_CDSNAME "HS_CDS"
/**\}*/

#define HS_TBL_VAL_ERR (-1)

/**
 * \name HS Invalid Execution Counter
 * \{
 */
#define HS_INVALID_EXECOUNT 0xFFFFFFFF
/**\}*/

/**
 * \name HS Table Name Strings
 * \{
 */
#define HS_AMT_TABLENAME "AppMon_Tbl"   /**< \brief AppMon Table Name */
#define HS_EMT_TABLENAME "EventMon_Tbl" /**< \brief EventMon Table Name */
#define HS_XCT_TABLENAME "ExeCount_Tbl" /**< \brief ExeCount Table Name */
#define HS_MAT_TABLENAME "MsgActs_Tbl"  /**< \brief MsgAction Table Name */
/**\}*/

/************************************************************************
 * Type Definitions
 ************************************************************************/

/**
 *  \brief HS CDS Data Structure
 */
typedef struct
{
    uint16 ResetsPerformed;    /**< \brief Number of Resets Performed */
    uint16 ResetsPerformedNot; /**< \brief Inverted Resets Performed for validation */
    uint16 MaxResets;          /**< \brief Max Number of Resets Allowed */
    uint16 MaxResetsNot;       /**< \brief Inverted Max Number of Resets Allowed for validation */
} HS_CDSData_t;

/**
 *  \brief HS Global Data Structure
 */
typedef struct
{
    CFE_SB_PipeId_t CmdPipe;    /**< \brief Pipe Id for HS command pipe */
    CFE_SB_PipeId_t WakeupPipe; /**< \brief Pipe Id for HS wakeup pipe */
    CFE_SB_PipeId_t EventPipe;  /**< \brief Pipe Id for HK event pipe */

    HS_State_Enum_t ServiceWatchdogFlag;   /**< \brief Flag of current watchdog servicing state */
    HS_State_Enum_t CurrentAppMonState;    /**< \brief Status of HS Application Monitor */
    HS_State_Enum_t CurrentEventMonState;  /**< \brief Status of HS Events Monitor */
    HS_State_Enum_t CurrentAlivenessState; /**< \brief Status of HS Aliveness Indicator */

    HS_State_Enum_t ExecMonLoaded;      /**< \brief Status of Execution Counter Table */
    HS_State_Enum_t MsgActsLoaded;      /**< \brief Status of Message Actions Table */
    HS_State_Enum_t CDSState;           /**< \brief Status of Critical Data Storing */
    HS_State_Enum_t AppMonLoaded;       /**< \brief If AppMon Table is loaded */
    HS_State_Enum_t EventMonLoaded;     /**< \brief If EventMon Table is loaded */
    HS_State_Enum_t CurrentCPUHogState; /**< \brief Status of HS CPU Hogging Indicator */

    uint8 SpareBytes[3]; /**< \brief Spare bytes for 32 bit alignment padding */

    uint8 CmdCount;    /**< \brief Number of valid commands received */
    uint8 CmdErrCount; /**< \brief Number of invalid commands received */

    uint32 EventsMonitoredCount;     /**< \brief Total count of event messages monitored */
    uint32 InactiveEventMonCount;    /**< \brief Count of event monitors for which the target is not active */
    uint32 InactiveExecMonitorCount; /**< \brief Count of execution monitors for which the target is not active */

    HS_MsgActState_t    MsgActState[HS_MAX_MSG_ACT_TYPES]; /**< \brief Counts until Message Actions is available */
    HS_AppMon_State_t   AppMonState[HS_MAX_MONITORED_APPS];
    HS_EventMon_State_t EventMonState[HS_MAX_MONITORED_EVENTS];
    HS_ExecMon_State_t  ExecMonState[HS_MAX_EXEC_CNT_SLOTS];

    uint32 AlivenessCounter; /**< \brief Current Count towards the CPU Aliveness output period */

    uint32 MsgActExec; /**< \brief Number of Software Bus Message Actions Executed */

    uint32 SysMonPspModuleId;  /**< \brief PSP module to track system health, cpu utilization */
    uint16 SysMonSubsystemId;  /**< \brief Subsystem ID for cpu utilization function */
    uint16 SysMonSubchannelId; /**< \brief Subchannel ID for cpu utilization function */

    uint32 UtilizationCycleCounter; /**< \brief Counter to run cpu utilization check */

    uint32 CurrentCPUHoggingTime; /**< \brief Count of cycles that CPU utilization is above hogging threshold */
    uint32 MaxCPUHoggingTime;     /**< \brief Count of hogging cycles after which an event reports hogging */
    uint32 CurrentCPUUtilIndex;   /**< \brief Current index into the Utilization Tracker */

    uint32 UtilizationTracker[HS_UTIL_PEAK_NUM_INTERVAL]; /**< \brief Utilization from previous intervals */

    uint32 UtilCpuAvg;  /**< \brief Current CPU Utilization Average */
    uint32 UtilCpuPeak; /**< \brief Current CPU Utilization Peak */

    CFE_TBL_Handle_t AMTableHandle; /**< \brief Apps Monitor table handle */
    CFE_TBL_Handle_t EMTableHandle; /**< \brief Events Monitor table handle */
    CFE_TBL_Handle_t MATableHandle; /**< \brief Message Actions table handle */
    CFE_TBL_Handle_t XCTableHandle; /**< \brief Execution Counters table handle */

    HS_XCTEntry_t *XCTablePtr; /**< \brief Ptr to Execution Counters table entry */
    HS_AMTEntry_t *AMTablePtr; /**< \brief Ptr to Apps Monitor table entry */
    HS_EMTEntry_t *EMTablePtr; /**< \brief Ptr to Events Monitor table entry */
    HS_MATEntry_t *MATablePtr; /**< \brief Ptr to Message Actions table entry */

    CFE_ES_CDSHandle_t MyCDSHandle; /* \brief Handle to CDS memory block */
    HS_CDSData_t       CDSData;     /* \brief Copy of Critical Data */

} HS_AppData_t;

/************************************************************************
 * Exported Data
 ************************************************************************/
extern HS_AppData_t HS_AppData;

/************************************************************************
 * Exported Functions
 ************************************************************************/

/**
 * \brief Obtain AM state entry
 *
 *  \par Description
 *       Returns a pointer to the given state entry
 *
 *  \par Assumptions, External Events, and Notes:
 *       No error checking is performed here.  States cannot be used without
 *       the associated configuration, so this should not be used unless
 *       HS_GetAMTEntryByIndex() returns non-NULL for the same index.
 *
 * \returns pointer to entry
 */
static inline HS_AppMon_State_t *HS_GetAMStateByIndex(uint32 TableIndex)
{
    return &HS_AppData.AppMonState[TableIndex];
}

/**
 * \brief Obtain EM state entry
 *
 *  \par Description
 *       Returns a pointer to the given state entry
 *
 *  \par Assumptions, External Events, and Notes:
 *       No error checking is performed here.  States cannot be used without
 *       the associated configuration, so this should not be used unless
 *       HS_GetEMTEntryByIndex() returns non-NULL for the same index.
 *
 * \returns pointer to entry
 */
static inline HS_EventMon_State_t *HS_GetEMStateByIndex(uint32 TableIndex)
{
    return &HS_AppData.EventMonState[TableIndex];
}

/**
 * \brief Obtain MA state entry
 *
 *  \par Description
 *       Returns a pointer to the given state entry
 *
 *  \par Assumptions, External Events, and Notes:
 *       No error checking is performed here.  States cannot be used without
 *       the associated configuration, so this should not be used unless
 *       HS_GetMATEntryByIndex() returns non-NULL for the same index.
 *
 * \returns pointer to entry
 */
static inline HS_MsgActState_t *HS_GetMAStateByIndex(uint32 TableIndex)
{
    return &HS_AppData.MsgActState[TableIndex];
}

/**
 * \brief Obtain XC state entry
 *
 *  \par Description
 *       Returns a pointer to the given state entry
 *
 *  \par Assumptions, External Events, and Notes:
 *       No error checking is performed here.  States cannot be used without
 *       the associated configuration, so this should not be used unless
 *       HS_GetXCTEntryByIndex() returns non-NULL for the same index.
 *
 * \returns pointer to entry
 */
static inline HS_ExecMon_State_t *HS_GetXCStateByIndex(uint32 TableIndex)
{
    return &HS_AppData.ExecMonState[TableIndex];
}

/**
 * \brief Safely obtain AM table entry
 *
 *  \par Description
 *       Returns a pointer to the given table entry, if the table is
 *       loaded and the entry is valid.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 * \retval  NULL if index is invalid or table not loaded
 * \returns pointer to entry if successful
 */
static inline HS_AMTEntry_t *HS_GetAMTEntryByIndex(uint32 TableIndex)
{
    HS_AMTEntry_t *AMTEntryPtr;

    if (TableIndex < HS_MAX_MONITORED_APPS)
    {
        AMTEntryPtr = HS_AppData.AMTablePtr;
    }
    else
    {
        AMTEntryPtr = NULL;
    }

    if (AMTEntryPtr)
    {
        AMTEntryPtr += TableIndex;
    }

    return AMTEntryPtr;
}

/**
 * \brief Safely obtain EM table entry
 *
 *  \par Description
 *       Returns a pointer to the given table entry, if the table is
 *       loaded and the entry is valid.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 * \retval  NULL if index is invalid or table not loaded
 * \returns pointer to entry if successful
 */
static inline HS_EMTEntry_t *HS_GetEMTEntryByIndex(uint32 TableIndex)
{
    HS_EMTEntry_t *EMTEntryPtr;

    if (TableIndex < HS_MAX_MONITORED_EVENTS)
    {
        EMTEntryPtr = HS_AppData.EMTablePtr;
    }
    else
    {
        EMTEntryPtr = NULL;
    }

    if (EMTEntryPtr)
    {
        EMTEntryPtr += TableIndex;
    }

    return EMTEntryPtr;
}

/**
 * \brief Safely obtain MA table entry
 *
 *  \par Description
 *       Returns a pointer to the given table entry, if the table is
 *       loaded and the entry is valid.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 * \retval  NULL if index is invalid or table not loaded
 * \returns pointer to entry if successful
 */
static inline HS_MATEntry_t *HS_GetMATEntryByIndex(uint32 TableIndex)
{
    HS_MATEntry_t *MATEntryPtr;

    if (TableIndex < HS_MAX_MSG_ACT_TYPES)
    {
        MATEntryPtr = HS_AppData.MATablePtr;
    }
    else
    {
        MATEntryPtr = NULL;
    }

    if (MATEntryPtr)
    {
        MATEntryPtr += TableIndex;
    }

    return MATEntryPtr;
}

/**
 * \brief Safely obtain XC table entry
 *
 *  \par Description
 *       Returns a pointer to the given table entry, if the table is
 *       loaded and the entry is valid.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 * \retval  NULL if index is invalid or table not loaded
 * \returns pointer to entry if successful
 */
static inline HS_XCTEntry_t *HS_GetXCTEntryByIndex(uint32 TableIndex)
{
    HS_XCTEntry_t *XCTEntryPtr;

    if (TableIndex < HS_MAX_EXEC_CNT_SLOTS)
    {
        XCTEntryPtr = HS_AppData.XCTablePtr;
    }
    else
    {
        XCTEntryPtr = NULL;
    }

    if (XCTEntryPtr)
    {
        XCTEntryPtr += TableIndex;
    }

    return XCTEntryPtr;
}

/**
 * \brief CFS Health and Safety (HS) application entry point
 *
 *  \par Description
 *       Health and Safety application entry point and main process loop.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 */
void HS_AppMain(void);

/**
 * \brief Initialize the CFS Health and Safety (HS) application
 *
 *  \par Description
 *       Health and Safety application initialization routine. This
 *       function performs all the required startup steps to
 *       initialize HS data structures and get the application
 *       registered with the cFE services so it can begin to receive
 *       command messages.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
CFE_Status_t HS_AppInit(void);

/**
 * \brief Initialize Software Bus
 *
 *  \par Description
 *       This function performs the steps required to setup the
 *       cFE software bus for use by the HS application
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
CFE_Status_t HS_SbInit(void);

/**
 * \brief Initialize cFE Table Services
 *
 *  \par Description
 *       This function performs those steps required to initialize the
 *       relationship between the HS App and the cFE Table Services.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
CFE_Status_t HS_TblInit(void);

/**
 * \brief Perform Normal Periodic Processing
 *
 *  \par Description
 *       This function performs the normal Health and Safety monitoring
 *       functions including application, event and execution counters,
 *       as well as servicing the watchdog, outputing the aliveness
 *       indicator, and receiving commands or HK requests.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
CFE_Status_t HS_ProcessMain(void);

/**
 * \brief Process commands received from cFE Software Bus
 *
 *  \par Description
 *       This function pulls messages from command pipe and processes
 *       them accordingly.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \return Execution status, see \ref CFEReturnCodes
 *  \retval #CFE_SUCCESS \copybrief CFE_SUCCESS
 */
CFE_Status_t HS_ProcessCommands(void);

/**
 * \brief Update and store CDS data
 *
 *  \par Description
 *       This function is called to update and then store the data in the
 *       critical data store.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [in]   ResetsPerformed     Number of HS caused processor resets
 *  \param [in]   MaxResets           Max number of resets allowed
 */
void HS_SetCDSData(uint16 ResetsPerformed, uint16 MaxResets);

/**
 * \brief Compute status flags for telemetry reporting
 *
 *  \par Description
 *       The Housekeeping data includes a field containing binary
 *       flags indicating whether the tables are loaded.  This function
 *       computes those flags from the internal runtime status info.
 *
 *  \par Assumptions, External Events, and Notes:
 *       None
 *
 *  \param [out]  StatusFlagsOut   Status flag buffer, will be set
 */
void HS_ComputeStatusFlags(uint8 *StatusFlagsOut);

#endif
