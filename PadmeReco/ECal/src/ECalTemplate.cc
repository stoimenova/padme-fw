#include "ECalTemplate.hh"
#include <iostream>
#include "cstdlib"
#include "TProfile.h"
#include "TSpline.h"
#include "TMath.h"
#include "TF1.h"
#include <vector>

ECalTemplate* ECalTemplate::fInstance = 0;

static Double_t fitf(Double_t * x, Double_t * par) {
  TSpline3 *  templateSpline =  ECalTemplate::GetInstance()->GetTemplateSpline();
  double arg = (x[0] - par[1])/par[3];
  return par[0]*templateSpline->Eval(arg)+par[2];
}

ECalTemplate* ECalTemplate::GetInstance()
{
  if ( fInstance == 0 ) { fInstance = new ECalTemplate(); }
  return fInstance;
}

void ECalTemplate::Init()
{
  std::cout<<"Creating template root file "<<std::endl;
  fileOut    = new TFile("ECalTemplate.root", "RECREATE");
  hListTemplate   = new TList();  
  ECal = new TTree("ECAL","ECAL");
  ECal->Branch("VMax",&fVMax);
  hListTemplate->Add(hSample   = new TH1D("hSample","hSample",1000,0.,1000.));
  hListTemplate->Add(hTMax    = new TH1D("hTimeMax","hTimeMax",1000,0.,1000.));
  hListTemplate->Add(hprof= new TProfile("hprof","Signal Profile",1000,0,1000,0,4096));
}

ECalTemplate::ECalTemplate()
{
  std::cout<<"Creating template class "<<std::endl;
}
ECalTemplate::~ECalTemplate(){
}

void ECalTemplate::WriteHist()
{
  std::cout<<"Saving trees "<<std::endl;
  fileOut->cd();
  ECal->Write();
  hListTemplate->Write();
  fileOut->Close();
}

void ECalTemplate::PrepareTemplate(Short_t * fSample, Double_t Time)
{
  fileOut->cd();
  //  hListTemplate->Print();
  TH1D * HistoTemp = (TH1D*) hListTemplate->FindObject("hSample");
  TH1D * hTemplate = (TH1D*) hListTemplate->FindObject("hprof");
  TH1D * hTMax     = (TH1D*) hListTemplate->FindObject("hTimeMax");
  hTMax->Fill(Time);

  double templateVec[1000];
  Int_t PeakPos = 400;
  Int_t RiseMargin = 45;

  double Baseline = TMath::Mean(200,&fSample[0]);
  Int_t shift = Time-PeakPos;
  for(Int_t kk=0;kk<1000;kk++){ 
    if (kk+shift > 0 && kk+shift < 1000) {
      //      if (kk < Time-RiseMargin) templateVec[kk] = Baseline;
      if (kk < Time-RiseMargin) templateVec[kk] = fSample[kk];
      else templateVec[kk] = fSample[kk+shift];
    }else{
      templateVec[kk] = Baseline;
      //      std::cout<<"aho "<<fSample[1000-((Int_t)Time-PeakPos)]<<std::endl;
      if(shift>0) templateVec[kk] =  fSample[1000-shift];
    }
  }
  TH1D* htmp    = new TH1D(Form("hVtemp%d",NHist),Form("hVtemp%d",NHist),1000,0.,1000.);
  TH1D* h = new TH1D(Form("hSig%d",NHist),Form("hSig%d",NHist),1000,0.,1000.);
  hVSig.push_back(h);
  hVTemp.push_back(htmp);
  for (int j = 0; j < 1000; j++) {
    hVTemp.at(NHist)->SetBinContent(j+1,templateVec[j]);
    //    hVSig.at(NHist)->SetBinContent(j+1,templateVec[j]);
    hVSig.at(NHist)->SetBinContent(j+1,fSample[j]);
    hTemplate->Fill(j+1,templateVec[j]); //sistemare
    // std::cout << "j: " << j << ", template: " << templateVec[j] << std::endl;
  }  
  fileOut->cd();
  hVSig.at(NHist)->Write();
  hVTemp.at(NHist)->Write();;
  NHist++;
}

void ECalTemplate::ReadTemplate()
{
  fileOut = new TFile("ECalTemplate.root","update");
  if(fileOut->IsOpen()){ 
    printf("File opened successfully\n");
  }else{
    printf("failed to open Template File \n");
    exit(-1);
  }
  std::cout<<"Reading template from File ECalTemplate.root"<<std::endl;
  TProfile* hECalTempl = (TProfile *) fileOut->Get("hprof");
  TH1D* hTest = (TH1D *) fileOut->Get("hSample");
  //  hECalTempl->Draw();
  Double_t templateVec[1000];
  Int_t Nent = hECalTempl->GetEntries();
  for(Int_t kk=0;kk<1000;kk++){ 
    templateVec[kk] = hECalTempl->GetBinContent(kk);
    //   std::cout<<" "<<templateVec[kk]<<std::endl;
  }
  fTemplateSpline = new TSpline3("templateSpline", -1, 1000, templateVec, 1000);
  fileOut->cd();
  std::cout<<"Writing template "<<std::endl;
  fTemplateSpline->Write();
}

void ECalTemplate::BuildFitFunction(){
  fECalFitFunction = new TF1("fitf",fitf,20,1000,4);
  fECalFitFunction -> SetParameters(1,0,0,1);
}

//void ECalTemplate::GetFitFunction()
//  hSample->Fit("fitf", "R");
//
//  Double_t Ampl=fitfunction->GetParameter(0);
//  Double_t TOff=fitfunction->GetParameter(1);
//  Double_t VOff=fitfunction->GetParameter(2);
//  Double_t Stre=fitfunction->GetParameter(3);
//  
//  hAmpl->Fill(fitfunction->GetParameter(0));
//  hOffset->Fill(fitfunction->GetParameter(1));
//  hBaseline->Fill(fitfunction->GetParameter(2));
//  hStretch->Fill(fitfunction->GetParameter(3));
//
//  fileOut->Close();
//}