
#include "hs_utils.h"

/* UT includes */
#include "uttest.h"
#include "utstubs.h"

bool HS_VerifyMsgLength(const CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    UT_Stub_RegisterContext(UT_KEY(HS_VerifyMsgLength), MsgPtr);
    UT_Stub_RegisterContextGenericArg(UT_KEY(HS_VerifyMsgLength), ExpectedLength);
    return UT_DEFAULT_IMPL(HS_VerifyMsgLength);
}

bool HS_AMTActionIsValid(uint16 ActionType)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(HS_AMTActionIsValid), ActionType);
    return UT_DEFAULT_IMPL(HS_AMTActionIsValid);
}

bool HS_EMTActionIsValid(uint16 ActionType)
{
    UT_Stub_RegisterContextGenericArg(UT_KEY(HS_EMTActionIsValid), ActionType);
    return UT_DEFAULT_IMPL(HS_EMTActionIsValid);
}
