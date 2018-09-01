#pragma once

#include <QtCore/QObject>

#include <QtCore/QHash>

class SignalCounter : public QObject
{
  Q_OBJECT
public:
  explicit SignalCounter(QObject *parent = 0);
  virtual ~SignalCounter();

  int eventCount() const;

public slots:
  void eventSlot();

private:
  int _eventCount;
};
