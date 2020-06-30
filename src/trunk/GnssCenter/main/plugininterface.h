#ifndef GnssCenter_PLUGININTERFACE_H
#define GnssCenter_PLUGININTERFACE_H

#include <QtGui>
#include <QtPlugin>

namespace GnssCenter {

class t_pluginFactoryInterface {
 public:
  virtual QWidget* create() = 0;
  virtual QString getName() const = 0;
};

} // namespace GnssCenter

Q_DECLARE_INTERFACE(GnssCenter::t_pluginFactoryInterface, "GnssCenter_pluginFactoryInterface")

#endif
