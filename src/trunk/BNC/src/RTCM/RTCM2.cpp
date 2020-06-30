//------------------------------------------------------------------------------
//
// RTCM2.cpp
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
// Notes:
//
// - The host computer is assumed to use little endian (Intel) byte order
//
// Last modified:
//
//   2006/09/17  OMO  Created
//   2006/09/19  OMO  Fixed getHeader() methods
//   2006/09/21  OMO  Reduced phase ambiguity to 2^23 cycles
//   2006/10/05  OMO  Specified const'ness of various member functions
//   2006/10/13  LMV  Fixed resolvedPhase to handle missing C1 range
//   2006/10/14  LMV  Fixed loop cunter in ThirtyBitWord
//   2006/10/14  LMV  Exception handling
//   2006/10/17  OMO  Removed obsolete check of multiple message indicator
//   2006/10/17  OMO  Fixed parity handling
//   2006/10/18  OMO  Improved screening of bad data in RTCM2_Obs::extract
//   2006/11/25  OMO  Revised check for presence of GLONASS data
//   2007/05/25  GW   Round time tag to 100 ms
//   2007/12/11  AHA  Changed handling of C/A- and P-Code on L1
//   2007/12/13  AHA  Changed epoch comparison in packet extraction
//   2008/03/01  OMO  Compilation flag for epoch rounding
//   2008/03/04  AHA  Fixed problems with PRN 32
//   2008/03/05  AHA  Implemeted fix for Trimble 4000SSI receivers
//   2008/03/07  AHA  Major revision of input buffer handling
//   2008/03/07  AHA  Removed unnecessary failure flag
//   2008/03/10  AHA  Corrected extraction of antenna serial number
//   2008/03/10  AHA  Corrected buffer length check in getPacket()
//   2008/03/11  AHA  isGPS-flag in RTCM2_Obs is now set to false on clear()
//   2008/03/14  AHA  Added checks for data consistency in extraction routines
//   2008/09/01  AHA  Harmonization with newest BNC version
//
// (c) DLR/GSOC
//
//------------------------------------------------------------------------------

#include <bitset>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "RTCM2.h"

// Activate (1) or deactivate (0) debug output for tracing parity errors and
// undersized packets in get(Unsigned)Bits

#define DEBUG 0

// Activate (1) or deactivate (0) rounding of measurement epochs to 100ms
//
// Note: A need to round the measurement epoch to integer tenths of a second was
// noted by BKG in the processing of RTCM2 data from various receivers in NTRIP
// real-time networks. It is unclear at present, whether this is due to an
// improper implementation of the RTCM2 standard in the respective receivers
// or an unclear formulation of the standard.

#define ROUND_EPOCH  1

// Fix for data streams originating from TRIMBLE_4000SSI receivers.
// GPS PRN32 is erroneously flagged as GLONASS satellite in the C/A
// pseudorange messages. We therefore use a majority voting to
// determine the true constellation for this message.
// This fix is only required for Trimble4000SSI receivers but can also
// be used with all other known receivers.

#define FIX_TRIMBLE_4000SSI 1

using namespace std;


// GPS constants

const double c_light   = 299792458.0;   // Speed of light  [m/s]; IAU 1976
const double f_L1      = 1575.42e6;     // L1 frequency [Hz] (10.23MHz*154)
const double f_L2      = 1227.60e6;     // L2 frequency [Hz] (10.23MHz*120)

const double lambda_L1 = c_light/f_L1;  // L1 wavelength [m] (0.1903m)
const double lambda_L2 = c_light/f_L2;  // L2 wavelength [m]

//
// Bits for message availability checks
//

const int bit_L1rngGPS =  0;
const int bit_L2rngGPS =  1;
const int bit_L1cphGPS =  2;
const int bit_L2cphGPS =  3;
const int bit_L1rngGLO =  4;
const int bit_L2rngGLO =  5;
const int bit_L1cphGLO =  6;
const int bit_L2cphGLO =  7;


//
// namespace rtcm2
//

namespace rtcm2 {

//------------------------------------------------------------------------------
//
// class ThirtyBitWord (implementation)
//
// Purpose:
//
//   Handling of RTCM2 30bit words
//
//------------------------------------------------------------------------------

// Constructor

ThirtyBitWord::ThirtyBitWord() : W(0) {
};

// Clear entire 30-bit word and 2-bit parity from previous word

void ThirtyBitWord::clear() {
  W = 0;
};

// Parity check

bool ThirtyBitWord::validParity() const {

  // Parity stuff

  static const unsigned int  PARITY_25 = 0xBB1F3480;
  static const unsigned int  PARITY_26 = 0x5D8F9A40;
  static const unsigned int  PARITY_27 = 0xAEC7CD00;
  static const unsigned int  PARITY_28 = 0x5763E680;
  static const unsigned int  PARITY_29 = 0x6BB1F340;
  static const unsigned int  PARITY_30 = 0x8B7A89C0;

  // Look-up table for parity of eight bit bytes
  // (parity=0 if the number of 0s and 1s is equal, else parity=1)
  static unsigned char byteParity[] = {
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
    1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0
  };

  // Local variables

  unsigned int t, w, p;

  // The sign of the data is determined by the D30* parity bit
  // of the previous data word. If  D30* is set, invert the data
  // bits D01..D24 to obtain the d01..d24 (but leave all other
  // bits untouched).

  w = W;
  if ( w & 0x40000000 )  w ^= 0x3FFFFFC0;

  // Compute the parity of the sign corrected data bits d01..d24
  // as described in the ICD-GPS-200

  t = w & PARITY_25;
  p = ( byteParity[t      &0xff] ^ byteParity[(t>> 8)&0xff] ^
        byteParity[(t>>16)&0xff] ^ byteParity[(t>>24)     ]   );

  t = w & PARITY_26;
  p = (p<<1) |
      ( byteParity[t      &0xff] ^ byteParity[(t>> 8)&0xff] ^
        byteParity[(t>>16)&0xff] ^ byteParity[(t>>24)     ]   );

  t = w & PARITY_27;
  p = (p<<1) |
      ( byteParity[t      &0xff] ^ byteParity[(t>> 8)&0xff] ^
        byteParity[(t>>16)&0xff] ^ byteParity[(t>>24)     ]   );

  t = w & PARITY_28;
  p = (p<<1) |
      ( byteParity[t      &0xff] ^ byteParity[(t>> 8)&0xff] ^
        byteParity[(t>>16)&0xff] ^ byteParity[(t>>24)     ]   );

  t = w & PARITY_29;
  p = (p<<1) |
      ( byteParity[t      &0xff] ^ byteParity[(t>> 8)&0xff] ^
        byteParity[(t>>16)&0xff] ^ byteParity[(t>>24)     ]   );

  t = w & PARITY_30;
  p = (p<<1) |
      ( byteParity[t      &0xff] ^ byteParity[(t>> 8)&0xff] ^
        byteParity[(t>>16)&0xff] ^ byteParity[(t>>24)     ]   );

  return ( (W & 0x3f) == p);

};


// Check preamble

bool ThirtyBitWord::isHeader() const {

  const unsigned char Preamble = 0x66;

  unsigned char b = (value()>>22) & 0xFF;

  return ( b==Preamble );

};


// Return entire 32-bit (current word and previous parity)

unsigned int ThirtyBitWord::all() const {
  return W;
};


// Return sign-corrected 30-bit (or zero if parity mismatch)

unsigned int ThirtyBitWord::value() const {

  unsigned int w = W;

  if (validParity()) {
    // Return data and current parity bits. Invert data bits if D30*
    // is set and discard old parity bits.
    if ( w & 0x40000000 )  w ^= 0x3FFFFFC0;
    return (w & 0x3FFFFFFF);
  }
  else {
    // Error; invalid parity
    return 0;
  };

};


// Append a byte with six data bits

void ThirtyBitWord::append(unsigned char b) {

  // Look up table for swap (left-right) of 6 data bits
  static const unsigned char
    swap[] = {
      0,32,16,48, 8,40,24,56, 4,36,20,52,12,44,28,60,
      2,34,18,50,10,42,26,58, 6,38,22,54,14,46,30,62,
      1,33,17,49, 9,41,25,57, 5,37,21,53,13,45,29,61,
      3,35,19,51,11,43,27,59, 7,39,23,55,15,47,31,63
    };

  // Bits 7 and 6 (of 0..7) must be "01" for valid data bytes
  if ( (b & 0x40) != 0x40 ) {
    // We simply skip the invalid input byte and leave the word unchanged
#if (DEBUG>0)
    cerr << "Error in append()" << bitset<32>(all()) << endl;
#endif
    return;
  };

  // Swap bits 0..5 to restore proper bit order for 30bit words
  b = swap[ b & 0x3f];

  // Fill word
  W = ( (W <<6) | (b & 0x3f) ) ;

};


// Get next 30bit word from string

void ThirtyBitWord::get(const std::string& buf) {

  // Check if string is long enough

  if (buf.size()<5) {
    // Ignore; users should avoid this case prior to calling get()

#if ( DEBUG > 0 )
    cerr << "Error in get(): packet too short (" << buf.size() <<")" << endl;
#endif

    return;
  };

  // Process 5 bytes

  for (int i=0; i<5; i++) append(buf[i]);

#if (DEBUG>0)
  if (!validParity()) {
    cerr << "Parity error in get()"
         << bitset<32>(all()) << endl;
  };
#endif

};

// Get next 30bit word from file

void ThirtyBitWord::get(std::istream& inp) {

  unsigned char b;

  for (int i=0; i<5; i++) {
    inp >> b;
    if (inp.fail()) { clear(); return; };
    append(b);
  };

#if (DEBUG>0)
  if (!validParity()) {
    cerr << "Parity error in get()"
         << bitset<32>(all()) << endl;
  };
#endif

};

// Get next header word from string

void ThirtyBitWord::getHeader(std::string& buf) {

  const unsigned int wordLen = 5; // Number of bytes representing a 30-bit word
  const unsigned int spare   = 1; // Number of spare words for resync of parity
                                  // (same value as inRTCM2packet::getPacket())
  unsigned int i;

  i=0;
  // append spare word (to get correct parity) and first consecutive word
  while (i<(spare+1)*wordLen) {
    // Process byte
    append(buf[i]);
    // Increment count
    i++;
  };

  // start searching for preamble in first word after spare word
  while (!isHeader() && i<buf.size() ) {
    // Process byte
    append(buf[i]);
    // Increment count
    i++;
  };

  // Remove processed bytes from buffer. Retain also the previous word to
  // allow a resync if getHeader() is called repeatedly on the same buffer.
  if (i>=(1+spare)*wordLen) buf.erase(0,i-(1+spare)*wordLen);

#if (DEBUG>0)
  if (!validParity()) {
    cerr << "Parity error in getHeader()"
         << bitset<32>(all()) << endl;
  };
#endif

};

// Get next header word from file

void ThirtyBitWord::getHeader(std::istream& inp) {

  unsigned char b;
  unsigned int  i;

  i=0;
  while ( !isHeader() || i<5 ) {
    inp >> b;
    if (inp.fail()) { clear(); return; };
    append(b); i++;
  };

#if (DEBUG>0)
  if (!validParity()) {
    cerr << "Parity error in getHeader()"
         << bitset<32>(all()) << endl;
  };
#endif

};


//------------------------------------------------------------------------------
//
// RTCM2packet (class implementation)
//
// Purpose:
//
//   A class for handling RTCM2 data packets
//
//------------------------------------------------------------------------------

// Constructor

RTCM2packet::RTCM2packet()  {
  clear();
};

// Initialization

void RTCM2packet::clear()  {

  W.clear();

  H1=0;
  H2=0;

  DW.resize(0,0);

};

// Complete packet, valid parity

bool RTCM2packet::valid() const {

  // The methods for creating a packet (get,">>") ensure
  // that a packet has a consistent number of data words
  // and a valid parity in all header and data words.
  // Therefore a packet is either empty or valid.

  return (H1!=0);

};


//
// Gets the next packet from the buffer
//

void RTCM2packet::getPacket(std::string& buf) {

  const int wordLen = 5; // Number of bytes representing a 30-bit word
  const int spare   = 1; // Number of spare words for resync of parity
                         // (same value as used in ThirtyBitWord::getHeader)
  unsigned int n;

  // Does the package content at least spare bytes and first header byte?
  if (buf.size()<(spare+1)*wordLen) {
      clear();
      return;
  };

  // Try to read a full packet. Processed bytes are removed from the input
  // buffer except for the latest spare*wordLen bytes to restore the parity
  // bytes upon subseqeunt calls of getPacket().

  // Locate and read the first header word
  W.getHeader(buf);
  if (!W.isHeader()) {
    // No header found; try again next time. buf retains only the spare
    // words. The packet contents is cleared to indicate an unsuccessful
    // termination of getPacket().
    clear();

#if ( DEBUG > 0 )
    cerr << "Error in getPacket(): W.isHeader() = false  for H1" << endl;
#endif

    return;
  };
  H1 = W.value();

  // Do we have enough bytes to read the next word? If not, the packet
  // contents is cleared to indicate an unsuccessful termination. The
  // previously read spare and header bytes are retained in the buffer
  // for use in the next call of getPacket().
  if (buf.size()<(spare+2)*wordLen) {
    clear();

#if ( DEBUG > 0 )
    cerr << "Error in getPacket(): buffer too short for complete H2" << endl;
#endif

    return;
  };

  // Read the second header word
  W.get(buf.substr((spare+1)*wordLen,buf.size()-(spare+1)*wordLen));
  H2 = W.value();
  if (!W.validParity()) {
    // Invalid H2 word; delete first buffer byte and try to resynch next time.
    // The packet contents is cleared to indicate an unsuccessful termination.
    clear();
    buf.erase(0,1);

#if ( DEBUG > 0 )
    cerr << "Error in getPacket(): W.validParity() = false for H2" << endl;
#endif

    return;
  };

  n = nDataWords();

  // Do we have enough bytes to read the next word? If not, the packet
  // contents is cleared to indicate an unsuccessful termination. The
  // previously read spare and header bytes are retained in the buffer
  // for use in the next call of getPacket().
  if (buf.size()<(spare+2+n)*wordLen) {
    clear();

#if ( DEBUG > 0 )
    cerr << "Error in getPacket(): buffer too short for complete " << n
         << " DWs" << endl;
#endif

    return;
  };

  DW.resize(n);
  for (unsigned int i=0; i<n; i++) {
    W.get(buf.substr((spare+2+i)*wordLen,buf.size()-(spare+2+i)*wordLen));
    DW[i] = W.value();
    if (!W.validParity()) {
      // Invalid data word; delete first byte and try to resynch next time.
      // The packet contents is cleared to indicate an unsuccessful termination.
      clear();
      buf.erase(0,1);

#if ( DEBUG > 0 )
    cerr << "Error in getPacket(): W.validParity() = false for DW"
         << i << endl;
#endif

      return;
    };
  };

  // Successful packet extraction; delete total number of message bytes
  // from buffer.
  // Note: a total of "spare" words remain in the buffer to enable a
  // parity resynchronization when searching the next header.

  buf.erase(0,(n+2)*wordLen);

  return;

};


//
// Gets the next packet from the input stream
//

void RTCM2packet::getPacket(std::istream& inp) {

  int n;

  W.getHeader(inp);
  H1 = W.value();
  if (inp.fail() || !W.isHeader()) { clear(); return; }

  W.get(inp);
  H2 = W.value();
  if (inp.fail() || !W.validParity()) { clear(); return; }

  n = nDataWords();
  DW.resize(n);
  for (int i=0; i<n; i++) {
    W.get(inp);
    DW[i] = W.value();
    if (inp.fail() || !W.validParity()) { clear(); return; }
  };

  return;

};

//
// Input operator
//
// Reads an RTCM2 packet from the input stream.
//

istream& operator >> (istream& is, RTCM2packet& p) {

  p.getPacket(is);

  return is;

};

// Access methods

unsigned int RTCM2packet::header1() const {
  return H1;
};

unsigned int RTCM2packet::header2() const {
  return H2;
};

unsigned int RTCM2packet::dataWord(int i) const {
  if ( (unsigned int)i < DW.size() ) {
    return DW[i];
  }
  else {
    return 0;
  }
};

unsigned int RTCM2packet::msgType()   const {
  return ( H1>>16 & 0x003F );
};

unsigned int RTCM2packet::stationID() const {
  return ( H1>> 6 & 0x03FF );
};

unsigned int RTCM2packet::modZCount() const {
  return ( H2>>17 & 0x01FFF );
};

unsigned int RTCM2packet::seqNumber() const {
  return ( H2>>14 & 0x0007 );
};

unsigned int RTCM2packet::nDataWords() const {
  return ( H2>> 9 & 0x001F );
};

unsigned int RTCM2packet::staHealth() const {
  return ( H2>> 6 & 0x0003 );
};


//
// Get unsigned bit field
//
// Bits are numbered from left (msb) to right (lsb) starting at bit 0
//

unsigned int RTCM2packet::getUnsignedBits ( unsigned int start,
                                            unsigned int n      ) const {

  unsigned int  iFirst = start/24;       // Index of first data word
  unsigned int  iLast  = (start+n-1)/24; // Index of last  data word
  unsigned int  bitField = 0;
  unsigned int  tmp;

  // Checks

  if (n>32) {
    throw("Error: can't handle >32 bits in RTCM2packet::getUnsignedBits");
  };

  if ( 24*DW.size() < start+n-1 ) {
#if (DEBUG>0)
    cerr << "Debug output RTCM2packet::getUnsignedBits" << endl
         << "  P.msgType:    " << setw(5) << msgType()    << endl
         << "  P.nDataWords: " << setw(5) << nDataWords() << endl
         << "  start:        " << setw(5) << start        << endl
         << "  n:            " << setw(5) << n            << endl
         << "  P.H1:         " << setw(5) << bitset<32>(H1) << endl
         << "  P.H2:         " << setw(5) << bitset<32>(H2) << endl
         << endl
         << flush;
#endif
    throw("Error: Packet too short in RTCM2packet::getUnsignedBits");
  }

  // Handle initial data word
  // Get all data bits. Strip parity and unwanted leading bits.
  // Store result in 24 lsb bits of tmp.

  tmp = (DW[iFirst]>>6) & 0xFFFFFF;
  tmp = ( ( tmp << start%24) & 0xFFFFFF ) >> start%24 ;

  // Handle central data word

  if ( iFirst<iLast ) {
    bitField = tmp;
    for (unsigned int iWord=iFirst+1; iWord<iLast; iWord++) {
      tmp = (DW[iWord]>>6) & 0xFFFFFF;
      bitField = (bitField << 24) | tmp;
    };
    tmp = (DW[iLast]>>6) & 0xFFFFFF;
  };

  // Handle last data word

  tmp = tmp >> (23-(start+n-1)%24);
  bitField = (bitField << ((start+n-1)%24+1)) | tmp;

  // Done

  return bitField;

};

//
// Get signed bit field
//
// Bits are numbered from left (msb) to right (lsb) starting at bit 0
//

int RTCM2packet::getBits ( unsigned int start,
                           unsigned int n      ) const {


  // Checks

  if (n>32) {
    throw("Error: can't handle >32 bits in RTCM2packet::getBits");
  };

  if ( 24*DW.size() < start+n-1 ) {
#if (DEBUG>0)
    cerr << "Debug output RTCM2packet::getUnsignedBits" << endl
         << "  P.msgType:    " << setw(5) << msgType()    << endl
         << "  P.nDataWords: " << setw(5) << nDataWords() << endl
         << "  start:        " << setw(5) << start        << endl
         << "  n:            " << setw(5) << n            << endl
         << "  P.H1:         " << setw(5) << bitset<32>(H1) << endl
         << "  P.H2:         " << setw(5) << bitset<32>(H2) << endl
         << endl
         << flush;
#endif
    throw("Error: Packet too short in RTCM2packet::getBits");
  }

  return ((int)(getUnsignedBits(start,n)<<(32-n))>>(32-n));

};


//------------------------------------------------------------------------------
//
// RTCM2_03 (class implementation)
//
// Purpose:
//
//   A class for handling RTCM 2 GPS Reference Station Parameters messages
//
//------------------------------------------------------------------------------


void RTCM2_03::extract(const RTCM2packet& P) {

  // Check validity, packet type and number of data words

  validMsg = (P.valid());
  if (!validMsg) return;

  validMsg = (P.ID()==03);
  if (!validMsg) return;

  validMsg = (P.nDataWords()==4);
  if (!validMsg) return;

  // Antenna reference point coordinates

  x  = P.getBits( 0,32)*0.01;    // X [m]
  y  = P.getBits(32,32)*0.01;    // Y [m]
  z  = P.getBits(64,32)*0.01;    // Z [m]

};

//------------------------------------------------------------------------------
//
// RTCM2_23 (class implementation)
//
// Purpose:
//
//   A class for handling RTCM 2 Antenna Type Definition messages
//
//------------------------------------------------------------------------------

void RTCM2_23::extract(const RTCM2packet& P) {

  unsigned int       nad, nas;

  const unsigned int nF1  = 8; // bits in first field (R,AF,SF,NAD)
  const unsigned int nF2  =16; // bits in second field (SETUP ID,R,NAS)
  const unsigned int nBits=24; // data bits in  30bit word

  // Check validity, packet type and number of data words

  validMsg = (P.valid());
  if (!validMsg) return;

  validMsg = (P.ID()==23);
  if (!validMsg) return;

  // Check number of data words (can nad be read in?)

  validMsg = (P.nDataWords()>=1);
  if (!validMsg){
    cerr << "RTCM2_23::extract: P.nDataWords()>=1" << endl;
    return;
  }

  // Antenna descriptor
  antType = "";
  nad = P.getUnsignedBits(3,5);

  // Check number of data words (can antenna description be read in?)
  validMsg = ( P.nDataWords() >=
               (unsigned int)ceil((nF1+nad*8)/(double)nBits) );

  if (!validMsg) return;

  for (unsigned int i=0;i<nad;i++)
    antType += (char)P.getUnsignedBits(nF1+i*8,8);

  // Optional antenna serial numbers
  if (P.getUnsignedBits(2,1)==1) {

    // Check number of data words (can nas be read in?)

    validMsg = ( P.nDataWords() >=
                 (unsigned int)ceil((nF1+nad*8+nF2)/(double)nBits) );
    if (!validMsg) return;

    nas = P.getUnsignedBits(19+8*nad,5);

    // Check number of data words (can antenna serial number be read in?)

    validMsg = ( P.nDataWords() >=
                 (unsigned int)ceil((nF1+nad*8+nF2+nas*8)/(double)nBits) );
    if (!validMsg) return;

    antSN = "";
    for (unsigned int i=0;i<nas;i++)
      antSN += (char)P.getUnsignedBits(nF1+8*nad+nF2+i*8,8);
  };

};


//------------------------------------------------------------------------------
//
// RTCM2_24 (class implementation)
//
// Purpose:
//
//   A class for handling RTCM 2 Reference Station Antenna
//   Reference Point Parameter messages
//
//------------------------------------------------------------------------------

void RTCM2_24::extract(const RTCM2packet& P) {

   double dx,dy,dz;

  // Check validity, packet type and number of data words

  validMsg = (P.valid());
  if (!validMsg) return;

  validMsg = (P.ID()==24);
  if (!validMsg) return;

  validMsg = (P.nDataWords()==6);
  if (!validMsg) return;

  // System indicator

  isGPS     = (P.getUnsignedBits(118,1)==0);
  isGLONASS = (P.getUnsignedBits(118,1)==1);

  // Antenna reference point coordinates

  x  = 64.0*P.getBits( 0,32);
  y  = 64.0*P.getBits(40,32);
  z  = 64.0*P.getBits(80,32);
  dx = P.getUnsignedBits( 32,6);
  dy = P.getUnsignedBits( 72,6);
  dz = P.getUnsignedBits(112,6);
  x = 0.0001*( x + (x<0? -dx:+dx) );
  y = 0.0001*( y + (y<0? -dy:+dy) );
  z = 0.0001*( z + (z<0? -dz:+dz) );

  // Antenna Height

  if (P.getUnsignedBits(119,1)==1) {
    h= P.getUnsignedBits(120,18)*0.0001;
  };


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
// Notes:
//
//   The class collects L1/L2 code and phase measurements for GPS and GLONASS.
//   Since the Multiple Message Indicator is inconsistently handled by various
//   receivers we simply require code and phase on L1 and L2 for a complete
//   set ob observations at a given epoch. GLONASS observations are optional,
//   but all four types (code+phase,L1+L2) must be provided, if at least one
//   is given. Also, the GLONASS message must follow the corresponding GPS
//   message.
//
//------------------------------------------------------------------------------

// Constructor

RTCM2_Obs::RTCM2_Obs() {

  clear();

};

// Reset entire block

void RTCM2_Obs::clear() {

  GPSonly = true;

  secs=0.0;                // Seconds of hour (GPS time)
  nSat=0;                  // Number of space vehicles
  PRN.resize(0);           // space vehicles
  rng_C1.resize(0);        // Pseudorange [m]
  rng_P1.resize(0);        // Pseudorange [m]
  rng_P2.resize(0);        // Pseudorange [m]
  cph_L1.resize(0);        // Carrier phase [m]
  cph_L2.resize(0);        // Carrier phase [m]
  slip_L1.resize(0);       // Slip counter
  slip_L2.resize(0);       // Slip counter

  availability.reset();    // Message status flags

};

// Availability checks

bool RTCM2_Obs::anyGPS() const {

  return  availability.test(bit_L1rngGPS) ||
          availability.test(bit_L2rngGPS) ||
          availability.test(bit_L1cphGPS) ||
          availability.test(bit_L2cphGPS);

};

bool RTCM2_Obs::anyGLONASS() const {

  return  availability.test(bit_L1rngGLO) ||
          availability.test(bit_L2rngGLO) ||
          availability.test(bit_L1cphGLO) ||
          availability.test(bit_L2cphGLO);

};

bool RTCM2_Obs::allGPS() const {

  return  availability.test(bit_L1rngGPS) &&
          availability.test(bit_L2rngGPS) &&
          availability.test(bit_L1cphGPS) &&
          availability.test(bit_L2cphGPS);

};

bool RTCM2_Obs::allGLONASS() const {

  return  availability.test(bit_L1rngGLO) &&
          availability.test(bit_L2rngGLO) &&
          availability.test(bit_L1cphGLO) &&
          availability.test(bit_L2cphGLO);

};

// Validity

bool RTCM2_Obs::valid() const {

  return ( allGPS() && ( GPSonly || allGLONASS() ) );

};


//
// Extract RTCM2 18 & 19 messages and store relevant data for future use
//

void RTCM2_Obs::extract(const RTCM2packet& P) {

  bool    isGPS,isCAcode,isL1,isOth;
  int     NSat,idx;
  int     sid,prn,slip_cnt;
  double  t,rng,cph;

  // Check validity and packet type

  if ( ! ( P.valid() &&
           (P.ID()==18 || P.ID()==19) ) ) return;

  // Check number of data words, message starts with 1 DW for epoch, then each
  // satellite brings 2 DW,
  // Do not start decoding if less than 3 DW are in package

  if ( P.nDataWords()<3 ) {
#if ( DEBUG > 0 )
    cerr << "Error in RTCM2_Obs::extract(): less than 3 DW ("
         << P.nDataWords() << ") detected" << endl;
#endif

    return;
  };

  // Check if number of data words is odd number

  if ( P.nDataWords()%2==0 ){
#if ( DEBUG > 0 )
    cerr << "Error in RTCM2_Obs::extract(): odd number of DW ("
         << P.nDataWords() << ") detected" << endl;
#endif

    return;
  };

  // Clear previous data if block was already complete

  if (valid()) clear();

  // Process carrier phase message

  if ( P.ID()==18 ) {

    // Number of satellites in current message
    NSat = (P.nDataWords()-1)/2;

    // Current epoch (mod 3600 sec)
    t = 0.6*P.modZCount()
        + P.getUnsignedBits(4,20)*1.0e-6;

#if (ROUND_EPOCH==1)
    // SC-104 V2.3 4-42 Note 1 4. Assume measurements at hard edges
    // of receiver clock with minimum divisions of 10ms
    // and clock error less then recommended 1.1ms
    // Hence, round time tag to 100 ms
    t = floor(t*100.0+0.5)/100.0;
#endif

    // Frequency (exit if neither L1 nor L2)
    isL1  = ( P.getUnsignedBits(0,1)==0 );
    isOth = ( P.getUnsignedBits(1,1)==1 );
    if (isOth) return;

    // Constellation (for first satellite in message)
    isGPS = ( P.getUnsignedBits(26,1)==0 );
    GPSonly = GPSonly && isGPS;

    // Multiple Message Indicator (only checked for first satellite)
    // pendingMsg = ( P.getUnsignedBits(24,1)==1 );

    // Handle epoch: store epoch of first GPS message and
    // check consistency of subsequent messages. GLONASS time tags
    // are different and have to be ignored
    if (isGPS) {
      if ( nSat==0 ) {
        secs = t; // Store epoch
      }
//    else if (t!=secs) {
      else if (abs(t-secs)>1e-6) {
        clear(); secs = t; // Clear all data, then store epoch
      };
    };

    // Discard GLONASS observations if no prior GPS observations
    // are available
    if (!isGPS && !anyGPS() ) return;

    // Set availability flags

    if ( isL1 &&  isGPS) availability.set(bit_L1cphGPS);
    if (!isL1 &&  isGPS) availability.set(bit_L2cphGPS);
    if ( isL1 && !isGPS) availability.set(bit_L1cphGLO);
    if (!isL1 && !isGPS) availability.set(bit_L2cphGLO);

#if ( DEBUG > 0 )
    cerr << "RTCM2_Obs::extract(): availability "
         << bitset<8>(availability) << endl;
#endif


    // Process all satellites

    for (int iSat=0;iSat<NSat;iSat++){

      // Code type
      isCAcode = ( P.getUnsignedBits(iSat*48+25,1)==0 );

      // Satellite
      sid = P.getUnsignedBits(iSat*48+27,5);
      if (sid==0) sid=32;

      prn = (isGPS? sid : sid+200 );

      // Carrier phase measurement (mod 2^23 [cy]; sign matched to range)
      cph = -P.getBits(iSat*48+40,32)/256.0;

      // Slip counter
      slip_cnt = P.getUnsignedBits(iSat*48+35,5);

      // Is this a new PRN?
      idx=-1;
      for (unsigned int i=0;i<PRN.size();i++) {
        if (PRN[i]==prn) { idx=i; break; };
      };
      if (idx==-1) {
        // Insert new sat at end of list
        nSat++; idx = nSat-1;
        PRN.push_back(prn);
        rng_C1.push_back(0.0);
        rng_P1.push_back(0.0);
        rng_P2.push_back(0.0);
        cph_L1.push_back(0.0);
        cph_L2.push_back(0.0);
        slip_L1.push_back(-1);
        slip_L2.push_back(-1);
      };

      // Store measurement
      if (isL1) {
        cph_L1[idx] = cph;
        slip_L1[idx] = slip_cnt;
      }
      else {
        cph_L2[idx] = cph;
        slip_L2[idx] = slip_cnt;
      };

    };

  };


  // Process pseudorange message

  if ( P.ID()==19 ) {

    // Number of satellites in current message
    NSat = (P.nDataWords()-1)/2;

    // Current epoch (mod 3600 sec)
    t = 0.6*P.modZCount()
        + P.getUnsignedBits(4,20)*1.0e-6;

#if (ROUND_EPOCH==1)
    // SC-104 V2.3 4-42 Note 1 4. Assume measurements at hard edges
    // of receiver clock with minimum divisions of 10ms
    // and clock error less then recommended 1.1ms
    // Hence, round time tag to 100 ms
    t = floor(t*100.0+0.5)/100.0;
#endif

    // Frequency (exit if neither L1 nor L2)
    isL1  = ( P.getUnsignedBits(0,1)==0 );
    isOth = ( P.getUnsignedBits(1,1)==1 );
    if (isOth) return;

#if (FIX_TRIMBLE_4000SSI==1)
    // Fix for data streams originating from TRIMBLE_4000SSI receivers.
    // GPS PRN32 is erroneously flagged as GLONASS satellite in the C/A
    // pseudorange messages. We therefore use a majority voting to
    // determine the true constellation for this message.
    // This fix is only required for Trimble4000SSI receivers but can also
    // be used with all other known receivers.
    int nGPS=0;
    for(int iSat=0; iSat<NSat; iSat++){
      // Constellation (for each satellite in message)
      isGPS = ( P.getUnsignedBits(iSat*48+26,1)==0 );
      if(isGPS) nGPS++;
    };
    isGPS = (2*nGPS>NSat);
#else
    // Constellation (for first satellite in message)
    isGPS = ( P.getUnsignedBits(26,1)==0 );
#endif
    GPSonly = GPSonly && isGPS;

    // Multiple Message Indicator (only checked for first satellite)
    // pendingMsg = ( P.getUnsignedBits(24,1)==1 );

    // Handle epoch: store epoch of first GPS message and
    // check consistency of subsequent messages. GLONASS time tags
    // are different and have to be ignored
    if (isGPS) {
      if ( nSat==0 ) {
        secs = t; // Store epoch
      }
//    else if (t!=secs) {
      else if (abs(t-secs)>1e-6) {
        clear(); secs = t; // Clear all data, then store epoch
      };
    };

    // Discard GLONASS observations if no prior GPS observations
    // are available
    if (!isGPS && !anyGPS() ) return;

    // Set availability flags
    if ( isL1 &&  isGPS) availability.set(bit_L1rngGPS);
    if (!isL1 &&  isGPS) availability.set(bit_L2rngGPS);
    if ( isL1 && !isGPS) availability.set(bit_L1rngGLO);
    if (!isL1 && !isGPS) availability.set(bit_L2rngGLO);

#if ( DEBUG > 0 )
    cerr << "RTCM2_Obs::extract(): availability "
         << bitset<8>(availability) << endl;
#endif

    // Process all satellites

    for (int iSat=0;iSat<NSat;iSat++){

      // Code type
      isCAcode = ( P.getUnsignedBits(iSat*48+25,1)==0 );

      // Satellite
      sid = P.getUnsignedBits(iSat*48+27,5);
      if (sid==0) sid=32;
      prn = (isGPS? sid : sid+200 );

      // Pseudorange measurement [m]
      rng = P.getUnsignedBits(iSat*48+40,32)*0.02;

      // Is this a new PRN?
      idx=-1;
      for (unsigned int i=0;i<PRN.size();i++) {
        if (PRN[i]==prn) { idx=i; break; };
      };
      if (idx==-1) {
        // Insert new sat at end of list
        nSat++; idx = nSat-1;
        PRN.push_back(prn);
        rng_C1.push_back(0.0);
        rng_P1.push_back(0.0);
        rng_P2.push_back(0.0);
        cph_L1.push_back(0.0);
        cph_L2.push_back(0.0);
        slip_L1.push_back(-1);
	      slip_L2.push_back(-1);
      };

      // Store measurement
      if (isL1) {
        if (isCAcode) {
          rng_C1[idx] = rng;
        }
        else {
          rng_P1[idx] = rng;
        }
      }
      else {
        rng_P2[idx] = rng;
      };

    };

  };

};

//
//  Resolution of 2^24 cy carrier phase ambiguity
//  caused by 32-bit data field restrictions
//
//  Note: the RTCM standard specifies an ambiguity of +/-2^23 cy.
//  However, numerous receivers generate data in the +/-2^22 cy range.
//  A reduced ambiguity of 2^23 cy appears compatible with both cases.
//

double RTCM2_Obs::resolvedPhase_L1(int i) const {

//const double  ambig = pow(2.0,24);   // as per RTCM2 spec
  const double  ambig = pow(2.0,23);   // used by many receivers

  double        rng;
  double        n;

  if (!valid() || i<0 || i>nSat-1) return 0.0;

  rng = rng_C1[i];
  if (rng==0.0) rng = rng_P1[i];
  if (rng==0.0) return 0.0;

  n = floor( (rng/lambda_L1-cph_L1[i]) / ambig + 0.5 );

  return cph_L1[i] + n*ambig;

};

double RTCM2_Obs::resolvedPhase_L2(int i) const {

//const double  ambig = pow(2.0,24);   // as per RTCM2 spec
  const double  ambig = pow(2.0,23);   // used by many receivers

  double        rng;
  double        n;

  if (!valid() || i<0 || i>nSat-1) return 0.0;

  rng = rng_C1[i];
  if (rng==0.0) rng = rng_P1[i];
  if (rng==0.0) return 0.0;

  n = floor( (rng/lambda_L2-cph_L2[i]) / ambig + 0.5 );

  return cph_L2[i] + n*ambig;

};

//
//  Resolution of epoch using reference date (GPS week and secs)
//

void RTCM2_Obs::resolveEpoch (int  refWeek,   double  refSecs,
                              int& epochWeek, double& epochSecs   ) const {

  const double secsPerWeek = 604800.0;

  epochWeek = refWeek;
  epochSecs = secs + 3600.0*(floor((refSecs-secs)/3600.0+0.5));

  if (epochSecs<0          ) { epochWeek--; epochSecs+=secsPerWeek; };
  if (epochSecs>secsPerWeek) { epochWeek++; epochSecs-=secsPerWeek; };

};

}; // End of namespace rtcm2
