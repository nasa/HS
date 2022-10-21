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
 *   The CFS Health and Safety (HS) provides several utilities including
 *   application monitoring, event monitoring, cpu utilization monitoring,
 *   aliveness indication, and watchdog servicing.
 */

/************************************************************************
** Includes
*************************************************************************/
#include "hs_app.h"
#include "hs_eventids.h"
#include "hs_msgids.h"
#include "hs_perfids.h"
#include "hs_monitors.h"
#include "hs_sysmon.h"
#include "hs_version.h"
#include "hs_cmds.h"
#include "hs_dispatch.h"
#include "hs_verify.h"

/************************************************************************
** Macro Definitions
*************************************************************************/

/************************************************************************
** HS global data
*************************************************************************/
HS_AppData_t HS_AppData;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* HS application entry point and main process loop                */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HS_AppMain(void)
{
    CFE_Status_t     Status;
    uint32           RunStatus = CFE_ES_RunStatus_APP_RUN;
    CFE_SB_Buffer_t *BufPtr    = NULL;

    /*
    ** Performance Log, Start
    */
    CFE_ES_PerfLogEntry(HS_APPMAIN_PERF_ID);

    /*
    ** Perform application specific initialization
    */
    Status = HS_AppInit();

    /*
    ** If no errors were detected during initialization, then wait for everyone to start
    */
    if (Status == CFE_SUCCESS)
    {
        CFE_ES_WaitForStartupSync(HS_STARTUP_SYNC_TIMEOUT);

        /*
        ** Enable and set the watchdog timer
        */
        CFE_PSP_WatchdogSet(HS_WATCHDOG_TIMEOUT_VALUE);
        CFE_PSP_WatchdogService();
        CFE_PSP_WatchdogEnable();
        CFE_PSP_WatchdogService();

        /*
        ** Subscribe to Event Messages
        */
        if (HS_AppData.CurrentEventMonState == HS_STATE_ENABLED)
        {
            Status = CFE_SB_SubscribeEx(CFE_SB_ValueToMsgId(CFE_EVS_LONG_EVENT_MSG_MID), HS_AppData.EventPipe,
                                        CFE_SB_DEFAULT_QOS, HS_EVENT_PIPE_DEPTH);
            if (Status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(HS_SUB_LONG_EVS_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Error Subscribing to long-format Events,RC=0x%08X", (unsigned int)Status);

                /* Set run status to terminate main loop */
                RunStatus = CFE_ES_RunStatus_APP_ERROR;
            }

            if (Status == CFE_SUCCESS)
            {
                Status = CFE_SB_SubscribeEx(CFE_SB_ValueToMsgId(CFE_EVS_SHORT_EVENT_MSG_MID), HS_AppData.EventPipe,
                                            CFE_SB_DEFAULT_QOS, HS_EVENT_PIPE_DEPTH);
                if (Status != CFE_SUCCESS)
                {
                    CFE_EVS_SendEvent(HS_SUB_SHORT_EVS_ERR_EID, CFE_EVS_EventType_ERROR,
                                      "Error Subscribing to short-format Events,RC=0x%08X", (unsigned int)Status);

                    /* Set run status to terminate main loop */
                    RunStatus = CFE_ES_RunStatus_APP_ERROR;
                }
            }
        }
    }

    /*
    ** Application main loop
    */
    while (CFE_ES_RunLoop(&RunStatus) == true)
    {
        /*
        ** Performance Log, Stop
        */
        CFE_ES_PerfLogExit(HS_APPMAIN_PERF_ID);

        /*
        ** Task Delay for a configured timeout
        */
#if HS_POST_PROCESSING_DELAY != 0
        OS_TaskDelay(HS_POST_PROCESSING_DELAY);
#endif

        /*
        ** Task Delay for a configured timeout
        */
        Status = CFE_SB_ReceiveBuffer(&BufPtr, HS_AppData.WakeupPipe, HS_WAKEUP_TIMEOUT);

        /*
        ** Performance Log, Start
        */
        CFE_ES_PerfLogEntry(HS_APPMAIN_PERF_ID);

        /*
        ** Process the software bus message
        */
        if ((Status == CFE_SUCCESS) || (Status == CFE_SB_NO_MESSAGE) || (Status == CFE_SB_TIME_OUT))
        {
            Status = HS_ProcessMain();
        }

        /*
        ** Note: If there were some reason to exit the task
        **       normally (without error) then we would set
        **       RunStatus = CFE_ES_APP_EXIT
        */
        if (Status != CFE_SUCCESS)
        {
            /*
            ** Set request to terminate main loop
            */
            RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    } /* end CFS_ES_RunLoop while */

    /*
    ** Check for "fatal" process error...
    */
    if (Status != CFE_SUCCESS)
    {
        /*
        ** Send an event describing the reason for the termination
        */
        CFE_EVS_SendEvent(HS_APP_EXIT_EID, CFE_EVS_EventType_CRITICAL, "Application Terminating, err = 0x%08X",
                          (unsigned int)Status);

        /*
        ** In case cFE Event Services is not working
        */
        CFE_ES_WriteToSysLog("HS App: Application Terminating, ERR = 0x%08X\n", (unsigned int)Status);
    }

    HS_SysMonCleanup();

    /*
    ** Performance Log, Stop
    */
    CFE_ES_PerfLogExit(HS_APPMAIN_PERF_ID);

    /*
    ** Exit the application
    */
    CFE_ES_ExitApp(RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* HS initialization                                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_AppInit(void)
{
    CFE_Status_t Status;

    /*
    ** Initialize operating data to default states...
    */
    memset(&HS_AppData, 0, sizeof(HS_AppData));
    HS_AppData.ServiceWatchdogFlag = HS_STATE_ENABLED;
    HS_AppData.RunStatus           = CFE_ES_RunStatus_APP_RUN;

    HS_AppData.CurrentAppMonState    = HS_APPMON_DEFAULT_STATE;
    HS_AppData.CurrentEventMonState  = HS_EVENTMON_DEFAULT_STATE;
    HS_AppData.CurrentAlivenessState = HS_ALIVENESS_DEFAULT_STATE;
    HS_AppData.CurrentCPUHogState    = HS_CPUHOG_DEFAULT_STATE;

    HS_AppData.ExeCountState  = HS_STATE_ENABLED;
    HS_AppData.MsgActsState   = HS_STATE_ENABLED;
    HS_AppData.AppMonLoaded   = HS_STATE_ENABLED;
    HS_AppData.EventMonLoaded = HS_STATE_ENABLED;
    HS_AppData.CDSState       = HS_STATE_ENABLED;

    HS_AppData.MaxCPUHoggingTime = HS_UTIL_HOGGING_TIMEOUT;

    /*
    ** Register for event services...
    */
    Status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (Status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("HS App: Error Registering For Event Services, RC = 0x%08X\n", (unsigned int)Status);
        return Status;
    }

    /*
    ** Create Critical Data Store
    */
    Status = CFE_ES_RegisterCDS(&HS_AppData.MyCDSHandle, sizeof(HS_CDSData_t), HS_CDSNAME);

    if (Status == CFE_ES_CDS_ALREADY_EXISTS)
    {
        /*
        ** Critical Data Store already existed, we need to get a
        ** copy of its current contents to see if we can use it
        */
        Status = CFE_ES_RestoreFromCDS(&HS_AppData.CDSData, HS_AppData.MyCDSHandle);

        if (Status == CFE_SUCCESS)
        {
            if ((HS_AppData.CDSData.ResetsPerformed != (uint16)~HS_AppData.CDSData.ResetsPerformedNot) ||
                (HS_AppData.CDSData.MaxResets != (uint16)~HS_AppData.CDSData.MaxResetsNot))
            {
                /*
                ** Report error restoring data
                */
                CFE_EVS_SendEvent(HS_CDS_CORRUPT_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Data in CDS was corrupt, initializing resets data");
                /*
                ** If data was corrupt, initialize data
                */
                HS_SetCDSData(0, HS_MAX_RESTART_ACTIONS);
            }
        }
        else
        {
            /*
            ** Report error restoring data
            */
            CFE_EVS_SendEvent(HS_CDS_RESTORE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Failed to restore data from CDS (Err=0x%08x), initializing resets data",
                              (unsigned int)Status);
            /*
            ** If data could not be retrieved, initialize data
            */
            HS_SetCDSData(0, HS_MAX_RESTART_ACTIONS);
        }

        Status = CFE_SUCCESS;
    }
    else if (Status == CFE_SUCCESS)
    {
        /*
        ** If CDS did not previously exist, initialize data
        */
        HS_SetCDSData(0, HS_MAX_RESTART_ACTIONS);
    }
    else
    {
        /*
        ** Disable saving to CDS
        */
        HS_AppData.CDSState = HS_STATE_DISABLED;

        /*
        ** Initialize values anyway (they will not be saved)
        */
        HS_SetCDSData(0, HS_MAX_RESTART_ACTIONS);
    }

    /*
    ** Set up the HS Software Bus
    */
    Status = HS_SbInit();
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    /*
    ** Register The HS Tables
    */
    Status = HS_TblInit();
    if (Status != CFE_SUCCESS)
    {
        return Status;
    }

    /*
    ** Perform initialization for system monitoring
    */
    Status = HS_SysMonInit();
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_SYSMON_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Error in system monitor initialization, RC=0x%08X", (unsigned int)Status);
        return Status;
    }

    /*
    ** Application initialization event
    */
    CFE_EVS_SendEvent(HS_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "HS Initialized.  Version %d.%d.%d.%d",
                      HS_MAJOR_VERSION, HS_MINOR_VERSION, HS_REVISION, HS_MISSION_REV);

    return Status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize the software bus interface                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_SbInit(void)
{
    CFE_Status_t Status;

    /* Initialize housekeeping packet  */
    CFE_MSG_Init(CFE_MSG_PTR(HS_AppData.HkPacket.TelemetryHeader), CFE_SB_ValueToMsgId(HS_HK_TLM_MID),
                 sizeof(HS_HkPacket_t));

    /* Create Command Pipe */
    Status = CFE_SB_CreatePipe(&HS_AppData.CmdPipe, HS_CMD_PIPE_DEPTH, HS_CMD_PIPE_NAME);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_CR_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "Error Creating SB Command Pipe,RC=0x%08X",
                          (unsigned int)Status);
        return Status;
    }

    /* Create Event Pipe */
    Status = CFE_SB_CreatePipe(&HS_AppData.EventPipe, HS_EVENT_PIPE_DEPTH, HS_EVENT_PIPE_NAME);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_CR_EVENT_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "Error Creating SB Event Pipe,RC=0x%08X",
                          (unsigned int)Status);
        return Status;
    }

    /* Create Wakeup Pipe */
    Status = CFE_SB_CreatePipe(&HS_AppData.WakeupPipe, HS_WAKEUP_PIPE_DEPTH, HS_WAKEUP_PIPE_NAME);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_CR_WAKEUP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "Error Creating SB Wakeup Pipe,RC=0x%08X",
                          (unsigned int)Status);
        return Status;
    }

    /* Subscribe to Housekeeping Request */
    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(HS_SEND_HK_MID), HS_AppData.CmdPipe);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_SUB_REQ_ERR_EID, CFE_EVS_EventType_ERROR, "Error Subscribing to HK Request,RC=0x%08X",
                          (unsigned int)Status);
        return Status;
    }

    /* Subscribe to HS ground commands */
    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(HS_CMD_MID), HS_AppData.CmdPipe);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_SUB_CMD_ERR_EID, CFE_EVS_EventType_ERROR, "Error Subscribing to Gnd Cmds,RC=0x%08X",
                          (unsigned int)Status);
        return Status;
    }

    /* Subscribe to HS Wakeup Message */
    Status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(HS_WAKEUP_MID), HS_AppData.WakeupPipe);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_SUB_WAKEUP_ERR_EID, CFE_EVS_EventType_ERROR, "Error Subscribing to Wakeup,RC=0x%08X",
                          (unsigned int)Status);
        return Status;
    }

    /*
    ** Event message subscription delayed until after startup synch
    */

    return Status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Initialize the table interface                                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_TblInit(void)
{
    uint32       TableSize  = 0;
    uint32       TableIndex = 0;
    CFE_Status_t Status;

    /* Register The HS Applications Monitor Table */
    TableSize = HS_MAX_MONITORED_APPS * sizeof(HS_AMTEntry_t);
    Status    = CFE_TBL_Register(&HS_AppData.AMTableHandle, HS_AMT_TABLENAME, TableSize, CFE_TBL_OPT_DEFAULT,
                              HS_ValidateAMTable);

    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_AMT_REG_ERR_EID, CFE_EVS_EventType_ERROR, "Error Registering AppMon Table,RC=0x%08X",
                          (unsigned int)Status);
        return Status;
    }

    /* Register The HS Events Monitor Table */
    TableSize = HS_MAX_MONITORED_EVENTS * sizeof(HS_EMTEntry_t);
    Status    = CFE_TBL_Register(&HS_AppData.EMTableHandle, HS_EMT_TABLENAME, TableSize, CFE_TBL_OPT_DEFAULT,
                              HS_ValidateEMTable);

    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_EMT_REG_ERR_EID, CFE_EVS_EventType_ERROR, "Error Registering EventMon Table,RC=0x%08X",
                          (unsigned int)Status);
        return Status;
    }

    /* Register The HS Message Actions Table */
    TableSize = HS_MAX_MSG_ACT_TYPES * sizeof(HS_MATEntry_t);
    Status    = CFE_TBL_Register(&HS_AppData.MATableHandle, HS_MAT_TABLENAME, TableSize, CFE_TBL_OPT_DEFAULT,
                              HS_ValidateMATable);

    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_MAT_REG_ERR_EID, CFE_EVS_EventType_ERROR, "Error Registering MsgActs Table,RC=0x%08X",
                          (unsigned int)Status);
        return Status;
    }

    /* Register The HS Execution Counters Table */
    TableSize = HS_MAX_EXEC_CNT_SLOTS * sizeof(HS_XCTEntry_t);
    Status    = CFE_TBL_Register(&HS_AppData.XCTableHandle, HS_XCT_TABLENAME, TableSize, CFE_TBL_OPT_DEFAULT,
                              HS_ValidateXCTable);

    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_XCT_REG_ERR_EID, CFE_EVS_EventType_ERROR, "Error Registering ExeCount Table,RC=0x%08X",
                          (unsigned int)Status);
        return Status;
    }

    /* Load the HS Execution Counters Table */
    Status = CFE_TBL_Load(HS_AppData.XCTableHandle, CFE_TBL_SRC_FILE, (const void *)HS_XCT_FILENAME);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_XCT_LD_ERR_EID, CFE_EVS_EventType_ERROR, "Error Loading ExeCount Table,RC=0x%08X",
                          (unsigned int)Status);
        HS_AppData.ExeCountState = HS_STATE_DISABLED;
        for (TableIndex = 0; TableIndex < HS_MAX_EXEC_CNT_SLOTS; TableIndex++)
        {
            /* HS 8005.1 Report 0xFFFFFFFF for all entries */
            HS_AppData.HkPacket.Payload.ExeCounts[TableIndex] = HS_INVALID_EXECOUNT;
        }
    }

    /* Load the HS Applications Monitor Table */
    Status = CFE_TBL_Load(HS_AppData.AMTableHandle, CFE_TBL_SRC_FILE, (const void *)HS_AMT_FILENAME);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_AMT_LD_ERR_EID, CFE_EVS_EventType_ERROR, "Error Loading AppMon Table,RC=0x%08X",
                          (unsigned int)Status);
        HS_AppData.CurrentAppMonState = HS_STATE_DISABLED;
        CFE_EVS_SendEvent(HS_DISABLE_APPMON_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Application Monitoring Disabled due to Table Load Failure");
        HS_AppData.AppMonLoaded = HS_STATE_DISABLED;
    }

    /* Load the HS Events Monitor Table */
    Status = CFE_TBL_Load(HS_AppData.EMTableHandle, CFE_TBL_SRC_FILE, (const void *)HS_EMT_FILENAME);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_EMT_LD_ERR_EID, CFE_EVS_EventType_ERROR, "Error Loading EventMon Table,RC=0x%08X",
                          (unsigned int)Status);
        HS_AppData.CurrentEventMonState = HS_STATE_DISABLED;
        CFE_EVS_SendEvent(HS_DISABLE_EVENTMON_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Event Monitoring Disabled due to Table Load Failure");
        HS_AppData.EventMonLoaded = HS_STATE_DISABLED;
    }

    /* Load the HS Message Actions Table */
    Status = CFE_TBL_Load(HS_AppData.MATableHandle, CFE_TBL_SRC_FILE, (const void *)HS_MAT_FILENAME);
    if (Status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(HS_MAT_LD_ERR_EID, CFE_EVS_EventType_ERROR, "Error Loading MsgActs Table,RC=0x%08X",
                          (unsigned int)Status);
        HS_AppData.MsgActsState = HS_STATE_DISABLED;
    }

    /*
    ** Get pointers to table data
    */
    HS_AcquirePointers();

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Main Processing Loop                                            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_ProcessMain(void)
{
    CFE_Status_t Status      = CFE_SUCCESS;
    const char * AliveString = HS_CPU_ALIVE_STRING;
    uint32       i           = 0;

    /*
    ** Get Tables
    */
    HS_AcquirePointers();

    /*
    ** Decrement Cooldowns for Message Actions
    */
    for (i = 0; i < HS_MAX_MSG_ACT_TYPES; i++)
    {
        if (HS_AppData.MsgActCooldown[i] != 0)
        {
            HS_AppData.MsgActCooldown[i]--;
        }
    }

    if (HS_AppData.UtilizationCycleCounter == 0)
    {
        HS_MonitorUtilization();
        HS_AppData.UtilizationCycleCounter = HS_CPU_UTILIZATION_CYCLES_PER_INTERVAL;
    }
    --HS_AppData.UtilizationCycleCounter;

    /*
    ** Monitor Applications
    */
    if (HS_AppData.CurrentAppMonState == HS_STATE_ENABLED)
    {
        HS_MonitorApplications();
    }

    /*
    ** Output Aliveness
    */
    if (HS_AppData.CurrentAlivenessState == HS_STATE_ENABLED)
    {
        HS_AppData.AlivenessCounter++;

        if (HS_AppData.AlivenessCounter >= HS_CPU_ALIVE_PERIOD)
        {
            OS_printf("%s", AliveString);
            HS_AppData.AlivenessCounter = 0;
        }
    }

    /*
    ** Check for Commands, Events, and HK Requests
    */
    Status = HS_ProcessCommands();

    /*
    ** Service the Watchdog
    */
    if (HS_AppData.ServiceWatchdogFlag == HS_STATE_ENABLED)
    {
        CFE_PSP_WatchdogService();
    }

    return Status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Process any Commands and Event Messages received this cycle     */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
CFE_Status_t HS_ProcessCommands(void)
{
    CFE_Status_t     Status = CFE_SUCCESS;
    CFE_SB_Buffer_t *BufPtr = NULL;

    /*
    ** Event Message Pipe (done first so EventMon does not get enabled without table checking)
    */
    if (HS_AppData.CurrentEventMonState == HS_STATE_ENABLED)
    {
        while (Status == CFE_SUCCESS)
        {
            Status = CFE_SB_ReceiveBuffer(&BufPtr, HS_AppData.EventPipe, CFE_SB_POLL);

            if ((Status == CFE_SUCCESS) && (BufPtr != NULL))
            {
                /*
                ** Pass Events to Event Monitor
                */
                HS_AppData.EventsMonitoredCount++;
                HS_MonitorEvent((CFE_EVS_LongEventTlm_t *)BufPtr);
            }
        }
    }

    if (Status == CFE_SB_NO_MESSAGE)
    {
        /*
        ** It's Good to not get a message -- we are polling
        */
        Status = CFE_SUCCESS;
    }

    /*
    ** Command and HK Requests Pipe
    */
    while (Status == CFE_SUCCESS)
    {
        /*
        ** Process pending Commands or HK Reqs
        */
        Status = CFE_SB_ReceiveBuffer(&BufPtr, HS_AppData.CmdPipe, CFE_SB_POLL);
        if ((Status == CFE_SUCCESS) && (BufPtr != NULL))
        {
            /*
            ** Pass Commands/HK Req to AppPipe Processing
            */
            HS_AppPipe(BufPtr);
        }
    }

    if (Status == CFE_SB_NO_MESSAGE)
    {
        /*
        ** It's Good to not get a message -- we are polling
        */
        Status = CFE_SUCCESS;
    }

    return Status;
}
