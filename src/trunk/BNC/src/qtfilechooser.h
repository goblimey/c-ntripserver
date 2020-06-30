
#ifndef QTFILECHOOSER
#define QTFILECHOOSER

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class qtFileChooser : public QWidget {
  Q_OBJECT

  Q_ENUMS( Mode )
  Q_PROPERTY( Mode mode READ mode WRITE setMode )
  Q_PROPERTY( QString fileName READ fileName WRITE setFileName )

 public:
  enum Mode {File, Files, Directory};

  qtFileChooser(QWidget* parent = 0, qtFileChooser::Mode mode = File);
  ~qtFileChooser();

  QString fileName() const;
  Mode mode() const {return _mode;}

  public slots:
   void setFileName(const QString& fileName);
   void setMode(Mode mode) {_mode = mode;}

  signals:
   void fileNameChanged(const QString&);

  private slots:
   void chooseFile();

  private:
   QLineEdit*   _lineEdit;
   QPushButton* _button;
   Mode         _mode;

};
#endif
