
#ifndef BNSHTML_H
#define BNSHTML_H

#include <QTextBrowser>

class bnsHtml : public QTextBrowser {
  Q_OBJECT

  public:
    bnsHtml();
    ~bnsHtml();

  public slots:
    void slotAnchorClicked(const QUrl& url);
};
#endif
