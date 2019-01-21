// --------------------------------------------------------------
// History:
//
// Created by Emanuele Leonardi (emanuele.leonardi@roma1.infn.it) 2016-03-23
// Last updated by M. Raggi 10/11/2018
// --------------------------------------------------------------
#include "Riostream.h"

#include "SACReconstruction.hh"

#include "TSACMCEvent.hh"
#include "TSACMCHit.hh"
#include "TSACMCDigi.hh"
//#include "DigitizerChannelReco.hh"
#include "DigitizerChannelSAC.hh"
#include "TH2F.h"
#include "TH1F.h"
#include "TCanvas.h"


Int_t SACReconstruction::FindSeed(Int_t nele, Int_t * Used, Double_t* Ene) {
  
  Int_t iMax=-1;
  Double_t fMax=0;
  for(UShort_t i = 0;i<nele;i++){
    if (fMax < Ene[i] && Used[i]==0 && Ene[i]>20.) { //ene
      fMax = Ene[i];
      iMax = i;
    }
  }
  if(iMax>1024) std::cout<<"N hits "<<nele<<" "<<iMax<<" ene "<<fMax<<std::endl;
  return iMax;
}

Int_t SACReconstruction::IsSeedNeig(Int_t seedID, Int_t cellID) {
  //uses cellID to find neig cells wrt seed of the cluster
  Int_t IsNeig=-1;
  Int_t SeedRow=seedID/10;
  Int_t SeedCol=seedID%10;

  Int_t CellRow=cellID/10;
  Int_t CellCol=cellID%10;
  //excludes the seed cell 
  if( abs(SeedRow-CellRow)<=1 && abs(SeedCol-CellCol)<=1) IsNeig=1;
  //  std::cout<<"seedID "<<seedID<<" cellID "<<cellID<<" Is Neig "<<IsNeig<<std::endl;
  return IsNeig;
}

void SACReconstruction::ProcessEvent(TRawEvent* rawEv){
  //  std::cout<<"processo nel SAC "<<std::endl;
  // If this is not a channel 
  // No idea what to do with it...
  if (!fChannelReco) return;

  //Perform some cleaning before:
  vector<TRecoVHit *> &Hits  = GetRecoHits();
  for(unsigned int iHit = 0;iHit < Hits.size();iHit++){
    delete Hits[iHit];
  }

  Hits.clear();

  UChar_t nBoards = rawEv->GetNADCBoards();

  TADCBoard* ADC;
  //New M. Raggi
  UInt_t  TrigMask=rawEv->GetEventTrigMask();
  for(Int_t iBoard = 0; iBoard < nBoards; iBoard++) {
    ADC = rawEv->ADCBoard(iBoard);
    if(GetConfig()->BoardIsMine( ADC->GetBoardId())) {
      //Loop over the channels and perform reco
      for(unsigned ich = 0; ich < ADC->GetNADCChannels();ich++) {
	TADCChannel* chn = ADC->ADCChannel(ich);
	fChannelReco->SetDigis(chn->GetNSamples(),chn->GetSamplesArray());
	//New M. Raggi
	Int_t ChID =GetChannelID(ADC->GetBoardId(),chn->GetChannelNumber());
	
	((DigitizerChannelSAC*)fChannelReco)->SetChID(ChID);
	((DigitizerChannelSAC*)fChannelReco)->SetTrigMask(TrigMask);
	unsigned int nHitsBefore = Hits.size();
	fChannelReco->Reconstruct(Hits);
	unsigned int nHitsAfter = Hits.size();
	for(unsigned int iHit = nHitsBefore; iHit < nHitsAfter;++iHit) {
	  Hits[iHit]->SetChannelId(GetChannelID(ADC->GetBoardId(),chn->GetChannelNumber()));
	}
      }
    } else {
    }
  }
   
  if(fChannelCalibration) fChannelCalibration->PerformCalibration(Hits);
  //Processing is over, let's analyze what's here, if foreseen
  AnalyzeEvent(rawEv);
}

SACReconstruction::SACReconstruction(TFile* HistoFile, TString ConfigFileName)
  : PadmeVReconstruction(HistoFile, "SAC", ConfigFileName)
{
  //  fRecoEvent = new TRecoSACEvent();
  ParseConfFile(ConfigFileName);
  //fChannelReco = new DigitizerChannelReco();
  fChannelReco = new DigitizerChannelSAC(); //produce hits array
}

void SACReconstruction::HistoInit(){
  AddHisto("SACOccupancy",new TH2F("SACOccupancy","SACOccupancy",5,0,5,5,0,5));
  AddHisto("SACOccupancy_last",new TH2F("SACOccupancy_last","SACOccupancy_last",5,0,5,5,0,5));
  //  AddHisto("SACCharge",new TH2F("SACCharge","SACCharge",5,0,5,5,0,5));
  AddHisto("SACClPos",new TH2F("SACClPos","SACClPos",400,-6,6,400,-6,6));
  AddHisto("SACVoters",new TH2F("SACVoters","SACVoters",1000,0.,1000.,26,-0.5,25.5));

  AddHisto("SACTime",new TH1F("SACTime","SACTime",200,0,400));
  AddHisto("SACTimeCut",new TH1F("SACTimeCut","SACTimeCut",200,0,400));
  AddHisto("SACClTime",new TH1F("SACClTime","SACClTime",200,0,400));
  AddHisto("SACClTimeCut",new TH1F("SACClTimeCut","SACClTimeCut",200,0,400));
  AddHisto("SACETot",new TH1F("SACETot","SACETot",500,0,2500));
  AddHisto("SACQTot",new TH1F("SACQTot","SACQTot",500,0,2500));
  AddHisto("SACNPart",new TH1F("SACNPart","SACNPart",200,0,200));
  AddHisto("SACNClus",new TH1F("SACNClus","SACNClus",200,0,200));

  AddHisto("SACDropHitE",new TH1F("SACDropHitE","SACDropHitE",500,0,50));
  AddHisto("SACHitE",new TH1F("SACHitE","SACHitE",550,0,550));

  AddHisto("SACRawClus",new TH1F("SACRawClus","SACRawClus",200,0,1000));
  AddHisto("SACClE",new TH1F("SACClE","SACClE",200,0,1000));

  AddHisto("SACClE2",new TH1F("SACClE2","SACClE2",200,0,1000));
  
  AddHisto("SACClNCry",new TH1F("SACClNCry","SACClNCry",30,0,30));
  AddHisto("SACClTDiff",new TH1F("SACClTDiff","SACClTDiff",100,-5,5));

  AddHisto("SACClSeed",new TH1F("SACClSeed","SACClSeed",50,0,50));
  AddHisto("SACClSeedEn",new TH1F("SACClSeedEn","SACClSeedEn",550,0,550));

  AddHisto("SACNeig",new TH1F("SACNeig","SACNeig",9,-4.5,4.5));

  //Waveform histograms
  for(int iCh=0; iCh<25 ; iCh++){
    char iName[100];
    sprintf(iName,"SACCh%d",iCh);
    AddHisto(iName, new TH1F(iName, iName,1024,0.,200. ));
    //    (GetHisto(iName))->GetXaxis()->SetTitle("Sampling #");
    //    (GetHisto(iName))->GetYaxis()->SetTitle("Amplitude[V]");
    //
    //    sprintf(iName,"SACLastCh%d",iCh);
    //    AddHisto(iName, new TH1F(iName, iName,  1024,  0, 1024 ));
    //    (GetHisto(iName))->GetXaxis()->SetTitle("Sampling #");
    //    (GetHisto(iName))->GetYaxis()->SetTitle("Amplitude[V]");
    //
    ////    sprintf(iName,"ESACLastCh%d",iCh);
    ////    AddHisto(iName, new TH1F(iName, iName,  200,  0, 1000 ));
    ////    (GetHisto(iName))->GetXaxis()->SetTitle("Energy MeV");
  }
}


SACReconstruction::~SACReconstruction()
{;}

// void SACReconstruction::Init(PadmeVReconstruction* MainReco)
// {

//   //common part for all the subdetectors
//   PadmeVReconstruction::Init(MainReco);

// }

// // Read SAC reconstruction parameters from a configuration file
// void SACReconstruction::ParseConfFile(TString ConfFileName) {

//   std::ifstream confFile(ConfFileName.Data());
//   if (!confFile.is_open()) {
//     perror(ConfFileName);
//     exit(1);
//   }

//   TString Line;
//   while (Line.ReadLine(confFile)) {
//     if (Line.BeginsWith("#")) continue;
//   }
//   confFile.close();
// }

/*
TRecoVEvent * SACReconstruction::ProcessEvent(TDetectorVEvent* tEvent, Event* tGenEvent)
{
  //common part for all the subdetectors 
  PadmeVReconstruction::ProcessEvent(tEvent, tGenEvent);

  TSACEvent* tSACEvent = (TSACEvent*)tEvent;
  const TClonesArray& Digis = (*(tSACEvent->GetHits()));
  return fRecoEvent;
}
*/
void SACReconstruction::ProcessEvent(TMCVEvent* tEvent, TMCEvent* tMCEvent)
{
  PadmeVReconstruction::ProcessEvent(tEvent,tMCEvent);
  TSACMCEvent* tSACEvent = (TSACMCEvent*)tEvent;
  std::cout << "--- SACReconstruction --- run/event/#hits/#digi " << tSACEvent->GetRunNumber() << " " << tSACEvent->GetEventNumber() << " " << tSACEvent->GetNHits() << " " << tSACEvent->GetNDigi() << std::endl;
  for (Int_t iH=0; iH<tSACEvent->GetNHits(); iH++) {
    TSACMCHit* hit = (TSACMCHit*)tSACEvent->Hit(iH);
    hit->Print();
  }
  for (Int_t iD=0; iD<tSACEvent->GetNDigi(); iD++) {
    TSACMCDigi* digi = (TSACMCDigi*)tSACEvent->Digi(iD);
  //------
    Int_t ch = digi->GetChannelId();
    Int_t iX = ch/10;
    Int_t iY = ch%10;
    cout<<"X "<<iX<<" Y "<<iY<<endl;
    //------
    digi->Print();
  }
}

// void SACReconstruction::EndProcessing()
// {;}

//  Last revised by M. Raggi 16/11/2018 puoi levare rawEvent
int SACReconstruction::SACBuildClusters(TRawEvent* rawEv){

  const int NMaxCl=100;
  const int NRows=5;
  const int NCols=5;
  const int NTotCh=NRows*NCols;
  double TTotSACCh[NTotCh][NMaxCl];
  double ETotSACCh[NTotCh][NMaxCl];
  Double_t Time=0;
  Double_t SACEtot=0;
  SdEn.clear();
  SdTime.clear();
  SdCell.clear();

  vector<TRecoVHit *> &Hits  = GetRecoHits();
  int NCry=0;
  if(Hits.size()==0){
    //    std::cout<<"No hits !!!!"<<std::endl;
    return -1;
  }
  int NClus =1;
  int NSeeds=0;
  int NGoodClus=0; 
  //  std::cout<<"Ci sono No hits !!!!"<<Hits.size()<<std::endl;
  for(Int_t ll=0;ll<NMaxCl;ll++){ 
    //    ETotSAC[ll]=0;
    for(Int_t kk=0;kk<NTotCh;kk++){ 
      ETotSACCh[kk][ll]=0.;
      TTotSACCh[kk][ll]=0.;
      //      XSACCh[kk][ll]=0.;
      //      YSACCh[kk][ll]=0.;
    }
  }

  Double_t cTime[Hits.size()]={0.};
  Double_t cEnergy[Hits.size()]={0.};
  Int_t cChID[Hits.size()]={0};
  Int_t cUsed[Hits.size()]={0};
  Int_t cCellUsed[50]={0};
  for(Int_t mm=0;mm<50;mm++) cCellUsed[mm]=0;
  //fill the vector with hits informations
  for(unsigned int iHit1 =  0; iHit1 < Hits.size(); ++iHit1) {
    cUsed[iHit1]  = {0};
    cTime[iHit1]  = Hits[iHit1]->GetTime();
    cEnergy[iHit1]= Hits[iHit1]->GetEnergy();
    cChID[iHit1]  = Hits[iHit1]->GetChannelId();
  }  

  Int_t iMax=0;
  Int_t HitUsed=0;
  Double_t LastCell;
  while(iMax>-1){
    iMax = FindSeed(Hits.size(),cUsed, cEnergy);
    if(iMax<0) break;
    NCry=0;
    //    std::cout<<"Imax "<<iMax<<" "<<cEnergy[iMax]<<" "<<NSeeds<<std::endl;
    //  if(NSeeds==0){
    SdTime.push_back(cTime[iMax]);    
    SdEn.push_back(cEnergy[iMax]);    
    SdCell.push_back(cChID[iMax]);    
    GetHisto("SACClSeed")->Fill(SdCell[NSeeds]);
    GetHisto("SACClSeedEn")->Fill(SdEn[NSeeds]);

    //create the cluster structure
    ClE.push_back(0.);
    ClTime.push_back(0.); //use seed time for the moment
    ClSeed.push_back(iMax);
     
    ClX.push_back(0.);  //carbon copy of the occupancy
    ClY.push_back(0.);  //carbon copy of the occupancy

    // we may want to use the space coordinates in this loop.
    for(unsigned int iHit1 =  0; iHit1 < Hits.size(); ++iHit1) {
      //      if( fabs(cTime[iHit1]-SdTime[NSeeds])<1.5 && cUsed[iHit1]==0 && IsSeedNeig(SdCell[NSeeds],cChID[iHit1])==1){
      if(fabs(cTime[iHit1]-SdTime[NSeeds])<1.5 && cUsed[iHit1]==0 && IsSeedNeig(SdCell[NSeeds],cChID[iHit1])==1 && cEnergy[iHit1]>5.){ //taglio in E
      //      if(fabs(cTime[iHit1]-SdTime[NSeeds])<3.5 && cUsed[iHit1]==0 ){
      	//	std::cout<<"is neig "<<pippo<<std::endl;
	Double_t XCl=(60.-cChID[iHit1]/10*30.);
	Double_t YCl=(-60.+cChID[iHit1]%10*30.);
	cUsed[iHit1]=1;
	ClTime[NSeeds]+=cTime[iHit1];
	ClE[NSeeds]+=cEnergy[iHit1];
	ClX[NSeeds]+=XCl*cEnergy[iHit1];
	ClY[NSeeds]+=YCl*cEnergy[iHit1];
	NCry++;
	HitUsed++;
	cCellUsed[cChID[iHit1]]=1;
      }
    }
    ClNCry.push_back(NCry);
    //    GetHisto("SACClE")->Fill(ClE[NSeeds]);
    ClTime[NSeeds]=ClTime[NSeeds]/NCry;  //average time of the hit
    ClX[NSeeds]=ClX[NSeeds]/ClE[NSeeds];
    ClY[NSeeds]=ClY[NSeeds]/ClE[NSeeds];
    GetHisto("SACClTDiff")->Fill(ClTime[NSeeds]-SdTime[NSeeds]);
    if(ClNCry[NSeeds]>9) std::cout<<NSeeds<<" Ncry >9 "<<NCry<<" Clseed "<<ClSeed[NSeeds]<<" ClE "<<ClE[NSeeds]<<std::endl;
    NSeeds++;
    if(NCry>1 || ClE[NSeeds]>10.) NGoodClus++;
  }  //end of while loop on seeds
  
  for(unsigned int iHit1 =  0; iHit1 < Hits.size(); ++iHit1) {
    //	 SACDropHitE
    SACEtot+=cEnergy[iHit1];
    GetHisto("SACHitE")->Fill(cEnergy[iHit1]);
    if(cUsed[iHit1]==0){ 
      GetHisto("SACDropHitE")->Fill(cEnergy[iHit1]);
    }
  }
  GetHisto("SACETot")->Fill(SACEtot);
  return NGoodClus;
}

void SACReconstruction::AnalyzeEvent(TRawEvent* rawEv){
  static int nevt;
  static TCanvas c;
 
  vector<TRecoVHit *> &Hits  = GetRecoHits();
  //  return;
  Double_t Energy=0;
  Double_t ECh[25]={0};
  Double_t Time=0;
  Double_t votes[1024]={0};
  Double_t ESums[1024]={0};
  Int_t SACNPart=0;
  GetHisto("SACOccupancy_last") -> Reset();
  for(unsigned int iHit1 =  0; iHit1 < Hits.size(); ++iHit1) {
    Time=Hits[iHit1]->GetTime();
    int ich  = Hits[iHit1]->GetChannelId();
    int ElCh = ich/10*5 +ich%5;
    Energy  += Hits[iHit1]->GetEnergy(); //SAC total energy
    ECh[ElCh]+= Hits[iHit1]->GetEnergy(); //SAC total energy
    GetHisto("SACOccupancy") -> Fill(4.5-(ich/10),0.5+ich%10); /* inserted 4.5- to swap PG */
    GetHisto("SACOccupancy_last") -> Fill(4.5-(ich/10),0.5+ich%10); /* inserted 4.5- to swap PG */
    //GetHisto("SACOccupancy") -> Fill(ich/10-2,ich%10+2);
    //GetHisto("SACOccupancy") -> Fill(-(ich%5-4),ich/5);
    GetHisto("SACTime") -> Fill(Time);
    GetHisto("SACVoters")->Fill((double)Time,(double)ElCh);
    //    std::cout<<"Energy "<<Hits[iHit1]->GetEnergy()<<" Time "<<Time<<" ich "<<ich<<std::endl;
    votes[(Int_t)Time]++;
    ESums[(Int_t)Time]+=Hits[iHit1]->GetEnergy();
  }
  GetHisto("SACQTot") -> Fill(Energy);
  for(int hh=0;hh<1000;hh++){
    if(votes[hh]>1.){ 
      //  std::cout<<" "<<hh<<" "<<votes[hh]<<std::endl;
      GetHisto("SACTimeCut")->Fill(hh);
      SACNPart++;
    }
    if(ESums[hh]>5.){ //nominal 30 MeV cut assuming 5.E5 GAIN and 2.pe/MeV
      //      std::cout<<" "<<hh<<" "<<votes[hh]<<std::endl;
      GetHisto("SACRawClus")->Fill(ESums[hh]);
    }
  }

  ClNCry.clear();
  ClTime.clear();
  ClE.clear();
  ClX.clear();
  ClY.clear();
  ClSeed.clear();
  SdTime.clear();
  SdEn.clear();
  SdCell.clear();
  EvTotE=0;
  //  std::cout<<"Builing clusters"<<std::endl;
  int NClusters = SACBuildClusters(rawEv);
  GetHisto("SACNPart")->Fill(SACNPart);
  if(NClusters>0) GetHisto("SACNClus")->Fill(NClusters);
  //  GetHisto("SACETot") ->Fill(EvTotE);
  // Fill charge histograms
  char iNameCh[100];
  for(int iCh=0; iCh<25 ; iCh++){
    sprintf(iNameCh,"SACCh%d",iCh);
    GetHisto(iNameCh)->Fill(ECh[iCh]);
  }
  for(int gg=0;gg<NClusters;gg++){
    GetHisto("SACClE")->Fill(ClE[gg]); //fottuto
    if(gg>1) GetHisto("SACClE2")->Fill(ClE[gg]); //fottuto
    GetHisto("SACClNCry")->Fill(ClNCry[gg]);
    GetHisto("SACClTime")->Fill(ClTime[gg]);
    if(ClNCry[gg]>1 && ClE[gg]>10.) {
      GetHisto("SACClTimeCut")->Fill(ClTime[gg]);
      GetHisto("SACClPos")->Fill(ClX[gg],ClY[gg]);
    }
    //    std::cout<<gg<<" Ncry "<<ClNCry[gg]<<" "<<" "<<std::endl;
  }
  //  std::cout<<" "<<hh<<" "<<votes[hh]<<std::endl;

  if(nevt % 10 == 0)std::cout<<"Nev "<<nevt<<std::endl;
  if(nevt % 100 == 0) {
    c.cd();
    GetHisto("SACOccupancy") -> Draw();
    c.Update();
  }
  nevt ++;

  //Waveform histograms
//  char iName[1000];
//  UChar_t nBoards = rawEv->GetNADCBoards();
//  Double_t adc_count[1024][25]        ; 
//  Double_t adc_pedestal   [25]        ;
//  Double_t adc_chsum    [1024]        ; 
//  for(UShort_t s=0;s<1024;s++){  
//     adc_chsum    [s] = 0;
//  }
//  for(UChar_t b=0;b<nBoards;b++)// Loop over boards
//  {
//    TADCBoard* adcB = rawEv->ADCBoard(b);
//    if(adcB->GetBoardId()!=27)continue; //should correspond to SAC board ;
//    UChar_t nChn       = adcB ->GetNADCChannels(  );
//    
//
//   for(UChar_t c=0;c<nChn;c++){// Loop over channels
//	 TADCChannel* chn = adcB->ADCChannel(c);
//         UChar_t ch = chn->GetChannelNumber();
//  
//         for(UShort_t s=0;s<1024;s++){// 1-st loop over sampling          
//              adc_count[s][ch]= (Double_t) (chn->GetSample(s) ) ;
//              adc_chsum[s] += adc_count[s][ch]/16; 
//	 }//End 1-st loop over sampling
//
//         adc_pedestal[ch]=0; 
//         for(UShort_t s=0;s<100;s++){// 2-nd loop over sampling to calculate event by event pedestal                    
//              adc_pedestal[ch] += adc_count[s][ch]/100 ;
//	 }//End 2-nd loop over sampling
//         
//        
//         for(UShort_t s=0;s<1024;s++){// 3-rd loop over sampling to remove event by event pedestal and fill waveform  
//             adc_count[s][ch] = adc_count[s][ch] - adc_pedestal[ch]*0-3800;//Pedestal subtraction as digitization
//             float adc = float(adc_count[s][ch])/4096;        
//             sprintf(iName,"SACCh%d",ch);
//             (GetHisto(iName))->Fill(s,adc);        
//             sprintf(iName,"SACLastCh%d",ch);
//             (GetHisto(iName))->SetBinContent(s+1, fabs(adc));
//             (GetHistoi(Name))->SetBinError  (s+1, 1/4096);          
//	 } 
//    }// End loop over channels
//   }// End loop over boards
}



