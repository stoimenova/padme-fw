#include "AnnihilationSelectionDataMCMethod.hh"

#include "TRecoEvent.hh"
#include "TRecoVObject.hh"
#include "TRecoVHit.hh"
#include "TRecoVClusCollection.hh"
#include "TRecoVCluster.hh"
#include "TTargetRecoBeam.hh"
#include "HistoSvc.hh"
#include "TVector3.h"
#include "TLorentzVector.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "TFile.h"


AnnihilationSelectionDataMCMethod::AnnihilationSelectionDataMCMethod()
{
}

AnnihilationSelectionDataMCMethod::~AnnihilationSelectionDataMCMethod()
{
}


Bool_t AnnihilationSelectionDataMCMethod::Init( Double_t EffInnerRRange_r1inFR[8], Double_t EffOuterRRange_r1inFR[8], Double_t EffInnerRRange_r1r2inFR[8], Double_t EffOuterRRange_r1r2inFR[8]){
  
  for(int i=0; i<8; i++){
    fEffInnerRRange_r1inFR[i]  =EffInnerRRange_r1inFR  [i];
    fEffOuterRRange_r1inFR[i]  =EffOuterRRange_r1inFR  [i];
    fEffInnerRRange_r1r2inFR[i]=EffInnerRRange_r1r2inFR[i];
    fEffOuterRRange_r1r2inFR[i]=EffOuterRRange_r1r2inFR[i];
    //std::cout<<"In DatMcMethod " << fEffInnerRRange_r1inFR[i] << std::endl;
  }

  fFRmin=115.82;
  fFRmid=172.83;
  fFRmax=258.0;

  return true;
}

Bool_t AnnihilationSelectionDataMCMethod::Process(Double_t E1,Double_t phi1, Double_t R1,Double_t E2, Double_t phi2, Double_t R2){

  fE1=E1;	 
  fR1=R1;	 
  fphi1=phi1;
  fE2=E2;	 
  fR2=R2;	 
  fphi2=phi2;
  
  fdata=checkIfItIsDataOrMC(phi1, phi2);  //data==1 so data otherwise MC
  if(fdata)FillHistoData();
  else FillHistoMC();

  return true;
}

Int_t AnnihilationSelectionDataMCMethod::checkIfItIsDataOrMC(Double_t phi1,Double_t phi2){
  Int_t data=0;
  Double_t phiDeg1=phi1*360./(2.*TMath::Pi());
  if(phiDeg1<0.)phiDeg1=360+phiDeg1;
  Double_t phiDeg2=phi2*360./(2.*TMath::Pi());
  if(phiDeg2<0.)phiDeg2=360+phiDeg2;

  //if(fabs(phiDeg1-90)<45 || fabs(phiDeg1-270)<45)data=1;
  if(fabs(phiDeg1-135.)<45. || fabs(phiDeg1-315.)<45.)data=1;  //for data exercise
  return data;

}

void AnnihilationSelectionDataMCMethod::FillHistoData(){
  HistoSvc* hSvc =  HistoSvc::GetInstance();
  std::string hname;

  hname="MethodDataMC_Data";
  hSvc->FillHisto(hname, fE1+fE2, 1.);
  if(fR1>fFRmin && fR1< fFRmax){
    hname="MethodDataMC_Datag1inFR";
    hSvc->FillHisto(hname, fE1+fE2, 1.);
    FillWeightedHisto_R1inFR("DataSample");
    if(fR2>fFRmin && fR2< fFRmax){
      FillWeightedHisto_R1R2inFR("DataSample");
        hname="MethodDataMC_Datag1g2inFR";
	hSvc->FillHisto(hname, fE1+fE2, 1.);

    }
  }
}

void AnnihilationSelectionDataMCMethod::FillHistoMC(){
  HistoSvc* hSvc =  HistoSvc::GetInstance();
  std::string hname;

  hname="MethodDataMC_MC";
  hSvc->FillHisto(hname, fE1+fE2, 1.);
  if(fR1>fFRmin && fR1< fFRmax){
    hname="MethodDataMC_MCg1inFR";
    hSvc->FillHisto(hname, fE1+fE2, 1.);
    FillWeightedHisto_R1inFR("MCSample_usualW");
    if(fR2>fFRmin && fR2< fFRmax)FillWeightedHisto_R1R2inFR("MCSample_usualW");
    FillWeightedHisto_R1inFR_MC("MCSample_DataMCW");
    if(fR2>fFRmin && fR2< fFRmax){
      FillWeightedHisto_R1R2inFR_MC("MCSample_DataMCW");
      hname="MethodDataMC_MCg1g2inFR";
      hSvc->FillHisto(hname, fE1+fE2, 1.);
    }
  }

}


void AnnihilationSelectionDataMCMethod::FillWeightedHisto_R1inFR(std::string sufix){
  HistoSvc* hSvc =  HistoSvc::GetInstance();
  std::string hname;
  
  //Double_t effg1=ReturnEfficiencyR1R2inFR(fR1,fphi1);
  Double_t effg1=ReturnEfficiencyR1inFR(fR1,fphi1);
  Double_t effg2=ReturnEfficiencyR1inFR(fR2,fphi2);
  Double_t effScaleFactor=(1./effg1)*(1./effg2);
  hname="MethodDataMC_WEffR1inFR"+sufix;
  hSvc->FillHisto(hname,(fE1+fE2) ,effScaleFactor );
  Double_t phiDeg1=fphi1*360/(2*TMath::Pi());
  if(phiDeg1<0.)phiDeg1=360+phiDeg1;
  Double_t phiDeg2=fphi2*360/(2*TMath::Pi());
  if(phiDeg2<0.)phiDeg2=360+phiDeg2;
  if(fdata){
    hname="MethodDataMC_datasample_g1inFR_Effg1vsPhig1";
    hSvc->FillHisto2(hname,phiDeg1, effg1, 1.);
    hname="MethodDataMC_datasample_g1inFR_Effg2vsPhig2";
    hSvc->FillHisto2(hname,phiDeg2, effg2, 1.);
  }
  else {
    hname="MethodDataMC_mcsample_g1inFR_Effg1vsPhig1";
    hSvc->FillHisto2(hname,phiDeg1, effg1, 1.);
    hname="MethodDataMC_mcsample_g1inFR_Effg2vsPhig2";
    hSvc->FillHisto2(hname,phiDeg2, effg2, 1.);
  }

}


void AnnihilationSelectionDataMCMethod::FillWeightedHisto_R1R2inFR(std::string sufix){
  HistoSvc* hSvc =  HistoSvc::GetInstance();
  std::string hname;
  //Double_t effg1=ReturnEfficiencyR1R2inFR(fR1,fphi1);
  Double_t effg1=ReturnEfficiencyR1inFR(fR1,fphi1);
  Double_t effg2=ReturnEfficiencyR1R2inFR(fR2,fphi2);
  Double_t effScaleFactor=(1./effg1)*(1./effg2);
  hname="MethodDataMC_WEffR1R2inFR"+sufix;
  hSvc->FillHisto(hname,(fE1+fE2) ,effScaleFactor );
  Double_t phiDeg1=fphi1*360/(2*TMath::Pi());
  if(phiDeg1<0.)phiDeg1=360+phiDeg1;
  Double_t phiDeg2=fphi2*360/(2*TMath::Pi());
  if(phiDeg2<0.)phiDeg2=360+phiDeg2;
  if(fdata){
    hname="MethodDataMC_datasample_g1g2inFR_Effg1vsPhig1";
    hSvc->FillHisto2(hname,phiDeg1, effg1, 1.);
    hname="MethodDataMC_datasample_g1g2inFR_Effg2vsPhig2";
    hSvc->FillHisto2(hname,phiDeg2, effg2, 1.);
  }
  else {
    hname="MethodDataMC_mcsample_g1g2inFR_Effg1vsPhig1";
    hSvc->FillHisto2(hname,phiDeg1, effg1, 1.);
    hname="MethodDataMC_mcsample_g1g2inFR_Effg2vsPhig2";
    hSvc->FillHisto2(hname,phiDeg2, effg2, 1.);
  }
 
}

void AnnihilationSelectionDataMCMethod::FillWeightedHisto_R1inFR_MC(std::string sufix){
  HistoSvc* hSvc =  HistoSvc::GetInstance();
  std::string hname;
  
  //Double_t effg1=ReturnEfficiencyR1R2inFR_MC(fR1,fphi1);
  Double_t effg1=ReturnEfficiencyR1inFR_MC(fR1,fphi1);
  Double_t effg2=ReturnEfficiencyR1inFR_MC(fR2,fphi2);
  //Double_t effScaleFactor=(1./effg1)*(1./effg2);
  Double_t effScaleFactor=effg1*effg2;
  hname="MethodDataMC_WEffR1inFRDataPlus90"+sufix;
  hSvc->FillHisto(hname,(fE1+fE2) ,effScaleFactor );
  Double_t phiDeg1=fphi1*360/(2*TMath::Pi());
  if(phiDeg1<0.)phiDeg1=360+phiDeg1;
  Double_t phiDeg2=fphi2*360/(2*TMath::Pi());
  if(phiDeg2<0.)phiDeg2=360+phiDeg2;
  if(fdata){
    hname="MethodDataMC_datasample_g1inFR_Effg1vsPhig1_datamcMethod";
    hSvc->FillHisto2(hname,phiDeg1, effg1, 1.);
    hname="MethodDataMC_datasample_g1inFR_Effg2vsPhig2_datamcMethod";
    hSvc->FillHisto2(hname,phiDeg2, effg2, 1.);
  }
  else {
    hname="MethodDataMC_mcsample_g1inFR_Effg1vsPhig1_datamcMethod";
    hSvc->FillHisto2(hname,phiDeg1, effg1, 1.);
    hname="MethodDataMC_mcsample_g1inFR_Effg2vsPhig2_datamcMethod";
    hSvc->FillHisto2(hname,phiDeg2, effg2, 1.);
  }

}


void AnnihilationSelectionDataMCMethod::FillWeightedHisto_R1R2inFR_MC(std::string sufix){
  HistoSvc* hSvc =  HistoSvc::GetInstance();
  std::string hname;
  //Double_t effg1=ReturnEfficiencyR1R2inFR_MC(fR1,fphi1);
  Double_t effg1=ReturnEfficiencyR1inFR_MC(fR1,fphi1);
  Double_t effg2=ReturnEfficiencyR1R2inFR_MC(fR2,fphi2);
  //Double_t effScaleFactor=(1./effg1)*(1./effg2);
  Double_t effScaleFactor=effg1*effg2;
  hname="MethodDataMC_WEffR1R2inFRDataPlus90"+sufix;
  hSvc->FillHisto(hname,(fE1+fE2) ,effScaleFactor );
  Double_t phiDeg1=fphi1*360/(2*TMath::Pi());
  if(phiDeg1<0.)phiDeg1=360+phiDeg1;
  Double_t phiDeg2=fphi2*360/(2*TMath::Pi());
  if(phiDeg2<0.)phiDeg2=360+phiDeg2;
  if(fdata){
    hname="MethodDataMC_datasample_g1g2inFR_Effg1vsPhig1_datamcMethod";
    hSvc->FillHisto2(hname,phiDeg1, effg1, 1.);
    hname="MethodDataMC_datasample_g1g2inFR_Effg2vsPhig2_datamcMethod";
    hSvc->FillHisto2(hname,phiDeg2, effg2, 1.);
  }
  else {
    hname="MethodDataMC_mcsample_g1g2inFR_Effg1vsPhig1_datamcMethod";
    hSvc->FillHisto2(hname,phiDeg1, effg1, 1.);
    hname="MethodDataMC_mcsample_g1g2inFR_Effg2vsPhig2_datamcMethod";
    hSvc->FillHisto2(hname,phiDeg2, effg2, 1.);
  }
 
}


Double_t AnnihilationSelectionDataMCMethod::ReturnEfficiencyR1inFR(Double_t radius, Double_t phi){
  HistoSvc* hSvc =  HistoSvc::GetInstance();
   std::string hname;
  Bool_t inner=false;
  Bool_t outer=false;
  if(radius<fFRmid )inner=true;
  if(radius>=fFRmid)outer=true;
  Double_t phiDeg=phi*360./(2*TMath::Pi());
  if(phiDeg<0.)phiDeg=360+phiDeg;
  int phiSlice=phiDeg/45.;
  Double_t eff;
 
  if(inner){
    eff=fEffInnerRRange_r1inFR[phiSlice];
  }
  else if(outer){
    eff=fEffOuterRRange_r1inFR[phiSlice];
  }

  return eff;
}

Double_t AnnihilationSelectionDataMCMethod::ReturnEfficiencyR1R2inFR(Double_t radius, Double_t phi){
  HistoSvc* hSvc =  HistoSvc::GetInstance();
  std::string hname;
  Bool_t inner=false;
  Bool_t outer=false;
  if(radius>fFRmin && radius<fFRmid)inner=true;
  if(radius>=fFRmid && radius<fFRmax)outer=true;
  Double_t phiDeg=phi*360/(2*TMath::Pi());
  if(phiDeg<0.)phiDeg=360+phiDeg;
  int phiSlice=phiDeg/45.;
  Double_t eff;
  if(inner){
    eff=fEffInnerRRange_r1r2inFR[phiSlice];
  }
  else if(outer){
    eff=fEffOuterRRange_r1r2inFR[phiSlice];
  }
 
  return eff;
}


Double_t AnnihilationSelectionDataMCMethod::ReturnEfficiencyR1inFR_MC(Double_t radius, Double_t phi){
  HistoSvc* hSvc =  HistoSvc::GetInstance();
   std::string hname;
  Bool_t inner=false;
  Bool_t outer=false;
  if(radius<fFRmid )inner=true;
  if(radius>=fFRmid)outer=true;
  Double_t phiDeg=phi*360./(2*TMath::Pi());
  if(phiDeg<0.)phiDeg=360+phiDeg;
  int phiSliceMC=phiDeg/45.;
  int phiSliceD=phiDeg/45.+2;
  if(phiSliceD>7){
    phiSliceD=phiSliceD-8;
  }
  Double_t eff;
 
  if(inner){
    eff=fEffInnerRRange_r1inFR[phiSliceD]/fEffInnerRRange_r1inFR[phiSliceMC];
  }
  else if(outer){
    eff=fEffOuterRRange_r1inFR[phiSliceD]/fEffOuterRRange_r1inFR[phiSliceMC];
  }
  //std::cout<<"..............................." << std::endl;
  //std::cout<<"Photon phi " << phiDeg << " phiSliceMC " <<phiSliceMC << " phiSliceData " << phiSliceD << std::endl;
  //std::cout<<"inner bool " << inner << " outer bool " << outer << " radius "<< radius << std::endl;
  //if(inner)std::cout<<"inner, eff data " << fEffInnerRRange_r1inFR[phiSliceD] << " eff MC " <<  fEffInnerRRange_r1inFR[phiSliceMC] << " ratio " << eff << std::endl;
  //else std::cout<<"outer, eff data " << fEffOuterRRange_r1inFR[phiSliceD] << " eff MC " <<  fEffOuterRRange_r1inFR[phiSliceMC] << " ratio " << eff << std::endl;

  return eff;
}

Double_t AnnihilationSelectionDataMCMethod::ReturnEfficiencyR1R2inFR_MC(Double_t radius, Double_t phi){
  HistoSvc* hSvc =  HistoSvc::GetInstance();
  std::string hname;
  Bool_t inner=false;
  Bool_t outer=false;
  if(radius>fFRmin && radius<fFRmid)inner=true;
  if(radius>=fFRmid && radius<fFRmax)outer=true;
  Double_t phiDeg=phi*360/(2*TMath::Pi());
  if(phiDeg<0.)phiDeg=360+phiDeg;
  int phiSliceMC=phiDeg/45.;
  int phiSliceD=phiDeg/45.+2;
  if(phiSliceD>7){
    phiSliceD=phiSliceD-8;
  }
  Double_t eff;
  if(inner){
    eff=fEffInnerRRange_r1r2inFR[phiSliceD]/fEffInnerRRange_r1r2inFR[phiSliceMC];
  }
  else if(outer){
    eff=fEffOuterRRange_r1r2inFR[phiSliceD]/fEffOuterRRange_r1r2inFR[phiSliceMC];
  }
 
  return eff;
}



Bool_t AnnihilationSelectionDataMCMethod::InitHistos(){
  
  HistoSvc* hSvc =  HistoSvc::GetInstance();
  std::string hname;
  int binX=500;
  double minX=0;
  double maxX=2000;
  int binY=500;
  double minY=0;
  double maxY=2000;

  hname="MethodDataMC_Data";
  hSvc->BookHisto(hname, binX, minX, maxX);
  hname="MethodDataMC_MC";
  hSvc->BookHisto(hname, binX, minX, maxX);
  hname="MethodDataMC_Datag1inFR";
  hSvc->BookHisto(hname, binX, minX, maxX);
  hname="MethodDataMC_MCg1inFR";
  hSvc->BookHisto(hname, binX, minX, maxX);
  hname="MethodDataMC_Datag1g2inFR";
  hSvc->BookHisto(hname, binX, minX, maxX);
  hname="MethodDataMC_MCg1g2inFR";
  hSvc->BookHisto(hname, binX, minX, maxX);


  std::vector<std::string> sufixW;
  sufixW.push_back("DataSample");
  sufixW.push_back("MCSample_usualW");
  for(int i=0; i<sufixW.size(); i++){
    binX=500;
    minX=0.;
    maxX=2000.;
    hname="MethodDataMC_WEffR1inFR"+sufixW.at(i);
    hSvc->BookHisto(hname, binX, minX, maxX);
    hname="MethodDataMC_WEffR1R2inFR"+sufixW.at(i);
    hSvc->BookHisto(hname, binX, minX, maxX);
  }

 std::vector<std::string> sufixW1;
 sufixW1.push_back("MCSample_DataMCW");
 for(int i=0; i<sufixW1.size(); i++){
   binX=500;
   minX=0.;
   maxX=2000.;
   hname="MethodDataMC_WEffR1inFRDataPlus90"+sufixW1.at(i);
   hSvc->BookHisto(hname, binX, minX, maxX);
   hname="MethodDataMC_WEffR1R2inFRDataPlus90"+sufixW1.at(i);
   hSvc->BookHisto(hname, binX, minX, maxX);
   }

 binX=100;
 minX=-10;
 maxX=370.;
 binY=500;
 minY=0.;
 maxY=2;
 hname="MethodDataMC_datasample_g1inFR_Effg1vsPhig1";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);
 hname="MethodDataMC_datasample_g1inFR_Effg2vsPhig2";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);
 hname="MethodDataMC_mcsample_g1inFR_Effg1vsPhig1";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);
 hname="MethodDataMC_mcsample_g1inFR_Effg2vsPhig2";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);
 hname="MethodDataMC_datasample_g1g2inFR_Effg1vsPhig1";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);
 hname="MethodDataMC_datasample_g1g2inFR_Effg2vsPhig2";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);
 hname="MethodDataMC_mcsample_g1g2inFR_Effg1vsPhig1";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);
 hname="MethodDataMC_mcsample_g1g2inFR_Effg2vsPhig2";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);
    
 hname="MethodDataMC_datasample_g1inFR_Effg1vsPhig1_datamcMethod";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);
 hname="MethodDataMC_datasample_g1inFR_Effg2vsPhig2_datamcMethod";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);
 hname="MethodDataMC_mcsample_g1inFR_Effg1vsPhig1_datamcMethod";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);
 hname="MethodDataMC_mcsample_g1inFR_Effg2vsPhig2_datamcMethod";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);
 hname="MethodDataMC_datasample_g1g2inFR_Effg1vsPhig1_datamcMethod";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);
 hname="MethodDataMC_datasample_g1g2inFR_Effg2vsPhig2_datamcMethod";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);
 hname="MethodDataMC_mcsample_g1g2inFR_Effg1vsPhig1_datamcMethod";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);
 hname="MethodDataMC_mcsample_g1g2inFR_Effg2vsPhig2_datamcMethod";
 hSvc->BookHisto2(hname, binX, minX, maxX, binY, minY, maxY);

 return true;
}


