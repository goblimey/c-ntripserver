#ifndef GnssCenter_APP_H
#define GnssCenter_APP_H

#include <QApplication>
#include <QFileOpenEvent>
#include <QtGui>


QCoreApplication* createApplication(int& argc, char* argv[], bool GUIenabled);

class t_app : public QApplication {
 Q_OBJECT
 public:
  t_app(int& argc, char* argv[]);
  virtual ~t_app();
 protected:
  virtual bool event(QEvent* ev);
 private:
};


#endif
