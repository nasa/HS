/*************************************************************************
** File: hs_utils.h 
**
** NASA Docket No. GSC-18,476-1, and identified as "Core Flight System 
** (cFS) Health and Safety (HS) Application version 2.3.2"
**
** Copyright © 2020 United States Government as represented by the 
** Administrator of the National Aeronautics and Space Administration.  
** All Rights Reserved. 
** 
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
** http://www.apache.org/licenses/LICENSE-2.0 
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License. 
**
** Purpose:
**   Utility functions for the cFS Health and Safety (HS) application.
**
**************************************************************************/
#ifndef _hs_utils_h_
#define _hs_utils_h_

/*************************************************************************
** Includes
*************************************************************************/
#include "cfe.h"


/************************************************************************/
/** \brief Verify message length
**
**  \par Description
**       Checks if the actual length of a software bus message matches
**       the expected length and sends an error event if a mismatch
**       occurs
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   msg              A #CFE_SB_MsgPtr_t pointer that
**                                 references the software bus message
**
**  \param [in]   ExpectedLength   The expected length of the message
**                                 based upon the command code
**
**  \returns
**  \retstmt Returns TRUE if the length is as expected      \endcode
**  \retstmt Returns FALSE if the length is not as expected \endcode
**  \endreturns
**
**  \sa #HS_LEN_ERR_EID
**
*************************************************************************/
bool    HS_VerifyMsgLength(CFE_SB_MsgPtr_t msg,
                           uint16          ExpectedLength);


/************************************************************************/
/** \brief Verify AMT Action Type
**
**  \par Description
**       Checks if the specified value is a valid AMT Action Type.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   msg              An ActionType to validate
**
**  \returns
**  \retstmt Returns TRUE if the ActionType is valid       \endcode
**  \retstmt Returns FALSE if the ActionType is not valid  \endcode
**  \endreturns
**
*************************************************************************/
bool    HS_AMTActionIsValid(uint16 ActionType);


/************************************************************************/
/** \brief Verify EMT Action Type
**
**  \par Description
**       Checks if the specified value is a valid EMT Action Type.
**
**  \par Assumptions, External Events, and Notes:
**       None
**
**  \param [in]   msg              An ActionType to validate
**
**  \returns
**  \retstmt Returns TRUE if the ActionType is valid       \endcode
**  \retstmt Returns FALSE if the ActionType is not valid  \endcode
**  \endreturns
**
*************************************************************************/
bool    HS_EMTActionIsValid(uint16 ActionType);


#endif /* _hs_utils_h_ */
