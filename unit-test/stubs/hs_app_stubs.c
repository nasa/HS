
#include "hs_app.h"

/* UT includes */
#include "uttest.h"
#include "utassert.h"
#include "utstubs.h"

HS_AppData_t HS_AppData;

void HS_AppMain(void)
{
    UT_DEFAULT_IMPL(HS_AppMain);
}

int32 HS_AppInit(void)
{
    return UT_DEFAULT_IMPL(HS_AppInit);
}

int32 HS_SbInit(void)
{
    return UT_DEFAULT_IMPL(HS_SbInit);
}

int32 HS_TblInit(void)
{
    return UT_DEFAULT_IMPL(HS_TblInit);
}

int32 HS_ProcessMain(void)
{
    return UT_DEFAULT_IMPL(HS_ProcessMain);
}

int32 HS_ProcessCommands(void)
{
    return UT_DEFAULT_IMPL(HS_ProcessCommands);
}
