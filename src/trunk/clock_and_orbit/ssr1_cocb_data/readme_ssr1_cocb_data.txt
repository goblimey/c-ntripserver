RTCM SSR Orbit, Clock and Code Bias Interoperability Test Datasets
==================================================================


Browsing zipfile archive
------------------------
ssr1_cocb_data/outfile_C.dbg
ssr1_cocb_data/outfile_C.raw
ssr1_cocb_data/outfile_E.dbg
ssr1_cocb_data/outfile_E.raw
ssr1_cocb_data/outfile_G.dbg
ssr1_cocb_data/outfile_G.raw
ssr1_cocb_data/outfile_J.dbg
ssr1_cocb_data/outfile_J.raw
ssr1_cocb_data/outfile_R.dbg
ssr1_cocb_data/outfile_R.raw
ssr1_cocb_data/outfile_S.dbg
ssr1_cocb_data/outfile_S.raw
ssr1_cocb_data/CLK801330.14C
ssr1_cocb_data/readme_InterOpTest.txt


Supported GNSS:
---------------
- G: GPS
- R: GLONASS
- E: Galileo
- J: QZSS
- S: SBAS
- C: BDS


Input data set in ASCII format
------------------------------
- ssr1_cocb_data/CLK801330.14C

  real GPS+GLONASS data set containing the following RTCM3 messages:

  message type  | interval [sec] | description
 ---------------+----------------+---------------------------------------------
     1059       |      5         | GPS code biases
     1060       |      5         | GPS combined orbit and clock corrections
     1065       |      5         | GLONASS code biases
     1066       |      5         | GLONASS combined orbit and clock corrections

- the GPS messages 1059 and 1060 are used for generation of
  synthetic input data for all other supported GNSS:

   GNSS       |  message types   | description
  ------------+------------------+---------------------------------------------
   GLONASS    |      1065        | GLONASS code biases
              |      1066        | GLONASS combined orbit and clock corrections
   Galileo    |      1242        | Galileo code biases
              |      1243        | Galileo combined orbit and clock corrections
   QZSS       |      1248        | QZSS code biases
              |      1249        | QZSS combined orbit and clock corrections
   SBAS       |      1254        | SBAS code biases
              |      1255        | SBAS combined orbit and clock corrections
   BDS        |      1260        | BDS code biases
              |      1261        | BDS combined orbit and clock corrections


generated synthetic data sets in RTCM3-SSR format
-------------------------------------------------
- ssr1_cocb_data/outfile_*.raw


encoding / decoding debug output in ASCII format
------------------------------------------------
- ssr1_cocb_data/outfile_*.dbg

