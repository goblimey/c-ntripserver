#ifndef GnssCenter_APP_H
#define GnssCenter_APP_H

#include <QtGui>

namespace GnssCenter {

class t_app : public QApplication {
  Q_OBJECT

  friend class t_settings;

 public:
  t_app(int& argc, char* argv[], bool GUIenabled);
  virtual ~t_app();  

  void setConfFileName(const QString& confFileName);
  const QString& confFileName() const {return _confFileName;}

 public slots:
  void slotMessage(QByteArray msg);

 private:
  QString                _confFileName;
  QSettings::SettingsMap _settings;
};

} // namespace GnssCenter

#endif

