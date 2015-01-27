#ifndef FILE_AROME_H
#define FILE_AROME_H
#include <netcdf.hh>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include "Netcdf.h"
#include "../Variable.h"

//! Represents a Netcdf data file
class FileArome : public FileNetcdf {
   public:
      FileArome(std::string iFilename, bool iReadOnly=false);
      ~FileArome();

      // Dimension sizes
      int getNumLat() const {return mNLat;};
      int getNumLon() const {return mNLon;};
      int getNumEns() const {return 1;};
      int getNumTime() const {return mNTime;};

      std::string getVariableName(Variable::Type iVariable) const;
      vec2 getLats() const;
      vec2 getLons() const;
      vec2 getElevs() const;
      int  getDate() const;
      // Is the file readable in this format?
      static bool isValid(std::string iFilename);
      std::string name() const {return "arome";};
   protected:
      void writeCore(std::vector<Variable::Type> iVariables);
      FieldPtr getFieldCore(Variable::Type iVariable, int iTime) const;
      vec2 mLats;
      vec2 mLons;
      vec2 mElevs;

      std::vector<int> mTimes;
      vec2 getLatLonVariable(std::string iVariable) const;

      int mNTime;
      int mNLat;
      int mNLon;
      int mDate;
};
#endif