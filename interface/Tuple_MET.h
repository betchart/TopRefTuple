#ifndef TUPLE_MET
#define TUPLE_MET

#include "TopQuarkAnalysis/TopRefTuple/interface/fTypes.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/Candidate/interface/Candidate.h"

template< typename T >
class Tuple_MET : public edm::EDProducer {
 public: 
  explicit Tuple_MET(const edm::ParameterSet&);
 private: 
  void produce(edm::Event&, const edm::EventSetup& );
  const edm::InputTag metTag,particlesTag;
  const std::string prefix,particlesPrefix;
};

template< typename T >
Tuple_MET<T>::Tuple_MET(const edm::ParameterSet& cfg) 
  : metTag(cfg.getParameter<edm::InputTag>("metTag"))
  , particlesTag(cfg.getParameter<edm::InputTag>("particlesTag"))
  , prefix(cfg.getParameter<std::string>("prefix"))
  , particlesPrefix(cfg.getParameter<std::string>("particlesPrefix"))
{
  produces <fTypes::dPolarLorentzV> ( prefix + "P4"   );
  produces <fTypes::dPolarLorentzV> ( particlesPrefix + "SumP4" );
  produces <float> ( prefix + "SumEt" );
  produces <bool>   ( prefix + "HandleValid" );
  produces <float> ( prefix + "Significance" );
  produces <float> ( prefix + "SigmaXX" );
  produces <float> ( prefix + "SigmaYY" );
  produces <float> ( prefix + "SigmaXY" );
}

template< typename T >
void Tuple_MET<T>::
produce(edm::Event& event, const edm::EventSetup& setup) {
  edm::Handle<std::vector<T> > metcollection;
  event.getByLabel(metTag, metcollection);
  
  const T* met = metcollection.isValid() ? &(metcollection->at(0)) : 0;
  event.put(std::auto_ptr<bool>(new bool(met)), prefix + "HandleValid" );
  event.put(std::auto_ptr<float>( new float( met ? met->sumEt() : 0 )),  prefix + "SumEt");
  event.put(std::auto_ptr<fTypes::dPolarLorentzV>( met ?
						   new fTypes::dPolarLorentzV(met->p4().pt(), met->p4().eta(), met->p4().phi(), met->p4().mass() ) :
						   new fTypes::dPolarLorentzV(0,0,0,0)),    prefix+"P4");
  float significance(0), sigmaXX(0), sigmaYY(0), sigmaXY(0);
  try {
    significance = !met ? 0 : met->significance();
    if(significance) {
      TMatrixD M = met->getSignificanceMatrix();
      sigmaXX = M(0,0); sigmaXY = M(0,1); sigmaYY = M(1,1);
    }
  }
  catch(...) { significance = -1; }
  event.put(std::auto_ptr<float>( new float(significance)),  prefix+"Significance");
  event.put(std::auto_ptr<float>( new float(sigmaXX)),  prefix+"SigmaXX" );
  event.put(std::auto_ptr<float>( new float(sigmaYY)),  prefix+"SigmaYY" );
  event.put(std::auto_ptr<float>( new float(sigmaXY)),  prefix+"SigmaXY" );

  reco::Candidate::LorentzVector sumP4(0,0,0,0);
  if( met ) {
    edm::Handle<edm::View<reco::Candidate> > candidates;
    event.getByLabel(particlesTag,candidates);
    for(edm::View<reco::Candidate>::const_iterator it = candidates->begin(); it != candidates->end() ; ++it)
      sumP4 += it->p4();
  }
  event.put(std::auto_ptr<fTypes::dPolarLorentzV>( new fTypes::dPolarLorentzV(sumP4.pt(), sumP4.eta(), sumP4.phi(), sumP4.mass()) ), particlesPrefix+"SumP4");
}

#endif
