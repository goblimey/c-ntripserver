#ifndef PPPTHREAD_H
#define PPPTHREAD_H

#include <deque>
#include <vector>
#include <QtCore>

#include "pppOptions.h"
#include "pppClient.h"
#include "pppRun.h"

namespace BNC_PPP {

class t_pppThread : public QThread {
 Q_OBJECT
 public:
  t_pppThread(const t_pppOptions* opt);
  ~t_pppThread();
  virtual void run();
  static void msleep(unsigned long msecs){QThread::msleep(msecs);}

 signals:
  void newMessage(QByteArray msg, bool showOnScreen);

 private:
  const t_pppOptions* _opt;
  t_pppRun*           _pppRun;
};

}

#endif
