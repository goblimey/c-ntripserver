/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      bncSslConfig
 *
 * Purpose:    Singleton Class that inherits QSslConfiguration class
 *
 * Author:     L. Mervart
 *
 * Created:    22-Aug-2011
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include <iostream>

#include <QApplication>
#include <QDir>
#include <QStringList>

#include "bncsslconfig.h"
#include "bncsettings.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bncSslConfig::bncSslConfig() : 
  QSslConfiguration(QSslConfiguration::defaultConfiguration()) 
{
  
  bncSettings settings;
  QString dirName = settings.value("sslCaCertPath").toString();
  if (dirName.isEmpty()) {
    dirName =  defaultPath();
  }

  QList<QSslCertificate> caCerts = this->caCertificates();

  // Bug in Qt: the wildcard does not work here:
  // -------------------------------------------
  // caCerts += QSslCertificate::fromPath(dirName + QDir::separator() + "*crt",
  //                                      QSsl::Pem, QRegExp::Wildcard);
  QDir dir(dirName);
  QStringList nameFilters; nameFilters << "*.crt";
  QStringList fileNames = dir.entryList(nameFilters, QDir::Files);
  QStringListIterator it(fileNames);
  while (it.hasNext()) {
    QString fileName = it.next();
    caCerts += QSslCertificate::fromPath(dirName+QDir::separator()+fileName);
  }
 
  this->setCaCertificates(caCerts);
}

// Destructor
////////////////////////////////////////////////////////////////////////////
bncSslConfig::~bncSslConfig() {
}

// Destructor
////////////////////////////////////////////////////////////////////////////
QString bncSslConfig::defaultPath() {
  return QDir::homePath() + QDir::separator() 
         + ".config" + QDir::separator() + qApp->organizationName();
}

