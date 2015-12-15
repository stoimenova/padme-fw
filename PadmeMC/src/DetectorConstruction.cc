// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo...... 
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//#include "InputParam.input"
#include "Constants.hh" 
#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"
#include "MagneticField.hh"
#include "TargetDetector.hh"
#include "ECalDetector.hh"
#include "SACDetector.hh"
//#include "ECalSD.hh"
#include "TRodSD.hh"
#include "MRodSD.hh"
#include "TrackerSD.hh"
#include "EVetoSD.hh"
#include "PosVetoSD.hh"
#include "EleVetoSD.hh"
//#include "SACSD.hh"
#include "LAVSD.hh"
#include "GFiltSD.hh"

#include "G4Material.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Cons.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVParameterised.hh"
#include "G4SDManager.hh"
#include "G4GeometryTolerance.hh"
#include "G4GeometryManager.hh"
#include "G4RotationMatrix.hh"
#include "G4SubtractionSolid.hh"

#include "G4UserLimits.hh"
#include "G4NistManager.hh"
#include "G4VSDFilter.hh"
#include "G4SDParticleFilter.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4PSEnergyDeposit.hh"
#include "G4PSNofSecondary.hh"
#include "G4PSTrackLength.hh"
#include "G4PSNofStep.hh"
#include "G4PSMinKinEAtGeneration.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
 
DetectorConstruction::DetectorConstruction()
:solidWorld(0),  logicWorld(0),  physiWorld(0),
 stepLimit(0), fMagField(0), fEmFieldSetup(0), //added M. Raggi
 fWorldLength(0.)
{

  fEmFieldSetup = new F03FieldSetup();
  fDetectorMessenger = new DetectorMessenger(this);

  fECalDetector   = new ECalDetector(0);
  fTargetDetector = new TargetDetector(0);
  fSACDetector    = new SACDetector(0);

  fEnableECal   = 1;
  fEnableTarget = 1;
  fEnableSAC    = 1;

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
 
DetectorConstruction::~DetectorConstruction()
{
  delete fMagField;
  if (fEmFieldSetup) delete fEmFieldSetup ;
  delete stepLimit;
  delete fDetectorMessenger;             
}

void DetectorConstruction::EnableSubDetector(G4String det)
{
  printf("Enabling subdetector %s\n",det.data());
  if      (det=="ECal")   { fEnableECal   = 1; }
  else if (det=="Target") { fEnableTarget = 1; }
  else if (det=="SAC")    { fEnableSAC    = 1; }
  else { printf("WARNING: request to enable unknown subdetector %s\n",det.data()); }
}

void DetectorConstruction::DisableSubDetector(G4String det)
{
  printf("Disabling subdetector %s\n",det.data());
  if      (det=="ECal")   { fEnableECal   = 0; }
  else if (det=="Target") { fEnableTarget = 0; }
  else if (det=="SAC")    { fEnableSAC    = 0; }
  else { printf("WARNING: request to disable unknown subdetector %s\n",det.data()); }
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{

//--------- Material definition ---------
  G4double a, z, density;
  G4int ncomponents, natoms;
  G4double fractionmass;  
  
  G4Material* Vacuum;
  G4Material* FewAir;

  //LSO crystals for the calorimeter L2S1O5
  G4Element* O  = new G4Element("Oxygen",   "O",  z= 8., a=16.00*g/mole);
  G4Element* N  = new G4Element("Nitrogen" ,"N" , z= 7., a=14.01*g/mole);
  G4Element* Lu = new G4Element("Lutetium", "Lu", z=71., a=174.97*g/mole);
  G4Element* Si = new G4Element("Silicon",  "Si", z=14., a=28.09*g/mole);

  if(IsResGasON==1) {
    //    Vacuum = new G4Material("Vacuum",z=1.,a=1.01*g/mole,VacuumDensity*mg/cm3,kStateGas,2.93*kelvin,VacuumP*bar);    
    Vacuum = new G4Material("Vacuum", density= (1.290*0.000001)*mg/cm3, ncomponents=2); //1mbar
    Vacuum->AddElement(N, fractionmass=70.*perCent);
    Vacuum->AddElement(O, fractionmass=30.*perCent);
  }else{
    Vacuum = new G4Material("Vacuum",z=1.,a=1.01*g/mole,universe_mean_density,kStateGas,2.73*kelvin,3.e-18*pascal);
  }
  G4Material* LSO = new G4Material("LSO", density=7.4*g/cm3,3);
  LSO->AddElement(Lu,2);
  LSO->AddElement(Si,1);
  LSO->AddElement(O,5);

  G4NistManager* man = G4NistManager::Instance();
  man->SetVerbose(1);
  G4Material* SiO2   = man->FindOrBuildMaterial("G4_SILICON_DIOXIDE");
  G4Material* Air    = man->FindOrBuildMaterial("G4_AIR");
  G4Material* Pb     = man->FindOrBuildMaterial("G4_Pb");
  G4Material* PbWO4  = man->FindOrBuildMaterial("G4_PbWO4");
  G4Material* elC    = man->FindOrBuildMaterial("G4_C");
  G4Material* W      = man->FindOrBuildMaterial("G4_W");
  G4Material* Concrete = man->FindOrBuildMaterial("G4_CONCRETE");
  G4Material* Iron     = man->FindOrBuildMaterial("G4_Fe");
  G4Material* Scint    = man->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
  G4Material* G4Vac    = man->FindOrBuildMaterial("G4_Galactic");
  G4Material* BGO      = man->FindOrBuildMaterial("G4_BGO");
  G4Material* BaF2     = man->FindOrBuildMaterial("G4_BARIUM_FLUORIDE");
  G4Material* Cu       = man->FindOrBuildMaterial("G4_Cu");
  G4Material* Al       = man->FindOrBuildMaterial("G4_Al");

  //Print all the materials defined.
//  G4cout << G4endl << "The materials defined are : " << G4endl << G4endl;
//  G4cout << *(G4Material::GetMaterialTable()) << G4endl;
  G4bool allLocal=true;
  
  //------------------------------ 
  // World Volume
  //------------------------------  
  G4GeometryManager::GetInstance()->SetWorldMaximumExtent(fWorldLength);
  //  G4cout<<"Computed tolerance = "<<G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()/mm<<" mm" << G4endl;
  WorldMater=Vacuum;
  G4double HalfWorldLength = 0.5*WorldLength*m;

  solidWorld= new G4Box("world",HalfWorldLength,HalfWorldLength,HalfWorldLength);
  logicWorld= new G4LogicalVolume( solidWorld, WorldMater, "World", 0, 0, 0);
  physiWorld = new G4PVPlacement(0,               // no rotation
                                 G4ThreeVector(), // at (0,0,0)
                                 logicWorld,      // its logical volume
                                 "World",         // its name
                                 0,               // its mother  volume
                                 false,           // no boolean operations
                                 0);              // copy number
 
  if(IsPipeON==1){
    // Create btf beam pipe
    G4double Pipe_ZLength      = 1.*m;
    G4double Pipe_ZLength1     = 12.*cm;
    G4double Pipe_AlThickness  = 3.*mm;
    G4double Pipe_InnerRadius  = 4.*cm-Pipe_AlThickness;
    G4double Pipe_OuterRadius  = 4.*cm;
    G4ThreeVector posPipe   = G4ThreeVector(0.,0.,-0.5*Pipe_ZLength-2.*cm);
    G4ThreeVector posPipe1  = G4ThreeVector(0.,0.,TargetPosiZ*cm+2.*cm+Pipe_ZLength1/2);
    
    G4Tubs* solidPipe = new G4Tubs("solidPipe",Pipe_InnerRadius,Pipe_OuterRadius,0.5*Pipe_ZLength,0.*rad,2.*M_PI*rad);
    G4LogicalVolume* logicPipe    = new G4LogicalVolume(solidPipe,Al,"LogicPipe",0,0,0);
    G4VPhysicalVolume * physiPipe = new G4PVPlacement(0,posPipe,logicPipe,"UpPipe",logicWorld,false,0);

    G4Tubs* solidPipe1 = new G4Tubs("solidPipe1",Pipe_InnerRadius,Pipe_OuterRadius,0.5*Pipe_ZLength1,0.*rad,2.*M_PI*rad);
    G4LogicalVolume* logicPipe1    = new G4LogicalVolume(solidPipe1,Al,"LogicPipe1",0,0,0);
    G4VPhysicalVolume * physiPipe1 = new G4PVPlacement(0,posPipe1,logicPipe1,"UpPipe1",logicWorld,false,0);

  }

  if(IsSpecInside==1){
    G4double Tolerance=1.*um; //avoid overlap with Yoke and Gem
    G4double solidVacChambXSize=3*mm; 
    G4double solidVacChambYSize=40*cm;
    G4double solidVacChambZSize=1.1*m;

    G4double solidVacChambXPos=10*cm-solidVacChambXSize/2; 
    G4double solidVacChambYPos=0*cm;
    G4double solidVacChambZPos=MagnetPosiZ*cm;

    //Flat Part
    G4ThreeVector posVacChambUp = G4ThreeVector(solidVacChambXPos,solidVacChambYPos,solidVacChambZPos);
    G4ThreeVector posVacChambDw = G4ThreeVector(-solidVacChambXPos,solidVacChambYPos,solidVacChambZPos);
    G4Box* solidVacChamb = new G4Box("solidVacChamb",(solidVacChambXSize-Tolerance)/2,(solidVacChambYSize-Tolerance)/2,solidVacChambZSize/2);
    G4LogicalVolume* logicVacChamb = new G4LogicalVolume(solidVacChamb,Al, "logicVacCham", 0, 0, 0);
    G4VPhysicalVolume * vacChambUp = new G4PVPlacement(0,posVacChambUp,logicVacChamb,"VacChambUp",logicWorld,false,0);
    G4VPhysicalVolume * vacChambDw = new G4PVPlacement(0,posVacChambDw,logicVacChamb,"VacChambDw",logicWorld,false,0);

    //curved lateral parts
    G4double Cham_ZLength = 1.1*m;
    G4double Cham_AlThick = 3*mm; //avoid overlap with Yoke and Gem
    G4double Cham_InnRad  = 10.*cm-Cham_AlThick;
    G4double Cham_OutRad  = 10.*cm;

    G4ThreeVector posVacChamSide  = G4ThreeVector(0.,20.*cm,MagnetPosiZ*cm);
    G4ThreeVector posVacChamSide1 = G4ThreeVector(0.,-20.*cm,MagnetPosiZ*cm);

    G4Tubs* solidVacChamSide = new G4Tubs("solidVacChamSide",Cham_InnRad,Cham_OutRad-Tolerance,0.5*Cham_ZLength,0.*rad,M_PI*rad);
    G4Tubs* solidVacChamSide1 = new G4Tubs("solidVacChamSide",Cham_InnRad,Cham_OutRad-Tolerance,0.5*Cham_ZLength,M_PI*rad,M_PI*rad);
    G4LogicalVolume* logicChamSide = new G4LogicalVolume(solidVacChamSide,Al,"LogicChamSide",0,0,0);
    G4LogicalVolume* logicChamSide1 = new G4LogicalVolume(solidVacChamSide1,Al,"LogicChamSide",0,0,0);
    G4VPhysicalVolume * vacChambLS = new G4PVPlacement(0,posVacChamSide,logicChamSide,"VacChambLS",logicWorld,false,0);
    G4VPhysicalVolume * vacChambRS = new G4PVPlacement(0,posVacChamSide1,logicChamSide1,"VacChambRS",logicWorld,false,0);
  }else if(IsSpecInside==0){
    G4ThreeVector posVacChamb = G4ThreeVector(0.*cm,0.*cm,MagnetPosiZ*cm);
    G4double VacChaX=20*cm;
    G4double VacChaY=20*cm;
    G4double VacChaZ=110*cm;
    G4double VacChaTh=0.4*cm;

    G4Box *outerBox = new G4Box("Outer Box",VacChaX/2.,VacChaY/2.,VacChaZ/2.);
    G4Box *innerBox = new G4Box("Inner Box",(VacChaX-VacChaTh)/2.,(VacChaY-VacChaTh)/2.,VacChaZ/2.+0.5*cm);
    G4SubtractionSolid *hollowChamb = new G4SubtractionSolid("Hollow Box",outerBox,innerBox);
    G4LogicalVolume* logicVacChamb = new G4LogicalVolume(hollowChamb,Al, "logicVacCham", 0, 0, 0);
    G4VPhysicalVolume * vacChamb   = new G4PVPlacement(0,posVacChamb,logicVacChamb,"VacChamb",logicWorld,false,0);
  }else{
    G4cout<<"No vaccuum equipment inside the magnet " <<G4endl;
  }
  
  if(IsWallON==1){  
    G4ThreeVector positionWall = G4ThreeVector(WallPosiX*cm,WallPosiY*cm,WallPosiZ*cm); 
    solidWall = new G4Box("wall",WallSizeX*0.5*cm,WallSizeY*0.5*cm,WallSizeZ*0.5*cm);
    logicWall = new G4LogicalVolume(solidWall,Concrete, "Wall", 0, 0, 0);
    physiWall = new G4PVPlacement(0,               // no rotation
				  positionWall,   // at (0,0,0)
				  logicWall,      // its logical volume
				  "Wall",         // its name
				  logicWorld,     // its mother  volume
				  false,          // no boolean operations
				  0,              // copy number
				  false);         // is overlap    
  }

  if(IsMagIronON==1){ 
    double poco=0.001*cm;
    G4ThreeVector positionMagUp = G4ThreeVector(MagUpX*cm,MagUpY*cm,MagUpZ*cm); 
    solidMagUpJoke= new G4Box("JokeUp",MagUpSizeX*0.5*cm-poco,MagUpSizeY*0.5*cm-poco,MagUpSizeZ*0.5*cm-poco);
    logicMagUpJoke = new G4LogicalVolume(solidMagUpJoke,Iron, "JokeUp", 0, 0, 0);
    physiMagUpJoke = new G4PVPlacement(0,               // no rotation
				  positionMagUp,   // at (0,0,0)
				  logicMagUpJoke, // its logical volume
				  "JokeUp",       // its name
				  logicWorld,     // its mother  volume
				  false,          // no boolean operations
				  0,              // copy number
				  false);         // is overlap    

    G4ThreeVector positionMagDw = G4ThreeVector(-1.*MagUpX*cm,MagUpY*cm,MagUpZ*cm); 
    physiMagDwJoke = new G4PVPlacement(0,               // no rotation
				  positionMagDw,   // at (0,0,0)
				  logicMagUpJoke, // its logical volume
				  "JokeDw",       // its name
				  logicWorld,     // its mother  volume
				  false,          // no boolean operations
				  0,              // copy number
				  false);         // is overlap    

    G4ThreeVector positionMagRg = G4ThreeVector(MagRgX*cm,MagRgY*cm,MagRgZ*cm); 
    solidMagRgJoke= new G4Box("JokeRg",MagRgSizeX*0.5*cm-poco,MagRgSizeY*0.5*cm-poco,MagRgSizeZ*0.5*cm-poco);
    logicMagRgJoke = new G4LogicalVolume(solidMagRgJoke,Iron, "JokeRg", 0, 0, 0);
    physiMagRgJoke = new G4PVPlacement(0,               // no rotation
				  positionMagRg,   // at (0,0,0)
				  logicMagRgJoke, // its logical volume
				  "JokeRg",       // its name
				  logicWorld,     // its mother  volume
				  false,          // no boolean operations
				  0,              // copy number
				  false);         // is overlap    

    G4ThreeVector positionMagLf = G4ThreeVector(MagRgX*cm,-1*MagRgY*cm,MagRgZ*cm); 
    physiMagLfJoke = new G4PVPlacement(0,               // no rotation
				  positionMagLf,   // at (0,0,0)
				  logicMagRgJoke, // its logical volume
				  "JokeLf",       // its name
				  logicWorld,     // its mother  volume
				  false,          // no boolean operations
				  0,              // copy number
				  false);         // is overlap    

    //Start rotating the iron plates
    G4RotationMatrix* xyzRot=new G4RotationMatrix; // Rotates X and Z axes only
    xyzRot->rotateX(M_PI*rad);  // Rotates 90 degrees
    xyzRot->rotateY(M_PI/2*rad);  // Rotates 90 degrees
    xyzRot->rotateZ(M_PI/2*rad);  // Rotates 90 degrees

    G4ThreeVector positionMagUpCoil = G4ThreeVector(MagCoilPosX*cm,MagCoilPosY*cm,MagCoilPosZ*cm); 
    solidMagCoil = new G4Tubs("solCoil",MagCoilRMin*cm,MagCoilRMax*cm,MagCoilDz*cm,startAngle*deg,spanningAngle*deg);
    logicMagCoil = new G4LogicalVolume(solidMagCoil,Cu,"CoilLog", 0, 0, 0);
    physiMagUpCoil = new G4PVPlacement(xyzRot,               // no rotation
				  positionMagUpCoil, // at (0,0,0)
				  logicMagCoil,    // its logical volume
				  "MagUpCoil",       // its name
				  logicWorld,     // its mother  volume
				  false,          // no boolean operations
				  0,              // copy number
				  false);         // is overlap    
    G4ThreeVector positionMagDwCoil = G4ThreeVector(-1.*MagCoilPosX*cm,MagCoilPosY*cm,MagCoilPosZ*cm); 
    physiMagDwCoil = new G4PVPlacement(xyzRot,       // no rotation
				  positionMagDwCoil, // at (0,0,0)
				  logicMagCoil,      // its logical volume
				  "MagDwCoil",       // its name
				  logicWorld,        // its mother  volume
				  false,             // no boolean operations
				  0,                 // copy number
				  false);         // is overlap    

    G4RotationMatrix* yzRot=new G4RotationMatrix; // Rotates X and Z axes only
    //    yzRot->rotateX(M_PI*rad);  // Rotates 90 degrees
    yzRot->rotateY(M_PI/2*rad);  // Rotates 90 degrees
    yzRot->rotateZ(M_PI/2*rad);  // Rotates 90 degrees

    G4ThreeVector positionMagUpBkCoil = G4ThreeVector(MagCoilPosX*cm,MagCoilPosY*cm,MagCoilPosZ*cm+100*cm); 
    physiMagUpBkCoil = new G4PVPlacement(yzRot,       // no rotation
				  positionMagUpBkCoil, // at (0,0,0)
				  logicMagCoil,      // its logical volume
				  "MagUpBkCoil",       // its name
				  logicWorld,        // its mother  volume
				  false,             // no boolean operations
				  0,                 // copy number
				  false);         // is overlap    

    G4ThreeVector positionMagDwBkCoil = G4ThreeVector(-1.*MagCoilPosX*cm,MagCoilPosY*cm,MagCoilPosZ*cm+100*cm); 
    physiMagDwBkCoil = new G4PVPlacement(yzRot,       // no rotation
				  positionMagDwBkCoil, // at (0,0,0)
				  logicMagCoil,      // its logical volume
				  "MagDwBkCoil",       // its name
				  logicWorld,        // its mother  volume
				  false,             // no boolean operations
				  0,                 // copy number
				  false);         // is overlap    
  }

//    G4double innerRad = 131.8*cm;
//    G4double outerRad = 204.6*cm;
//    G4double hz       = 20.*cm;
//    G4double startAngle = 46.*deg;
//    G4double spanningAngle = 44.*deg;
    
//    //Start rotating the iron plates
//    G4RotationMatrix* yRot=new G4RotationMatrix; // Rotates X and Z axes only
//    yRot->rotateY(M_PI/2.*rad);  // Rotates 90 degrees
//    G4ThreeVector zTrans(0.,0.,0.);    
//
//    solidMagIron = new G4Tubs("magIron",innerRad,outerRad,hz,startAngle,spanningAngle);
//    logicMagIron = new G4LogicalVolume(solidMagIron,Iron, "MagIron", 0, 0, 0);
//
//    for(int kk=0;kk<2;kk++){
//      float sign;
//      if(kk==0) sign=1.;
//      if(kk==1) sign=-1.;
//      G4ThreeVector positionMagIron = G4ThreeVector(MagIronX*cm+sign*40*cm,MagIronY*cm-(innerRad+(outerRad-innerRad)/2.),MagIronZ*cm); 
//      physiMagIron = new G4PVPlacement(yRot,              // no rotation
//				       positionMagIron,   // at (0,0,0)
//				       logicMagIron,      // its logical volume
//				       "MagIron",         // its name
//				       logicWorld,        // its mother  volume
//				       false,             // no boolean operations
//				       0,                 // copy number
//				       true);            //Check for overlaps
//    }
//
//    solidMagInnJoke = new G4Tubs("magInnJoke",innerRad,innerRad+19.5*cm,hz,startAngle,spanningAngle);
//    logicMagInnJoke = new G4LogicalVolume(solidMagInnJoke,Iron, "MagIronJoke", 0, 0, 0);
//
//    solidMagOutJoke = new G4Tubs("magOutJoke",outerRad-19.5*cm,outerRad,hz,startAngle+24*deg,spanningAngle-24*deg);
//    logicMagOutJoke = new G4LogicalVolume(solidMagOutJoke,Iron, "MagIronJoke", 0, 0, 0);
//
//    solidMagBArea = new G4Tubs("magBArea",innerRad+23.5*cm,outerRad-23.5*cm,hz,startAngle,spanningAngle);
//    logicMagBArea = new G4LogicalVolume(solidMagBArea,Vacuum,"MagBArea", 0, 0, 0);
//    logicMagBArea ->SetFieldManager(fEmFieldSetup->GetLocalFieldManager(),allLocal);
//
//    G4ThreeVector positionMagInnJoke = G4ThreeVector(MagIronX*cm,MagIronY*cm-(innerRad+(outerRad-innerRad)/2.),MagIronZ*cm); 
//    G4ThreeVector positionMagOutJoke = G4ThreeVector(MagIronX*cm,MagIronY*cm-(innerRad+(outerRad-innerRad)/2.),MagIronZ*cm); 
//    G4ThreeVector positionMagBArea   = G4ThreeVector(MagIronX*cm,MagIronY*cm-(innerRad+(outerRad-innerRad)/2.),MagIronZ*cm); 
//    
//    physiMagInnJoke = new G4PVPlacement(yRot,                 // no rotation
//					positionMagInnJoke,   // at (0,0,0)
//					logicMagInnJoke,      // its logical volume
//					"InnJoke",            // its name
//					logicWorld,          // its mother  volume
//					false,               // no boolean operations
//					0,                   // copy number
//					true);               //Check for overlaps
//    physiMagOutJoke = new G4PVPlacement(yRot,                 // no rotation
//					positionMagOutJoke,   // at (0,0,0)
//					logicMagOutJoke,      // its logical volume
//					"OutJoke",            // its name
//					logicWorld,          // its mother  volume
//					false,               // no boolean operations
//					0,                   // copy number
//					true);               //Check for overlaps
//
//    physiMagBArea  = new G4PVPlacement(yRot,                 // no rotation
//				       positionMagBArea,   // at (0,0,0)
//				       logicMagBArea,      // its logical volume
//				       "BArea",            // its name
//				       logicWorld,          // its mother  volume
//				       false,               // no boolean operations
//				       0,                   // copy number
//				       true);               //Check for overlaps
//} 

  if(IsBFieldON==1){
    //------------------------------ 
    // Magnet
    //------------------------------
    double poco=0.01*cm;
    G4ThreeVector positionSwepMag = G4ThreeVector(MagnetPosiX*cm,MagnetPosiY*cm,MagnetPosiZ*cm); 
    G4double SwepMagDx=  MagnetSizeX*cm;//+0.5*mm;
    G4double SwepMagDy=  MagnetSizeY*cm;//+0.5*mm;
    G4double SwepMagDz=  MagnetSizeZ*cm;//+0.5*mm;//+5*cm; // margin to avoit grazing tracks
    solidSwepMag = new G4Box("swepMag",SwepMagDx*0.5-poco,SwepMagDy*0.5-poco,SwepMagDz*0.5+poco);
    logicSwepMag = new G4LogicalVolume(solidSwepMag,WorldMater,"SwepMag",0,0,0);
    logicSwepMag ->SetFieldManager(fEmFieldSetup->GetLocalFieldManager(),allLocal);
    physiSwepMag = new G4PVPlacement(0,             // no rotation
				     positionSwepMag,  // at (x,y,z)
				     logicSwepMag,     // its logical volume                                 
				     "SwepMag",           // its name
				     logicWorld,       // its mother  volume
				     false,            // no boolean operations
				     0,                // copy number 
				     true);           // Overlap check    
  }
  
  if (fEnableTarget) {
    fTargetDetector->SetMotherVolume(logicWorld);
    fTargetDetector->CreateGeometry();
  }
  /*
  if(IsTargetON==1){
   //------------------------------------------------- 
   // Target Defintion two layers of fused silica rods 
   //-------------------------------------------------
   //  G4ThreeVector positionTarget = G4ThreeVector(0,0,0); 
   G4ThreeVector positionTarget = G4ThreeVector(TargetPosiX*cm,TargetPosiY*cm,TargetPosiZ*cm); 
   
   G4double TargetX      = TargetSizeX*cm;
   G4double TargetY      = TargetSizeY*cm;
   G4double TargetLength = TargetSizeZ*cm;

   solidTarget = new G4Box("target",TargetX*0.5,TargetY*0.5,TargetLength*0.5);
   logicTarget = new G4LogicalVolume(solidTarget,elC,"Target",0,0,0);
   physiTarget = new G4PVPlacement(0,               // no rotation
                                   positionTarget,  // at (x,y,z)
				   logicTarget,     // its logical volume                     
				   "Target",        // its name
				   logicWorld,      // its mother  volume
				   false,           // no boolean operations
				   0,               // copy number 
				   false);          //Check for overlaps
//  //Start Rods Description for Monitor station
//  //Start target Rods Description
//  G4int    NRodRows=5;
//  G4double TXRodLength = 2*mm;
//  G4double TXRodX      = TargetX;
//  G4double TXRodY      = 2*mm;
//
//  G4double TYRodLength = 2*mm;
//  G4double TYRodX      = 2*mm;
//  G4double TYRodY      = TargetY;
//
//  solidTXRod  = new G4Box("TXRod",TXRodX*0.5,TXRodY*0.5,TXRodLength*0.5);
//  logicTXRod  = new G4LogicalVolume(solidTXRod,SiO2,"TXRod");
//
//  solidTYRod  = new G4Box("TYRod",TYRodX*0.5,TYRodY*0.5,TYRodLength*0.5);
//  logicTYRod  = new G4LogicalVolume(solidTYRod,SiO2,"TYRod");
//
//  for (G4int i=0;i<NRodRows;i++){
//    G4ThreeVector positionTXRod = G4ThreeVector(0.   ,-TargetY*0.5+0.5*TXRodY+1*cm+i*TXRodY,+0.*cm);
//    G4ThreeVector positionTYRod = G4ThreeVector(-TargetX*0.5+0.5*TYRodX+i*TYRodX+1*cm,0.,1.*cm);
//    
//    physiTXRod  = new G4PVPlacement(0,             // no rotation
//				    positionTXRod,  // at (x,y,z)
//				    logicTXRod,      // its logical volume                                  
//				    "TXRod",        // its name
//				    logicTarget,     // its mother  volume
//				    false,         // no boolean operations
//				    i);            // copy number 
//    
//    physiTYRod  = new G4PVPlacement(0,               // no rotation
//				    positionTYRod,   // at (x,y,z)
//				    logicTYRod,      // its logical volume                                  
//				    "TYRod",         // its name
//				    logicTarget,     // its mother  volume
//				    false,           // no boolean operations
//				    i+NRodRows);     // copy number 
  }
  */

  if (fEnableSAC) {
    fSACDetector->SetMotherVolume(logicWorld);
    fSACDetector->CreateGeometry();
  }
  /*
  if(IsSACON==1){
   //------------------------------------------------- 
   // ZERO ANGLE PHOTON VETO made of BaF2 
   //-------------------------------------------------
   G4ThreeVector positionSAC = G4ThreeVector(0,0,ECalPosiZ*cm+50.*cm); 
   solidSAC = new G4Box("SolSAC",SACX*0.5*cm,SACY*0.5*cm,SACLength*0.5*cm);
   logicSAC = new G4LogicalVolume(solidSAC,BaF2,"SolSAC",0,0,0);
   physiSAC = new G4PVPlacement(0,               // no rotation
				positionSAC,  // at (x,y,z)
				logicSAC,     // its logical volume                          
				"SAC", // its name
				logicWorld,       // its mother  volume
				false,            // no boolean operations
				0,                // copy number 
				false);           //Check for overlaps
  }
  */

 if(IsLAVON==1){
  //------------------------------------------------- 
  // ZERO ANGLE PHOTON VETO made of BaF2 
  //-------------------------------------------------
   //  G4ThreeVector positionLAV = G4ThreeVector(0,0,TargetPosiZ*cm+5.*cm+LAVLength*0.5*cm); 
   G4ThreeVector positionLAV = G4ThreeVector(0,0,0.); 
  solidLAV = new G4Tubs("SolLAV",LAVInnRad*cm,LAVOutRad*cm,LAVLength*0.5*cm,0.*rad,2.*M_PI*rad);
  logicLAV = new G4LogicalVolume(solidLAV,PbWO4,"SolLAV",0,0,0);
  physiLAV = new G4PVPlacement(0,               // no rotation
				  positionLAV,  // at (x,y,z)
				  logicLAV,     // its logical volume                          
				  "LAV", // its name
				  logicWorld,       // its mother  volume
				  false,            // no boolean operations
				  0,                // copy number 
				  true);           //Check for overlaps
 }

if(IsTDumpON==1){
	//BTF DUMP
	 G4ThreeVector positionTDump = G4ThreeVector(TDumpPosiX*cm,TDumpPosiY*cm,TDumpPosiZ*cm);
	 solidTDump = new G4Tubs("Tdump",TDumpInnerRad*cm,TDumpOuterRad*cm,TDumpHz*cm,0*deg,360*deg);
	 logicTDump = new G4LogicalVolume(solidTDump,W, "DumpTarg", 0, 0, 0);
	 physiTDump  = new G4PVPlacement(0,               // no rotati
			 	 positionTDump,   // at (x,y,z)
			 	 logicTDump,      // its logical volume
			 	 "Dump",           // its name
			 	 logicWorld,      // its mother  volume
			 	 false,           // no boolean operations
			 	 0);              // copy number


	 solidLeadBrick = new G4Box("wall",LeadBrickSizeX*0.5*cm,LeadBrickSizeY*0.5*cm,LeadBrickSizeZ*0.5*cm);
	 logicLeadBrick = new G4LogicalVolume(solidLeadBrick,Pb, "Brick", 0, 0, 0);
	 for(int i=0;i<4;i++){ // Replica lungo l'asse Z
		 //	int ii=0;
		 for(int ii=0;ii<2;ii++){  // Replica lungo l'asse X
   //         G4ThreeVector positionBrickL = G4ThreeVector((+LeadBrickSizeX+LeadBrickSizeX*0.5)*cm+LeadBrickSizeX*ii*cm,0*cm,(TDumpFiltPosiZ+TDumpHz)*cm-LeadBrickSizeZ*i*cm);
		 	G4ThreeVector positionBrickR = G4ThreeVector(0.*cm,(+LeadBrickSizeX+LeadBrickSizeX*0.5)*cm+LeadBrickSizeX*ii*cm,(TDumpFiltPosiZ+TDumpHz)*cm-LeadBrickSizeZ*i*cm);
		 	G4RotationMatrix* zRotR=new G4RotationMatrix; // Rotates X and Z axes only
		 	zRotR->rotateZ(-M_PI/2.*rad);  // Rotates 90 degrees
            physiLeadBrickR = new G4PVPlacement(zRotR,               // no rotation
            		 positionBrickR,   // at (0,0,0)
            		 logicLeadBrick,      // its logical volume
            		 "BrickWall",         // its name
            		 logicWorld,     // its mother  volume
            		 false,          // no boolean operations
            		 0,              // copy number
            		 false);         // is overlap

            G4ThreeVector positionBrickL = G4ThreeVector(0.*cm,-(+LeadBrickSizeX+LeadBrickSizeX*0.5)*cm-LeadBrickSizeX*ii*cm,(TDumpFiltPosiZ+TDumpHz)*cm-LeadBrickSizeZ*i*cm);
		 	G4RotationMatrix* zRotL=new G4RotationMatrix; // Rotates X and Z axes only
		 	zRotL->rotateZ(-M_PI/2.*rad);  // Rotates 90 degrees
            physiLeadBrickL = new G4PVPlacement(zRotL,               // no rotation
            		 positionBrickL,   // at (0,0,0)
            		 logicLeadBrick,      // its logical volume
            		 "BrickWall",         // its name
            		 logicWorld,     // its mother  volume
            		 false,          // no boolean operations
            		 0,              // copy number
            		 false);         // is overlap
		 }
		 for(int ii=-1;ii<2;ii++){
             //G4ThreeVector positionBrickT = G4ThreeVector(+LeadBrickSizeY*ii*cm,+(LeadBrickSizeY*0.5+LeadBrickSizeX*0.5)*cm,(TDumpFiltPosiZ+TDumpHz)*cm-LeadBrickSizeZ*i*cm);
             G4ThreeVector positionBrickT = G4ThreeVector(+(LeadBrickSizeY*0.5+LeadBrickSizeX*0.5)*cm,+LeadBrickSizeY*ii*cm,(TDumpFiltPosiZ+TDumpHz)*cm-LeadBrickSizeZ*i*cm);
             physiLeadBrickT = new G4PVPlacement(0,               // no rotation
           		 positionBrickT,   // at (0,0,0)
           		 logicLeadBrick,      // its logical volume
           		 "BrickWall",         // its name
           		 logicWorld,     // its mother  volume
           		 false,          // no boolean operations
           		 0,              // copy number
           		 false);         // is overlap

                          //G4ThreeVector positionBrickT = G4ThreeVector(+LeadBrickSizeY*ii*cm,+(LeadBrickSizeY*0.5+LeadBrickSizeX*0.5)*cm,(TDumpFiltPosiZ+TDumpHz)*cm-LeadBrickSizeZ*i*cm);
             G4ThreeVector positionBrickB = G4ThreeVector(-(LeadBrickSizeY*0.5+LeadBrickSizeX*0.5)*cm,+LeadBrickSizeY*ii*cm,(TDumpFiltPosiZ+TDumpHz)*cm-LeadBrickSizeZ*i*cm);
             physiLeadBrickB = new G4PVPlacement(0,               // no rotation
           		 positionBrickB,   // at (0,0,0)
           		 logicLeadBrick,      // its logical volume
           		 "BrickWall",         // its name
           		 logicWorld,     // its mother  volume
           		 false,          // no boolean operations
           		 0,              // copy number
           		 false);         // is overlap
		 }
	 }
//     G4ThreeVector positionDumpGFilt = G4ThreeVector(TDumpFiltPosiX*cm,TDumpFiltPosiY*cm,TDumpFiltPosiZ*cm);
//	 solidDumpGFilt = new G4Cons("DumpGFilt",TDumpFiltInnerRad1*cm,TDumpFiltOuterRad1*cm,TDumpFiltInnerRad2*cm,TDumpFiltOuterRad2*cm,TDumpFiltHz*cm,0*deg,360*deg);
//	 logicDumpGFilt = new G4LogicalVolume(solidDumpGFilt,W,"DumpTargFilt", 0, 0, 0);
//	 physiDumpGFilt = new G4PVPlacement(0,               // no rotati
//			 	 positionDumpGFilt,   // at (x,y,z)
//			 	 logicDumpGFilt,      // its logical volume
//			 	 "DumpFilt",          // its name
//			 	 logicWorld,          // its mother  volume
//			 	 false,               // no boolean operations
//			 	 0);                  // copy number
 }
 //------------------------------------------------- 
 // Direction monitor fused silica rods 
 //-------------------------------------------------
 if(IsMonitorON==1){
   G4ThreeVector positionMonitor = G4ThreeVector(MonitorPosiX*cm,MonitorPosiY*cm,MonitorPosiZ*cm); 
   G4double MonitorX      = MonitorSizeX*cm;
   G4double MonitorY      = MonitorSizeY*cm;
   G4double MonitorLength = MonitorSizeZ*cm;
   
   solidMonitor = new G4Box("target",MonitorX*0.5,MonitorY*0.5,MonitorLength*0.5);
   logicMonitor = new G4LogicalVolume(solidMonitor,WorldMater,"Monitor",0,0,0);
   physiMonitor = new G4PVPlacement(0,              // no rotation
				    positionMonitor,   // at (x,y,z)
				    logicMonitor,      // its logical volume                                  
				    "Monitor",         // its name
				    logicWorld,        // its mother  volume
				    false,             // no boolean operations
				    0,              // copy number 
				    false);          //Check for overlaps
   
   G4double MXRodLength = 2*mm;
   G4double MXRodX      = MonitorX-1;
   G4double MXRodY      = 2*mm;
   
   G4double MYRodLength = 2*mm;
   G4double MYRodX      = 2*mm;
   G4double MYRodY      = MonitorY-1;
   
   solidMXRod  = new G4Box("MXRod",MXRodX*0.5,MXRodY*0.5,MXRodLength*0.5);
   logicMXRod  = new G4LogicalVolume(solidMXRod,SiO2,"MXRod");
   
   solidMYRod  = new G4Box("MYRod",MYRodX*0.5,MYRodY*0.5,MYRodLength*0.5);
   logicMYRod  = new G4LogicalVolume(solidMYRod,SiO2,"MYRod");
   //
   G4int NMRodRows=10;
   for (G4int i=0;i<NMRodRows;i++){
     G4ThreeVector positionMXRod = G4ThreeVector(0.   ,-MonitorY*0.5+0.5*MXRodY+1*cm+i*MXRodY,+0.*cm);
     G4ThreeVector positionMYRod = G4ThreeVector(-MonitorX*0.5+0.5*MYRodX+i*MYRodX+1*cm,0.,1.*cm);
     
     physiMXRod  = new G4PVPlacement(0,             // no rotation
				     positionMXRod, // at (x,y,z)
				     logicMXRod,    // its logical volume                                  
				     "MXRod",       // its name
				     logicMonitor,  // its mother  volume
				     false,         // no boolean operations
				     i);            // copy number 
     
     physiMYRod  = new G4PVPlacement(0,              // no rotation
				     positionMYRod,  // at (x,y,z)
				     logicMYRod,     // its logical volume                                  
				     "MYRod",        // its name
				     logicMonitor,   // its mother  volume
				     false,          // no boolean operations
				     i+NMRodRows);   // copy number 
   }
 }
 
 if (fEnableECal) {
   fECalDetector->SetMotherVolume(logicWorld);
   fECalDetector->CreateGeometry();
 }
 /*
 if(IsEcalON==1){
   //------------------------------ 
   // ECal Defintion
   //------------------------------  
   G4ThreeVector positionEcal = G4ThreeVector(ECalPosiX*cm,ECalPosiY*cm,ECalPosiZ*cm); 
   G4double ECalX      = ECalSizeX*cm;
   G4double ECalY      = ECalSizeY*cm;
   G4double ECalLength = ECalSizeZ*cm;

   solidEcal = new G4Box("ECalSolid",ECalX*0.5+0.1*cm,ECalY*0.5+0.1*cm,ECalLength*0.5+0.1*cm);
   logicEcal = new G4LogicalVolume(solidEcal,Vacuum,"ECalLogic",0, 0, 0);
   physiEcal  = new G4PVPlacement(0,             // no rotation
				  positionEcal,  // at (x,y,z)
				  logicEcal,      // its logical volume
				  "ECal",        // its name
				  logicWorld,     // its mother  volume
				  false,         // no boolean operations
				  0,     // copy number 
				  false);          //Check for overlaps 
   
   G4double ECryLength = ECalLength-0.001*cm;
   G4double ECryX      = ECalX/ECalNRow-0.001*cm;;
   G4double ECryY      = ECalY/ECalNCol-0.001*cm;;
   G4int NCry=0;
   //   G4cout <<"Mother " <<  ECalX <<" " <<  ECalY<<" " <<  ECalLength<<G4endl;
   //   G4cout <<"Crys   " <<  ECryX <<" " <<  ECryY<<" " <<  ECryLength<<G4endl;
   solidCry  = new G4Box("Ecry",ECryX*0.5,ECryY*0.5,ECryLength*0.5);
   logicCry  = new G4LogicalVolume(solidCry,LSO,"ECry",0, 0, 0);
   
   G4int ncry=0;
   for (G4int i=0;i<ECalNRow;i++){
     for (G4int j=0;j<ECalNCol;j++){
       G4double PosXCry=-ECalX*0.5+0.5*ECryX+i*ECryX;
       G4double PosYCry=-ECalY*0.5+0.5*ECryY+j*ECryY;
       G4ThreeVector positionCry = G4ThreeVector(PosXCry,PosYCry,0.);
       G4int HoleFlag=0; //should be parametric in NRow NCol
       
       //       if( fabs(PosXCry)<ECalInnHole*cm && fabs(PosYCry)<ECalInnHole*cm ){
       //Inner radius ECalInnHole outer radius ECalSizeX
       if( (sqrt( PosXCry*PosXCry + PosYCry*PosYCry ) <ECalInnHole*cm ) || (sqrt( PosXCry*PosXCry + PosYCry*PosYCry ) > ECalSizeX*cm/2.) ){ 
	 HoleFlag=1;
       }else{
	 HoleFlag=0;
       }
       //       G4cout<<" i "<<i<<" "<<j<<" "<<PosXCry<<" "<<" InnHole "<<ECalInnHole<<" "<<HoleFlag<<G4endl;
       //     G4cout<<" i "<<i<<" "<<j<<" "<<PosXCry<<" "<<"Y"<<PosYCry<<" "<<HoleFlag<<G4endl;
       if(HoleFlag!=1){
	 physiCry  = new G4PVPlacement(0,             // no rotation
				       positionCry,   // at (x,y,z)
				       logicCry,      // its logical volume                                  
				       "ECry",        // its name
				       logicEcal,     // its mother  volume
				       false,         // no boolean operations
				       i+j*ECalNCol,  // copy number 
				       false);        //Check for overlaps
	 ncry ++;
	 NCry++;
       }
     }
   }//end of crystal placements 
   G4cout << "Total number of LYSO crystals:  " << ncry << G4endl;
   G4cout<<"placed "<<NCry<<" cristals "<<" at Z "<< positionEcal.getZ()<<G4endl;
 }
 */

 if(IsPosVetoON==1){
   //   solidPosVeto = new G4Box("posveto",PosVetoSizeX*cm*0.5,PosVetoSizeY*cm*0.5,PosVetoSizeZ*cm*0.5);
//   solidPosVeto = new G4Box("posveto",PosVetoSizeX*cm*0.5,PosVetoSizeY*cm*0.5,PosVetoSizeZ*cm*0.50);
//   logicPosVeto = new G4LogicalVolume(solidPosVeto,Scint,"PosVeto",0,0,0);
//   G4ThreeVector positionPosVeto = G4ThreeVector(PosVetoPosiX*cm,PosVetoPosiY*cm,PosVetoPosiZ*cm);  
//   physiPosVeto = new G4PVPlacement(0,              // no rotation
//				  positionPosVeto,  // at (x,y,z)
//				  logicPosVeto,     // its logical volume    
//				  "PositronVeto",   // its name
//				  logicSwepMag,       // its mother volume
//				  false,            // no boolean operations
//				  0,                // Copy number 
//				  true);
   G4int PosNFingers=PosVetoSizeZ/PosVetoFingerSize-1;
   solidPosVetoFinger = new G4Box("PosVetoF",PosVetoSizeX*cm*0.5-0.01*mm,PosVetoSizeY*cm*0.5-0.01*mm,PosVetoFingerSize*cm*0.5-0.01*mm);
   logicPosVetoFinger = new G4LogicalVolume(solidPosVetoFinger,Scint,"PosVetoF",0,0,0);
   //   G4cout<<"Number of positron veto fingers "<<PosNFingers<<G4endl;
   for(G4int ii=0;ii<PosNFingers;ii++){
     //     G4ThreeVector positionPosVetoFinger = G4ThreeVector(0*cm,0*cm,-PosVetoSizeZ/2.*cm+PosVetoFingerSize/2*cm+PosVetoFingerSize*ii*cm);
     G4ThreeVector positionPosVetoFinger = G4ThreeVector(PosVetoPosiX*cm,PosVetoPosiY*cm,PosVetoPosiZ*cm-PosVetoSizeZ/2.*cm+PosVetoFingerSize/2*cm+PosVetoFingerSize*ii*cm);
     //     G4cout<<"Number of positron veto fingers "<<ii<<" "<<positionPosVetoFinger<<G4endl;
     physiPosVetoFinger = new G4PVPlacement(0,
					 positionPosVetoFinger,  // at (x,y,z)
					 logicPosVetoFinger,     // its logical
					 "PosVetoFinger",       // its name
					 logicSwepMag,          // its mother
					 false,               // no boolean
					 ii,                  // Copy number
					 false);
   }
 }

 if(IsEleVetoON==1){
   //   solidEleVeto = new G4Box("eleveto",EleVetoSizeX*cm*0.5,EleVetoSizeY*cm*0.5,EleVetoSizeZ*cm*0.5);
//   solidEleVeto = new G4Box("eleveto",EleVetoSizeX*cm*0.5,EleVetoSizeY*cm*0.5,EleVetoSizeZ*cm*0.5);
//   logicEleVeto = new G4LogicalVolume(solidEleVeto,Scint,"EleVeto",0,0,0);  
//   G4ThreeVector positionEleVeto = G4ThreeVector(EleVetoPosiX*cm,EleVetoPosiY*cm,EleVetoPosiZ*cm);  
//   physiPosVeto = new G4PVPlacement(0,              // no rotation
//				  positionEleVeto,  // at (x,y,z)
//				  logicEleVeto,     // its logical volume    
//				  "ElectronVeto",   // its name
//				  logicSwepMag,     // its mother volume
//				  false,            // no boolean operations
//				  0,                // Copy number 
//				  true);
   G4int EleNFingers=EleVetoSizeZ/EleVetoFingerSize-1;
   solidEleVetoFinger = new G4Box("EleVetoF",EleVetoSizeX*cm*0.5-0.01*mm,EleVetoSizeY*cm*0.5-0.01*mm,EleVetoFingerSize*cm*0.5-0.01*mm);
   logicEleVetoFinger = new G4LogicalVolume(solidEleVetoFinger,Scint,"EleVetoF",0,0,0);
   for(G4int ii=0;ii<EleNFingers;ii++){
     G4ThreeVector positionEleVetoFinger = G4ThreeVector(EleVetoPosiX*cm,EleVetoPosiY*cm,EleVetoPosiZ*cm-EleVetoSizeZ/2.*cm+EleVetoFingerSize/2*cm+EleVetoFingerSize*ii*cm);
     //     G4cout<<"Number of positron veto fingers "<<ii<<" "<<positionEleVetoFinger<<G4endl;

     physiEleVetoFinger = new G4PVPlacement(0,
					    positionEleVetoFinger,  // at (x,y,z)
					    logicEleVetoFinger,     // its logical
					    "EleVetoFinger",       // its name
					    logicSwepMag,          // its mother
					    false,               // no boolean
					    ii,                  // Copy number
					    false);
   }
 }

 //---------------------------------------------------------------
 // High Energy positron veto scintillating part ouside the magnet
 //--------------------------------------------------------------- 
 if(IsEVetoON==1){
   solidEVeto = new G4Box("eveto",EVetoSizeX*cm*0.5,EVetoSizeY*cm*0.5,EVetoSizeZ*cm*0.5);
   logicEVeto = new G4LogicalVolume(solidEVeto,Scint,"EVeto",0,0,0);
   
   solidVetoFinger = new G4Box("evetoF",EVetoSizeX*cm*0.5,EVetoFingerSize*cm*0.5,EVetoSizeZ*cm*0.5);
   logicVetoFinger = new G4LogicalVolume(solidVetoFinger,Scint,"EVetoF",0,0,0);
   
   G4ThreeVector positionEVeto = G4ThreeVector(EVetoPosiX*cm,EVetoPosiY*cm,EVetoPosiZ*cm);  
   G4RotationMatrix* xRotL =new G4RotationMatrix; // Rotates X and Z axes only
   xRotL->rotateX(-0.593*rad);  // Rotates 90 degrees
   physiEVeto = new G4PVPlacement(xRotL,          // no rotation
				  positionEVeto,  // at (x,y,z)
				  logicEVeto,     // its logical volume    
				  "EVeto",        // its name
				  logicWorld,     // its mother volume
				  false,          // no boolean operations
				  0,              //Copy number 
				  false);
   G4int NFingers=EVetoSizeY/EVetoFingerSize;
   for(G4int ii=0;ii<NFingers;ii++){
     G4ThreeVector positionVetoFinger = G4ThreeVector(0*cm,-EVetoSizeY/2.*cm+EVetoFingerSize/2*cm+1*ii*cm,0*cm);
     physiVetoFinger = new G4PVPlacement(0,
					 positionVetoFinger,  // at (x,y,z)
					 logicVetoFinger,     // its logical
					 "EVetoFinger",       // its name
					 logicEVeto,          // its mother
					 false,               // no boolean
					 ii,                  // Copy number
					 false);
   }
 }

 //PLANAR GEM BASED SPECTROMETER
 if(IsPlanarGEMON==1){
   solidPGEM = new G4Box("pgem",PGEMSizeX*cm,PGEMSizeY*cm,PGEMSizeZ*cm);
   logicPGEM = new G4LogicalVolume(solidPGEM,Air,"PGEM",0,0,0);
   
   for(G4int ii=0;ii<NChambers;ii++){
     G4ThreeVector positionGEMPlane;
     if(ii<NChambers/2){
       positionGEMPlane = G4ThreeVector(PGEMPosiX*cm,PGEMPosiY*cm-(PGEMSizeY*cm+2*ii*PGEMSizeY*cm+0.01*mm),0.);
     }else{
       positionGEMPlane = G4ThreeVector(PGEMPosiX*cm,-PGEMPosiY*cm+(PGEMSizeY*cm+2*(ii-NChambers/2)*PGEMSizeY*cm+0.01*mm),0.);
     }
     G4cout<<positionGEMPlane<<" "<<PGEMSizeY<<" Nchambers "<<NChambers<<" "<<ii<<G4endl;
     physiPGEM = new G4PVPlacement(0,
				   positionGEMPlane,  // at (x,y,z)
				   logicPGEM,         // its logical
				   "PlanarGEM",       // its name
				   logicSwepMag,      // its mother
				   false,             // no boolean
				   ii,                // Copy number
				   false);
   }
 }
 
  //------------------------------------------------- 
  // Spectrometer chambers cilindrical
  //-------------------------------------------------

 if(IsTrackerON==1 && TrackerNLayers==1.){
   solidTracker[0]= new G4Tubs("TrackerBox",TrackerInnerRad*cm,TrackerOuterRad*cm,TrackerHz*cm,0.*deg,360.*deg);
   logicTracker[0]= new G4LogicalVolume(solidTracker[0],Air,"LogTracker",0,0,0);
   logicTracker[0]->SetFieldManager(fEmFieldSetup->GetLocalFieldManager(),allLocal);
   for(int kk=0;kk<TrackerNRings;kk++){
     G4ThreeVector positionTracker = G4ThreeVector(TrackerPosiX*cm,TrackerPosiY*cm,TrackerPosiZ*cm+kk*TrackerHz*cm); 
     physiTracker[0]=new G4PVPlacement(0,               // no rotation
				    positionTracker,  // at (x,y,z)
				    logicTracker[0],     // its logical volume    
				    "Tracker",        // its name
				    logicSwepMag,       // its mother volume
				    false,            // no boolean operations
				    kk,
				    false);               // copy number 
   }
 } else if(IsTrackerON==1 && TrackerNLayers>1.){
   for(int kk=0;kk<TrackerNLayers;kk++){
     //   G4cout<<TrackerInnerRad+kk*TrackerLayerTick<<" "<<(TrackerOuterRad+kk*TrackerLayerTick)<<G4endl;
     //     G4cout<<TrackerInnerRad<<" "<<TrackerOuterRad<<G4endl;
     G4double NewInn=TrackerInnerRad+kk*TrackerLayerTick;
     G4double NewOut=TrackerOuterRad+kk*TrackerLayerTick;
     G4cout<<NewInn<<" "<<NewOut<<G4endl;
     solidTracker[kk]= new G4Tubs("TrackerBox",NewInn*cm,NewOut*cm,TrackerHz*cm,0.*deg,360.*deg);
     logicTracker[kk]= new G4LogicalVolume(solidTracker[kk],Air,"LogTracker",0,0,0);
     logicTracker[kk]->SetFieldManager(fEmFieldSetup->GetLocalFieldManager(),allLocal);
     
     G4ThreeVector positionTracker = G4ThreeVector(TrackerPosiX*cm,TrackerPosiY*cm,TrackerPosiZ*cm); 
     physiTracker[0]= new G4PVPlacement(0,                // no rotation
				     positionTracker,  // at (x,y,z)
				     logicTracker[kk], // its logical volume    
				     "Tracker",        // its name
				     logicSwepMag,     // its mother volume
				     false,            // no boolean operations
				     kk,
				     false);              // copy number 
   }
 }

 //------------------------------------------------ 
 // Sensitive detectors
 //------------------------------------------------ 
 G4SDManager* SDman     = G4SDManager::GetSDMpointer();
 //G4String ECrySDname    = "ECrySD";      //Ecal sensitive detector
 G4String TrackerSDname = "TraSD";       //GEM Tracker sensitive detector
 G4String TRodSDname    = "TRodSD";      //target rods
 G4String MRodSDname    = "MRodSD";      //monitor rods
 G4String EVetoSDname   = "EVetoSD";     //High Energy Positron Veto
 G4String PosVetoSDname = "PosVetoSD";   //Positron Veto
 G4String EleVetoSDname = "EleVetoSD";   //Electron Veto
 //G4String SACSDname     = "SACSD";       //SAC detector
 G4String LAVSDname     = "LAVSD";       //LAV detector
 // G4String GFiltSDname   = "GFiltSD";     //Gamma filter

 /*
 if(IsEcalON==1){
   ECalSD* ECrySD = new ECalSD( ECrySDname );
   fECalDetector->GetCrystalLogicalVolume()->SetSensitiveDetector( ECrySD );
   //logicCry->SetSensitiveDetector( ECrySD );
   SDman->AddNewDetector( ECrySD );
 }
 */

  if(IsTrackerON==1){
    TrackerSD* TrackSD = new TrackerSD( TrackerSDname );
    SDman->AddNewDetector( TrackSD );
    for(int kk=0;kk<TrackerNLayers;kk++){
      logicTracker[kk]->SetSensitiveDetector( TrackSD );
    }
  }

  if(IsPlanarGEMON==1){
    TrackerSD* TrackSD = new TrackerSD( TrackerSDname );
    SDman->AddNewDetector( TrackSD );
    logicPGEM->SetSensitiveDetector( TrackSD );
  }

  /*
  if(IsTargetON){
    //Target SD
    TRodSD* TRodSDet = new TRodSD( TRodSDname );
    SDman->AddNewDetector( TRodSDet );
    logicTarget->SetSensitiveDetector( TRodSDet );
  }
  */
  
  if(IsTDumpON){
    //Dump as Sensitive detector
    TRodSD* TRodSDet = new TRodSD( TRodSDname );
    SDman->AddNewDetector( TRodSDet );
    logicTDump->SetSensitiveDetector( TRodSDet );
  }

  if(IsMonitorON==1){
    MRodSD* MRodSDet = new MRodSD( MRodSDname );
    SDman->AddNewDetector( MRodSDet );
    logicMXRod->SetSensitiveDetector( MRodSDet );
    logicMYRod->SetSensitiveDetector( MRodSDet );
  }

  if(IsEVetoON==1){ //CE DEVI METTERE LE STRIP MO SENNO' NON BECCHI IL replica NUMBB
    EVetoSD* EVetoSDet = new EVetoSD( EVetoSDname );
    SDman->AddNewDetector( EVetoSDet );
    logicVetoFinger->SetSensitiveDetector( EVetoSDet );
  }
  
  if(IsPosVetoON==1){ //CE DEVI METTERE LE STRIP MO SENNO' NON BECCHI IL replica NUMBB
    PosVetoSD* PosVetoSDet = new PosVetoSD( PosVetoSDname );
    SDman->AddNewDetector( PosVetoSDet );
    logicPosVetoFinger->SetSensitiveDetector( PosVetoSDet );
  }
  
  if(IsEleVetoON==1){ //CE DEVI METTERE LE STRIP MO SENNO' NON BECCHI IL replica NUMBB
    EleVetoSD* EleVetoSDet = new EleVetoSD( EleVetoSDname );
    SDman->AddNewDetector( EleVetoSDet );
    logicEleVetoFinger->SetSensitiveDetector( EleVetoSDet );
  }

  /*
  if(IsSACON==1){ //CE DEVI METTERE LE STRIP MO SENNO' NON BECCHI IL replica NUMBB
    SACSD* SACSDet = new SACSD( SACSDname );
    SDman->AddNewDetector( SACSDet );
    logicSAC->SetSensitiveDetector( SACSDet );
  }
  */

  if(IsLAVON==1){ //CE DEVI METTERE LE STRIP MO SENNO' NON BECCHI IL replica NUMBB
    LAVSD* LAVSDet = new LAVSD( LAVSDname );
    SDman->AddNewDetector( LAVSDet );
    logicLAV->SetSensitiveDetector( LAVSDet );
  }

  //  TRodSD* TRodSDet = new TRodSD( TRodSDname );
  //  SDman->AddNewDetector( TRodSDet );
  //  logicTXRod->SetSensitiveDetector( TRodSDet );
  //  logicTYRod->SetSensitiveDetector( TRodSDet );
//--------- Visualization attributes -------------------------------

  logicWorld  ->SetVisAttributes(G4VisAttributes::Invisible);
  // if(IsTargetON)  logicTarget ->SetVisAttributes(G4VisAttributes::Invisible);
  //  if(IsMonitorON) logicMonitor->SetVisAttributes(G4VisAttributes::Invisible);
  //if(IsEcalON)    logicEcal   ->SetVisAttributes(G4VisAttributes::Invisible);
  //if(IsEcalON) fECalDetector->GetECalLogicalVolume()->SetVisAttributes(G4VisAttributes::Invisible);
  //  logicSwepMag   ->SetVisAttributes(G4VisAttributes::Invisible);
  //  logicVetoFinger->SetVisAttributes(G4VisAttributes::Invisible);
  //  logicEVeto->SetVisAttributes(G4VisAttributes::Invisible);
  //  logicTracker   ->SetVisAttributes(G4VisAttributes::Invisible);
  //  if(IsEcalON)    logicCry    ->SetVisAttributes(G4VisAttributes::Invisible);
  //  logicCry    ->SetVisAttributes(G4VisAttributes::Invisible);
  //  logicGFilt   ->SetVisAttributes(G4VisAttributes::Invisible);

  //  logicTXRod  ->SetVisAttributes(G4VisAttributes::Invisible);
  //  logicTYRod  ->SetVisAttributes(G4VisAttributes::Invisible);

  G4VisAttributes* BoxVisAtt= new G4VisAttributes(G4Colour(1.0,1.0,1.0));
  //  logicWorld  ->SetVisAttributes(BoxVisAtt);  
  //  logicTarget ->SetVisAttributes(BoxVisAtt);

  return physiWorld;  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetupDetectors()
{
//  G4SDManager::GetSDMpointer()->SetVerboseLevel(1);
//  G4String filterName, particleName;
//
//  G4SDParticleFilter* gammaFilter   = new G4SDParticleFilter(filterName="gammaFilter",particleName="gamma");
//  G4SDParticleFilter* electronFilter= new G4SDParticleFilter(filterName="electronFilter",particleName="e-");
//  G4SDParticleFilter* positronFilter= new G4SDParticleFilter(filterName="positronFilter",particleName="e+");
//  G4SDParticleFilter* epFilter      = new G4SDParticleFilter(filterName="epFilter");
//  epFilter->add(particleName="e-");
//  epFilter->add(particleName="e+");
//
//  G4MultiFunctionalDetector* det = new G4MultiFunctionalDetector("SDcalo");
//  G4VPrimitiveScorer* primitive;
//  primitive = new G4PSEnergyDeposit("eDep",0);
//  det->RegisterPrimitive(primitive);
//  primitive = new G4PSNofSecondary("nGamma",0);
//  primitive->SetFilter(gammaFilter);
//  det->RegisterPrimitive(primitive);
//  G4SDManager::GetSDMpointer()->AddNewDetector(det);
//  logicEcal->SetSensitiveDetector(det);
}
 
void DetectorConstruction::DefineMaterials()
{}

void DetectorConstruction::setTargetMaterial(G4String materialName)
{
  // search the material by its name 
  G4Material* pttoMaterial = G4Material::GetMaterial(materialName);  
  if (pttoMaterial) {
    //TargetMater = pttoMaterial;
    //logicTarget->SetMaterial(pttoMaterial); 
    fTargetDetector->GetTargetLogicalVolume()->SetMaterial(pttoMaterial); 
    //      G4cout << "\n----> The target is " << fTargetLength/cm << " cm of "
    //             << materialName << G4endl;
  }             
}
 
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

//void DetectorConstruction::setChamberMaterial(G4String materialName)
//{
//  // search the material by its name 
//  G4Material* pttoMaterial = G4Material::GetMaterial(materialName);  
//  if (pttoMaterial)
//     {ChamberMater = pttoMaterial;
//      logicChamber->SetMaterial(pttoMaterial); 
//      G4cout << "\n----> The chambers are " << ChamberWidth/cm << " cm of "
//             << materialName << G4endl;
//     }             
//}
 
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
 
//void DetectorConstruction::SetMagField(G4double fieldValue)
//{
//  MagField->SetMagFieldValue(fieldValue);  
//}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::SetMaxStep(G4double maxStep)
{
  if ((stepLimit)&&(maxStep>0.)) stepLimit->SetMaxAllowedStep(maxStep);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
