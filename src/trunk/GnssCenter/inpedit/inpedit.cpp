
/* -------------------------------------------------------------------------
 * RTNet GUI
 * -------------------------------------------------------------------------
 *
 * Class:      t_inpEdit
 *
 * Purpose:    RTNet Input File
 *
 * Author:     L. Mervart
 *
 * Created:    05-Jan-2013
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "inpedit.h" 
#include "keyword.h" 
#include "panel.h" 

using namespace std;
using namespace GnssCenter;

Q_EXPORT_PLUGIN2(gnsscenter_inpedit, GnssCenter::t_inpEditFactory)

// Constructor
////////////////////////////////////////////////////////////////////////////
t_inpEdit::t_inpEdit() : QMainWindow() {

  _tabWidget = new t_tabWidget();
  setCentralWidget(_tabWidget);
 
  QMenu*   menuFile = menuBar()->addMenu(tr("&File"));

  QAction* actOpenFile = new QAction(tr("&Open"), this);
  connect(actOpenFile, SIGNAL(triggered()), this, SLOT(slotOpenFile()));
  menuFile->addAction(actOpenFile);

  QAction* actSaveFile = new QAction(tr("&Save"), this);
  connect(actSaveFile, SIGNAL(triggered()), this, SLOT(slotSaveFile()));
  menuFile->addAction(actSaveFile);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
t_inpEdit::~t_inpEdit() {
}

//
////////////////////////////////////////////////////////////////////////////
void t_inpEdit::slotOpenFile() {
  QString fileName = QFileDialog::getOpenFileName(this);
  if (!fileName.isEmpty()) {
    _tabWidget->readInputFile(fileName);
  }
}

//
////////////////////////////////////////////////////////////////////////////
void t_inpEdit::slotSaveFile() {
  QString fileName = QFileDialog::getSaveFileName(this);
  if (!fileName.isEmpty()) {
    _tabWidget->writeInputFile(fileName);
  }
}
