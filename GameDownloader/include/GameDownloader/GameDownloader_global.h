/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#ifndef DOWNLOADSERVICE_GLOBAL_H
#define DOWNLOADSERVICE_GLOBAL_H

#include <QtCore/qglobal.h>

// Необходимо для отладки утечек памяти.
#ifdef VLD_CHECK_ENABLED
#include <vld.h>
#pragma comment(lib, "vld.lib")
#endif

#ifdef DOWNLOADSERVICE_LIB
# define DOWNLOADSERVICE_EXPORT Q_DECL_EXPORT
#else
# define DOWNLOADSERVICE_EXPORT Q_DECL_IMPORT
#endif

#endif // DOWNLOADSERVICE_GLOBAL_H
