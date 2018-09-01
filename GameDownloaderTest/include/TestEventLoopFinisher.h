#pragma once

#include <QtCore/QObject>
#include <QtCore/QEventLoop>
class TestEventLoopFinisher : public QObject
{
  Q_OBJECT
public:
  TestEventLoopFinisher(QEventLoop *loop, int timeout);
  virtual ~TestEventLoopFinisher();
  
  bool isKilledByTimeout() const;
  bool setTerminateSignal(QObject *sender, const char* signal);

public slots:
  void terminateLoop();
  
private slots:
  void timeoutTick();

private:
  bool _timeoutKill;
  QEventLoop *_loop;
};
