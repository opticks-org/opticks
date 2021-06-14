#ifndef ossimQuickbirdRpcHeader_HEADER
#define ossimQuickbirdRpcHeader_HEADER

#include <iostream>
#include <ossim/base/ossimErrorStatusInterface.h>
#include <ossim/base/ossimFilename.h>

class OSSIM_DLL ossimQuickbirdRpcHeader : public ossimErrorStatusInterface
{
public:
  friend OSSIM_DLL std::ostream& operator << (std::ostream& out,
			       const ossimQuickbirdRpcHeader& data);
  ossimQuickbirdRpcHeader();

  bool open(const ossimFilename& file);

  bool isAPolynomial()const
  {
    return theSpecId.contains("A");
  }
  bool isBPolynomial()const
  {
    return theSpecId.contains("B");
  }
  bool isQuickbird() const
  {
    return theSatId.upcase().contains("QB");
  }
  bool isWorldView() const
  {
    return theSatId.upcase().contains("WV");
  }
  bool isWorldView1() const
  {
    return theSatId.upcase().contains("WV01");
  }
  bool isWorldView2() const
  {
    return theSatId.upcase().contains("WV02");
  }
  bool isWorldView3() const
  {
    return theSatId.upcase().contains("WV03");
  }
  bool isWorldView4() const
  {
    return theSatId.upcase().contains("WV04");
  }
  bool isGlobal()const;
  
  bool parseXml();

  ossimFilename theFilename;

  ossimString theSatId;
  ossimString theBandId;
  ossimString theSpecId;
  double    theErrBias;
  double    theErrRand;
  ossim_int32 theLineOffset;
  ossim_int32 theSampOffset;
  double    theLatOffset;
  double    theLonOffset;
  double    theHeightOffset;
  double    theLineScale;
  double    theSampScale;
  double    theLatScale;
  double    theLonScale;
  double    theHeightScale;
 
  std::vector<double> theLineNumCoeff;
  std::vector<double> theLineDenCoeff;
  std::vector<double> theSampNumCoeff;
  std::vector<double> theSampDenCoeff;


  bool readCoeff(std::istream& in,
		 std::vector<double>& coeff);
  bool parseNameValue(const ossimString& line);
		     
};

#endif
