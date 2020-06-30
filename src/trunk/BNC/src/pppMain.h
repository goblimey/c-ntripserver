#ifndef PPPMAIN_H
#define PPPMAIN_H

#include <QtCore>
#include "pppOptions.h"
#include "pppThread.h"
#include "bnccore.h"

namespace BNC_PPP {

class t_pppMain {
 public:
  t_pppMain();
  ~t_pppMain();
  void start();
  void stop();

 private:
  void readOptions();

  QList<t_pppOptions*> _options;
  QList<t_pppThread*>  _pppThreads;
  bool     _running;
  bool     _realTime;
};

}; // namespace BNC_PPP

#endif
