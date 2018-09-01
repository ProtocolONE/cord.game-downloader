#pragma once

#include <QtCore/qglobal.h>
#include <QtCore/QDebug>

#ifdef DOWNLOADSERVICE_STATIC_LIB
  # define DOWNLOADSERVICE_EXPORT 
#else

#ifdef DOWNLOADSERVICE_LIB
# define DOWNLOADSERVICE_EXPORT Q_DECL_EXPORT
#else
# define DOWNLOADSERVICE_EXPORT Q_DECL_IMPORT
#endif

#endif

#define CRITICAL_LOG qCritical() << __FILE__ << __LINE__ << __FUNCTION__
#define WARNING_LOG qWarning() << __FILE__ << __LINE__ << __FUNCTION__
#define DEBUG_LOG qDebug() << __FILE__ << __LINE__ << __FUNCTION__

#define SIGNAL_CONNECT_CHECK(X) { bool result = X; Q_ASSERT_X(result, __FUNCTION__ , #X); }
