#ifndef PPPCRDFILE_H
#define PPPCRDFILE_H

#include <string>
#include <vector>
#include "newmat.h"

namespace BNC_PPP {

class t_pppCrdFile {
 public:
  class t_staInfo {
   public:
    t_staInfo() {
      _xyz.ReSize(3);    _xyz    = 0.0;
      _neuAnt.ReSize(3); _neuAnt = 0.0;
    }
    std::string  _name;
    std::string  _antenna;
    std::string  _receiver;
    ColumnVector _xyz;
    ColumnVector _neuAnt;
  };

  static void readCrdFile(const std::string& fileName, std::vector<t_staInfo>& staInfoVec);
};

}

#endif
