#ifndef GnssCenter_TABWIDGET_H
#define GnssCenter_TABWIDGET_H

#include <QtGui>

namespace GnssCenter {

class t_keyword;
class t_panel;

class t_tabWidget : public QTabWidget {
 public:
  t_tabWidget();
  ~t_tabWidget();
  void readInputFile(const QString&);
  void writeInputFile(const QString&);
 private:
  QString                   _fileName;
  QMap<QString, t_keyword*> _keywords;
  QStringList               _staticLines;
};

} // namespace GnssCenter

#endif

