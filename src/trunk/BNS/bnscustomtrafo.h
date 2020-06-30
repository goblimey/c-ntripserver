#ifndef BNSCUSTOMTRAFO_H
#define BNSCUSTOMTRAFO_H

#include <QtCore>
#include <QtGui>
#include <QWhatsThis>

class bnsCustomTrafo : public QDialog {
  Q_OBJECT

  public:
    bnsCustomTrafo(QWidget* parent);
    ~bnsCustomTrafo();

  signals:
    void newMountPoints(QStringList* mountPoints);

  private slots:
    virtual void accept();
    void slotWhatsThis();

  private:
    QLineEdit*   _dxLineEdit;
    QLineEdit*   _dyLineEdit;
    QLineEdit*   _dzLineEdit;
    QLineEdit*   _dxrLineEdit;
    QLineEdit*   _dyrLineEdit;
    QLineEdit*   _dzrLineEdit;
    QLineEdit*   _oxLineEdit;
    QLineEdit*   _oyLineEdit;
    QLineEdit*   _ozLineEdit;
    QLineEdit*   _oxrLineEdit;
    QLineEdit*   _oyrLineEdit;
    QLineEdit*   _ozrLineEdit;
    QLineEdit*   _scLineEdit;
    QLineEdit*   _scrLineEdit;
    QLineEdit*   _t0LineEdit;

    QPushButton* _buttonGet;
    QPushButton* _buttonCancel;
    QPushButton* _buttonOK;
    QPushButton* _buttonWhatsThis;
};

#endif
