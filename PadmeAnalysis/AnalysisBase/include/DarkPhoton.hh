#ifndef DarkPhoton_h
#define DarkPhoton_h 1

#include "TObject.h"
#include "TString.h"


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class TVector3;
class TRecoEvent;
class TRecoVObject;
class TRecoVClusCollection;
class TTargetRecoBeam;

class DarkPhoton : public TObject
{

public:
  DarkPhoton();
  DarkPhoton(Int_t processingMode, Int_t verbosityFlag);
  ~DarkPhoton();
  virtual Bool_t Init(TRecoEvent* eventHeader, 
		      TRecoVObject* ECALev, TRecoVClusCollection* ECALcl,
		      TRecoVObject* PVetoev, TRecoVClusCollection* PVetocl,
		      TRecoVObject* EVetoev, TRecoVClusCollection* EVetocl,
		      TRecoVObject* HEPVetoev, TRecoVClusCollection* HEPVetocl,
		      TRecoVObject* SACev, TRecoVClusCollection* SACcl,
		      TRecoVObject* TARGETev, TTargetRecoBeam* TargetBeam);
  virtual Bool_t Finalize(){return true;}

  virtual Bool_t InitHistos();
  virtual Bool_t ProcessDarkPhoton(Bool_t isTargetOut, Bool_t externalPass, Bool_t makeClSelection ,std::vector<int> selCl);
  Double_t CalculateMomentumPositron(Double_t z,Double_t x);
  Bool_t phiSymmetricalInECal(TVector3 P1, TVector3 P2,  double& distPhi);
   
  Bool_t passPreselection(Bool_t isTargetOut, Bool_t isMC, Bool_t externalPass);
  Double_t MissingMass(TVector3 V, Double_t E);

  Bool_t passAnnPreSelection(Double_t clE, Double_t clTheta);
  Double_t extractEff(Double_t R, Double_t phi);

protected:
  TRecoEvent*           fRecoEvent;
  TRecoVObject*         fECal_hitEvent   ;
  TRecoVObject*         fPVeto_hitEvent  ;
  TRecoVObject*         fEVeto_hitEvent  ;
  TRecoVObject*         fSAC_hitEvent    ;
  TRecoVObject*         fTarget_hitEvent ;
  TRecoVObject*         fHEPVeto_hitEvent;

  TRecoVClusCollection* fECal_ClColl      ;
  TRecoVClusCollection* fPVeto_ClColl     ;
  TRecoVClusCollection* fEVeto_ClColl     ;
  TRecoVClusCollection* fSAC_ClColl       ;
  TTargetRecoBeam*      fTarget_RecoBeam  ;
  TRecoVClusCollection* fHEPVeto_ClColl   ;

  
  // fVerbose = 0 (minimal printout),  = 1 (info mode),  = 2 (debug quality)...
  TString  fAlgoName;
  Int_t    fVerbose;
  Int_t    fProcessingMode;

  Bool_t fInitToComplete;
  Double_t fdistanceTarget;//from calchep g4hits studies
  Double_t fFRmin;
  Double_t fFRmid;
  Double_t fFRmax;
  Double_t fEBeam;

  Bool_t   fPreSelectionAnnihilation;


};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif