#ifndef PRN_H
#define PRN_H

#include <string>

class t_prn {
public:
  static const unsigned MAXPRN_GPS = 32;
  static const unsigned MAXPRN_GLONASS = 26;
  static const unsigned MAXPRN_GALILEO = 36;
  static const unsigned MAXPRN_QZSS = 10;
  static const unsigned MAXPRN_SBAS = 38;
  static const unsigned MAXPRN_BDS = 65;
  static const unsigned MAXPRN_IRNSS = 7;
  static const unsigned MAXPRN = MAXPRN_GPS + MAXPRN_GLONASS + MAXPRN_GALILEO
      + MAXPRN_QZSS + MAXPRN_SBAS + MAXPRN_BDS + MAXPRN_IRNSS;

  t_prn() :
      _system('G'), _number(0), _flags(0) {
  }
  t_prn(char system, int number) :
      _system(system), _number(number), _flags(0) {
  }

  t_prn(char system, int number, int flags) :
      _system(system), _number(number), _flags(flags) {
  }

  ~t_prn() {
  }

  void set(char system, int number) {
    _system = system;
    _number = number;
    _flags  = 0;
  }

  void set(char system, int number, int flags) {
    _system = system;
    _number = number;
    _flags  = flags;
  }

  void setFlags(int flags) {
    _flags  = flags;
  }

  void set(const std::string& str);

  char system() const {
    return _system;
  }
  int number() const {
    return _number;
  }
  int flags() const {
    return _flags;
  }
  int toInt() const;
  std::string toString() const;
  std::string toInternalString() const;

  bool operator==(const t_prn& prn2) const {
    if (_system == prn2._system &&
        _number == prn2._number &&
        _flags  == prn2._flags) {
      return true;
    }
    else {
      return false;
    }
  }

  /**
   * Cleanup function resets all elements to initial state.
   */
  inline void clear(void) {
    _system = 'G';
    _number = 0;
    _flags = 0;
  }

  operator unsigned() const;

  friend std::istream& operator >>(std::istream& in, t_prn& prn);

private:
  char _system;
  int _number;
  int _flags;
};

std::istream& operator >>(std::istream& in, t_prn& prn);

#endif
