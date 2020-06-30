#include <iostream>
#include <iomanip>
#include <algorithm>

#include "RTCM2_2021.h"

using namespace rtcm2;
using namespace std;

const double ZEROVALUE = 1e-100;


void RTCM2_2021::extract(const RTCM2packet& P) {
  if ( !P.valid() || (P.ID() != 20 && P.ID() != 21) ) {
    return;
  }

  // Error: at least 4 data words
  if ( P.nDataWords()<5 ) {
#if ( DEBUG > 0 )
    cerr << "Error in RTCM2_Obs::extract(): less than 3 DW ("
         << P.nDataWords() << ") detected" << endl;
#endif
    return;
  };

  // Error: number of data words has to be odd number
  if ( P.nDataWords()%2==0 ){
#if ( DEBUG > 0 )
    cerr << "Error in RTCM2_Obs::extract(): odd number of DW ("
         << P.nDataWords() << ") detected" << endl;
#endif
    return;
  };

  // Current epoch (mod 3600 sec)
  double tt = 0.6*P.modZCount()
            + P.getUnsignedBits(4,20)*1.0e-6;

  // Clear old epoch
  if ( tt != tt_ || valid_ ) {
    clear();
    tt_    = tt;
    valid_ = false;
  }


  // Frequency (exit if neither L1 nor L2)
  bool isL1 = ( P.getUnsignedBits(0,1)==0 );
  if ( P.getUnsignedBits(1,1)==1 ) {
    return;
  }

  // Number of satellites
  unsigned nSat = (P.nDataWords() - 1) / 2;

  double multipleMsgInd = true;
  for (unsigned iSat = 0; iSat < nSat; iSat++) {
    bool     multInd   =   P.getBits        (iSat*48 + 24, 1);
    bool     isGPS     = ( P.getUnsignedBits(iSat*48 + 26, 1)==0 );
    unsigned PRN       =   P.getUnsignedBits(iSat*48 + 27, 5);

    multipleMsgInd = multipleMsgInd && multInd;

    if ( !isGPS ) {
      PRN += 200;
    }
    if ( PRN == 0 ) {
      PRN = 32;
    }

    HiResCorr* corr = 0;
    if ( !(corr = find_i(PRN)) ) {
      data_i_[PRN] = HiResCorr();
      corr = &(data_i_[PRN]);
    }
    if ( !find(PRN) ) {
      data[PRN] = corr;
    }

    corr->PRN = PRN;
    corr->tt  = tt_;

    // Message number 20
    if ( P.ID() == 20 ) {
      unsigned lossLock  =   P.getUnsignedBits(iSat*48 + 35,  5);
      unsigned IOD       =   P.getUnsignedBits(iSat*48 + 40,  8);
      double   corrVal   =   P.getBits        (iSat*48 + 48, 24) / 256.0;

      if ( isL1 ) {
	corr->phase1 = (corrVal ? corrVal : ZEROVALUE);
	corr->slip1  = (corr->lock1 != lossLock);
	corr->lock1  = lossLock;
	corr->IODp1  = IOD;
      }
      else {
	corr->phase2 = (corrVal ? corrVal : ZEROVALUE);
	corr->slip2  = (corr->lock2 != lossLock);
	corr->lock2  = lossLock;
	corr->IODp2  = IOD;
      }
    }

    // Message number 21
    else if ( P.ID() == 21 ) {
      bool   P_CA_Ind  =   P.getBits        (iSat*48 + 25, 1);
      double dcorrUnit = ( P.getUnsignedBits(iSat*48 + 32, 1) ? 0.032 : 0.002);
      double  corrUnit = ( P.getUnsignedBits(iSat*48 + 36, 1) ? 0.320 : 0.020);
      unsigned    IOD  =   P.getUnsignedBits(iSat*48 + 40, 8);
      double  corrVal  =   P.getBits        (iSat*48 + 48, 16) *  corrUnit;
      double dcorrVal  =   P.getBits        (iSat*48 + 64,  8) * dcorrUnit;

      if ( isL1 ) {
	corr-> range1 = (corrVal ? corrVal : ZEROVALUE);
	corr->drange1 = dcorrVal;
	corr->IODr1   = IOD;
        corr->Pind1   = P_CA_Ind;
      }
      else {
	corr-> range2 = (corrVal ? corrVal : ZEROVALUE);
	corr->drange2 = dcorrVal;
	corr->IODr2   = IOD;
        corr->Pind2   = P_CA_Ind;
      }
    }
  }

  valid_ = !multipleMsgInd;
}

const RTCM2_2021::HiResCorr* RTCM2_2021::find(unsigned PRN) {
  std::map<unsigned, const HiResCorr*>::const_iterator ii = data.find(PRN);
  return (ii != data.end() ? ii->second : 0);
}


RTCM2_2021::HiResCorr* RTCM2_2021::find_i(unsigned PRN) {
  std::map<unsigned, HiResCorr>::iterator ii = data_i_.find(PRN);
  return (ii != data_i_.end() ? &(ii->second) : 0);
}


void RTCM2_2021::clear() {
  tt_    = 0;
  valid_ = false;
  for (map<unsigned, HiResCorr>::iterator
	 ii = data_i_.begin(); ii != data_i_.end(); ii++) {
    ii->second.reset();
  }
  data.clear();
}


RTCM2_2021::HiResCorr::HiResCorr() :
  PRN(0), tt(0),
  phase1 (0),     phase2 (2),
  lock1  (0),     lock2  (0),
  slip1  (false), slip2  (false),
  IODp1  (0),     IODp2  (0),
  range1 (0),     range2 (0),
  drange1(0),     drange2(0),
  Pind1  (false), Pind2  (false),
  IODr1  (0),     IODr2  (0) {
}

void RTCM2_2021::HiResCorr::reset() {
  // does not reset 'lock' indicators and PRN
  tt      = 0;
  phase1  = 0;
  phase2  = 0;
  slip1   = false;
  slip2   = false;
  IODp1   = 0;
  IODp2   = 0;

  range1  = 0;
  range2  = 0;
  drange1 = 0;
  drange2 = 0;
  IODr1   = 0;
  IODr2   = 0;
  Pind1   = false;
  Pind2   = false;
}

std::ostream& operator << (std::ostream& out, const RTCM2_2021::HiResCorr& cc) {
  out.setf(ios::fixed);
  out << setw(8) << setprecision(8) << cc.tt
      << ' ' << setw(2)  << cc.PRN
      << " L1 "
      << ' ' << setw(8)  << setprecision(3) << (cc.phase1 ? cc.phase1 : 9999.999)
      << ' ' << setw(1)  		    << (cc.phase1 ? (cc.slip1 ? '1' : '0') : '.')
      << ' ' << setw(2)                     << (cc.phase1 ? cc.lock1  : 99)
      << ' ' << setw(3)                     << (cc.phase1 ? cc.IODp1  : 999)
      << " L2 "
      << ' ' << setw(8)  << setprecision(3) << (cc.phase2 ? cc.phase2 : 9999.999)
      << ' ' << setw(1)  		    << (cc.phase2 ? (cc.slip2 ? '1' : '0') : '.')
      << ' ' << setw(2)                     << (cc.phase2 ? cc.lock2  : 99)
      << ' ' << setw(3)                     << (cc.phase2 ? cc.IODp2  : 999)
      << " P1 "
      << ' ' << setw(8)  << setprecision(3) << (cc.range1 ? cc.range1 : 9999.999)
      << ' ' << setw(3)                     << (cc.range1 ? cc.IODr1  : 999)
      << " P2 "
      << ' ' << setw(8)  << setprecision(3) << (cc.range2 ? cc.range2 : 9999.999)
      << ' ' << setw(3)                     << (cc.phase2 ? cc.IODr2  : 999);

  return out;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////


void RTCM2_22::extract(const RTCM2packet& P) {
  if ( P.ID() != 22 ) {
    return;
  }

  const double dL1unit = 0.01 / 256;

  validMsg = true;

  dL1[0] = P.getBits( 0, 8) * dL1unit;
  dL1[1] = P.getBits( 8, 8) * dL1unit;
  dL1[2] = P.getBits(16, 8) * dL1unit;

  dL2[0] = 0.0;
  dL2[1] = 0.0;
  dL2[2] = 0.0;
}

///////////////////////////////

