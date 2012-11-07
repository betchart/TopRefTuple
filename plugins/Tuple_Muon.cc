#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_Muon.h"


// init method in case of RECO data
template<>
void Tuple_Muon<reco::Muon>::initTemplate() { initRECO(); }

// init method in case of PAT data
template<>
void Tuple_Muon<pat::Muon>::initTemplate()  { initRECO(); initPAT(); }

// produce method in case of RECO data
template<>
void Tuple_Muon<reco::Muon>::produceTemplate(edm::Event& iEvent, const edm::EventSetup& iSetup, edm::Handle<std::vector<reco::Muon> >& collection)
{
  produceRECO(iEvent, iSetup, collection);
}

// produce method in case of PAT data
template<>
void Tuple_Muon<pat::Muon>::produceTemplate(edm::Event& iEvent, const edm::EventSetup& iSetup, edm::Handle<std::vector<pat::Muon> >& collection)
{
  produceRECO(iEvent, iSetup, collection);
  producePAT(iEvent, iSetup, collection);
}

template<class T>
bool Tuple_Muon<T>::isInCollection(const T& e, const std::vector<T>& ve) {
  typename std::vector<T>::const_iterator it(ve.begin()),end(ve.end());
  for(;it!=end;++it)
    if( e.p4() == it->p4() ) return true;
  return false;
}



template<class T>
reco::Candidate::LorentzVector Tuple_Muon<T>::muonP4FromP(const reco::Candidate::Vector & p) {
  double E = sqrt(pow(p.r(),2) + pow(0.105,2));
  return reco::Candidate::LorentzVector(p.x(),p.y(),p.z(),E);
}
