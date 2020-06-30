//------------------------------------------------------------------------------
//
// RTCM2.h
//
// Purpose:
//
//   Module for extraction of RTCM2 messages
//
// References:
//
//   RTCM 10402.3 Recommended Standards for Differential GNSS (Global
//     Navigation Satellite Systems) Service; RTCM Paper 136-2001/SC104-STD,
//     Version 2.3, 20 Aug. 2001; Radio Technical Commission For Maritime
//     Services, Alexandria, Virgina (2001).
//   ICD-GPS-200; Navstar GPS Space Segment / Navigation User Interfaces;
//     Revison C; 25 Sept. 1997; Arinc Research Corp., El Segundo (1997).
//   Jensen M.; RTCM2ASC Documentation;
//     URL http://kom.aau.dk/~borre/masters/receiver/rtcm2asc.htm;
//     last accessed 17 Sep. 2006
//   Sager J.; Decoder for RTCM SC-104 data from a DGPS beacon receiver;
//     URL http://www.wsrcc.com/wolfgang/ftp/rtcm-0.3.tar.gz;
//     last accessed 17 Sep. 2006
//
// Last modified:
//
//   2006/09/17  OMO  Created
//   2006/10/05  OMO  Specified const'ness of various member functions
//   2006/10/17  OMO  Removed obsolete check of multiple message indicator
//   2006/11/25  OMO  Revised check for presence of GLONASS data
//   2008/03/07  AHA  Removed unnecessary failure flag
//   2008/09/01  AHA  Harmonization with newest BNC version
//
// (c) DLR/GSOC
//
//------------------------------------------------------------------------------

#ifndef INC_RTCM2_H
#define INC_RTCM2_H

#include <bitset>
#include <fstream>
#include <string>
#include <vector>

//
// namespace rtcm2
//

namespace rtcm2 {


//------------------------------------------------------------------------------
//
// class thirtyBitWord (specification)
//
// Purpose:
//
//   Handling of RTCM2 30bit words
//
//------------------------------------------------------------------------------

class ThirtyBitWord {

  public:

    // Constructor and initialization

    ThirtyBitWord();

    void         clear();

    // Status queries

    bool         fail() const;
    bool         validParity() const;
    bool         isHeader() const;

    // Access methods

    unsigned int all() const;
    unsigned int value() const;

    // Input

    void         get(const std::string& buf);
    void         get(std::istream& inp);
    void         getHeader(std::string& buf);
    void         getHeader(std::istream& inp);

  private:

    // Input

    void         append(unsigned char c);

  private:

//    bool         failure;

    //
    // A 32-bit integer is used to store the 30-bit RTCM word as well as 2
    // parity bits retained from the previous word
    //
    // Bits 31..30 (from left to right) hold the parity bits D29*..D30* of
    //             the previous 30-bit word
    // Bits 29..06 (from left to right) hold the current data bits D01..D24
    // Bits 05..00 (from left to right) hold the current parity bits D25..D30
    //

    unsigned int W;

};



//------------------------------------------------------------------------------
//
// RTCM2packet (class definition)
//
// Purpose:
//
//   A class for handling RTCM2 data packets
//
//------------------------------------------------------------------------------

class RTCM2packet {

  public:

    // Constructor and initialization

    RTCM2packet();

    void clear();

    // Status queries

    bool valid() const;

    // Input

    void                 getPacket(std::string&  buf);
    void                 getPacket(std::istream& inp);
    friend std::istream& operator >> (std::istream& is, RTCM2packet& p);

    //
    // Access methods
    //

    // Header and data words contents (parity corrected)

    unsigned int  header1() const;
    unsigned int  header2() const;
    unsigned int  dataWord(int i) const;

    // Header information

    unsigned int  msgType()    const;
    unsigned int  ID()         const { return msgType(); };
    unsigned int  stationID()  const;
    unsigned int  modZCount()  const;
    unsigned int  seqNumber()  const;
    unsigned int  nDataWords() const;
    unsigned int  staHealth()  const;

    // Data access

    unsigned int  getUnsignedBits (unsigned int start,
                                   unsigned int n     ) const;
    int           getBits         (unsigned int start,
                                   unsigned int n     ) const;

  private:

    // All input of RTCM data uses a single instance, W, of a 30-bit word
    // to maintain parity bits between consecutive inputs.

    ThirtyBitWord  W;

    // Two 30-bit words make up the header of an RTCM2 message
    // (parity corrected)

    unsigned int  H1;
    unsigned int  H2;

    // Data words (parity corrected)

    std::vector<unsigned int> DW;

};


//------------------------------------------------------------------------------
//
// RTCM2_03 (class definition)
//
// Purpose:
//
//   A class for handling RTCM 2 GPS Reference Station Parameters messages
//
//------------------------------------------------------------------------------

class RTCM2_03 {

  public:
    // Constructor
    RTCM2_03() {
      validMsg = false;
      x = 0.0;
      y = 0.0;
      z = 0.0;
    }
    void extract(const RTCM2packet& P);
    bool    validMsg;          // Validity flag
    double  x,y,z;             // Station coordinates

};


//------------------------------------------------------------------------------
//
// RTCM2_23 (class definition)
//
// Purpose:
//
//   A class for handling RTCM 2 Antenna Type Definition messages
//
//------------------------------------------------------------------------------

class RTCM2_23 {

  public:
    RTCM2_23 () {
      validMsg = false;
    }
    void extract(const RTCM2packet& P);
    bool         validMsg;        // Validity flag
    std::string  antType;         // Antenna descriptor
    std::string  antSN  ;         // Antenna Serial Number

};


//------------------------------------------------------------------------------
//
// RTCM2_24 (class definition)
//
// Purpose:
//
//   A class for handling RTCM 2 Reference Station Antenna
//   Reference Point Parameter messages
//
//------------------------------------------------------------------------------

class RTCM2_24 {

  public:
    RTCM2_24 () {
      validMsg  = false;
      isGPS     = false;
      isGLONASS = false;
      x         = 0.0;
      y         = 0.0;
      z         = 0.0;
      h         = 0.0;
    }
    void extract(const RTCM2packet& P);
    bool    validMsg;          // Validity flag
    bool    isGPS;             // Flag for GPS supporting station
    bool    isGLONASS;         // Flag for GLONASS supporting station
    double  x,y,z;             // Station coordinates (ECEF,[m])
    double  h;                 // Antenna height [m]

};



//------------------------------------------------------------------------------
//
// RTCM2_Obs (class definition)
//
// Purpose:
//
//   A class for handling blocks of RTCM2 18 & 19 packets that need to be
//   combined to get a complete set of measurements
//
//------------------------------------------------------------------------------

class RTCM2_Obs {

  public:

    RTCM2_Obs();                           // Constructor

    void   extract(const RTCM2packet& P);  // Packet handler
    void   clear();                        // Initialization
    bool   valid() const;                  // Check for complete obs block

    double resolvedPhase_L1(int i) const;  // L1 & L2 carrier phase of i-th sat
    double resolvedPhase_L2(int i) const;  // with resolved 2^24 cy ambiguity
                                           // (based on rng_C1)

    void   resolveEpoch (int     refWeek,  // Resolve epoch using reference
                         double  refSecs,  // epoch (GPS week and secs)
                         int&    epochWeek,
                         double& epochSecs  ) const;


  public:

    double               secs;             // Seconds of hour (GPS time)
    int                  nSat;             // Number of space vehicles
    std::vector<int>     PRN;              // PRN (satellite number)
    std::vector<double>  rng_C1;           // C/A code pseudorange on L1 [m]
    std::vector<double>  rng_P1;           // P(Y) code pseudorange on L1 [m]
    std::vector<double>  rng_P2;           // Pseudorange on L2 [m]
    std::vector<double>  cph_L1;           // Carrier phase on L1 [cy]
    std::vector<double>  cph_L2;           // Carrier phase on L2 [cy]
    std::vector<int>     slip_L1;          // Carrier phase slip counter, L1
    std::vector<int>     slip_L2;          // Carrier phase slip counter, L1

  private:

    bool anyGPS() const;
    bool anyGLONASS() const;
    bool allGPS() const;
    bool allGLONASS() const;

  private:

    typedef std::bitset<8> msgflags;

    msgflags             availability;      // Msg availability flags
    bool                 GPSonly;           // Flag for GPS-only station

};


}; // End of namespace rtcm2

#endif  // include blocker
