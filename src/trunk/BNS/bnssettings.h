
#ifndef BNSSETTINGS_H
#define BNSSETTINGS_H

#include <QSettings>

class bnsSettings : public QSettings {
  public:
    bnsSettings();
    virtual ~bnsSettings() {};
  private:
};
#endif

