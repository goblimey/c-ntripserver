
/* -------------------------------------------------------------------------
 * BKG NTRIP Server
 * -------------------------------------------------------------------------
 *
 * Class:      bnsHtml
 *
 * Purpose:    HTML Browser
 *
 * Author:     L. Mervart
 *
 * Created:    30-Mar-2008
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "bnshtml.h" 

using namespace std;

// Constructor
////////////////////////////////////////////////////////////////////////////
bnsHtml::bnsHtml() : QTextBrowser() {

  connect(this,SIGNAL(anchorClicked(const QUrl&)),
          this,SLOT(slotAnchorClicked(const QUrl&)));
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bnsHtml::~bnsHtml() {

}

// 
////////////////////////////////////////////////////////////////////////////
void bnsHtml::slotAnchorClicked(const QUrl& url) {

 
  QString href = url.toString();
  if (href.indexOf(':') != 0) {
    QUrl urlNew; urlNew.setPath(":bnshelp.html" + href);
    setSource(url);
  }
}
