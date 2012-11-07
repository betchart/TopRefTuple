#ifndef TUPLE_MET
#define TUPLE_MET

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/METReco/interface/PFMET.h"
#include "DataFormats/METReco/interface/CaloMET.h"
#include "DataFormats/PatCandidates/interface/MET.h"

template< typename T >
class Tuple_MET : public edm::EDProducer {
 public: 
  explicit Tuple_MET(const edm::ParameterSet&);
 private: 
  void produce(edm::Event&, const edm::EventSetup& );
  void produceSpecial(edm::Event&, const T* );
  void produceCalo(edm::Event&, const T* );
  void initSpecial();
  void initCalo();
  const edm::InputTag inputTag;
  const bool special, caloSpecific;
  const std::string Prefix,Suffix;
};


template< typename T >
Tuple_MET<T>::Tuple_MET(const edm::ParameterSet& cfg) :
  inputTag(cfg.getParameter<edm::InputTag>("InputTag")),
  special(cfg.getParameter<bool>("ProduceSpecial")),
  caloSpecific(cfg.getParameter<bool>("Calo")),
  Prefix(cfg.getParameter<std::string>("Prefix")),
  Suffix(cfg.getParameter<std::string>("Suffix"))
{
  produces <reco::Candidate::LorentzVector> ( Prefix + "P4"  + Suffix );
  produces <double>                         ( Prefix + "SumEt" + Suffix );
  produces <bool>             ( Prefix + "HandleValid" + Suffix );
  produces <double>           ( Prefix + "Significance"  + Suffix );
  produces <double>           ( Prefix + "SigmaXX"  + Suffix );
  produces <double>           ( Prefix + "SigmaYY"  + Suffix );
  produces <double>           ( Prefix + "SigmaXY"  + Suffix );
  if(special) initSpecial();
}

template< typename T >
void Tuple_MET<T>::
produce(edm::Event& event, const edm::EventSetup& setup) {
  edm::Handle<std::vector<T> > metcollection;
  event.getByLabel(inputTag, metcollection);
  
  const T* met = metcollection.isValid() ? &(metcollection->at(0)) : 0;
  event.put(std::auto_ptr<bool>(new bool(met)), Prefix + "HandleValid" + Suffix);
  event.put(std::auto_ptr<double>( new double( met ? met->sumEt() : 0 )),  Prefix + "SumEt" +Suffix);
  event.put(std::auto_ptr<reco::Candidate::LorentzVector>( met ?
							   new reco::Candidate::LorentzVector(met->p4() ) :
							   new reco::Candidate::LorentzVector(0,0,0,0)),    Prefix+"P4"+Suffix);
  double significance(0), sigmaXX(0), sigmaYY(0), sigmaXY(0);
  try {
    significance = !met ? 0 : met->significance();
    if(significance) {
      TMatrixD M = met->getSignificanceMatrix();
      sigmaXX = M(0,0); sigmaXY = M(0,1); sigmaYY = M(1,1);
    }
  }
  catch(...) { significance = -1; }
  event.put(std::auto_ptr<double>( new double(significance)),  Prefix+"Significance" +Suffix);
  event.put(std::auto_ptr<double>( new double(sigmaXX)),  Prefix+"SigmaXX" +Suffix);
  event.put(std::auto_ptr<double>( new double(sigmaYY)),  Prefix+"SigmaYY" +Suffix);
  event.put(std::auto_ptr<double>( new double(sigmaXY)),  Prefix+"SigmaXY" +Suffix);

  if(special) produceSpecial(event, met);
}

template<> void Tuple_MET<reco::GenMET>::initSpecial() { }
template<> void Tuple_MET<reco::CaloMET>::initSpecial() { initCalo(); }
template<> void Tuple_MET<pat::MET>::initSpecial() {  initCalo();}

template<> void Tuple_MET<reco::GenMET>::produceSpecial(edm::Event& event, const reco::GenMET* met) { }
template<> void Tuple_MET<reco::CaloMET>::produceSpecial(edm::Event& event, const reco::CaloMET* met) { produceCalo(event,met);}
template<> void Tuple_MET<pat::MET>::produceSpecial(edm::Event& event, const pat::MET* met) { produceCalo(event,met);}

template<> void Tuple_MET<reco::PFMET>::initSpecial() {
  produces<double>( Prefix+ "PhotonEtFraction"        + Suffix);
  produces<double>( Prefix+ "ElectronEtFraction"      + Suffix);
  produces<double>( Prefix+ "NeutralHadronEtFraction" + Suffix);
  produces<double>( Prefix+ "ChargedHadronEtFraction" + Suffix);
  produces<double>( Prefix+ "MuonEtFraction"	      + Suffix);
  produces<double>( Prefix+ "HFHadronEtFraction"      + Suffix);
  produces<double>( Prefix+ "HFEMEtFraction"          + Suffix);
}

template<> void Tuple_MET<reco::PFMET>::produceSpecial(edm::Event& event, const reco::PFMET* met) {
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->photonEtFraction() )),        Prefix + "PhotonEtFraction"        + Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->electronEtFraction() )),	     Prefix + "ElectronEtFraction"      + Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->neutralHadronEtFraction() )), Prefix + "NeutralHadronEtFraction" + Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->chargedHadronEtFraction() )), Prefix + "ChargedHadronEtFraction" + Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->muonEtFraction() )),	     Prefix + "MuonEtFraction"	        + Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->HFHadronEtFraction() )),	     Prefix + "HFHadronEtFraction"      + Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->HFEMEtFraction() )),          Prefix + "HFEMEtFraction"          + Suffix);
}


template<class T>
void Tuple_MET<T>::
initCalo() {
  if(!caloSpecific) return;
  produces <double> ( Prefix + "CaloMETInmHF"  + Suffix );
  produces <double> ( Prefix + "CaloMETInpHF"  + Suffix );
  produces <double> ( Prefix + "CaloMETPhiInmHF"  + Suffix );
  produces <double> ( Prefix + "CaloMETPhiInpHF"  + Suffix );
  produces <double> ( Prefix + "CaloSETInmHF"  + Suffix );
  produces <double> ( Prefix + "CaloSETInpHF"  + Suffix );

  produces <double> ( Prefix + "EmEtFraction"  + Suffix );
  produces <double> ( Prefix + "EtFractionHadronic"  + Suffix );
  produces <double> ( Prefix + "MaxEtInEmTowers"  + Suffix );
  produces <double> ( Prefix + "MaxEtInHadTowers"  + Suffix );
  produces <double> ( Prefix + "EmEtInEB"  + Suffix );
  produces <double> ( Prefix + "EmEtInEE"  + Suffix );
  produces <double> ( Prefix + "EmEtInHF"  + Suffix );
  produces <double> ( Prefix + "HadEtInHB"  + Suffix );
  produces <double> ( Prefix + "HadEtInHE"  + Suffix );
  produces <double> ( Prefix + "HadEtInHF"  + Suffix );
  produces <double> ( Prefix + "HadEtInHO"  + Suffix );
}

template< typename T>
void Tuple_MET<T>::
produceCalo(edm::Event& event, const T* met) {
  if(!caloSpecific) return;
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->CaloMETInmHF())), Prefix+"CaloMETInmHF"+Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->CaloMETInpHF())), Prefix+"CaloMETInpHF"+Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->CaloMETPhiInmHF())), Prefix+"CaloMETPhiInmHF"+Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->CaloMETPhiInpHF())), Prefix+"CaloMETPhiInpHF"+Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->CaloSETInmHF())), Prefix+"CaloSETInmHF"+Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->CaloSETInpHF())), Prefix+"CaloSETInpHF"+Suffix);
				    							  
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->emEtFraction())), Prefix+"EmEtFraction"+Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->etFractionHadronic())), Prefix+"EtFractionHadronic"+Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->maxEtInEmTowers())), Prefix+"MaxEtInEmTowers"+Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->maxEtInHadTowers())), Prefix+"MaxEtInHadTowers"+Suffix);
				    							  
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->emEtInEB())), Prefix+"EmEtInEB"+Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->emEtInEE())), Prefix+"EmEtInEE"+Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->emEtInHF())), Prefix+"EmEtInHF"+Suffix);
				    							  
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->hadEtInHB())), Prefix+"HadEtInHB"+Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->hadEtInHE())), Prefix+"HadEtInHE"+Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->hadEtInHF())), Prefix+"HadEtInHF"+Suffix);
  event.put( std::auto_ptr<double>( new double(!met ? 0 : met->hadEtInHF())), Prefix+"HadEtInHO"+Suffix);
}


#endif
