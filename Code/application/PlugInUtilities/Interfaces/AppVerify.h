/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */



#ifndef APPVERIFY_H
#define APPVERIFY_H

/** @file AppVerify.h
 *  @brief Macros which perform verification with logging.
 *
 *  These macros allow for concise verification of boolean expressions. When an expression
 *  is false, they log an error to the message log then return, with an optional return value,
 *  or throw an exception.
 */

/** @def LogVerificationError(pExpression,pMsg)
 *  @brief Add a log message stating that \a pExpression failed and add \a pMsg if not NULL.
 */
#define LogVerificationError(pExpression,pMsg) LogVerificationErrorProc(pExpression, __FILE__, __LINE__, pMsg)

void LogVerificationErrorProc(const char *pExpression, const char *pFilename, unsigned int line, const char *pMsg);

/** @def VERIFY_ACTION(expr,action,msg)
 *  @brief If \a expr is false, log \a msg and perform the specified action.
 */
#define VERIFY_ACTION(expr,action,msg) \
if (!(expr)) \
{\
   LogVerificationError(#expr,msg);\
   action;\
}

/** @def VERIFY_VALUE(expr,value1,value2,msg)
 *  @brief If \a expr is false, log \a msg and evaluate to value1, else evaluate to value2.
 */
#define VERIFY_VALUE(expr,value1,value2,msg) \
( \
   !(expr) ? \
      LogVerificationError(#expr,msg), \
      value1 \
   : \
   value2 \
)

/** @def VERIFYRV_MSG(expr,rv,msg)
 *  @brief If \a expr is false, log \a msg and return \a rv from the current function.
 */
#define VERIFYRV_MSG(expr,rv,msg) VERIFY_ACTION(expr,return rv,msg)

/** @def VERIFYNRV_MSG(expr,msg)
 *  @brief If \a expr is false, log \a msg and return from the current function.
 */
#define VERIFYNRV_MSG(expr,msg) VERIFY_ACTION(expr,return,msg)

/** @def VERIFYNR_MSG(expr,msg)
 *  @brief If \a expr is false, log \a msg and evaluate to false, else evaluate to true.
 */
#define VERIFYNR_MSG(expr,msg) VERIFY_VALUE(expr,false,true,msg)

/** @def VERIFYNR(expr)
 *  @brief If \a expr is false, log a message and evaluate to false, else evaluate to true.
 */
#define VERIFYNR(expr) VERIFYNR_MSG(expr,NULL)

/** @def NN(ptr)
 *  @brief If \a ptr is NULL, log a message and evaluate to false, else evaluate to true.
 */
#define NN(ptr) VERIFYNR(ptr!=NULL)

/** @def VERIFY_MSG(expr,msg)
 *  @brief If \a expr is false, log \a msg and return false from the current function.
 */
#define VERIFY_MSG(expr,msg) VERIFYRV_MSG(expr,false,msg)

/** @def VERIFYRV(expr,rv)
 *  @brief If \a expr is false, log a message and return \a rv from the current function.
 */
#define VERIFYRV(expr,rv) VERIFYRV_MSG(expr,rv,NULL)

/** @def VERIFY(expr)
 *  @brief If \a expr is false, log a message and return false from the current function.
 */
#define VERIFY(expr) VERIFYRV(expr,false)

/** @def VERIFYNRV(expr)
 *  @brief If \a expr is false, log a message and return from the current function.
 */
#define VERIFYNRV(expr) VERIFYNRV_MSG(expr,NULL)

/** @def LOG_IF(expr,action)
 *  @brief If \a expr is true, log \a msg and perform the specified action.
 */
#define LOG_IF(expr,action) VERIFY_ACTION(!(expr),action,NULL)

/** @def LOG_IF_ELSE(expr,value1,value2)
 *  @brief If \a expr is true, log \a msg and evaluate to value1, else evaluate to value2.
 */
#define LOG_IF_ELSE(expr,value1,value2) VERIFY_VALUE(!(expr),value1,value2,NULL)

/** @def DO_IF(expr,action)
 *  @brief If \a expr is true, perform the specified action. This is like LOG_IF without logging.
 */
#define DO_IF(expr,action) \
if(expr) \
{ \
   action; \
}

#ifdef NDEBUG
#define VERIFY_DEBUG(s) s
#define VERIFYRV_DEBUG(s,rv) s
#define VERIFYNRV_DEBUG(s) s
#else
#define VERIFY_DEBUG(s) VERIFY(s)
#define VERIFYRV_DEBUG(s,rv) VERIFYRV(s,rv)
#define VERIFYNRV_DEBUG(s) VERIFYNRV(s)
#endif

#endif
