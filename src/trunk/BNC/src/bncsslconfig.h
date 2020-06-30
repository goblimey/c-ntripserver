#ifndef BNCSSLCONFIG_H
#define BNCSSLCONFIG_H

#include <QSslConfiguration>

// Singleton Class
// ---------------
class bncSslConfig : public QSslConfiguration {
 public:
  bncSslConfig();
  ~bncSslConfig();
  static QString defaultPath();
 private:
};

#endif
