#include "TopQuarkAnalysis/TopRefTuple/interface/fTypes.h"

std::map<std::string,fTypes::LEAFTYPE> fTypes::dict() {
  std::map<std::string,LEAFTYPE> dict;
  
  dict["bool"]      = BOOL;       dict["bools"]     = BOOL_V;
  dict["short int"] = SHORT;      dict["shorts"]    = SHORT_V;
  dict["ushort int"]= U_SHORT;    dict["ushorts"]   = U_SHORT_V;
  dict["int"]       = INT;        dict["ints"]      = INT_V;
  dict["uint"]      = U_INT;      dict["uints"]     = U_INT_V;
  dict["float"]     = FLOAT;      dict["floats"]    = FLOAT_V;
  dict["double"]    = DOUBLE;     dict["doubles"]   = DOUBLE_V;
  dict["lint"]      = LONG;       dict["longs"]     = LONG_V;
  dict["ulint"]     = U_LONG;     dict["ulongs"]    = U_LONG_V;
  dict["String"] = STRING;
  dict["Stringboolstdmap"] = STRING_BOOL_M;
  dict["Stringintstdmap"]  = STRING_INT_M;

  dict["doubleROOTMathPxPyPzE4DROOTMathLorentzVector"] = LORENTZVD;
  dict["doubleROOTMathPxPyPzE4DROOTMathLorentzVectors"] = LORENTZVD_V;

  dict["doubleROOTMathPtEtaPhiM4DROOTMathLorentzVector"] = LORENTZVP;
  dict["doubleROOTMathPtEtaPhiM4DROOTMathLorentzVectors"] = LORENTZVP_V;

  dict["doubleROOTMathCartesian3DROOTMathDefaultCoordinateSystemTagROOTMathPositionVector3D"] = POINTD;
  dict["doubleROOTMathCartesian3DROOTMathDefaultCoordinateSystemTagROOTMathPositionVector3Ds"] = POINTD_V;

  dict["doubleROOTMathCartesian3DROOTMathDefaultCoordinateSystemTagROOTMathDisplacementVector3D"] = VECTORD;
  dict["doubleROOTMathCartesian3DROOTMathDefaultCoordinateSystemTagROOTMathDisplacementVector3Ds"] = VECTORD_V;

  return dict;
}
