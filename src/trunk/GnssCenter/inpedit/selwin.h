#ifndef GnssCenter_SELWIN_H
#define GnssCenter_SELWIN_H

#include <QtGui>

namespace GnssCenter {

class t_selWin : public QWidget {
 Q_OBJECT

 public:
  enum Mode {File, Files, Directory};

  t_selWin(t_selWin::Mode mode, QWidget* parent = 0);
  ~t_selWin();

  const QStringList& fileNames() const;
  Mode        mode() const {return _mode;}
  void setFileNames(const QStringList& fileNames);

  signals:
   void fileNamesChanged();

  private slots:
   void slotTextEdited();
   void slotChooseFile();

  private:
   void setLineEditText();
   QLineEdit*   _lineEdit;
   QPushButton* _button;
   Mode         _mode;
   QStringList  _fileNames;
};

} // namespace GnssCenter

#endif
