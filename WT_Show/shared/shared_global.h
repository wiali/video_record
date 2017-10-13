/*! \file shared_global.h
 *  \brief This file has the definitions to export classes and functions to the dll
 *  \date February, 2017
 *  \copyright 2017 HP Development Company, L.P.
 */
#ifndef SHARED_GLOBAL_H
#define SHARED_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SHARED_LIBRARY)
#  define SHAREDSHARED_EXPORT Q_DECL_EXPORT
#elif defined(USE_SHARED_AS_DLL)
#  define SHAREDSHARED_EXPORT Q_DECL_IMPORT
#else
#  define SHAREDSHARED_EXPORT
#endif

#endif // SHARED_GLOBAL_H
