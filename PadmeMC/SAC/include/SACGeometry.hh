// SACGeometry.hh
// --------------------------------------------------------------
// History:
//
// Created by Emanuele Leonardi (emanuele.leonardi@roma1.infn.it) 2105-12-14
// --------------------------------------------------------------

#ifndef SACGeometry_H
#define SACGeometry_H 1

#include "globals.hh"

class G4LogicalVolume;

class SACGeometry
{

public:

  ~SACGeometry();
  static SACGeometry* GetInstance();

private:

  static SACGeometry* fInstance;

protected:

  SACGeometry();

public:

  // Position of center of SAC box
  G4double GetSACPosX();
  G4double GetSACPosY();
  G4double GetSACPosZ();

  // Size of SAC box
  G4double GetSACSizeX();
  G4double GetSACSizeY();
  G4double GetSACSizeZ();

  // Number of rows and columns of crystals in SAC
  G4int GetSACNRows() { return fSACNRows; }
  G4int GetSACNCols() { return fSACNCols; }
  void  SetSACNRows(G4int r) { fSACNRows = r; }
  void  SetSACNCols(G4int c) { fSACNCols = c; }

  // Check if crystal exists at given row/column
  G4int ExistsCrystalAt(G4int,G4int);

  // Position of center of crystal at given row/column
  G4double GetCrystalPosX(G4int,G4int);
  G4double GetCrystalPosY(G4int,G4int);
  G4double GetCrystalPosZ(G4int,G4int);

  // Size of SAC BaF2 crystal
  G4double GetCrystalSizeX();
  G4double GetCrystalSizeY();
  G4double GetCrystalSizeZ();

  // Set nominal size of crystal
  void SetCrystalNominalSizeX(G4double s) { fCrystalNominalSizeX = s; }
  void SetCrystalNominalSizeY(G4double s) { fCrystalNominalSizeY = s; }
  void SetCrystalNominalSizeZ(G4double s) { fCrystalNominalSizeZ = s; }

  // Set position along Z of SAC front face
  void SetSACFrontFacePosZ(G4double z) { fSACFrontFacePosZ = z; }

  // Get name of SAC sensitive detector
  G4String GetSACSensitiveDetectorName() { return fSACSensitiveDetectorName; }

private:

  G4double fCrystalNominalSizeX;
  G4double fCrystalNominalSizeY;
  G4double fCrystalNominalSizeZ;

  G4int fSACNRows;
  G4int fSACNCols;

  G4double fCrystalGap; // Gap size between adjacent crystals

  G4double fSACFrontFacePosZ; // Position along Z axis of SAC front face

  G4String fSACSensitiveDetectorName;

};

#endif
