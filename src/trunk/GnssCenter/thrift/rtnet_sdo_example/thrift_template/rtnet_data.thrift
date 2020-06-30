namespace cpp com.gpssolutions.rtnet
namespace java com.gpssolutions.rtnet
namespace perl gpss

struct PosNEU {
  1: double n, // north (meters)
  2: double e, // east (meters)
  3: double u  // up (meters)
}

struct PosXYZ {
  1: double x, // x (meters)
  2: double y, // y (meters)
  3: double z  // z (meters)
}

struct StationInfo {
  1: string ID,
  2: PosNEU sigma0,
  3: PosNEU sigmaNoise,
  4: PosXYZ xyz,
}

struct StationAuxInfo {
  1: string ID,
  2: string receiverID,
  3: string antennaID,
  4: PosNEU eccentricity,
}

enum ConstellationType {
  GPS     = 1,
  GLONASS = 2,
  SBAS    = 3,
  GALILEO = 4,
  QZSS    = 5,
  COMPASS = 6,
}

// consider adding health flags
struct SatelliteXYZ {
  1: byte ID,
  2: ConstellationType constellation,
  3: PosXYZ xyz,
//  4: double lon,
//  5: double lat
}

enum FrequencyType {
  // GPS phase
  Gp1=1,
  Gp2=2,
  Gp5=3,
  GpIF=4,
  GpGF=5,
  GpWL=6,
  GpMW=7,

  // Galileo phase
  Ep1=8,
  Ep5=9,
  Ep6=10,
  Ep7=11,
  Ep8=12,
  EpIF=13,
  EpGF=14,
  EpWL=15,
  EpMW=16,

  // GLONASS phase
  Rp1=17,
  Rp2=18,
  RpIF=19,
  RpGF=20,
  RpWL=21,
  RpMW=22,

  // GPS code
  Gc1=23,
  Gc2=24,
  Gc5=25,
  GcIF=26,
  GcGF=27,

  // Galileo code
  Ec1=28,
  Ec5=29,
  Ec6=30,
  Ec7=31,
  Ec8=32,
  EcIF=33,
  EcGF=34,

  // GLONASS code
  Rc1=35,
  Rc2=36,
  RcIF=37,
  RcGF=38,

  // GPS doppler
  Gd1=39,
  Gd2=40,
}

struct ZDAmb {
  1: byte satelliteID,
  2: ConstellationType constellation,
  3: string stationID,
  4: FrequencyType frequency,
  5: bool constrainedFlag,
  6: i16 duration,
  7: bool hasObsFlag,
 10: bool isNewAmb,
 11: bool isResetAmbL1p,
 12: bool isResetAmbClock,
  8: optional double azimuth,
  9: optional double elevation,
}

enum AmbresStatus
{
  AmbFloat = 1,
  AmbFixed = 2,
  AmbDumped = 3
}

struct DDAmbres
{
  1: byte satelliteID,
  2: ConstellationType constellation,
  3: AmbresStatus statusFlag,
  4: bool hasObsFlag,
}

struct DDAmbresBaseline
{
  1: string stationID_1,
  2: string stationID_2,
  3: bool redundant,
  4: optional list<DDAmbres> wideLaneStatisticList,
  5: optional list<DDAmbres> narrowLaneStatisticList,
  6: optional list<DDAmbres> l1StatisticList,
  7: optional list<DDAmbres> l2StatisticList,
}

/*struct DDAmbresStatistic {
  1: list<DDAmbresBaseline> baselineList
}*/

/**
 * Stores information about rtnet job.
 * For now, it only stores the rtnet job's name (should be unique).
 * Eventually, may store other useful information such as the
 * input file contents, the name of who started the job, etc.
 */
struct RtnetInformation {
  1: string jobName
}

// Data needed for plots

struct SatelliteClock {
  1: byte satelliteID,
  2: ConstellationType constellation,
  3: double clock,    // satellite clock estimate (s)
  4: double clock_err, // satellite clock error (s)
}

struct DGPSCorr {
  1: byte satelliteID,
  2: ConstellationType constellation,
  3: double a0,      // non-dispersive correction (ie, clock)
  4: double a0e,     // formal error for a0
  5: double aN,      // non-dispersive correction, N-S gradient
  6: double aE,      // non-dispersive correction, E-W gradient
  7: double i0,      // dispersive correction
  8: double iN,      // dispersive correction, N-S gradient
  9: double iE,      // dispersive correction, E-W gradient
 10: i16 nsdfix,     // number of stations with fixed ZD ambiguity
 11: i16 nsta,       // number of stations
 12: i16 cjumpCnt,   // correction jump counter
 13: double deltaMW, // MW bias
 14: double deltaP3, // P3 bias
 15: double te0,     // vertical TEC
 16: double teN,     // vertical TEC, N-S gradient
 17: double teE,     // vertical TEC, E-W gradient
 18: double teNN,    // vertical TEC, gradient, 2nd order
 19: double teEE,    // vertical TEC, gradient, 2nd order
 20: double teNE,    // vertical TEC, gradient, 2nd order
 21: optional double d0, // P4 system (not used)
 22: optional double dN, // P4 system (not used)
 23: optional double dE, // P4 system (not used)
}

enum SolutionQuality {
  NoData=0,       // No data for receiver for the epoch. In network mode, may also mean
                  // unable to create a baseline to a fixed station.
  LowNsv=1,       // Not enough SVs or solution has very high formal error
  NoCorrection=2, // Have a solution but no correction values (legacy from P4)
  HighRMS=3,      // Have a solution with correction values, but the RMS is high
  LowRMS=4,       // Have a solution with correction values with a good RMS
}

struct StationPosition {
   1: double x,    // x position (m) (a-priori + estimate)
   2: double xe,   // x error (m)
   3: double y,    // y position (m) (a-priori + estimate)
   4: double ye,   // y error (m)
   5: double z,    // z position (m) (a-priori + estimate)
   6: double ze,   // z error (m)
   7: double eh,   // ellipsoid height (m)
   8: double ehe,  // ellipsoid height error (m)
   9: double dn,   // north position (m)
  10: double dne,  // north position error (m)
  11: double de,   // east position (m)
  12: double dee,  // east position error (m)
  13: double dh,   // height position (m)
  14: double dhe,  // height position error (m)
  15: double GDOP, // (m)
  16: double PDOP, // (m)
  17: byte  nsv_gps_used, // number of GPS satellites used in position calculation
  18: byte  nsv_glonass_used, // number of GLONASS satellites used in position calculation
  19: SolutionQuality qFlag, // flag used to estimate quality of solution
}

struct SatelliteEleAzi {
  1: byte ID, // satellite PRN/slot number
  2: ConstellationType constellation,
  3: double elevation, // elevation in degrees
  4: double azimuth    // azimuth in degrees
}

enum LinearCombination {
  L1   =  1,
  L2   =  2,
  L3   =  3,
  L1L2 =  4,
  IF   =  5,
  GF   =  6,
  MW   =  7,
  MWIF =  8,
  PL   =  9, // (P1+L1)/2 (ionospheric-free)
  WL   = 10,
}

enum ProcessedLC {
  Gp1, Gp2, Gp5,           GpIF, GpGF, GpWL, GpMW, // GPS Phase
  Ep1, Ep5, Ep6, Ep7, Ep8, EpIF, EpGF, EpWL, EpMW, // Galileo Phase
  Rp1, Rp2,                RpIF, RpGF, RpWL, RpMW, // Glonass Phase
  Gc1, Gc2, Gc5,           GcIF, GcGF,             // GPS Code
  Ec1, Ec5, Ec6, Ec7, Ec8, EcIF, EcGF,             // Galileo Code
  Rc1, Rc2,                RcIF, RcGF,             // Glonass Code
  Gd1, Gd2,                                        // GPS Doppler
  GpPL,                                            // GPS (P1+L1)/2
  RpPL,                                            // GLONASS (P1+L1)/2
  invalidLC
}

struct ObservationMarkCount {
  1: byte n_mark_none, //  0 (not marked)
  2: byte n_mark_mis,  //  1 (missing marked -- if processing dual-frequency data, marked if one frequency is missing)
  3: byte n_mark_ele,  //  2 (elevation marked)
  4: byte n_mark_cor,  //  3 (DGPS correction marked)
  5: byte n_mark_orb,  //  4 (orbit marked)
  6: byte n_mark_unp,  //  5 (unpaired observable between LC1 and LC2)
  7: byte n_mark_ca,   //  6 (CA marked)
  8: byte n_mark_out,  //  7 (outlier)
  9: byte n_mark_crx,  //  8 (SATCRUX marked)
 10: byte n_mark_iod,  //  9 (not used by current version of rtnet)
 11: byte n_mark_dcb,  // 10 (satellite DCB marked)
 12: byte n_mark_few,  // 11 (not used by current version of rtnet)
 13: byte n_mark_blu,  // 12 (blunder marked -- something to do with outlier detection. Perhaps if set, internal logic error in rtnet?)
 14: byte n_mark_ban,  // 13 (not enough satellites for Bancroft solution)
 15: byte n_mark_l1m,  // 14 (L1 missing)
 16: byte n_mark_l1i,  // 15 (not used by current version of rtnet)
 17: byte n_mark_ion,  // 16 (ionospheric delay marked)
 18: byte n_mark_trp,  // 17 (troposphere marked)
 19: byte n_mark_glo,  // 18 (GLONASS phase marked because channel number is unknown)
 20: byte n_mark_exc,  // 19 (satellite excluded from processing by user or because satellite is unhealthy)
 21: byte n_mark_hlt,  // 20 (not used by current version of rtnet)
 22: byte n_mark_old,  // 21 (marked because orbit is too old)
 23: byte n_mark_rho,  // 22 (computing range to satellite failed--solution did not converge)
 24: byte n_mark_oth,  // 23 (other marked)
}

struct SvResidualInfo {
  1: byte ID, // satellite PRN/slot number
  2: ConstellationType constellation,
  3: double residual, // meters
}

struct ResidualInfo {
  1: ProcessedLC lc,
  2: double meanResid,
  3: double pv,
  4: list<SvResidualInfo> svResid,
}

struct StationResults {
  1: string stationName,
  2: LinearCombination lc, // linear combination of solution
  // number of observations
  3: byte n_p1, // # of P1 observations
  4: byte n_l1, // # of L1 observations
  5: byte n_p2, // # of P2 observations
  6: byte n_l2, // # of L2 observations
  7: byte n_g1, // # of G1 observations (GLONASS P1)
  8: byte n_g2, // # of G2 observations (GLONASS P2)
  9: list<SatelliteEleAzi> svPosList, // list of satellites used in calculation
 10: double rcv_clk,  // receiver clock (m)
 11: double rcv_clke, // receiver clock error (m)
 12: double GDOP,     // GDOP obtained with satellites used in processing
 13: double PDOP,     // PDOP obtained with satellites used in processing
 // number of observations for each mark (including mark_none, ie, not marked)
 14: ObservationMarkCount n_mark_p1,
 15: ObservationMarkCount n_mark_l1,
 16: ObservationMarkCount n_mark_p2,
 17: ObservationMarkCount n_mark_l2,
 18: ObservationMarkCount n_mark_l3,
 // station position and estimates
 19: optional StationPosition filtered_result,    // primary position solution (optional because station could be fixed)
 20: optional StationPosition se_result, // single epoch result
 // troposphere data
 21: optional double ztd,  // zenith delay (m)
 22: optional double ztde, // zenith delay error (m)
 23: list<ResidualInfo> residList, // residual information for each LC
}
// satellite elevation and azimuth (list)

enum MissingEpochData
{
  NoError                       = 0, // solution was generated
  MissingAllStationObservation  = 1, // no station observations available
  MissingOrbit                  = 2, // no orbit information available
  MissingCorrection             = 3, // correction is unavailable
  KalmanFilterFailed            = 4, // Kalman filter step failed
}

enum MissingStationData
{
  NoError                    = 0, // station could be processed
  StationFixed               = 1, // no position result because station is fixed
  MissingStationObservation  = 2, // no observations available for station
  TooFewBancroft             = 3, // not enough satellites to generate Bancroft solution
  MissingStationInfo         = 4, // station missing from coordinate file
  MissingAntennaInfo         = 5, // station's antenna not present in ATX file
                                  // (if this is an immediate error by rtnet, this flag isn't needed)
  StationUserMarked          = 6, // station was removed from processing by user
}

struct RtnetEpoch {
  1: i16 mjlDay,  // MJL day of epoch
  2: i32 msecDay, // time of day (msec)
  3: list<StationResults> stationResultList, // set for each linear combination
  4: MissingEpochData missing, // set to NoError if data is present,
                               // otherwise solution could not be generated because of specified reason
  5: map<string,MissingStationData> dataAvailableList, // flag for each station in STALIST
  6: i32 processTime, // process time (dt), msec
  7: i32 compTime,    // computation time (dt_proc), msec
  8: i32 latency,     // latency, msec
}

// Needed for plots
//Per station data:
//X Troposphere: quality flag, ztda, ztd, ztde
//X Number of observations per station (ie, # P1, # L1, # L2, # G1, etc)
//X GDOP, PDOP, # sv_used
//X mres, pv, res_code_flag
//X x, xi, xe, y, yi, ye, z, zi, ze, dn, dne, de, dee, dh, dhe
//X receiver clk, clki, clke
//List of satellites used
//# new ambiguities, # clock resets, # L1 ambiguity resets

//# of each mark flag for P1, L1 and L2
//-- so perhaps 10 mark 0s for P1, 2 mark 3s for P1, 10 mark 0s for L1, etc.

// other ideas:
// Send basic input file information or entire input file.
// Send current file used for orbits (or host/port if using external orbits).


//----
//Data latency per epoch. Processing time per step.
//slant delay (maybe same as mres) (per station, per satellite, per LC, per epoch)

