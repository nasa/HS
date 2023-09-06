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
 *   Health and Safety (HS) application system monitor component.
 *
 * Access functions to read system monitor variables such as CPU utilization,
 * temperature, and other health metrics.  Typically these go through the PSP
 * as they are platform-specific.
 */

/*************************************************************************
** Includes
*************************************************************************/
#include "hs_sysmon.h"
#include "hs_platform_cfg.h"
#include "hs_app.h"

#include "cfe.h"
#include "cfe_psp.h"
#include "iodriver_base.h"
#include "iodriver_analog_io.h"
#include "osapi.h"

/*
 * --------------------------------------------------------
 * Initialize The System Monitor functions
 * --------------------------------------------------------
 */
CFE_Status_t HS_SysMonInit(void)
{
    CFE_PSP_IODriver_Location_t Location;
    CFE_Status_t                StatusCode;

    if (CFE_PSP_IODriver_FindByName(HS_SYSTEM_MONITOR_DEVICE, &HS_AppData.SysMonPspModuleId) != CFE_PSP_SUCCESS)
    {
        CFE_ES_WriteToSysLog("%s(): No system monitor device available, CPU usage will not be reported\n", __func__);
        StatusCode = CFE_SUCCESS;
    }
    else
    {
        CFE_ES_WriteToSysLog("%s(): System Monitor using PSP device ID %08lx\n", __func__,
                             (unsigned long)HS_AppData.SysMonPspModuleId);

        Location.PspModuleId  = HS_AppData.SysMonPspModuleId;
        Location.SubsystemId  = 0;
        Location.SubchannelId = 0;

        if (CFE_PSP_IODriver_Command(&Location, CFE_PSP_IODriver_SET_RUNNING, CFE_PSP_IODriver_U32ARG(1)) !=
            CFE_PSP_SUCCESS)
        {
            CFE_ES_WriteToSysLog("%s(): Unable to start device %08lx\n", __func__,
                                 (unsigned long)HS_AppData.SysMonPspModuleId);
            StatusCode = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        }
        else
        {
            /* find the subsystem */
            StatusCode = CFE_PSP_IODriver_Command(&Location, CFE_PSP_IODriver_LOOKUP_SUBSYSTEM,
                                                  CFE_PSP_IODriver_CONST_STR(HS_SYSTEM_MONITOR_SUBSYSTEM_NAME));

            if (StatusCode < 0)
            {
                CFE_ES_WriteToSysLog("%s(): Unable to find subsystem \'%s\'\n", __func__,
                                     HS_SYSTEM_MONITOR_SUBSYSTEM_NAME);
                HS_AppData.SysMonSubsystemId = 0;
            }
            else
            {
                HS_AppData.SysMonSubsystemId = StatusCode;
            }

            Location.SubsystemId = HS_AppData.SysMonSubsystemId;
            StatusCode           = CFE_PSP_IODriver_Command(&Location, CFE_PSP_IODriver_LOOKUP_SUBCHANNEL,
                                                  CFE_PSP_IODriver_CONST_STR(HS_SYSTEM_MONITOR_SUBCHANNEL_NAME));

            if (StatusCode < 0)
            {
                CFE_ES_WriteToSysLog("%s(): Unable to find channel \'%s\'\n", __func__,
                                     HS_SYSTEM_MONITOR_SUBCHANNEL_NAME);
                HS_AppData.SysMonSubchannelId = 0;
            }
            else
            {
                HS_AppData.SysMonSubchannelId = StatusCode;
            }

            /* return success to caller */
            StatusCode = CFE_SUCCESS;
        }
    }

    return StatusCode;
}

void HS_SysMonCleanup(void)
{
    CFE_PSP_IODriver_Location_t Location;

    if (HS_AppData.SysMonPspModuleId != 0)
    {
        Location.PspModuleId  = HS_AppData.SysMonPspModuleId;
        Location.SubsystemId  = 0;
        Location.SubchannelId = 0;

        if (CFE_PSP_IODriver_Command(&Location, CFE_PSP_IODriver_SET_RUNNING, CFE_PSP_IODriver_U32ARG(0)) !=
            CFE_PSP_SUCCESS)
        {
            CFE_ES_WriteToSysLog("%s(): Unable to stop device %08lx\n", __func__,
                                 (unsigned long)HS_AppData.SysMonPspModuleId);
        }
    }
}

/*
 * --------------------------------------------------------
 * Obtain the System CPU utilization information
 * --------------------------------------------------------
 */
CFE_Status_t HS_SysMonGetCpuUtilization(void)
{
    const CFE_PSP_IODriver_Location_t Location = {.PspModuleId  = HS_AppData.SysMonPspModuleId,
                                                  .SubsystemId  = HS_AppData.SysMonSubsystemId,
                                                  .SubchannelId = HS_AppData.SysMonSubchannelId};
    CFE_PSP_IODriver_AdcCode_t        Sample   = 0;
    CFE_PSP_IODriver_AnalogRdWr_t     RdWr     = {.NumChannels = 1, .Samples = &Sample};
    CFE_Status_t                      StatusCode;
    int32                             Value;

    if (HS_AppData.SysMonPspModuleId == 0)
    {
        /* No device to get info from */
        StatusCode = CFE_PSP_ERROR_NOT_IMPLEMENTED;
    }
    else
    {
        StatusCode = CFE_PSP_IODriver_Command(&Location, CFE_PSP_IODriver_ANALOG_IO_READ_CHANNELS,
                                              CFE_PSP_IODriver_VPARG(&RdWr));
    }

    if (StatusCode == CFE_PSP_SUCCESS)
    {
        Value = ((Sample >> 8) * HS_CPU_UTILIZATION_MAX) / 0xFFFF;
    }
    else
    {
        /* Unable to read value */
        Value = -1;
    }

    return Value;
}
