#ifndef TOPREF_FTYPES
#define TOPREF_FTYPES

#include <map>
#include <string>
#include "Math/LorentzVector.h"
#include "Math/PtEtaPhiE4D.h"
#include "Math/PtEtaPhiM4D.h"
#include "Math/Vector3D.h"
#include "Math/Point3D.h"

struct fTypes {

  typedef ROOT::Math::PositionVector3D<ROOT::Math::Cartesian3D<double> >      dPoint;
  typedef ROOT::Math::DisplacementVector3D<ROOT::Math::Cartesian3D<double> > dVector;
  typedef ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >     dXYZLorentzV;
  typedef ROOT::Math::LorentzVector<ROOT::Math::PtEtaPhiM4D<double> > dPolarLorentzV;

  typedef std::map<std::string,       bool> mapStringBool;
  typedef std::map<std::string,        int> mapStringInt;

  enum LEAFTYPE {BOOL=1,  BOOL_V,          
		 SHORT,   SHORT_V,           
		 U_SHORT, U_SHORT_V,       
		 INT,     INT_V,             
		 U_INT,   U_INT_V,
		 FLOAT,   FLOAT_V,           
		 DOUBLE,  DOUBLE_V,
		 LONG,    LONG_V,	     
		 U_LONG,  U_LONG_V,
		 LORENTZVD,   LORENTZVP,   
		 LORENTZVD_V, LORENTZVP_V, 
		 POINTD,   VECTORD,
		 POINTD_V, VECTORD_V,
		 STRING,     STRING_BOOL_M, STRING_INT_M };

  static std::map<std::string,LEAFTYPE> dict();
};

#endif
