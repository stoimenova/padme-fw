#ifndef ECalSel_h
#define ECalSel_h 1

#include "TObject.h"
#include "TVector2.h"
#include "PadmeAnalysisEvent.hh"
#include "OfflineServer.hh"
#include "HistoSvc.hh"


using namespace std;
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class TRecoEvent;
class TRecoVObject;
class TRecoVClusCollection;

enum eventFlag{
  ev_gg  = 0,
  ev_ee  = 1,
  ev_ggg = 2,
  ev_eeg = 3
};

struct ECalSelEvent{
  eventFlag flagEv;
  int indexECal[3];
  double totalE;
  double avgT;
  TVector2 cog;
};



class ECalSel : public TObject
{

protected:
  ECalSel();

public:
  ~ECalSel();
  static ECalSel* GetInstance();

private:
  static ECalSel* fInstance;
  virtual Bool_t InitHistos();

public:
  virtual Bool_t Init(PadmeAnalysisEvent* event);
  Bool_t Finalize();
  virtual Bool_t Process();

  int getNECalEvents(){return fECalEvents.size();}
  ECalSelEvent getECalEvent(int i){return fECalEvents.at(i);} //to be protected

protected:
  TRecoEvent*           fRecoEvent;
  TRecoVObject*         fECal_hitEvent   ;
  TRecoVClusCollection* fECal_clEvent    ;

  // fVerbose = 0 (minimal printout),  = 1 (info mode),  = 2 (debug quality)...
  Int_t    fVerbose;

private:
  Double_t fdistanceTarget;
  Int_t TwoClusSel();
  std::vector<ECalSelEvent> fECalEvents;
  OfflineServer* fOfflineServerDB;
  Int_t fRunOld;
  HistoSvc* fhSvcVal; 

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif