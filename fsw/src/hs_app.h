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
 *   Unit specification for the Core Flight System (CFS)
 *   Health and Safety (HS) Application.
 */
#ifndef HS_APP_H
#define HS_APP_H

/************************************************************************
 * Includes
 ************************************************************************/
#include "hs_msg.h"
#include "hs_tbldefs.h"
#include "hs_tbl.h"
#include "hs_platform_cfg.h"
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

    uint8 ServiceWatchdogFlag; /**< \brief Flag of current watchdog servicing state */

    uint8 CurrentAppMonState;    /**< \brief Status of HS Application Monitor */
    uint8 CurrentEventMonState;  /**< \brief Status of HS Events Monitor */
    uint8 CurrentAlivenessState; /**< \brief Status of HS Aliveness Indicator */
    uint8 ExeCountState;         /**< \brief Status of Execution Counter Table */

    uint8 MsgActsState;   /**< \brief Status of Message Actions Table */
    uint8 CDSState;       /**< \brief Status of Critical Data Storing */
    uint8 AppMonLoaded;   /**< \brief If AppMon Table is loaded */
    uint8 EventMonLoaded; /**< \brief If EventMon Table is loaded */

    uint8 CurrentCPUHogState; /**< \brief Status of HS CPU Hogging Indicator */
    uint8 SpareBytes[3];      /**< \brief Spare bytes for 32 bit alignment padding */

    uint8 CmdCount;    /**< \brief Number of valid commands received */
    uint8 CmdErrCount; /**< \brief Number of invalid commands received */

    uint32 EventsMonitoredCount; /**< \brief Total count of event messages monitored */

    uint16 MsgActCooldown[HS_MAX_MSG_ACT_TYPES];          /**< \brief Counts until Message Actions is available */
    uint16 AppMonCheckInCountdown[HS_MAX_MONITORED_APPS]; /**< \brief Counts until Application Monitor times out */

    uint32 AppMonEnables[((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE) +
                         1]; /**< \brief AppMon state by monitor */

    uint32 AppMonLastExeCount[HS_MAX_MONITORED_APPS]; /**< \brief Last Execution Count for application being checked */

    uint32 AlivenessCounter; /**< \brief Current Count towards the CPU Aliveness output period */

    uint32 MsgActExec; /**< \brief Number of Software Bus Message Actions Executed */

    uint32 RunStatus; /**< \brief HS App run status */

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
    HS_XCTEntry_t *  XCTablePtr;    /**< \brief Ptr to Execution Counters table entry */

    HS_AMTEntry_t *AMTablePtr; /**< \brief Ptr to Apps Monitor table entry */
    HS_EMTEntry_t *EMTablePtr; /**< \brief Ptr to Events Monitor table entry */
    HS_MATEntry_t *MATablePtr; /**< \brief Ptr to Message Actions table entry */

    CFE_ES_CDSHandle_t MyCDSHandle; /* \brief Handle to CDS memory block */
    HS_CDSData_t       CDSData;     /* \brief Copy of Critical Data */

    HS_HkPacket_t HkPacket; /**< \brief HK Housekeeping Packet */
} HS_AppData_t;

/************************************************************************
 * Exported Data
 ************************************************************************/
extern HS_AppData_t HS_AppData;

/************************************************************************
 * Exported Functions
 ************************************************************************/

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

#endif
