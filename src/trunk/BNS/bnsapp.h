
#ifndef BNSAPP_H
#define BNSAPP_H

#include <QApplication>
#include <QDir>

class bnsApp : public QApplication {
  Q_OBJECT
  public:
    bnsApp(int& argc, char* argv[], bool GUIenabled);
    virtual ~bnsApp();  
    void setConfFileName(const QString& confFileName);
    QString confFileName() const {return _confFileName;}

  private:
    QString _confFileName;
};
#endif
