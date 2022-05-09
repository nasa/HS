#include "hs_app.h"
#include "hs_monitors.h"
#include "hs_custom.h"
#include "hs_tbldefs.h"
#include "hs_events.h"
#include "hs_utils.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

void HS_MonitorApplications(void)
{
    UT_DEFAULT_IMPL(HS_MonitorApplications);
}

void HS_MonitorEvent(const CFE_SB_Buffer_t *BufPtr)
{
    UT_DEFAULT_IMPL(HS_MonitorEvent);
}

void HS_MonitorUtilization(void)
{
    UT_DEFAULT_IMPL(HS_MonitorUtilization);
}

int32 HS_ValidateAMTable(void *TableData)
{
    UT_Stub_RegisterContext(UT_KEY(HS_ValidateAMTable), TableData);
    return UT_DEFAULT_IMPL(HS_ValidateAMTable);
}

int32 HS_ValidateEMTable(void *TableData)
{
    UT_Stub_RegisterContext(UT_KEY(HS_ValidateEMTable), TableData);
    return UT_DEFAULT_IMPL(HS_ValidateEMTable);
}

int32 HS_ValidateXCTable(void *TableData)
{
    UT_Stub_RegisterContext(UT_KEY(HS_ValidateXCTable), TableData);
    return UT_DEFAULT_IMPL(HS_ValidateXCTable);
}

int32 HS_ValidateMATable(void *TableData)
{
    UT_Stub_RegisterContext(UT_KEY(HS_ValidateMATable), TableData);
    return UT_DEFAULT_IMPL(HS_ValidateMATable);
}

void HS_SetCDSData(uint16 ResetsPerformed, uint16 MaxResets)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(HS_SetCDSData), ResetsPerformed);
    UT_Stub_RegisterContextGenericArg(UT_KEY(HS_SetCDSData), MaxResets);
    UT_DEFAULT_IMPL(HS_SetCDSData);
}
