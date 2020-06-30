#ifndef BNCSINEXTRO_H
#define BNCSINEXTRO_H


#include <fstream>
#include <newmat.h>
#include <QtCore>

#include "bncoutf.h"
#include "bncversion.h"
#include "pppOptions.h"
#include "bncsettings.h"
#include "bncantex.h"

using namespace BNC_PPP;

class bncSinexTro : public bncoutf {
 public:
  bncSinexTro(const t_pppOptions* opt,
              const QString& sklFileName, const QString& intr,
              int sampl);
  virtual ~bncSinexTro();
  virtual t_irc write(QByteArray staID, int GPSWeek, double GPSWeeks,
                      double trotot, double stdev);

 private:
  virtual void writeHeader(const QDateTime& datTim);
  virtual void closeFile();
  QString _roverName;
  int _sampl;
  const t_pppOptions*  _opt;
  bncAntex* _antex;
  double _antPCO[t_frequency::max];
};


#endif
