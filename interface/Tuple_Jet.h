#ifndef TUPLE_JET
#define TUPLE_JET

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/JetReco/interface/JPTJet.h"
#include "DataFormats/JetReco/interface/CaloJet.h"
#include "DataFormats/PatCandidates/interface/Jet.h"

#include "PhysicsTools/SelectorUtils/interface/JetIDSelectionFunctor.h"
#include "PhysicsTools/SelectorUtils/interface/PFJetIDSelectionFunctor.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackBase.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"

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
  void initCalo();    void produceCalo(edm::Event&, const edm::Handle<edm::View<T> >&);
  void initJetID();   void produceJetID(edm::Event&, const edm::Handle<edm::View<T> >&);
  void initBJetTag(); void produceBJetTag(edm::Event&, const edm::Handle<edm::View<T> >&);
  void initMPT();     void produceMPT(edm::Event&, const edm::Handle<edm::View<T> >&);
  void initGenJetMatch(); void produceGenJetMatch(edm::Event&, const edm::Handle<edm::View<T> >&);

  template <class I> int indexOfMatch( const reco::GenJet*, const I, const I);
  std::auto_ptr<std::vector<double> > correctionFactors(const edm::Handle<edm::View<T> >&);
  const reco::CaloJet* getJet( const edm::RefToBase<T>& );
  
  const edm::ParameterSet& config;
  const edm::InputTag jetsInputTag, genJetsInputTag, allJetsInputTag;
  const std::string Prefix,Suffix,JecRecord;
  const bool caloSpecific, jptSpecific, pfSpecific, mpt, jetid, gen;


  enum VERTEX {VINIT=0, V_PRIMARY=0, V_PILEUP=1, VSIZE=2};
  static const std::string vertex_names[];

  struct dsz {
    const reco::Track& track;
    dsz(const reco::Track& t) : track(t) {}
    double operator()(const reco::Vertex& v) {return fabs(track.dsz(v.position()));}
  };
};

template <class T> 
const std::string Tuple_Jet<T>::vertex_names[2] = {"Primary", "PileUp"};

template<class T> Tuple_Jet<T>::
Tuple_Jet(const edm::ParameterSet& cfg) :
  config(cfg),
  jetsInputTag(cfg.getParameter<edm::InputTag>("InputTag")),
  genJetsInputTag(cfg.getParameter<edm::InputTag>("GenInputTag")),
  allJetsInputTag(cfg.getParameter<edm::InputTag>("AllJets")),
  Prefix(cfg.getParameter<std::string>("Prefix")),
  Suffix(cfg.getParameter<std::string>("Suffix")),
  JecRecord(cfg.getParameter<std::string>("JecRecord")),
  caloSpecific(cfg.getParameter<bool>("Calo")),
  jptSpecific(cfg.getParameter<bool>("JPT")),
  pfSpecific(cfg.getParameter<bool>("PF")),
  mpt(cfg.getParameter<bool>("MPT")),
  jetid(cfg.getParameter<bool>("JetID")),
  gen(cfg.getParameter<bool>("GenInfo"))
{
  produces <std::vector<reco::Candidate::LorentzVector> > ( Prefix + "CorrectedP4"  + Suffix );
  produces <std::vector<double> >                         ( Prefix + "CorrFactor"  + Suffix );
  produces <std::vector<float> >                          ( Prefix + "JecUnc"     + Suffix );
  produces <std::vector<float> >                          ( Prefix + "Area"       + Suffix );
  produces <std::vector<float> >                          ( Prefix + "Eta2Moment" + Suffix );
  produces <std::vector<float> >                          ( Prefix + "Phi2Moment" + Suffix );
  produces <reco::Candidate::LorentzVector>               ( Prefix + "DroppedSumP4" + Suffix );
  produces <float>                                        ( Prefix + "DroppedSumPT" + Suffix );
  produces <float>                                        ( Prefix + "DroppedSumET" + Suffix );
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
  edm::Handle<edm::View<T> > jets;     evt.getByLabel(jetsInputTag, jets);
  edm::Handle<edm::View<T> > allJets;  evt.getByLabel(allJetsInputTag, allJets);

  std::auto_ptr<std::vector<LorentzV> >   p4   ( new std::vector<LorentzV>()  )  ;
  std::auto_ptr<std::vector<float> >  jetArea  ( new std::vector<float>()  )  ;
  std::auto_ptr<std::vector<float> >  jecUnc   ( new std::vector<float>()  )  ;
  std::auto_ptr<std::vector<float> >  eta2mom  ( new std::vector<float>()  )  ;
  std::auto_ptr<std::vector<float> >  phi2mom  ( new std::vector<float>()  )  ;
  std::auto_ptr<LorentzV>  droppedSumP4  ( new LorentzV()  )  ;
  std::auto_ptr<float>  droppedSumPT  ( new float(0)  )  ;
  std::auto_ptr<float>  droppedSumET  ( new float(0)  )  ;

  JetCorrectionUncertainty* jCU = jetCorrUnc(setup, JecRecord);

  for(unsigned i=0; jets.isValid() && i<(*jets).size(); i++) {
    p4->push_back((*jets)[i].p4());
    eta2mom->push_back((*jets)[i].etaetaMoment());
    phi2mom->push_back((*jets)[i].phiphiMoment());
    jetArea->push_back((*jets)[i].jetArea());
    *droppedSumP4 -= (*jets)[i].p4();
    *droppedSumPT -= (*jets)[i].pt();
    *droppedSumET -= (*jets)[i].p4().Et();

    jecUnc->push_back(uncFunc(jCU, (*jets)[i].p4()));
  }
  for(unsigned i=0; i<(*allJets).size(); i++) {
    *droppedSumP4 += (*allJets)[i].p4();
    *droppedSumPT += (*allJets)[i].pt();
    *droppedSumET += (*allJets)[i].p4().Et();
  }

  delete jCU;

  evt.put(                      p4, Prefix + "CorrectedP4" + Suffix );
  evt.put( correctionFactors(jets), Prefix + "CorrFactor"  + Suffix) ;
  evt.put(                  jecUnc, Prefix + "JecUnc"     + Suffix );
  evt.put(                 jetArea, Prefix + "Area"       + Suffix );
  evt.put(                 eta2mom, Prefix + "Eta2Moment" + Suffix );
  evt.put(                 phi2mom, Prefix + "Phi2Moment" + Suffix );
  evt.put(            droppedSumP4, Prefix + "DroppedSumP4" + Suffix );
  evt.put(            droppedSumPT, Prefix + "DroppedSumPT" + Suffix );
  evt.put(            droppedSumET, Prefix + "DroppedSumET" + Suffix );
  
  produceSpecial(evt, jets);
}

template<class T> 
std::auto_ptr<std::vector<double> > Tuple_Jet<T>::
correctionFactors(const edm::Handle<edm::View<T> >& jets) {
  if(jets.isValid()) return std::auto_ptr<std::vector<double> >(new std::vector<double>(jets->size(),1.0) ) ;
  else return std::auto_ptr<std::vector<double> >(new std::vector<double>());
}

template<> 
std::auto_ptr<std::vector<double> > Tuple_Jet<pat::Jet>::
correctionFactors(const edm::Handle<edm::View<pat::Jet> >& jets) {
  std::auto_ptr<std::vector<double> > correction ( new std::vector<double>() );
  for(unsigned i=0; jets.isValid() && i<(*jets).size(); i++)
    correction->push_back( (*jets)[i].jecSetsAvailable() ? 
			   (*jets)[i].energy() / (*jets)[i].correctedJet("Uncorrected").energy() :
			   1.0 );
  return correction;
}




template<> void Tuple_Jet<reco::PFJet>::initSpecial() { initPF(); }
template<> void Tuple_Jet<reco::CaloJet>::initSpecial() { initCalo(); }
template<> void Tuple_Jet<reco::JPTJet>::initSpecial() { initCalo(); }
template<> void Tuple_Jet<pat::Jet>::initSpecial() {
  initPF();
  initCalo();
  initMPT();
  initJetID();
  initBJetTag();
  initGenJetMatch();
}

template<> void Tuple_Jet<reco::PFJet>::produceSpecial(edm::Event& e,const edm::Handle<edm::View<reco::PFJet> >& h) {producePF(e,h);}
template<> void Tuple_Jet<reco::CaloJet>::produceSpecial(edm::Event& e,const edm::Handle<edm::View<reco::CaloJet> >& h) {produceCalo(e,h);}
template<> void Tuple_Jet<reco::JPTJet>::produceSpecial(edm::Event& e,const edm::Handle<edm::View<reco::JPTJet> >& h) {produceCalo(e,h);}
template<> void Tuple_Jet<pat::Jet>::produceSpecial(edm::Event& e,const edm::Handle<edm::View<pat::Jet> >& h) {
  producePF(e,h);
  produceCalo(e,h);
  produceMPT(e,h);
  produceJetID(e,h);
  produceBJetTag(e,h);
  produceGenJetMatch(e,h);
}




template<class T> void Tuple_Jet<T>::
initPF() { if(!pfSpecific) return;
  produces <std::vector<float> > (Prefix + "FchargedHad" + Suffix);
  produces <std::vector<float> > (Prefix + "FneutralHad" + Suffix);
  produces <std::vector<float> > (Prefix + "FchargedEm" + Suffix);
  produces <std::vector<float> > (Prefix + "FneutralEm" + Suffix);
  produces <std::vector<float> > (Prefix + "FchargedMu" + Suffix);

  produces <std::vector<float> > (Prefix + "EchargedHad" + Suffix);
  produces <std::vector<float> > (Prefix + "EneutralHad" + Suffix);
  produces <std::vector<float> > (Prefix + "EchargedEM" + Suffix);
  produces <std::vector<float> > (Prefix + "EneutralEM" + Suffix); 
  produces <std::vector<float> > (Prefix + "Ephoton" + Suffix);
  produces <std::vector<float> > (Prefix + "Eelectron" + Suffix);
  produces <std::vector<float> > (Prefix + "Emuon" + Suffix);
  produces <std::vector<float> > (Prefix + "EHFHad" + Suffix);
  produces <std::vector<float> > (Prefix + "EHFEM" + Suffix);

  produces <std::vector<unsigned> > (Prefix + "Ncharged" + Suffix);
  produces <std::vector<unsigned> > (Prefix + "Nneutral" + Suffix);
  produces <std::vector<unsigned> > (Prefix + "Nmuon" + Suffix);
  
  produces <std::vector<unsigned> > (Prefix + "NchargedHad" + Suffix);
  produces <std::vector<unsigned> > (Prefix + "NneutralHad" + Suffix);
  produces <std::vector<unsigned> > (Prefix + "Nphoton" + Suffix);
  produces <std::vector<unsigned> > (Prefix + "Nelectron" + Suffix);
  produces <std::vector<unsigned> > (Prefix + "NHFHad" + Suffix);
  produces <std::vector<unsigned> > (Prefix + "NHFEM" + Suffix);
 
  produces <std::vector<int> > ( Prefix + "PFJetIDloose"  + Suffix );
  produces <std::vector<int> > ( Prefix + "PFJetIDtight"  + Suffix );

}

template<class T> void Tuple_Jet<T>::
producePF(edm::Event& evt, const edm::Handle<edm::View<T> >& jets) { if(!pfSpecific) return;
  std::auto_ptr<std::vector<float> > FchargedHad( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > FneutralHad( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > FchargedEm( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > FneutralEm( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > FchargedMu( new std::vector<float>() );
  
  std::auto_ptr<std::vector<float> > EchargedHad( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > EneutralHad( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > EchargedEM( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > EneutralEM( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > Ephoton( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > Eelectron( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > Emuon( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > EHFHad( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > EHFEM( new std::vector<float>() );  
  
  std::auto_ptr<std::vector<unsigned> > Ncharged( new std::vector<unsigned>() );
  std::auto_ptr<std::vector<unsigned> > Nneutral( new std::vector<unsigned>() );
  std::auto_ptr<std::vector<unsigned> > Nmuon( new std::vector<unsigned>() );

  std::auto_ptr<std::vector<unsigned> > NchargedHad( new std::vector<unsigned>() );
  std::auto_ptr<std::vector<unsigned> > NneutralHad( new std::vector<unsigned>() );
  std::auto_ptr<std::vector<unsigned> > Nphoton( new std::vector<unsigned>() );  
  std::auto_ptr<std::vector<unsigned> > Nelectron( new std::vector<unsigned>() );
  std::auto_ptr<std::vector<unsigned> > NHFHad( new std::vector<unsigned>() );
  std::auto_ptr<std::vector<unsigned> > NHFEM( new std::vector<unsigned>() );
  
  std::auto_ptr<std::vector<int> >  pfjetidloose  ( new std::vector<int>()  ) ;
  std::auto_ptr<std::vector<int> >  pfjetidtight  ( new std::vector<int>()  ) ;

  PFJetIDSelectionFunctor
    pfLooseJetID(PFJetIDSelectionFunctor::FIRSTDATA, PFJetIDSelectionFunctor::LOOSE),
    pfTightJetID(PFJetIDSelectionFunctor::FIRSTDATA, PFJetIDSelectionFunctor::TIGHT);

  for( unsigned i=0; jets.isValid() && i<(*jets).size(); i++) {
    pat::strbitset
      passLooseCuts(   pfLooseJetID  .getBitTemplate() ),
      passTightCuts(   pfTightJetID  .getBitTemplate() );

    FchargedHad->push_back( (*jets)[i].chargedHadronEnergyFraction() );
    FneutralHad->push_back( (*jets)[i].neutralHadronEnergyFraction() );
    FchargedEm->push_back( (*jets)[i].chargedEmEnergyFraction() );
    FneutralEm->push_back( (*jets)[i].neutralEmEnergyFraction() );
    FchargedMu->push_back( (*jets)[i].chargedMuEnergyFraction() );
    Ncharged->push_back( (unsigned) (*jets)[i].chargedMultiplicity() );
    Nneutral->push_back( (unsigned) (*jets)[i].neutralMultiplicity() );
    Nmuon->push_back( (unsigned) (*jets)[i].muonMultiplicity() );
 
    EchargedHad->push_back( (*jets)[i].chargedHadronEnergy() );
    EneutralHad->push_back( (*jets)[i].neutralHadronEnergy() );
    EchargedEM ->push_back( (*jets)[i].chargedEmEnergy() );
    EneutralEM ->push_back( (*jets)[i].neutralEmEnergy() );
    Ephoton    ->push_back( (*jets)[i].photonEnergy() ); 
    Eelectron  ->push_back( (*jets)[i].electronEnergy() );
    Emuon      ->push_back( (*jets)[i].muonEnergy() );  
    EHFHad     ->push_back( (*jets)[i].HFHadronEnergy() );
    EHFEM      ->push_back( (*jets)[i].HFEMEnergy() );
    
    NchargedHad->push_back( (unsigned) (*jets)[i].chargedHadronMultiplicity() );	
    NneutralHad->push_back( (unsigned) (*jets)[i].neutralHadronMultiplicity() );
    Nphoton    ->push_back( (unsigned) (*jets)[i].photonMultiplicity() );
    Nelectron  ->push_back( (unsigned) (*jets)[i].electronMultiplicity() );
    NHFHad     ->push_back( (unsigned) (*jets)[i].HFHadronMultiplicity() );
    NHFEM      ->push_back( (unsigned) (*jets)[i].HFEMMultiplicity() );
   
    pfjetidloose->push_back(pfLooseJetID( (*jets)[i], passLooseCuts  ));
    pfjetidtight->push_back(pfTightJetID( (*jets)[i], passTightCuts  )); 
    
 }

  evt.put(FchargedHad, Prefix + "FchargedHad" + Suffix);
  evt.put(FneutralHad, Prefix + "FneutralHad" + Suffix);
  evt.put(FchargedEm,  Prefix + "FchargedEm"  + Suffix);
  evt.put(FneutralEm,  Prefix + "FneutralEm"  + Suffix);
  evt.put(FchargedMu,  Prefix + "FchargedMu"  + Suffix);
  evt.put(Ncharged,    Prefix + "Ncharged"    + Suffix);
  evt.put(Nneutral,    Prefix + "Nneutral"    + Suffix);
  evt.put(Nmuon,       Prefix + "Nmuon"       + Suffix);
 
    evt.put(EchargedHad, Prefix + "EchargedHad" + Suffix); 
    evt.put(EneutralHad, Prefix + "EneutralHad" + Suffix); 
    evt.put(EchargedEM , Prefix + "EchargedEM" + Suffix); 
    evt.put(EneutralEM , Prefix + "EneutralEM" + Suffix);  
    evt.put(Ephoton    , Prefix + "Ephoton" + Suffix); 
    evt.put(Eelectron  , Prefix + "Eelectron" + Suffix); 
    evt.put(Emuon      , Prefix + "Emuon" + Suffix); 
    evt.put(EHFHad     , Prefix + "EHFHad" + Suffix); 
    evt.put(EHFEM      , Prefix + "EHFEM" + Suffix);  

    evt.put(NchargedHad, Prefix + "NchargedHad" + Suffix); 
    evt.put(NneutralHad, Prefix + "NneutralHad" + Suffix); 
    evt.put(Nphoton    , Prefix + "Nphoton" + Suffix); 
    evt.put(Nelectron  , Prefix + "Nelectron" + Suffix); 
    evt.put(NHFHad     , Prefix + "NHFHad" + Suffix); 
    evt.put(NHFEM      , Prefix + "NHFEM" + Suffix); 

  evt.put( pfjetidloose,  Prefix + "PFJetIDloose"  + Suffix );
  evt.put( pfjetidtight,  Prefix + "PFJetIDtight"  + Suffix );

}

template<class T> void Tuple_Jet<T>::
initCalo() { 
  if( !( caloSpecific || jptSpecific ) ) return;
  produces <std::vector<float> > ( Prefix + "EmEnergyFraction"  + Suffix );
  produces <std::vector<float> > ( Prefix + "EnergyFractionHadronic"  + Suffix );
  produces <std::vector<float> > ( Prefix + "TowersArea"  + Suffix );
  produces <std::vector<float> > ( Prefix + "MaxEInEmTowers"  + Suffix );
  produces <std::vector<float> > ( Prefix + "MaxEInHadTowers"  + Suffix );
  produces <std::vector<float> > ( Prefix + "HadEnergyInHB"  + Suffix );
  produces <std::vector<float> > ( Prefix + "HadEnergyInHE"  + Suffix );
  produces <std::vector<float> > ( Prefix + "HadEnergyInHO"  + Suffix );
  produces <std::vector<float> > ( Prefix + "HadEnergyInHF"  + Suffix );
  produces <std::vector<float> > ( Prefix + "EmEnergyInEB"  + Suffix );
  produces <std::vector<float> > ( Prefix + "EmEnergyInEE"  + Suffix );
  produces <std::vector<float> > ( Prefix + "EmEnergyInHF"  + Suffix );
  produces <std::vector<int> > ( Prefix + "N60Towers"  + Suffix ); 
  produces <std::vector<int> > ( Prefix + "N90Towers"  + Suffix );

}

template<class T> void Tuple_Jet<T>::
produceCalo(edm::Event& evt, const edm::Handle<edm::View<T> >& jets) { 

  if( !( caloSpecific || jptSpecific ) ) return;
  std::auto_ptr<std::vector<float> >  emEnergyFraction  ( new std::vector<float>()  ) ;
  std::auto_ptr<std::vector<float> >  energyFractionHadronic ( new std::vector<float>()  ) ;
  std::auto_ptr<std::vector<float> >  towersArea   ( new std::vector<float>()  ) ;
  std::auto_ptr<std::vector<float> >  maxEInEmTowers   ( new std::vector<float>()  ) ;
  std::auto_ptr<std::vector<float> >  maxEInHadTowers   ( new std::vector<float>()  ) ;
  std::auto_ptr<std::vector<float> >  hadEnergyInHB   ( new std::vector<float>()  ) ;
  std::auto_ptr<std::vector<float> >  hadEnergyInHE   ( new std::vector<float>()  ) ;
  std::auto_ptr<std::vector<float> >  hadEnergyInHO   ( new std::vector<float>()  ) ;
  std::auto_ptr<std::vector<float> >  hadEnergyInHF   ( new std::vector<float>()  ) ;
  std::auto_ptr<std::vector<float> >  emEnergyInEB   ( new std::vector<float>()  ) ;
  std::auto_ptr<std::vector<float> >  emEnergyInEE   ( new std::vector<float>()  ) ;
  std::auto_ptr<std::vector<float> >  emEnergyInHF   ( new std::vector<float>()  ) ;
  std::auto_ptr<std::vector<int> >             n60   ( new std::vector<int>()  ) ; 
  std::auto_ptr<std::vector<int> >             n90   ( new std::vector<int>()  ) ; 

  for( unsigned i=0; jets.isValid() && i<(*jets).size(); i++) {

    edm::RefToBase<T> jet_ref( jets, i );
    const reco::CaloJet* jet = getJet( jet_ref );
    if ( !jet ) continue;

    emEnergyFraction->push_back(jet->emEnergyFraction());
    energyFractionHadronic->push_back(jet->energyFractionHadronic());
    towersArea->push_back(jet->towersArea());
    maxEInEmTowers->push_back(jet->maxEInEmTowers());
    maxEInHadTowers->push_back(jet->maxEInHadTowers());
    hadEnergyInHB->push_back(jet->hadEnergyInHB());
    hadEnergyInHE->push_back(jet->hadEnergyInHE());
    hadEnergyInHO->push_back(jet->hadEnergyInHO());
    hadEnergyInHF->push_back(jet->hadEnergyInHF());
    emEnergyInEB->push_back(jet->emEnergyInEB());
    emEnergyInEE->push_back(jet->emEnergyInEE());
    emEnergyInHF->push_back(jet->emEnergyInHF());
    n60->push_back(jet->n60());
    n90->push_back(jet->n90());
  }

  evt.put( emEnergyFraction,        Prefix + "EmEnergyFraction"  + Suffix );
  evt.put( energyFractionHadronic,  Prefix + "EnergyFractionHadronic"  + Suffix );
  evt.put( towersArea,              Prefix + "TowersArea"  + Suffix );
  evt.put( maxEInEmTowers,          Prefix + "MaxEInEmTowers"  + Suffix );
  evt.put( maxEInHadTowers,         Prefix + "MaxEInHadTowers"  + Suffix );
  evt.put( hadEnergyInHB,           Prefix + "HadEnergyInHB"  + Suffix );
  evt.put( hadEnergyInHE,           Prefix + "HadEnergyInHE"  + Suffix );
  evt.put( hadEnergyInHO,           Prefix + "HadEnergyInHO"  + Suffix );
  evt.put( hadEnergyInHF,           Prefix + "HadEnergyInHF"  + Suffix );
  evt.put( emEnergyInEB,            Prefix + "EmEnergyInEB"  + Suffix );
  evt.put( emEnergyInEE,            Prefix + "EmEnergyInEE"  + Suffix );
  evt.put( emEnergyInHF,            Prefix + "EmEnergyInHF"  + Suffix );
  evt.put( n60,                     Prefix + "N60Towers"  + Suffix ); 
  evt.put( n90,                     Prefix + "N90Towers"  + Suffix );
 
}



template<class T> void Tuple_Jet<T>::
initJetID() { 
  if( !( ( caloSpecific || jptSpecific ) && jetid ) ) return;
  produces <std::vector<double> > ( Prefix + "JetIDFHPD"  + Suffix );
  produces <std::vector<double> > ( Prefix + "JetIDFRBX"  + Suffix );
  produces <std::vector<double> > ( Prefix + "JetIDFSubDet1"  + Suffix );
  produces <std::vector<double> > ( Prefix + "JetIDFSubDet2"  + Suffix );
  produces <std::vector<double> > ( Prefix + "JetIDFSubDet3"  + Suffix );
  produces <std::vector<double> > ( Prefix + "JetIDFSubDet4"  + Suffix );
  produces <std::vector<double> > ( Prefix + "JetIDResEMF"  + Suffix );
  produces <std::vector<int> > ( Prefix + "JetIDN90Hits"  + Suffix );
  produces <std::vector<int> > ( Prefix + "JetIDminimal"  + Suffix );
  produces <std::vector<int> > ( Prefix + "JetIDloose"  + Suffix );
  produces <std::vector<int> > ( Prefix + "JetIDtight"  + Suffix );
  produces <std::vector<int> > ( Prefix + "JetIDnECALTowers"  + Suffix );
  produces <std::vector<int> > ( Prefix + "JetIDnHCALTowers"  + Suffix );
}

template<class T> void Tuple_Jet<T>::
produceJetID(edm::Event& evt, const edm::Handle<edm::View<T> >& jets) { 
  if( !( ( caloSpecific || jptSpecific ) && jetid ) ) return;
  std::auto_ptr<std::vector<double> >  fHPD  ( new std::vector<double>() ) ;
  std::auto_ptr<std::vector<double> >  fRBX  ( new std::vector<double>() ) ;
  std::auto_ptr<std::vector<double> >  fSubDet1  ( new std::vector<double>() ) ;
  std::auto_ptr<std::vector<double> >  fSubDet2  ( new std::vector<double>() ) ;
  std::auto_ptr<std::vector<double> >  fSubDet3  ( new std::vector<double>() ) ;
  std::auto_ptr<std::vector<double> >  fSubDet4  ( new std::vector<double>() ) ;
  std::auto_ptr<std::vector<double> >  resEMF  ( new std::vector<double>()  ) ;
  std::auto_ptr<std::vector<int> >  n90Hits  ( new std::vector<int>()  ) ;
  std::auto_ptr<std::vector<int> >  jetidminimal  ( new std::vector<int>()  ) ;
  std::auto_ptr<std::vector<int> >  jetidloose  ( new std::vector<int>()  ) ;
  std::auto_ptr<std::vector<int> >  jetidtight  ( new std::vector<int>()  ) ;
  std::auto_ptr<std::vector<int> >  NECALTowers  ( new std::vector<int>()  ) ;
  std::auto_ptr<std::vector<int> >  NHCALTowers  ( new std::vector<int>()  ) ;

  JetIDSelectionFunctor
    minimalJetID(JetIDSelectionFunctor::PURE09, JetIDSelectionFunctor::MINIMAL),
    looseJetID(JetIDSelectionFunctor::PURE09, JetIDSelectionFunctor::LOOSE),
    tightJetID(JetIDSelectionFunctor::PURE09, JetIDSelectionFunctor::TIGHT);  
  for( unsigned i=0; jets.isValid() && i<(*jets).size(); i++ ) {

    edm::RefToBase<T> jet_ref( jets, i );
    const reco::CaloJet* jet = getJet( jet_ref );
    if ( !jet ) continue;
    
    pat::strbitset 
      passMinimalCuts( minimalJetID.getBitTemplate() ),
      passLooseCuts(   looseJetID  .getBitTemplate() ),
      passTightCuts(   tightJetID  .getBitTemplate() );

    fHPD->push_back((*jets)[i].jetID().fHPD);
    fRBX->push_back((*jets)[i].jetID().fRBX);
    fSubDet1->push_back((*jets)[i].jetID().fSubDetector1);
    fSubDet2->push_back((*jets)[i].jetID().fSubDetector2);
    fSubDet3->push_back((*jets)[i].jetID().fSubDetector3);
    fSubDet4->push_back((*jets)[i].jetID().fSubDetector4);
    resEMF->push_back((*jets)[i].jetID().restrictedEMF);
    n90Hits->push_back((int)((*jets)[i].jetID().n90Hits));
    jetidminimal->push_back(minimalJetID(*jet, (*jets)[i].jetID(), passMinimalCuts));
    jetidloose->push_back(looseJetID(*jet, (*jets)[i].jetID(), passLooseCuts  ));
    jetidtight->push_back(tightJetID(*jet, (*jets)[i].jetID(), passTightCuts  ));
    NECALTowers->push_back((*jets)[i].jetID().nECALTowers);
    NHCALTowers->push_back((*jets)[i].jetID().nHCALTowers);  
  }

  evt.put( fHPD,  Prefix + "JetIDFHPD"  + Suffix );
  evt.put( fRBX,  Prefix + "JetIDFRBX"  + Suffix );
  evt.put( fSubDet1,  Prefix + "JetIDFSubDet1"  + Suffix );
  evt.put( fSubDet2,  Prefix + "JetIDFSubDet2"  + Suffix );
  evt.put( fSubDet3,  Prefix + "JetIDFSubDet3"  + Suffix );
  evt.put( fSubDet4,  Prefix + "JetIDFSubDet4"  + Suffix );
  evt.put( resEMF,  Prefix + "JetIDResEMF"  + Suffix );
  evt.put( n90Hits,  Prefix + "JetIDN90Hits"  + Suffix );
  evt.put( jetidminimal,  Prefix + "JetIDminimal"  + Suffix );
  evt.put( jetidloose,  Prefix + "JetIDloose"  + Suffix );
  evt.put( jetidtight,  Prefix + "JetIDtight"  + Suffix );
  evt.put( NECALTowers,  Prefix + "JetIDnECALTowers"  + Suffix );
  evt.put( NHCALTowers,  Prefix + "JetIDnHCALTowers"  + Suffix );
}



template<class T> void Tuple_Jet<T>::
initMPT() { if(!mpt) return;
  const std::vector<int> qualities(config.getParameter<std::vector<int> >("TrackQualities"));
  typedef reco::TrackBase::Vector Vector; 
  for(unsigned j=0; j < qualities.size(); ++j) {
    std::string name = (qualities[j]<0 ? "All" : reco::Track::qualityNames[qualities[j]]) + "Tracks" + Suffix;
    name[0] = toupper((unsigned char) name[0]);
    for(unsigned v=VINIT; v<VSIZE; v++) {
      std::string name2 = "with" + vertex_names[v] + name;
      produces <std::vector<unsigned> > (Prefix + "Count" + name2);
      produces   <std::vector<Vector> > (Prefix + "SumP3" + name2);
    }
  }
}

template<class T> void Tuple_Jet<T>::
produceMPT(edm::Event& evt, const edm::Handle<edm::View<T> >& jets) { if(!mpt) return;
  typedef reco::TrackBase::Vector Vector; 
  const std::vector<int> qualities(config.getParameter<std::vector<int> >("TrackQualities"));
  const double maxD0(config.getParameter<double>("MaxD0Trk"));
  edm::Handle<reco::VertexCollection> vertices;   
  evt.getByLabel(config.getParameter<edm::InputTag>("PrimaryVertexTag"), vertices);
  const reco::Vertex& PrimaryVertex = vertices->front();

  std::vector< std::vector< std::vector<unsigned> > > ntracks( VSIZE, std::vector< std::vector<unsigned> > (qualities.size(), std::vector<unsigned>(jets->size(),0) )) ;
  std::vector< std::vector< std::vector  <Vector> > >   sump3( VSIZE, std::vector< std::vector  <Vector> > (qualities.size(), std::vector<Vector>(jets->size(), Vector()) )) ;
 
  for( unsigned i=0; jets.isValid() && i< jets->size(); ++i ) {    
    for (reco::TrackRefVector::iterator trk = (*jets)[i].associatedTracks().begin(); trk != (*jets)[i].associatedTracks().end(); ++trk) {
      if ( fabs((*trk)->dxy(PrimaryVertex.position())) < maxD0 ) {

	std::vector<double> dist; std::transform(vertices->begin(), vertices->end(), back_inserter(dist), dsz(*(*trk)) );
	std::vector<double>::const_iterator dmin = min_element(dist.begin(), dist.end());
	unsigned v = (dmin==dist.begin()) ? V_PRIMARY : V_PILEUP;
	
	for(unsigned q=0; q<qualities.size(); ++q) {
	  if((*trk)->quality(reco::TrackBase::TrackQuality(qualities[q]))) {
	    ntracks[v][q][i]++;
	    sump3[v][q][i] += (*trk)->momentum();
	  }
	}
      }
    }
  }
  
  for(unsigned q=0; q<qualities.size(); ++q) {
    std::string name = (qualities[q]<0 ? "All" : reco::Track::qualityNames[qualities[q]]) + "Tracks" + Suffix;
    name[0] = toupper((unsigned char) name[0]);
    for(unsigned v=VINIT; v<VSIZE; v++) {
      std::string name2 = "with" + vertex_names[v] + name;
      evt.put( std::auto_ptr<std::vector<unsigned> > ( new std::vector<unsigned>(      ntracks[v][q].begin(),      ntracks[v][q].end())), Prefix + "Count" + name2);
      evt.put( std::auto_ptr<std::vector<Vector> >   ( new std::vector<Vector>(          sump3[v][q].begin(),        sump3[v][q].end())), Prefix + "SumP3" + name2);
    }
  }
}


template<class T> void Tuple_Jet<T>::
initBJetTag(){
  //btag discriminators
  produces <std::vector<float> > (Prefix + "TrkCountingHighEffBJetTags" + Suffix);
  produces <std::vector<float> > (Prefix + "TrkCountingHighPurBJetTags" + Suffix);
  produces <std::vector<float> > (Prefix + "SimpleSecondaryVertexHighEffBJetTags" + Suffix);
  produces <std::vector<float> > (Prefix + "SimpleSecondaryVertexHighPurBJetTags" + Suffix);
  produces <std::vector<float> > (Prefix + "CombinedSecondaryVertexBJetTags" + Suffix);
  produces <std::vector<float> > (Prefix + "CombinedSecondaryVertexMVABJetTags" + Suffix);
  produces <std::vector<float> > (Prefix + "JetProbabilityBJetTags" + Suffix);
  produces <std::vector<float> > (Prefix + "JetBProbabilityBJetTags" + Suffix);
  produces <std::vector<float> > (Prefix + "SoftElectronByIP3dBJetTags" + Suffix);
  produces <std::vector<float> > (Prefix + "SoftElectronByPtBJetTags" + Suffix);
  produces <std::vector<float> > (Prefix + "SoftMuonBJetTags" + Suffix);
  produces <std::vector<float> > (Prefix + "SoftMuonByIP3dBJetTags" + Suffix);
  produces <std::vector<float> > (Prefix + "SoftMuonByPtBJetTags" + Suffix);
  produces <std::vector<int> > (Prefix + "genJetFlavour" + Suffix);

}

template<class T> void Tuple_Jet<T>::
produceBJetTag(edm::Event& evt, const edm::Handle<edm::View<T> >& jets){

//btag discriminators
  std::auto_ptr<std::vector<float> > TrkCountHighEffBJetTags (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > TrkCountHighPurBJetTags (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > SimpSecVertHEBJetTags (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > SimpSecVertHPBJetTags (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > CombSecVertBJetTags (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > CombSecVertMVABJetTags (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > jetProbBJetTags (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > jetBProbBJetTags (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > SoftElecIPBJetTags (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > SoftElecPtBJetTags (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > SoftMuonBJetTags (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > SoftMuonIPBJetTags (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > SoftMuonPtBJetTags (new std::vector<float>() );
  std::auto_ptr<std::vector<int> > JetFlavour(new std::vector<int>() ); 

  if(jets.isValid()){
    for (unsigned i=0; i<(*jets).size(); i++) {
//btag discriminators
    TrkCountHighEffBJetTags->push_back((*jets)[i].bDiscriminator("trackCountingHighEffBJetTags"));
    TrkCountHighPurBJetTags->push_back((*jets)[i].bDiscriminator("trackCountingHighPurBJetTags"));
    SimpSecVertHEBJetTags->push_back((*jets)[i].bDiscriminator("simpleSecondaryVertexHighEffBJetTags"));
    SimpSecVertHPBJetTags->push_back((*jets)[i].bDiscriminator("simpleSecondaryVertexHighPurBJetTags"));
    CombSecVertBJetTags->push_back((*jets)[i].bDiscriminator("combinedSecondaryVertexBJetTags"));
    CombSecVertMVABJetTags->push_back((*jets)[i].bDiscriminator("combinedSecondaryVertexMVABJetTags"));
    jetProbBJetTags->push_back((*jets)[i].bDiscriminator("jetProbabilityBJetTags"));
    jetBProbBJetTags->push_back((*jets)[i].bDiscriminator("jetBProbabilityBJetTags"));
    SoftElecIPBJetTags->push_back((*jets)[i].bDiscriminator("softElectronByIP3dBJetTags"));
    SoftElecPtBJetTags->push_back((*jets)[i].bDiscriminator("softElectronByPtBJetTags"));
    SoftMuonBJetTags->push_back((*jets)[i].bDiscriminator("softMuonBJetTags"));
    SoftMuonIPBJetTags->push_back((*jets)[i].bDiscriminator("softMuonByIP3dBJetTags"));
    SoftMuonPtBJetTags->push_back((*jets)[i].bDiscriminator("softMuonByPtBJetTags"));
    JetFlavour->push_back((*jets)[i].partonFlavour());
    }
  }


 //btag discriminators
  evt.put(TrkCountHighEffBJetTags, Prefix + "TrkCountingHighEffBJetTags" + Suffix);
  evt.put(TrkCountHighPurBJetTags, Prefix + "TrkCountingHighPurBJetTags" + Suffix);
  evt.put(SimpSecVertHEBJetTags, Prefix + "SimpleSecondaryVertexHighEffBJetTags" + Suffix);
  evt.put(SimpSecVertHPBJetTags, Prefix + "SimpleSecondaryVertexHighPurBJetTags" + Suffix);
  evt.put(CombSecVertBJetTags, Prefix + "CombinedSecondaryVertexBJetTags" + Suffix);
  evt.put(CombSecVertMVABJetTags, Prefix + "CombinedSecondaryVertexMVABJetTags" + Suffix);
  evt.put(jetProbBJetTags, Prefix + "JetProbabilityBJetTags" + Suffix);
  evt.put(jetBProbBJetTags, Prefix + "JetBProbabilityBJetTags" + Suffix);
  evt.put(SoftElecIPBJetTags, Prefix + "SoftElectronByIP3dBJetTags" + Suffix);
  evt.put(SoftElecPtBJetTags, Prefix + "SoftElectronByPtBJetTags" + Suffix);
  evt.put(SoftMuonBJetTags, Prefix + "SoftMuonBJetTags" + Suffix);
  evt.put(SoftMuonIPBJetTags, Prefix + "SoftMuonByIP3dBJetTags" + Suffix);
  evt.put(SoftMuonPtBJetTags, Prefix + "SoftMuonByPtBJetTags" + Suffix);
  evt.put(JetFlavour, Prefix + "genJetFlavour" + Suffix);
}



template<class T> void Tuple_Jet<T>::
initGenJetMatch() { if(gen) produces <std::vector<int> > (Prefix + "GenJetMatchIndex" + Suffix);}

template<class T> void Tuple_Jet<T>::
produceGenJetMatch(edm::Event& evt, const edm::Handle<edm::View<T> >& jets){
  if(!gen) return;
  edm::Handle<edm::View<reco::GenJet> > genjets; 
  evt.getByLabel(genJetsInputTag, genjets);
  std::auto_ptr<std::vector<int> > genjetMatchIndex( new std::vector<int>() );
  if(jets.isValid() && genjets.isValid())
    for (typename edm::View<T>::const_iterator jet = jets->begin(); jet!=jets->end(); ++jet ) 
      genjetMatchIndex->push_back( indexOfMatch( jet->genJet(), genjets->begin(), genjets->end()) );
  evt.put(genjetMatchIndex, Prefix + "GenJetMatchIndex" + Suffix);
}

template<class T> template<class I> int Tuple_Jet<T>::
indexOfMatch( const reco::GenJet* genjet, const I begin, const I end) {
  for(I it=begin; it!=end; ++it) if ( genjet && genjet->p4() == it->p4() ) return it-begin; //p4 comparisons
  return -1;
}



template<class T> const reco::CaloJet* Tuple_Jet<T>::getJet( const edm::RefToBase<T>& jet_ref ) { 
  std::cout << "Don't use this!!!" << std::endl;
  return 0;
}

template<> const reco::CaloJet* Tuple_Jet<reco::CaloJet>::getJet( const edm::RefToBase<reco::CaloJet>& jet_ref ) { 
  return jet_ref.get(); 
}

template<> const reco::CaloJet* Tuple_Jet<reco::JPTJet>::getJet( const edm::RefToBase<reco::JPTJet>& jet_ref ) { 
  const reco::JPTJet* tmp = jet_ref.get(); 
  if ( tmp ) { return static_cast<const reco::CaloJet*>( tmp->getCaloJetRef().get() ); }
  else { return 0; }
}

template<> const reco::CaloJet* Tuple_Jet<pat::Jet>::getJet( const edm::RefToBase<pat::Jet>& jet_ref ) { 
  if ( caloSpecific ) { return static_cast<const reco::CaloJet*>( jet_ref->originalObjectRef().get() ); }
  else if ( jptSpecific ) { 
    const reco::JPTJet* tmp = static_cast<const reco::JPTJet*>( jet_ref->originalObjectRef().get() ); 
    if ( tmp ) { return static_cast<const reco::CaloJet*>( tmp->getCaloJetRef().get() ); }
  } 
  return 0;
}

#endif


