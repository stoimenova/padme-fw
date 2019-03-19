#ifndef ECalAnalysis_h
#define ECalAnalysis_h 1

#include "TObject.h"
#include "ValidationBase.hh"


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class TECalRecoEvent;
class TRecoVClusCollection;

class ECalAnalysis : public ValidationBase
{

public:
  ECalAnalysis();
  ECalAnalysis(Int_t  Validation, Int_t verb);
  ~ECalAnalysis();

  Bool_t Init(TECalRecoEvent* ev, TRecoVClusCollection* cl);
  Bool_t Process();
  Bool_t ProcessValidation();
  Bool_t InitHistos();
  Bool_t InitHistosValidation();
  Bool_t Finalize(){return true;}


private:

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
