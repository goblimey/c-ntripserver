#ifndef BNSHLPDLG_H
#define BNSHLPDLG_H

#include <QtCore>
#include <QtGui>

class bnsHtml;

class bnsHlpDlg : public QDialog {
  Q_OBJECT

  public:
    bnsHlpDlg(QWidget* parent, const QUrl& url);
    ~bnsHlpDlg();

  signals:
 
  public slots:
    void backwardAvailable(bool);
    void forwardAvailable(bool);

  private:
    bnsHtml*     _tb;
    QPushButton* _backButton;
    QPushButton* _forwButton;
    QPushButton* _closeButton;
};

#endif
