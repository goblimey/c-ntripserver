/* -------------------------------------------------------------------------
 * BKG NTRIP Server 
 * -------------------------------------------------------------------------
 *
 * Class:      bnsSettings
 *
 * Purpose:    Subclasses the QSettings
 *
 * Author:     G. Weber
 *
 * Created:    27-Feb-2009
 *
 * Changes:    
 *
 * -----------------------------------------------------------------------*/

#include "bnssettings.h"
#include "bnsapp.h"

// Constructor
////////////////////////////////////////////////////////////////////////////
bnsSettings::bnsSettings() : 
  QSettings(((bnsApp*) qApp)->confFileName(), QSettings::IniFormat) {

  if (allKeys().size() == 0) {
    setValue("proxyHost",   "");
    setValue("proxyPort",   "");

    setValue("logFile",     "");
    setValue("fileAppend",  "0");
    setValue("autoStart",   "0");

    setValue("ephHost",     "");
    setValue("ephPort",     "");
    setValue("ephEcho",     "");

    setValue("clkPort",     "");
    setValue("inpEcho",     "");

    setValue("outHost1",    "");
    setValue("outPort1",    "");
    setValue("mountpoint_1","");
    setValue("password1",   "");
    setValue("refSys_1",    "IGS05");
    setValue("CoM_1",      "0");
    setValue("outFile_1",   "");

    setValue("outHost2",    "");
    setValue("outPort2",    "");
    setValue("mountpoint_2","");
    setValue("password2",   "");
    setValue("refSys_2",    "IGS05");
    setValue("CoM_2",      "0");
    setValue("outFile_2",   "");

    setValue("outHost3",    "");
    setValue("outPort3",    "");
    setValue("mountpoint_3","");
    setValue("password3",   "");
    setValue("refSys_3",    "IGS05");
    setValue("CoM_3",       "0");
    setValue("outFile_3",   "");

    setValue("outHost4",    "");
    setValue("outPort4",    "");
    setValue("mountpoint_4","");
    setValue("password4",   "");
    setValue("refSys_4",    "IGS05");
    setValue("CoM_4",       "0");
    setValue("outFile_4",   "");

    setValue("outHost5",    "");
    setValue("outPort5",    "");
    setValue("mountpoint_5","");
    setValue("password5",   "");
    setValue("refSys_5",    "IGS05");
    setValue("CoM_5",       "0");
    setValue("outFile_5",   "");

    setValue("outHost6",    "");
    setValue("outPort6",    "");
    setValue("mountpoint_6","");
    setValue("password6",   "");
    setValue("refSys_6",    "IGS05");
    setValue("CoM_6",       "0");
    setValue("outFile_6",   "");

    setValue("outHost7",    "");
    setValue("outPort7",    "");
    setValue("mountpoint_7","");
    setValue("password7",   "");
    setValue("refSys_7",    "IGS05");
    setValue("CoM_7",       "0");
    setValue("outFile_7",   "");

    setValue("outHost8",    "");
    setValue("outPort8",    "");
    setValue("mountpoint_8","");
    setValue("password8",   "");
    setValue("refSys_8",    "IGS05");
    setValue("CoM_8",       "0");
    setValue("outFile_8",   "");

    setValue("outHost9",    "");
    setValue("outPort9",    "");
    setValue("mountpoint_9","");
    setValue("password9",   "");
    setValue("refSys_9",    "IGS05");
    setValue("CoM_9",       "0");
    setValue("outFile_9",   "");

    setValue("outHost10",   "");
    setValue("outPort10",   "");
    setValue("mountpoint_10","");
    setValue("password10",  "");
    setValue("refSys_10",   "IGS05");
    setValue("CoM_10",      "0");
    setValue("outFile_10",  "");

    setValue("outHostEph",    "");
    setValue("outPortEph",    "");
    setValue("mountpoint_Eph","");
    setValue("passwordEph",   "");
    setValue("samplEph",      "5");

    setValue("trafo_dx",    "0.0541");
    setValue("trafo_dy",    "0.0502");
    setValue("trafo_dz",   "-0.0538");
    setValue("trafo_dxr",  "-0.0002");
    setValue("trafo_dyr",   "0.0001");
    setValue("trafo_dzr",  "-0.0018");
    setValue("trafo_ox",  "0.000891");
    setValue("trafo_oy",  "0.005390");
    setValue("trafo_oz", "-0.008712");
    setValue("trafo_oxr", "0.000081");
    setValue("trafo_oyr", "0.000490");
    setValue("trafo_ozr","-0.000792");
    setValue("trafo_sc",      "0.40");
    setValue("trafo_scr",     "0.08");
    setValue("trafo_t0",    "2000.0");

    setValue("rnxPath",     "");
    setValue("rnxIntr",     "1 min");
    setValue("rnxSampl",    "0");

    setValue("sp3Path",     "");
    setValue("sp3Intr",     "10 min");
    setValue("sp3Sampl",    "0");

    setValue("startTab",    "0");
    sync();
  }
}

