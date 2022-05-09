#include "hs_app.h"
#include "hs_cmds.h"
#include "hs_custom.h"
#include "hs_monitors.h"
#include "hs_msgids.h"
#include "hs_events.h"
#include "hs_utils.h"
#include "hs_version.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

void HS_AppPipe(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_AppPipe), BufPtr);
    UT_DEFAULT_IMPL(HS_AppPipe);
}

void HS_ResetCounters(void)
{
    UT_DEFAULT_IMPL(HS_ResetCounters);
}

void HS_AcquirePointers(void)
{
    UT_DEFAULT_IMPL(HS_AcquirePointers);
}

void HS_HousekeepingReq(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_HousekeepingReq), BufPtr);
    UT_DEFAULT_IMPL(HS_HousekeepingReq);
}

void HS_NoopCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_NoopCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_NoopCmd);
}

void HS_ResetCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_ResetCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_ResetCmd);
}

void HS_EnableAppMonCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_EnableAppMonCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_EnableAppMonCmd);
}

void HS_DisableAppMonCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_DisableAppMonCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_DisableAppMonCmd);
}

void HS_EnableEventMonCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_EnableEventMonCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_EnableEventMonCmd);
}

void HS_DisableEventMonCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_DisableEventMonCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_DisableEventMonCmd);
}

void HS_EnableAlivenessCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_EnableAlivenessCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_EnableAlivenessCmd);
}

void HS_DisableAlivenessCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_DisableAlivenessCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_DisableAlivenessCmd);
}

void HS_EnableCPUHogCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_EnableCPUHogCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_EnableCPUHogCmd);
}

void HS_DisableCPUHogCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_DisableCPUHogCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_DisableCPUHogCmd);
}

void HS_ResetResetsPerformedCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_ResetResetsPerformedCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_ResetResetsPerformedCmd);
}

void HS_SetMaxResetsCmd(const CFE_SB_Buffer_t *BufPtr)
{
    UT_Stub_RegisterContext(UT_KEY(HS_SetMaxResetsCmd), BufPtr);
    UT_DEFAULT_IMPL(HS_SetMaxResetsCmd);
}

void HS_AppMonStatusRefresh(void)
{
    UT_DEFAULT_IMPL(HS_AppMonStatusRefresh);
}

void HS_MsgActsStatusRefresh(void)
{
    UT_DEFAULT_IMPL(HS_MsgActsStatusRefresh);
}
