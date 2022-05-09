#include "cfe.h"
#include "cfe_psp.h"
#include "osapi.h"
#include "hs_app.h"
#include "hs_cmds.h"
#include "hs_msg.h"
#include "hs_utils.h"
#include "hs_custom.h"
#include "hs_events.h"
#include "hs_monitors.h"
#include "hs_perfids.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

HS_CustomData_t HS_CustomData;

int32 HS_CustomInit(void)
{
    return UT_DEFAULT_IMPL(HS_CustomInit);
}

void HS_CustomCleanup(void)
{
    UT_DEFAULT_IMPL(HS_CustomCleanup);
}

void HS_CustomMonitorUtilization(void)
{
    UT_DEFAULT_IMPL(HS_CustomMonitorUtilization);
}

int32 HS_CustomGetUtil(void)
{
    return UT_DEFAULT_IMPL(HS_CustomGetUtil);
}

int32 HS_CustomCommands(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_CustomCommands), BufPtr);
    return UT_DEFAULT_IMPL(HS_CustomCommands);
}

void HS_IdleTask(void)
{
    UT_DEFAULT_IMPL(HS_IdleTask);
}

void HS_UtilizationIncrement(void)
{
    UT_DEFAULT_IMPL(HS_UtilizationIncrement);
}

void HS_UtilizationMark(void)
{
    UT_DEFAULT_IMPL(HS_UtilizationMark);
}

void HS_MarkIdleCallback(void)
{
    UT_DEFAULT_IMPL(HS_MarkIdleCallback);
}

void HS_UtilDiagReport(void)
{
    UT_DEFAULT_IMPL(HS_UtilDiagReport);
}

void HS_SetUtilParamsCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_SetUtilParamsCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_SetUtilParamsCmd);
}

void HS_SetUtilDiagCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_SetUtilDiagCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_SetUtilDiagCmd);
}
