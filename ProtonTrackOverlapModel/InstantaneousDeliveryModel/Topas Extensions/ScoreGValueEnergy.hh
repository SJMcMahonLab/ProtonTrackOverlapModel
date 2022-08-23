//
// ********************************************************************
// *                                                                  *
// * This file is part of the TOPAS-nBio extensions to the            *
// *   TOPAS Simulation Toolkit.                                      *
// * The TOPAS-nBio extensions are freely available under the license *
// *   agreement set forth at: https://topas-nbio.readthedocs.io/     *
// *                                                                  *
// ********************************************************************
//
// G value scorer modified to weight simulations between runs by energy, 
// rather than a simple average. Intended to give better agreement with
// simultaneous simulations
//


#ifndef ScoreGValueEnergy_hh
#define ScoreGValueEnergy_hh

#include "TsVNtupleScorer.hh"

#include <stdint.h>

class G4MolecularConfiguration;

class ScoreGValueEnergy : public TsVNtupleScorer
{
public:
    ScoreGValueEnergy(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
                      G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer);
    ~ScoreGValueEnergy();
    
    virtual G4bool ProcessHits(G4Step*, G4TouchableHistory*);
    void AbsorbResultsFromWorkerScorer(TsVScorer* workerScorer);
    
protected:
    void AccumulateEvent();
    
    void Output();
    void Clear();
    
    // Output variables
    G4float fGValue;
    G4float fGValueError;
    G4float fTime;
    G4String fMoleculeName;
    
    std::map<G4String, std::map<G4double, G4double> > fGValuePerSpeciePerTime;
    std::map<G4String, std::map<G4double, G4double> > fGValuePerSpeciePerTime2;
    
private:
    TsParameterManager* fPm;
    
    G4double fEnergyDepositPerEvent;
    std::vector<G4double> fTimeToRecord;
    G4int fNbOfScoredEvents;
    G4double fEnergyLoss;
    G4double fEnergyLossKill;
    G4double fEnergyLossAbort;
    G4double fMaximumTrackLength;
    G4double fTotalTrackLength;
    G4double fTotalEnergyDeposit;
};

#endif

