/* -------------------------------------------------------------------------
 * BKG NTRIP Client
 * -------------------------------------------------------------------------
 *
 * Class:      t_pppUtils
 *
 * Purpose:    Auxiliary Functions for PPP
 *
 * Author:     A. Stürze
 *
 * Created:    18-Aug-2015
 *
 * Changes:
 *
 * -----------------------------------------------------------------------*/

#include "pppUtils.h"
#include "bncutils.h"
#include "pppModel.h"

using namespace BNC_PPP;


// Constructor
//////////////////////////////////////////////////////////////////////////////
t_pppUtils::t_pppUtils() {
  for (unsigned ii = 0; ii <= t_prn::MAXPRN; ii++) {
    _satCodeBiases[ii] = 0;
  }
}

// Destructor
//////////////////////////////////////////////////////////////////////////////
t_pppUtils::~t_pppUtils() {
  for (unsigned ii = 0; ii <= t_prn::MAXPRN; ii++) {
    delete _satCodeBiases[ii];
  }
}

//
//////////////////////////////////////////////////////////////////////////////
void t_pppUtils::putCodeBias(t_satCodeBias* satCodeBias) {
  int iPrn = satCodeBias->_prn.toInt();
  delete _satCodeBiases[iPrn];
  _satCodeBiases[iPrn] = satCodeBias;
}

