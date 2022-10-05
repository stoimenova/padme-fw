#include <stdio.h>
#include <sys/stat.h>

#include "LeadGlassMonitor.hh"

#include "Configuration.hh"

#include "TMath.h"

LeadGlassMonitor::LeadGlassMonitor(TString cfgFile)
{

  // Connect to configuration class
  fConfig = Configuration::GetInstance();

  // Connect to analysis configuration parser
  fConfigParser = new utl::ConfigParser((const std::string)cfgFile);
  if (fConfig->Verbose()>1) fConfigParser->Print();

  // Initialize all counters
  Initialize();

}

LeadGlassMonitor::~LeadGlassMonitor()
{
  if (fConfigParser) { delete fConfigParser; fConfigParser = 0; }
  if (fHLGPedestalBM) { delete fHLGPedestalBM; fHLGPedestalBM = 0; }
  if (fHLGPedRMSBM) { delete fHLGPedRMSBM; fHLGPedRMSBM = 0; }
  if (fHLGTotChargeBM) { delete fHLGTotChargeBM; fHLGTotChargeBM = 0; }
}

void LeadGlassMonitor::Initialize()
{

  if (fConfig->Verbose() > 1)
    printf("--- LeadGlass on board %d channel %d ---\n",LEADGLASS_BOARD,LEADGLASS_CHANNEL);

  // Get output rates from config file
  fBeamOutputRate = fConfigParser->HasConfig("RECO","BeamOutputRate")?std::stoi(fConfigParser->GetSingleArg("RECO","BeamOutputRate")):500;
  fOffBeamOutputRate = fConfigParser->HasConfig("RECO","OffBeamOutputRate")?std::stoi(fConfigParser->GetSingleArg("RECO","OffBeamOutputRate")):100;
  fCosmicsOutputRate = fConfigParser->HasConfig("RECO","CosmicsOutputRate")?std::stoi(fConfigParser->GetSingleArg("RECO","CosmicsOutputRate")):0;
  fRandomOutputRate = fConfigParser->HasConfig("RECO","RandomOutputRate")?std::stoi(fConfigParser->GetSingleArg("RECO","RandomOutputRate")):100;

  // Get pedestal and charge reconstruction parameters from config file
  fPedestalSamples = fConfigParser->HasConfig("RECO","PedestalSamples")?std::stoi(fConfigParser->GetSingleArg("RECO","PedestalSamples")):100;
  fSignalSamplesStart = fConfigParser->HasConfig("RECO","SignalSamplesStart")?std::stoi(fConfigParser->GetSingleArg("RECO","SignalSamplesStart")):200;
  fSignalSamplesEnd = fConfigParser->HasConfig("RECO","SignalSamplesEnd")?std::stoi(fConfigParser->GetSingleArg("RECO","SignalSamplesEnd")):600;

  // Create histograms
  fHLGPedestalBM = new TH1D("LG_PedestalBM","LG_Pedestal",120,3500.,4100.);
  fHLGPedRMSBM = new TH1D("LG_PedRMSBM","LG_PedRMS",100,0.,50.);
  fHLGTotChargeBM = new TH1D("LG_TotChargeBM","LG_TotCharge",1000,0.,10000.);

  // Reset global counters
  fBeamEventCount = 0;
  fOffBeamEventCount = 0;
  fCosmicsEventCount = 0;
  fRandomEventCount = 0;

}

void LeadGlassMonitor::StartOfEvent()
{

  // Check if event was triggered by BTF beam
  if (fConfig->GetEventTrigMask() & 0x01) {
    fIsBeam = true;
  } else {
    fIsBeam = false;
  }

  // Check if event was triggered by cosmics
  if (fConfig->GetEventTrigMask() & 0x02) {
    fIsCosmics = true;
  } else {
    fIsCosmics = false;
  }
 
  // Check if event was a random trigger
  if (fConfig->GetEventTrigMask() & 0x40) {
    fIsRandom = true;
  } else {
    fIsRandom = false;
  }
  
  // Check if event was an off-beam trigger
  if (fConfig->GetEventTrigMask() & 0x80) {
    fIsOffBeam = true;
  } else {
    fIsOffBeam = false;
  }

}

void LeadGlassMonitor::EndOfEvent()
{

  if (fIsBeam) {

    if (fBeamOutputRate && (fBeamEventCount % fBeamOutputRate == 0)) {

      // Write beam events data to output PadmeMonitor file
      OutputBeam();

    }

    // Count beam event
    fBeamEventCount++;

  } // End of beam output

  if (fIsOffBeam) {

    if (fOffBeamOutputRate && (fOffBeamEventCount % fOffBeamOutputRate == 0)) {

      // Write off-beam events data to output PadmeMonitor file
      OutputOffBeam();

    }

    // Count off-beam event
    fOffBeamEventCount++;

  } // End of off-beam output

  if (fIsCosmics) {

    if (fCosmicsOutputRate && (fCosmicsEventCount % fCosmicsOutputRate == 0)) {

      OutputCosmics();

    }

    // Count cosmics event
    fCosmicsEventCount++;

  }

  if (fIsRandom) {

    if (fRandomOutputRate && (fRandomEventCount % fRandomOutputRate == 0)) {

      // Write random events data to output PadmeMonitor file
      OutputRandom();

    }

    // Count cosmics event
    fRandomEventCount++;

  } // End of random output

}

void LeadGlassMonitor::Finalize()
{
  printf("LeadGlassMonitor::Finalize - Total number of beam     events: %d\n",fBeamEventCount);
  printf("LeadGlassMonitor::Finalize - Total number of off-beam events: %d\n",fOffBeamEventCount);
  printf("LeadGlassMonitor::Finalize - Total number of cosmics  events: %d\n",fCosmicsEventCount);
  printf("LeadGlassMonitor::Finalize - Total number of random   events: %d\n",fRandomEventCount);
}

void LeadGlassMonitor::AnalyzeBoard(UChar_t board)
{;}

void LeadGlassMonitor::AnalyzeChannel(UChar_t board,UChar_t channel,Short_t* samples)
{

  // Compute pedestal and total charge in leadglass and save them to histogram
  ComputeTotalCharge(samples);
  if (fIsBeam) {
    fHLGPedestalBM->Fill(fChannelPedestal);
    fHLGPedRMSBM->Fill(fChannelPedRMS);
    fHLGTotChargeBM->Fill(fChannelCharge);
    //if (fChannelCharge<100.) printf("%d %f %f %f\n",fConfig->GetEventNumber(),fChannelPedestal,fChannelPedRMS,fChannelCharge);
  }

}

void LeadGlassMonitor::ComputeTotalCharge(Short_t* samples)
{

  // Get total signal area using first fPedestalSamples samples as pedestal
  Int_t sum = 0;
  Int_t sum_ped = 0;
  ULong_t sum2_ped = 0;
  for(UInt_t s = 0; s<1024; s++) {
    if (s<fPedestalSamples) {
      sum_ped += samples[s];
      sum2_ped += samples[s]*samples[s];
    } else if (s >= fSignalSamplesStart) {
      if (s < fSignalSamplesEnd) {
	sum += samples[s];
      } else {
	break;
      }
    }
  }

  fChannelPedestal = (Double_t)sum_ped/(Double_t)fPedestalSamples;
  fChannelPedRMS = sqrt(((Double_t)sum2_ped - (Double_t)sum_ped*fChannelPedestal)/((Double_t)fPedestalSamples-1.));
  fChannelCharge = fChannelPedestal*(Double_t)(fSignalSamplesEnd-fSignalSamplesStart)-(Double_t)sum;
  // Convert counts to charge in pC
  //charge = counts/(4096.*50.)*(1.E-9/1.E-12);
  fChannelCharge *= 4.8828E-3;

}

Int_t LeadGlassMonitor::OutputBeam()
{
  if (fConfig->Verbose()>0) printf("LeadGlassMonitor::OutputBeam - Writing beam output files\n");

  // Write LeadGlass histograms
  TString ftname = fConfig->TmpDirectory()+"/LeadGlassMon_Beam.txt";
  TString ffname = fConfig->OutputDirectory()+"/LeadGlassMon_Beam.txt";
  FILE* outf = fopen(ftname.Data(),"w");

  // Pedestal
  fprintf(outf,"PLOTID LeadGlassMon_beampedestal\n");
  fprintf(outf,"PLOTTYPE histo1d\n");
  fprintf(outf,"PLOTNAME LeadGlass Beam Pedestal - Run %d - %s\n",fConfig->GetRunNumber(),fConfig->FormatTime(time(0)));
  fprintf(outf,"CHANNELS %d\n",fHLGPedestalBM->GetNbinsX());
  fprintf(outf,"RANGE_X %.3f %.3f\n",fHLGPedestalBM->GetXaxis()->GetXmin(),fHLGPedestalBM->GetXaxis()->GetXmax());
  fprintf(outf,"TITLE_X pC\n");
  fprintf(outf,"TITLE_Y Counts\n");
  fprintf(outf,"DATA [[");
  for(Int_t b = 1; b <= fHLGPedestalBM->GetNbinsX(); b++) {
    if (b>1) fprintf(outf,",");
    fprintf(outf,"%.0f",fHLGPedestalBM->GetBinContent(b));
  }
  fprintf(outf,"]]\n\n");

  // Pedestal RMS
  fprintf(outf,"PLOTID LeadGlassMon_beampedrms\n");
  fprintf(outf,"PLOTTYPE histo1d\n");
  fprintf(outf,"PLOTNAME LeadGlass Beam Pedestal RMS - Run %d - %s\n",fConfig->GetRunNumber(),fConfig->FormatTime(time(0)));
  fprintf(outf,"CHANNELS %d\n",fHLGPedRMSBM->GetNbinsX());
  fprintf(outf,"RANGE_X %.3f %.3f\n",fHLGPedRMSBM->GetXaxis()->GetXmin(),fHLGPedRMSBM->GetXaxis()->GetXmax());
  fprintf(outf,"TITLE_X pC\n");
  fprintf(outf,"TITLE_Y Counts\n");
  fprintf(outf,"DATA [[");
  for(Int_t b = 1; b <= fHLGPedRMSBM->GetNbinsX(); b++) {
    if (b>1) fprintf(outf,",");
    fprintf(outf,"%.0f",fHLGPedRMSBM->GetBinContent(b));
  }
  fprintf(outf,"]]\n\n");

  // Total Charge
  fprintf(outf,"PLOTID LeadGlassMon_beamtotcharge\n");
  fprintf(outf,"PLOTTYPE histo1d\n");
  fprintf(outf,"PLOTNAME LeadGlass Beam Total Charge - Run %d - %s\n",fConfig->GetRunNumber(),fConfig->FormatTime(time(0)));
  fprintf(outf,"CHANNELS %d\n",fHLGTotChargeBM->GetNbinsX());
  fprintf(outf,"RANGE_X %.3f %.3f\n",fHLGTotChargeBM->GetXaxis()->GetXmin(),fHLGTotChargeBM->GetXaxis()->GetXmax());
  fprintf(outf,"TITLE_X pC\n");
  fprintf(outf,"TITLE_Y Counts\n");
  fprintf(outf,"DATA [[");
  for(Int_t b = 1; b <= fHLGTotChargeBM->GetNbinsX(); b++) {
    if (b>1) fprintf(outf,",");
    fprintf(outf,"%.0f",fHLGTotChargeBM->GetBinContent(b));
  }
  fprintf(outf,"]]\n\n");

  fclose(outf);
  if ( std::rename(ftname.Data(),ffname.Data()) ) {
    printf("LeadGlassMonitor::OutputBeam - ERROR - could not rename file from %s to %s\n",ftname.Data(),ffname.Data());
    return 1;
  }

  return 0;
}

Int_t LeadGlassMonitor::OutputOffBeam()
{
  return 0;
}

Int_t LeadGlassMonitor::OutputCosmics()
{
  return 0;
}

Int_t LeadGlassMonitor::OutputRandom()
{
  return 0;
}