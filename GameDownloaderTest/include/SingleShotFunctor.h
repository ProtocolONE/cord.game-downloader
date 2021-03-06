#pragma once
#include <QtCore/QObject>
#include <functional>

// ���� ��������� ��� ������������� ����� �������. ����������� ��������� ��������� ����� ���� ������� ���������
// � �� ����� �������� �� �������, �.�. ��������� [=].
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
