#ifndef TUPLE_JET
#define TUPLE_JET

#include "boost/foreach.hpp"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"

#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/PatCandidates/interface/Jet.h"

#include "PhysicsTools/SelectorUtils/interface/PFJetIDSelectionFunctor.h"

#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"
#include "JetMETCorrections/Objects/interface/JetCorrectionsRecord.h"

template< typename T >
class Tuple_Jet : public edm::EDProducer {
 public: 
  explicit Tuple_Jet(const edm::ParameterSet&);
 private: 
  void produce(edm::Event&, const edm::EventSetup& );
  void initSpecial(); void produceSpecial(edm::Event&, const edm::Handle<edm::View<T> >&);
  void initPF();      void producePF(edm::Event&, const edm::Handle<edm::View<T> >&);
  void initBJetTag(); void produceBJetTag(edm::Event&, const edm::Handle<edm::View<T> >&);
  void initGenJetMatch(); void produceGenJetMatch(edm::Event&, const edm::Handle<edm::View<T> >&);

  template <class I> int indexOfMatch( const reco::GenJet*, const I, const I);
  std::auto_ptr<std::vector<float> > correctionFactors(const edm::Handle<edm::View<T> >&);
  
  const edm::InputTag jetsTag, genTag;
  const std::string prefix,jecRecord;
  const std::vector<std::string> btagNames;
  const bool pfSpecific, gen;

};

template<class T> Tuple_Jet<T>::
Tuple_Jet(const edm::ParameterSet& cfg) :
  jetsTag(cfg.getParameter<edm::InputTag>("jetsTag")),
  genTag(cfg.getParameter<edm::InputTag>("genTag")),
  prefix(cfg.getParameter<std::string>("prefix")),
  jecRecord(cfg.getParameter<std::string>("jecRecord")),
  btagNames(cfg.getParameter<std::vector<std::string> >("bTags")),
  pfSpecific(cfg.getParameter<bool>("pfInfo")),
  gen(cfg.getParameter<bool>("genInfo"))
{
  produces <std::vector<reco::Candidate::LorentzVector> > ( prefix + "P4"   );
  produces <std::vector<float> >                          ( prefix + "JecUnc"      );
  produces <std::vector<float> >                          ( prefix + "Area"        );
  produces <std::vector<float> >                          ( prefix + "Eta2Moment"  );
  produces <std::vector<float> >                          ( prefix + "Phi2Moment"  );
  initSpecial();
}

JetCorrectionUncertainty* jetCorrUnc(const edm::EventSetup& setup, const std::string& jecRecord) {
  edm::ESHandle<JetCorrectorParametersCollection> JetCorParColl;
  setup.get<JetCorrectionsRecord>().get(jecRecord,JetCorParColl);
  return new JetCorrectionUncertainty((*JetCorParColl)["Uncertainty"]);
}

float uncFunc(JetCorrectionUncertainty* jCU, const reco::Candidate::LorentzVector& jet) {
  jCU->setJetEta(jet.eta());
  jCU->setJetPt(jet.pt());// the uncertainty is a function of the corrected pt
  try {return jCU->getUncertainty(true);} // sigh,,, they are still throwing exceptions
  catch (...) {
    std::cout << "JetCorrectionUncertainty::getUncertainty threw exception on jet with pt( " << jet.pt() << " ) and eta ( " << jet.eta() << " )." << std::endl;
    return 0.0;
  }
}

template< typename T > 
void Tuple_Jet<T>::
produce(edm::Event& evt, const edm::EventSetup& setup) {
  typedef reco::Candidate::LorentzVector LorentzV;
  edm::Handle<edm::View<T> > jets;     evt.getByLabel(jetsTag, jets);

  std::auto_ptr<std::vector<LorentzV> >   p4   ( new std::vector<LorentzV>()  )  ;
  std::auto_ptr<std::vector<float> >  jetArea  ( new std::vector<float>()  )  ;
  std::auto_ptr<std::vector<float> >  jecUnc   ( new std::vector<float>()  )  ;
  std::auto_ptr<std::vector<float> >  eta2mom  ( new std::vector<float>()  )  ;
  std::auto_ptr<std::vector<float> >  phi2mom  ( new std::vector<float>()  )  ;

  if(jets.isValid()) {
    JetCorrectionUncertainty* jCU = jetCorrUnc(setup, jecRecord);
    for(typename edm::View<T>::const_iterator jet = jets->begin(); jet!=jets->end(); jet++) {
      p4->push_back(jet->p4());
      eta2mom->push_back(jet->etaetaMoment());
      phi2mom->push_back(jet->phiphiMoment());
      jetArea->push_back(jet->jetArea());
      
      jecUnc->push_back(uncFunc(jCU, jet->p4()));
    }
    delete jCU;
  }

  evt.put(      p4, prefix + "P4"  );
  evt.put(  jecUnc, prefix + "JecUnc"      );
  evt.put( jetArea, prefix + "Area"        );
  evt.put( eta2mom, prefix + "Eta2Moment"  );
  evt.put( phi2mom, prefix + "Phi2Moment"  );
  
  produceSpecial(evt, jets);
}

template<> void Tuple_Jet<pat::Jet>::initSpecial() {
  initPF();
  initBJetTag();
  initGenJetMatch();
}

template<> void Tuple_Jet<pat::Jet>::produceSpecial(edm::Event& e,const edm::Handle<edm::View<pat::Jet> >& h) {
  producePF(e,h);
  produceBJetTag(e,h);
  produceGenJetMatch(e,h);
}


template<class T> void Tuple_Jet<T>::
initPF() { if(!pfSpecific) return;
  produces <std::vector<float> > (prefix + "FchargedHad" );
  produces <std::vector<float> > (prefix + "FneutralHad" );
  produces <std::vector<float> > (prefix + "FchargedEm" );
  produces <std::vector<float> > (prefix + "FneutralEm" );
  produces <std::vector<float> > (prefix + "FchargedMu" );


  produces <std::vector<unsigned> > (prefix + "Ncharged" );
  produces <std::vector<unsigned> > (prefix + "Nneutral" );
  produces <std::vector<unsigned> > (prefix + "Nmuon" );
  
  produces <std::vector<bool> > ( prefix + "PFJetIDloose"   );
  produces <std::vector<bool> > ( prefix + "PFJetIDtight"   );

}

template<class T> void Tuple_Jet<T>::
producePF(edm::Event& evt, const edm::Handle<edm::View<T> >& jets) { if(!pfSpecific) return;
  std::auto_ptr<std::vector<float> > FchargedHad( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > FneutralHad( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > FchargedEm( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > FneutralEm( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > FchargedMu( new std::vector<float>() );
  
  std::auto_ptr<std::vector<unsigned> > Ncharged( new std::vector<unsigned>() );
  std::auto_ptr<std::vector<unsigned> > Nneutral( new std::vector<unsigned>() );
  std::auto_ptr<std::vector<unsigned> > Nmuon( new std::vector<unsigned>() );

  std::auto_ptr<std::vector<bool> >  pfjetidloose  ( new std::vector<bool>()  ) ;
  std::auto_ptr<std::vector<bool> >  pfjetidtight  ( new std::vector<bool>()  ) ;

  PFJetIDSelectionFunctor
    pfLooseJetID(PFJetIDSelectionFunctor::FIRSTDATA, PFJetIDSelectionFunctor::LOOSE),
    pfTightJetID(PFJetIDSelectionFunctor::FIRSTDATA, PFJetIDSelectionFunctor::TIGHT);

  if(jets.isValid()) {
    for(typename edm::View<T>::const_iterator jet = jets->begin(); jet!=jets->end(); jet++) {
      pat::strbitset
	passLooseCuts(   pfLooseJetID  .getBitTemplate() ),
	passTightCuts(   pfTightJetID  .getBitTemplate() );

      FchargedHad->push_back( jet->chargedHadronEnergyFraction() );
      FneutralHad->push_back( jet->neutralHadronEnergyFraction() );
      FchargedEm->push_back( jet->chargedEmEnergyFraction() );
      FneutralEm->push_back( jet->neutralEmEnergyFraction() );
      FchargedMu->push_back( jet->chargedMuEnergyFraction() );
      Ncharged->push_back( (unsigned) jet->chargedMultiplicity() );
      Nneutral->push_back( (unsigned) jet->neutralMultiplicity() );
      Nmuon->push_back( (unsigned) jet->muonMultiplicity() );
      
      pfjetidloose->push_back(pfLooseJetID( *jet, passLooseCuts  ));
      pfjetidtight->push_back(pfTightJetID( *jet, passTightCuts  ));
    }
  }

  evt.put(FchargedHad, prefix + "FchargedHad" );
  evt.put(FneutralHad, prefix + "FneutralHad" );
  evt.put(FchargedEm,  prefix + "FchargedEm"  );
  evt.put(FneutralEm,  prefix + "FneutralEm"  );
  evt.put(FchargedMu,  prefix + "FchargedMu"  );
  evt.put(Ncharged,    prefix + "Ncharged"    );
  evt.put(Nneutral,    prefix + "Nneutral"    );
  evt.put(Nmuon,       prefix + "Nmuon"       );
 
  evt.put( pfjetidloose,  prefix + "PFJetIDloose"   );
  evt.put( pfjetidtight,  prefix + "PFJetIDtight"   );

}


template<class T> void Tuple_Jet<T>::
initBJetTag(){
  produces <std::vector<int> > (prefix + "GenJetFlavour" );
  BOOST_FOREACH(const std::string& btag, btagNames)
    produces <std::vector<float> > (prefix + btag);
}

template<class T> void Tuple_Jet<T>::
produceBJetTag(edm::Event& evt, const edm::Handle<edm::View<T> >& jets) {
  std::auto_ptr<std::vector<int> > flavour(new std::vector<int>() );
  std::map<std::string, std::vector<float>* > btags;
  BOOST_FOREACH(const std::string& btag, btagNames)
    btags[btag] = new std::vector<float>();

  if(jets.isValid()) {
    for(typename edm::View<T>::const_iterator jet = jets->begin(); jet!=jets->end(); jet++) {
      flavour->push_back(jet->partonFlavour());
      BOOST_FOREACH(const std::string& btag, btagNames)
	btags[btag]->push_back(jet->bDiscriminator(btag+"BJetTags"));
    }
  }

  evt.put( flavour, prefix + "GenJetFlavour" );
  BOOST_FOREACH(const std::string& btag, btagNames)
    evt.put( std::auto_ptr<std::vector<float> >(btags[btag]), prefix + btag );
}

template<class T> void Tuple_Jet<T>::
initGenJetMatch() { if(gen) produces <std::vector<int> > (prefix + "GenJetMatchIndex" );}

template<class T> void Tuple_Jet<T>::
produceGenJetMatch(edm::Event& evt, const edm::Handle<edm::View<T> >& jets){
  if(!gen) return;
  edm::Handle<edm::View<reco::GenJet> > genjets; 
  evt.getByLabel(genTag, genjets);
  std::auto_ptr<std::vector<int> > genjetMatchIndex( new std::vector<int>() );
  if(jets.isValid() && genjets.isValid())
    for (typename edm::View<T>::const_iterator jet = jets->begin(); jet!=jets->end(); ++jet ) 
      genjetMatchIndex->push_back( indexOfMatch( jet->genJet(), genjets->begin(), genjets->end()) );
  evt.put(genjetMatchIndex, prefix + "GenJetMatchIndex" );
}

template<class T> template<class I> int Tuple_Jet<T>::
indexOfMatch( const reco::GenJet* genjet, const I begin, const I end) {
  for(I it=begin; it!=end; ++it) if ( genjet && genjet->p4() == it->p4() ) return it-begin; //p4 comparisons
  return -1;
}


#endif


