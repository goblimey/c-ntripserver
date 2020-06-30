#ifndef GnssCenter_KEYWORD_H
#define GnssCenter_KEYWORD_H

#include <QtGui>

namespace GnssCenter {

class t_keyword {
 public:
  t_keyword(QString line, QTextStream& inStream, QStringList& staticLines);
  ~t_keyword();

  const QString& name() const {return _name;}
  bool ok() const {return _ok;}
  QWidget* createWidget(const QString& fldMask);
  QStringList values() const;

 private:
  bool                   _ok;
  QString                _name;
  QStringList            _origValues;
  QWidget*               _widget;
  QMap<QString, QString> _desc;
};

} // namespace GnssCenter

#endif
