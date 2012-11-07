#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_Electron.h"


// init method in case of RECO data
template<>
void Tuple_Electron<reco::GsfElectron>::initTemplate() { initRECO(); }

// init method in case of PAT data
template<>
void Tuple_Electron<pat::Electron>::initTemplate() { initRECO(); initPAT(); }

// produce method in case of RECO data
template<>
void Tuple_Electron<reco::GsfElectron>::produceTemplate(edm::Event& iEvent, const edm::EventSetup& iSetup, edm::Handle<std::vector<reco::GsfElectron> >& collection)
{
  produceRECO(iEvent, iSetup, collection);
}

// produce method in case of PAT data
template<>
void Tuple_Electron<pat::Electron>::produceTemplate(edm::Event& iEvent, const edm::EventSetup& iSetup, edm::Handle<std::vector<pat::Electron> >& collection)
{
  produceRECO(iEvent, iSetup, collection);
  producePAT(iEvent, iSetup, collection);
}

template<class T>
bool Tuple_Electron<T>::isInCollection(const T& e, const std::vector<T>& ve) {
  typename std::vector<T>::const_iterator it(ve.begin()),end(ve.end());
  for(;it!=end;++it)
    if( e.p4() == it->p4() ) return true;
  return false;
}
