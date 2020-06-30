#include <iostream>
#include <map>
#include "RTCM2.h"

namespace rtcm2 {

class RTCM2_2021 {

  public:
    RTCM2_2021() {// Constructor
      tt_    = 0.0;
      valid_ = false;
    }

    void   extract(const RTCM2packet& P);  // Packet handler
    void   clear();                        // Initialization
    bool   valid() const { return valid_; }                  // Check for complete obs block

    double resolvedPhase_L1(int i) const;  // L1 & L2 carrier phase of i-th sat
    double resolvedPhase_L2(int i) const;  // with resolved 2^24 cy ambiguity
                                           // (based on rng_C1)

    struct HiResCorr {

      HiResCorr();
      void reset();

      unsigned PRN;
      double   tt;
      double   phase1;
      double   phase2;
      unsigned lock1;
      unsigned lock2;
      bool     slip1;
      bool     slip2;
      unsigned IODp1;
      unsigned IODp2;

      double   range1;
      double   range2;
      double   drange1;
      double   drange2;
      bool     Pind1;
      bool     Pind2;
      unsigned IODr1;
      unsigned IODr2;

      friend std::ostream& operator << (std::ostream& out, const HiResCorr& cc);
    };

    double hoursec() const { return tt_; }
    std::map<unsigned, const HiResCorr*> data;

    typedef std::map<unsigned, const HiResCorr*>::const_iterator c_data_iterator;
    typedef std::map<unsigned, const HiResCorr*>::iterator         data_iterator;

 private:
    const HiResCorr* find  (unsigned PRN);
          HiResCorr* find_i(unsigned PRN);

    std::map<unsigned, HiResCorr> data_i_;
    double                        tt_;
    bool                          valid_;
};

class RTCM2_22 {
 public:
  RTCM2_22() {
    validMsg = false;
    for (unsigned ii = 0; ii < 3; ii++){
      dL1[ii] = 0.0;
      dL2[ii] = 0.0;
    }
  }

  void extract(const RTCM2packet& P);

  bool   validMsg;
  double dL1[3];
  double dL2[3];
};

}; // end of namespace rtcm2


