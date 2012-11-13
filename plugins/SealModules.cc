#include "FWCore/Framework/interface/MakerMacros.h"

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"

#include "TopQuarkAnalysis/TopRefTuple/interface/TreeMaker.h"
#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_Event.h"
#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_Triggers.h"
//#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_L1Triggers.h"
#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_MET.h"
#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_Jet.h"
#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_Electron.h"
#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_Muon.h"
//#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_BeamSpot.h"
//#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_BeamHaloSummary.h"
//#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_LogError.h"
//#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_Vertex.h"
#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_PileupSummary.h"
#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_Gen.h"

typedef Tuple_MET<pat::MET> Tuple_PatMET;
typedef Tuple_Jet<pat::Jet> Tuple_PatJet;
typedef Tuple_Gen<reco::GenParticle> Tuple_GenParticle;

DEFINE_FWK_MODULE(TreeMaker);
DEFINE_FWK_MODULE(Tuple_Event);
DEFINE_FWK_MODULE(Tuple_Triggers);
//DEFINE_FWK_MODULE(Tuple_L1Triggers);
DEFINE_FWK_MODULE(Tuple_PatMET);
DEFINE_FWK_MODULE(Tuple_PatJet);
DEFINE_FWK_MODULE(Tuple_Electron);
DEFINE_FWK_MODULE(Tuple_Muon);
//DEFINE_FWK_MODULE(Tuple_BeamSpot);
//DEFINE_FWK_MODULE(Tuple_BeamHaloSummary);
//DEFINE_FWK_MODULE(Tuple_LogError);
//DEFINE_FWK_MODULE(Tuple_Vertex);
DEFINE_FWK_MODULE(Tuple_PileupSummary);
DEFINE_FWK_MODULE(Tuple_GenParticle);
