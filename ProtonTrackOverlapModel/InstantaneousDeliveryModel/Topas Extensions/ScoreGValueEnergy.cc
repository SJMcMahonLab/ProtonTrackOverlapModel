// Scorer for GValueEnergy
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

#include "ScoreGValueEnergy.hh"

#include "G4RunManager.hh"
#include "G4EventManager.hh"
#include "G4SystemOfUnits.hh"

#include "G4DNAChemistryManager.hh"
#include "G4MoleculeCounter.hh"
#include "G4MolecularConfiguration.hh"
#include "G4Molecule.hh"

ScoreGValueEnergy::ScoreGValueEnergy(TsParameterManager* pM, TsMaterialManager* mM, TsGeometryManager* gM, TsScoringManager* scM, TsExtensionManager* eM,
							 G4String scorerName, G4String quantity, G4String outFileName, G4bool isSubScorer)
: TsVNtupleScorer(pM, mM, gM, scM, eM, scorerName, quantity, outFileName, isSubScorer),
fPm(pM), fEnergyDepositPerEvent(0)
{
	G4MoleculeCounter::Instance()->Use();
	G4DNAChemistryManager::Instance()->ResetCounterWhenRunEnds(false);
	
	fNtuple->RegisterColumnF(&fGValue, "GValue: number of molecules per energy deposit per eV", "");
	fNtuple->RegisterColumnF(&fGValueError, "GValue statistical error", "");
	fNtuple->RegisterColumnF(&fTime,    "Time", "picosecond");
	fNtuple->RegisterColumnS(&fMoleculeName, "MoleculeName");
	
	G4int tBins = 10;
        if ( fPm->ParameterExists(GetFullParmName("TimeBins")))
		tBins = fPm->GetIntegerParameter(GetFullParmName("TimeBins"));

	G4double tBinMin = 1.0 * ps;
	if ( fPm->ParameterExists(GetFullParmName("TimeBinMin")))
		tBinMin = fPm->GetDoubleParameter(GetFullParmName("TimeBinMin"),"Time");

	G4double tBinMax = 1.0 * us;
	if ( fPm->ParameterExists(GetFullParmName("TimeBinMax")))
		tBinMax = fPm->GetDoubleParameter(GetFullParmName("TimeBinMax"),"Time");

	G4bool tBinLog = true;
	if ( fPm->ParameterExists(GetFullParmName("TimeBinLog")))
		tBinLog = fPm->GetBooleanParameter(GetFullParmName("TimeBinLog"));
	
	if ( !tBinLog ) {
		for ( int i = 0; i < tBins; i++ )
			fTimeToRecord.push_back(tBinMin + i * (tBinMax - tBinMin)/tBins);
		
	} else {
		if ( tBinMin < 1.0 * ps ) {
			G4cerr << "Topas is exiting due to a serious error in scoring." << G4endl;
			G4cerr << "The scorer named: " << GetName() << " has TimeBinMin lower than 1 ps" << G4endl;
			fPm->AbortSession(1);
		}
		
		G4double logXMin = std::log10(tBinMin);
		G4double logXMax = std::log10(tBinMax);
		for ( int i = 0; i < tBins; i++ )
			fTimeToRecord.push_back( std::pow(10, logXMin + i * (logXMax - logXMin)/(tBins-1)) );
	}
	
	fEnergyLossKill = 0.0;
	if ( fPm->ParameterExists(GetFullParmName("KillIfPrimaryAccumulatedEnergyLossExceeds")) )
		fEnergyLossKill = fPm->GetDoubleParameter(GetFullParmName("KillIfPrimaryAccumulatedEnergyLossExceeds"),"Energy");
	
	fEnergyLossAbort = 0.0;
	if ( fPm->ParameterExists(GetFullParmName("AbortEventIfPrimaryAccumulatedEnergyLossExceeds") ) )
		fEnergyLossAbort = fPm->GetDoubleParameter(GetFullParmName("AbortEventIfPrimaryAccumulatedEnergyLossExceeds"), "Energy");
	
	fMaximumTrackLength = 0.0;
	if ( fPm->ParameterExists(GetFullParmName("KillIfPrimaryAccumulatedTrackLengthExceeds")) )
		fMaximumTrackLength = fPm->GetDoubleParameter(GetFullParmName("KillIfPrimaryAccumulatedTrackLengthExceeds"), "Length");
	
	fNbOfScoredEvents = 0;
}


ScoreGValueEnergy::~ScoreGValueEnergy()
{;}


G4bool ScoreGValueEnergy::ProcessHits(G4Step* aStep, G4TouchableHistory*)
{
	if (!fIsActive) {
		fSkippedWhileInactive++;
		return false;
	}
	
	if ( -1 < aStep->GetTrack()->GetTrackID() ) { // physical tracks
		if ( aStep->GetTrack()->GetParentID() == 0 ) {
			G4double kineticE = aStep->GetPostStepPoint()->GetKineticEnergy();
			G4double eLoss = aStep->GetPreStepPoint()->GetKineticEnergy() - kineticE;
			fEnergyLoss += eLoss;
			fTotalTrackLength += aStep->GetStepLength();
			
			if ( 0 < fEnergyLossAbort && fEnergyLoss > fEnergyLossAbort ) {
				std::cout << " -- Aborting event " << GetEventID() << std::endl;
				G4RunManager::GetRunManager()->AbortEvent();
				return true;
			}
			
			if ( 0 < fEnergyLossKill && fEnergyLoss >= fEnergyLossKill ) {
				std::cout << " -- Killing primary based on maximum accumulated energ loss." << std::endl;
				aStep->GetTrack()->SetTrackStatus(fStopAndKill);
				return true;
			}
			
			if ( 0 < fMaximumTrackLength && fTotalTrackLength >= fMaximumTrackLength ) {
				aStep->GetTrack()->SetTrackStatus(fStopAndKill);
				std::cout << " -- Killing primary based on maximum accumulated track-length." << std::endl;
				return true;
			}
		}
	}
	
	G4double edep = aStep->GetTotalEnergyDeposit();
	if ( edep > 0 ) {
		fEnergyDepositPerEvent += edep * aStep->GetPreStepPoint()->GetWeight();
		return true;
	}
	
	return false;
}


void ScoreGValueEnergy::AccumulateEvent() {
	
	if (!G4EventManager::GetEventManager()->GetConstCurrentEvent()->IsAborted() && fEnergyDepositPerEvent > 0 ) {
		G4MoleculeCounter::RecordedMolecules species = G4MoleculeCounter::Instance()->GetRecordedMolecules();
		
		if ( species.get() == 0 ) return;
		
		auto molecule_it = species->begin();
		
		while ( molecule_it != species->end() ) {
			for ( int iTime = 0; iTime < (int)fTimeToRecord.size(); iTime++ ) {
				G4int moleculeAtSpecificTime = G4MoleculeCounter::Instance()->GetNMoleculesAtTime((*molecule_it), fTimeToRecord[iTime]);
				G4String moleculeID = (*molecule_it)->GetName();
				G4double gvalue = moleculeAtSpecificTime/(fEnergyDepositPerEvent/eV);
				fGValuePerSpeciePerTime[moleculeID][fTimeToRecord[iTime]] += gvalue * fEnergyDepositPerEvent;
				fGValuePerSpeciePerTime2[moleculeID][fTimeToRecord[iTime]] += gvalue * gvalue;
				// G4cout << "Scoring at time " << fTimeToRecord[iTime] << " for molecule:\t" << moleculeID << "\twith count:\t" << moleculeAtSpecificTime 
				// << "\tand energy:\t" << fEnergyDepositPerEvent << "\tfor a G value of:\t" << gvalue <<"\tand logged scaled value:\t" << gvalue*fEnergyDepositPerEvent << G4endl;
			}
			molecule_it++;
		}
		fNbOfScoredEvents++;
		fTotalEnergyDeposit += fEnergyDepositPerEvent;
	}
	
	G4MoleculeCounter::Instance()->ResetCounter();
	fEnergyDepositPerEvent = 0.0;
	fEnergyLoss = 0.0;
	fTotalTrackLength = 0.0;
	
}


void ScoreGValueEnergy::AbsorbResultsFromWorkerScorer(TsVScorer* workerScorer) {
	TsVNtupleScorer::AbsorbResultsFromWorkerScorer(workerScorer);
	
	ScoreGValueEnergy* workerGvalueScorer = dynamic_cast<ScoreGValueEnergy*>(workerScorer);
	
	fNbOfScoredEvents += workerGvalueScorer->fNbOfScoredEvents;
	fTotalEnergyDeposit += workerGvalueScorer->fTotalEnergyDeposit;
	
	std::map<G4String, std::map<G4double, G4double> >::iterator wIter;
	std::map<G4String, std::map<G4double, G4double> >::iterator mIter;
	
	for ( wIter = workerGvalueScorer->fGValuePerSpeciePerTime.begin(); wIter != workerGvalueScorer->fGValuePerSpeciePerTime.end(); wIter++) {
		mIter = fGValuePerSpeciePerTime.find( wIter->first );
		if ( mIter == fGValuePerSpeciePerTime.end() ) {
			fGValuePerSpeciePerTime.insert(std::pair<G4String, std::map<G4double, G4double> > ( wIter->first, wIter->second));
		} else {
			std::map<G4double, G4double>::iterator witer;
			std::map<G4double, G4double>::iterator miter;
			for ( witer = (wIter->second).begin(); witer != (wIter->second).end(); witer++ ) {
				miter = (mIter->second).find( witer->first );
				miter->second += witer->second;
			}
		}
	}
	
	std::map<G4String, std::map<G4double, G4double> >::iterator wIter2;
	std::map<G4String, std::map<G4double, G4double> >::iterator mIter2;
	
	for ( wIter2 = workerGvalueScorer->fGValuePerSpeciePerTime2.begin(); wIter2 != workerGvalueScorer->fGValuePerSpeciePerTime2.end(); wIter2++) {
		mIter2 = fGValuePerSpeciePerTime2.find( wIter2->first );
		if ( mIter2 == fGValuePerSpeciePerTime2.end() ) {
			fGValuePerSpeciePerTime2.insert(std::pair<G4String, std::map<G4double, G4double> > ( wIter2->first, wIter2->second));
		} else {
			std::map<G4double, G4double>::iterator witer2;
			std::map<G4double, G4double>::iterator miter2;
			for ( witer2 = (wIter2->second).begin(); witer2 != (wIter2->second).end(); witer2++ ) {
				miter2 = (mIter2->second).find( witer2->first );
				miter2->second += witer2->second;
			}
		}
	}
	
	workerGvalueScorer->fGValuePerSpeciePerTime.clear();
	workerGvalueScorer->fGValuePerSpeciePerTime2.clear();
	workerGvalueScorer->fNbOfScoredEvents = 0;
	workerGvalueScorer->fEnergyDepositPerEvent = 0.0;
	workerGvalueScorer->fTotalEnergyDeposit = 0.0;

}


void ScoreGValueEnergy::Output() {
	
	if ( fNbOfScoredEvents == 0 )
		return;
	
	std::map<G4String, std::map<G4double, G4double> >::iterator wIter;
	std::map<G4String, std::map<G4double, G4double> >::iterator wIter2;
	std::map<G4double, G4double>::iterator iter;
	std::map<G4double, G4double>::iterator iter2;
	
	for ( wIter = fGValuePerSpeciePerTime.begin(); wIter != fGValuePerSpeciePerTime.end(); wIter++ ) {
		wIter2 = fGValuePerSpeciePerTime2.find(wIter->first);
		
		for ( iter = (wIter->second).begin(); iter != (wIter->second).end(); iter++ ) {
			iter2 = (wIter2->second).find( iter->first );
			
			//fGValue = iter->second/fNbOfScoredEvents;
			fGValue = iter->second/fTotalEnergyDeposit;
			if ( fNbOfScoredEvents > 1 ) { // Statistical uncertainty, not standard deviation
				fGValueError = sqrt( (1.0/(fNbOfScoredEvents-1)) * ( (iter2->second)/fNbOfScoredEvents - fGValue*fGValue));
			} else {
				fGValueError = 1.0;
			}
			fTime = iter->first;
			fMoleculeName = wIter->first;
			fNtuple->Fill();
		}
	}
	
	fNtuple->Write();
}


void ScoreGValueEnergy::Clear() {
	fGValuePerSpeciePerTime.clear();
	fNbOfScoredEvents = 0;
	fTotalEnergyDeposit = 0;
	UpdateFileNameForUpcomingRun();
}


