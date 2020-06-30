/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnsHlpDlg
 *
 * Purpose:    Displays the help
 *
 * Author:     L. Mervart
 *
 * Created:    30-Mar-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "bnshlpdlg.h"
#include "bnshtml.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bnsHlpDlg::bnsHlpDlg(QWidget* parent, const QUrl& url) :
                    QDialog(parent) {

  const int ww = QFontMetrics(font()).width('w');

  bnsHtml* _tb = new bnsHtml;
  setWindowTitle("Help Contents");
  _tb->setSource(url);
  _tb->setReadOnly(true);
  connect(_tb, SIGNAL(backwardAvailable(bool)),
          this, SLOT(backwardAvailable(bool)));
  connect(_tb, SIGNAL(forwardAvailable(bool)),
          this, SLOT(forwardAvailable(bool)));

  QVBoxLayout* dlgLayout = new QVBoxLayout;
  dlgLayout->addWidget(_tb);

  QHBoxLayout* butLayout = new QHBoxLayout;

  _backButton = new QPushButton("Backward");
  _backButton->setMaximumWidth(10*ww);
  _backButton->setEnabled(false);
  connect(_backButton, SIGNAL(clicked()), _tb, SLOT(backward()));
  butLayout->addWidget(_backButton);

  _forwButton = new QPushButton("Forward");
  _forwButton->setMaximumWidth(10*ww);
  _forwButton->setEnabled(false);
  connect(_forwButton, SIGNAL(clicked()), _tb, SLOT(forward()));
  butLayout->addWidget(_forwButton);

  _closeButton = new QPushButton("Close");
  _closeButton->setMaximumWidth(10*ww);
  butLayout->addWidget(_closeButton);
  connect(_closeButton, SIGNAL(clicked()), this, SLOT(close()));

  dlgLayout->addLayout(butLayout);

  setLayout(dlgLayout);
  resize(60*ww, 60*ww);
  show();
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bnsHlpDlg::~bnsHlpDlg() {
  delete _tb;
  delete _backButton;
  delete _forwButton;
  delete _closeButton;
}

// Slots
////////////////////////////////////////////////////////////////////////////
void bnsHlpDlg::backwardAvailable(bool avail) {
  _backButton->setEnabled(avail);
}

void bnsHlpDlg::forwardAvailable(bool avail) {
  _forwButton->setEnabled(avail);
}
