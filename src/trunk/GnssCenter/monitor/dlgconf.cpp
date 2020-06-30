
/* -------------------------------------------------------------------------
 * RTNet DlgConf
 * -------------------------------------------------------------------------
 *
 * Class:      t_dlgConf
 *
 * Purpose:    Set configuration
 *
 * Author:     L. Mervart
 *
 * Created:    15-Sep-2013
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include "dlgconf.h"
#include "settings.h"
#include "const.h"

using namespace std;
using namespace GnssCenter;

// Constructor
/////////////////////////////////////////////////////////////////////////////
t_dlgConf::t_dlgConf(QWidget* parent) : QDialog(parent) {

  t_settings settings(pluginName);

  QString host = settings.value("host").toString();
  if (host.isEmpty()) {
    host = "localhost";
  }
  _hostLineEdit = new QLineEdit(host, this);
  _portLineEdit = new QLineEdit(settings.value("port").toString(), this);


  QFormLayout* formLayout = new QFormLayout;
  formLayout->addRow("Host:", _hostLineEdit);
  formLayout->addRow("Port:", _portLineEdit);

  QPushButton* cancelButton = new QPushButton("Cancel", this);
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  QPushButton* okButton = new QPushButton("OK", this);
  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(cancelButton);
  buttonLayout->addWidget(okButton);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addLayout(formLayout);
  mainLayout->addLayout(buttonLayout);
  setLayout(mainLayout);
}

// Destructor
/////////////////////////////////////////////////////////////////////////////
t_dlgConf::~t_dlgConf() {
}

// Accept (virtual slot)
/////////////////////////////////////////////////////////////////////////////
void t_dlgConf::accept() {
  t_settings settings(pluginName);
  QString host = _hostLineEdit->text();
  if (host.isEmpty()) {
    host = "localhost";
  }
  settings.setValue("host", host);
  settings.setValue("port", _portLineEdit->text());
  QDialog::accept();
}

