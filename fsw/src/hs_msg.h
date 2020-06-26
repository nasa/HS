/************************************************************************
** File: hs_msg.h 
**
** NASA Docket No. GSC-16,151-1, and identified as "Core Flight Software System (CFS)
** Health and Safety Application Version 2"
** 
** Copyright © 2007-2014 United States Government as represented by the
** Administrator of the National Aeronautics and Space Administration. All Rights
** Reserved. 
** 
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
** http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License. 
**
** Purpose:
**   Specification for the CFS Health and Safety (HS) command and telemetry
**   message data types.
**
** Notes:
**   Constants and enumerated types related to these message structures
**   are defined in hs_msgdefs.h. They are kept separate to allow easy
**   integration with ASIST RDL files which can't handle typedef
**   declarations (see the main comment block in hs_msgdefs.h for more
**   info).
**
**
*************************************************************************/
#ifndef _hs_msg_h_
#define _hs_msg_h_

/************************************************************************
** Includes
*************************************************************************/
#include "hs_msgdefs.h"
#include "hs_platform_cfg.h"
#include "cfe.h"

/************************************************************************
** Macro Definitions
*************************************************************************/
/**
** \brief HS Bits per AppMon Enable entry */
/** \{ */
#define HS_BITS_PER_APPMON_ENABLE 32
/** \} */

/************************************************************************
** Type Definitions
*************************************************************************/
/**
**  \brief No Arguments Command
**  For command details see #HS_NOOP_CC, #HS_RESET_CC, #HS_ENABLE_APPMON_CC, #HS_DISABLE_APPMON_CC,
**  #HS_ENABLE_EVENTMON_CC, #HS_DISABLE_EVENTMON_CC, #HS_ENABLE_ALIVENESS_CC, #HS_DISABLE_ALIVENESS_CC,
**  #HS_RESET_RESETS_PERFORMED_CC
**  Also see #HS_SEND_HK_MID
*/
typedef struct
{
    uint8          CmdHeader[CFE_SB_CMD_HDR_SIZE];

} HS_NoArgsCmd_t;

/**
**  \brief Set Max Resets Command
**  For command details see #HS_SET_MAX_RESETS_CC
*/
typedef struct
{
    uint8          CmdHeader[CFE_SB_CMD_HDR_SIZE];
    uint16         MaxResets;

} HS_SetMaxResetsCmd_t;

/**
**  \hstlm Housekeeping Packet Structure
*/
typedef struct
{
    uint8     TlmHeader[CFE_SB_TLM_HDR_SIZE]; /**< \brief cFE SB Tlm Msg Hdr */

    uint8     CmdCount;                       /**< \hstlmmnemonic \HS_CMDPC
                                                        \brief HS Application Command Counter       */
    uint8     CmdErrCount;                    /**< \hstlmmnemonic \HS_CMDEC
                                                        \brief HS Application Command Error Counter */
    uint8     CurrentAppMonState;             /**< \hstlmmnemonic \HS_APPMONSTATE
                                                        \brief Status of HS Application Monitor     */
    uint8     CurrentEventMonState;           /**< \hstlmmnemonic \HS_EVTMONSTATE
                                                        \brief Status of HS Event Monitor */
    uint8     CurrentAlivenessState;          /**< \hstlmmnemonic \HS_CPUALIVESTATE
                                                        \brief Status of HS Aliveness Indicator     */
    uint8     CurrentCPUHogState;             /**< \hstlmmnemonic \HS_CPUHOGSTATE
                                                        \brief Status of HS Hogging Indicator     */
    uint8     StatusFlags;                    /**< \hstlmmnemonic \HS_STATUSFLAGS
                                                        \brief Internal HS Error States*/
    uint8     SpareBytes;                     /**< \hstlmmnemonic \HS_SPAREBYTES
                                                        \brief Alignment Spares*/
    uint16    ResetsPerformed;                /**< \hstlmmnemonic \HS_PRRESETCNT
                                                        \brief HS Performed Processor Reset Count   */
    uint16    MaxResets;                      /**< \hstlmmnemonic \HS_MAXRESETCNT
                                                        \brief HS Maximum Processor Reset Count   */
    uint32    EventsMonitoredCount;           /**< \hstlmmnemonic \HS_EVTMONCNT
                                                        \brief Total count of Event Messages
                                                         Monitored by the Events Monitor   */
    uint32    InvalidEventMonCount;           /**< \hstlmmnemonic \HS_INVALIDEVTAPPCNT
                                                        \brief Total count of Invalid Event Monitors
                                                         Monitored by the Events Monitor   */
    uint32    AppMonEnables[((HS_MAX_MONITORED_APPS - 1) / HS_BITS_PER_APPMON_ENABLE)+1];/**< \hstlmmnemonic \HS_APPSTATUS
                                                        \brief Enable states of App Monitor Entries */
    uint32    MsgActExec;                     /**< \hstlmmnemonic \HS_MSGACTEXEC
                                                        \brief Number of Software Bus Message Actions Executed */
    uint32    UtilCpuAvg;                     /**< \hstlmmnemonic \HS_UTILAVG
                                                        \brief Current CPU Utilization Average */
    uint32    UtilCpuPeak;                    /**< \hstlmmnemonic \HS_UTILPEAK
                                                        \brief Current CPU Utilization Peak */
#if HS_MAX_EXEC_CNT_SLOTS != 0
    uint32    ExeCounts[HS_MAX_EXEC_CNT_SLOTS]; /**< \hstlmmnemonic \HS_EXECUTIONCTR
                                                             \brief Execution Counters              */
#endif

} HS_HkPacket_t;

#endif /* _hs_msg_h_ */

/************************/
/*  End of File Comment */
/************************/

