#pragma once
#include <QtCore/QObject>
#include <functional>

// Ѕуть аккуратен при использовании этого макроса. ”дал€ющиес€ локальные указатели могут быть большой проблемой
// и их лучше замыкать по значени, т.е. использу€ [=].
#define SINGLESHOTFUNCTOR(x) std::function<void ()> x = [&]() mutable -> void

class SingleShotFunctor : public QObject
{
  Q_OBJECT
public:
  SingleShotFunctor(int ms, std::function<void ()> f, QObject *parent = 0);
  SingleShotFunctor(QObject *parent = 0);
  ~SingleShotFunctor();

  void setCallback(std::function<void ()> f);
  void start(int msec);
private slots:
  void internalFunctorCaller();

private:
  std::function<void ()> _functor;
};
