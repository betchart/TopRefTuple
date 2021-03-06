#ifndef TUPLE_JET
#define TUPLE_JET

#include "boost/foreach.hpp"
#include "TopQuarkAnalysis/TopRefTuple/interface/fTypes.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"

#include "DataFormats/PatCandidates/interface/Jet.h"

#include "PhysicsTools/SelectorUtils/interface/PFJetIDSelectionFunctor.h"

#include "CondFormats/JetMETObjects/interface/JetResolution.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"
#include "JetMETCorrections/Objects/interface/JetCorrectionsRecord.h"
#include "TH1D.h"


JetCorrectionUncertainty* jetCorrUnc(const edm::EventSetup& setup, const std::string& jecRecord, const std::string fileName, const std::string& jecName) {
  if(jecName=="") {
    edm::ESHandle<JetCorrectorParametersCollection> JetCorParColl;
    setup.get<JetCorrectionsRecord>().get(jecRecord,JetCorParColl);
    return new JetCorrectionUncertainty((*JetCorParColl)["Uncertainty"]);
  }
  JetCorrectorParameters p(fileName, jecName);
  return new JetCorrectionUncertainty(p);
}

float uncFunc(JetCorrectionUncertainty* jCU, const reco::Candidate::LorentzVector& jet) {
  jCU->setJetEta(jet.eta());
  jCU->setJetPt(jet.pt());
  try {return jCU->getUncertainty(true);}
  catch (...) {
    std::cout << "Tuple_Jet.h: JetCorrectionUncertainty::getUncertainty" 
	      << "threw exception on jet with pt( " << jet.pt() << " ) "
	      << "and eta ( " << jet.eta() << " )." << std::endl;
    return 0.0;
  }
}



template< typename T >
class Tuple_Jet : public edm::EDProducer {
 public: 
  explicit Tuple_Jet(const edm::ParameterSet&);
  typedef fTypes::dPolarLorentzV LorentzV;
  typedef edm::Handle<edm::View<T> > Handle_t;
 private: 
  void produce(edm::Event&, const edm::EventSetup& );
  void initPF();   void producePF(edm::Event&, const Handle_t&, const Handle_t&);
  void initGen();  void produceGen(edm::Event&, const Handle_t&, const Handle_t&);

  const edm::InputTag jetsTag, allJetsTag;
  const std::string prefix,jecRecord,juf;
  const std::vector<std::string> btagNames, jecNames;
  const bool pfInfo, genInfo;
  JetResolution jer;
  TH1D dataMcResRatio;
};

template<class T> Tuple_Jet<T>::
Tuple_Jet(const edm::ParameterSet& cfg) :
  jetsTag(cfg.getParameter<edm::InputTag>("jetsTag")),
  allJetsTag(cfg.getParameter<edm::InputTag>("allJetsTag")),
  prefix(cfg.getParameter<std::string>("prefix")),
  jecRecord(cfg.getParameter<std::string>("jecRecord")),
  juf(cfg.getParameter<std::string>("jetUncertaintyFile")),
  btagNames(cfg.getParameter<std::vector<std::string> >("bTags")),
  jecNames(cfg.getParameter<std::vector<std::string> >("jecNames")),
  pfInfo(cfg.getParameter<bool>("pfInfo")),
  genInfo(cfg.getParameter<bool>("genInfo")),
  jer(cfg.getParameter<std::string>("jetResolutionFile")),
  dataMcResRatio("dataMcResRatio",
		 "",
		 cfg.getParameter<std::vector<double> >("resolutionRatioBins").size()-1,
		 &cfg.getParameter<std::vector<double> >("resolutionRatioBins").at(0) )
{
  produces <std::vector<LorentzV> > ( prefix + "P4"   );
  produces <std::vector<float> >    ( prefix + "JecFactor"   );
  BOOST_FOREACH(const std::string& jecName, jecNames)
    produces <std::vector<float> >    ( prefix + "JecUnc" + jecName    );
  produces <std::vector<float> >    ( prefix + "Area"        );
  produces <std::vector<float> >    ( prefix + "Resolution"  );
  produces <std::vector<float> >    ( prefix + "Charge"      );
  produces <std::vector<float> >    ( prefix + "Eta2Moment"  );
  produces <std::vector<float> >    ( prefix + "Phi2Moment"  );
  BOOST_FOREACH(const std::string& btag, btagNames) 
    produces <std::vector<float> > (prefix + btag);

  if(pfInfo) initPF();
  if(genInfo) {
    initGen();
    for(int bin=1; bin<=dataMcResRatio.GetNbinsX(); ++bin) {
      dataMcResRatio.SetBinContent( bin, cfg.getParameter<std::vector<double> >("resolutionRatio").at(bin-1) );
      dataMcResRatio.SetBinError( bin, cfg.getParameter<std::vector<double> >("resolutionRatioErr").at(bin-1) );
    }
    dataMcResRatio.SetBinContent( 1 + dataMcResRatio.GetNbinsX(), 1.0);
    dataMcResRatio.SetBinError( 1 + dataMcResRatio.GetNbinsX(), 0.0);
  }
}

template< typename T > 
void Tuple_Jet<T>::
produce(edm::Event& evt, const edm::EventSetup& setup) {
  Handle_t jets;    evt.getByLabel(jetsTag, jets);
  Handle_t all;     evt.getByLabel(allJetsTag, all);

  std::auto_ptr<std::vector<LorentzV> >   p4   ( new std::vector<LorentzV>()  )  ;
  std::auto_ptr<std::vector<float> >  jecFactor( new std::vector<float>()  )  ;
  std::map<std::string, std::vector<float>* > jecUnc;
  BOOST_FOREACH(const std::string& jecName, jecNames)
    jecUnc[jecName] = new std::vector<float>();

  std::auto_ptr<std::vector<float> >  reso     ( new std::vector<float>()  )  ;
  std::auto_ptr<std::vector<float> >  area     ( new std::vector<float>()  )  ;
  std::auto_ptr<std::vector<float> >  charge   ( new std::vector<float>()  )  ;
  std::auto_ptr<std::vector<float> >  eta2mom  ( new std::vector<float>()  )  ;
  std::auto_ptr<std::vector<float> >  phi2mom  ( new std::vector<float>()  )  ;
  std::map<std::string, std::vector<float>* > btags;
  BOOST_FOREACH(const std::string& btag, btagNames) 
    btags[btag] = new std::vector<float>();

  if(jets.isValid()) {
    std::map<std::string, JetCorrectionUncertainty*> jCU;
    BOOST_FOREACH(const std::string& jecName, jecNames)
      jCU[jecName] = jetCorrUnc(setup, jecRecord, juf, jecName);

    for(typename edm::View<T>::const_iterator jet = jets->begin(); jet!=jets->end(); jet++) {
      p4->push_back(LorentzV(jet->pt(),jet->eta(),jet->phi(),jet->mass()));
      jecFactor->push_back( jet->jecSetsAvailable() ? jet->energy() / jet->correctedJet("Uncorrected").energy() : 1.0 );
      BOOST_FOREACH(const std::string& jecName, jecNames)
	jecUnc[jecName]->push_back(uncFunc(jCU[jecName], jet->p4()));
      area->push_back(jet->jetArea());
      charge->push_back(jet->jetCharge());
      eta2mom->push_back(jet->etaetaMoment());
      phi2mom->push_back(jet->phiphiMoment());

      BOOST_FOREACH(const std::string& btag, btagNames)	
	btags[btag]->push_back(jet->bDiscriminator(btag+"BJetTags"));

      TF1 * jerEta = jer.parameterEta("sigma",jet->eta());
      reso->push_back( jerEta->Eval( jet->pt() )  );
      delete jerEta;
    }
    BOOST_FOREACH(const std::string& jecName, jecNames)
      delete jCU[jecName];
  }

  evt.put(      p4,   prefix + "P4"  );
  evt.put(jecFactor,  prefix + "JecFactor"   );
  BOOST_FOREACH(const std::string& jecName, jecNames)
    evt.put(  std::auto_ptr<std::vector<float> >(jecUnc[jecName]),   prefix + "JecUnc" + jecName );

  evt.put(    reso,   prefix + "Resolution"  );
  evt.put(    area,   prefix + "Area"        );
  evt.put(  charge,   prefix + "Charge"      );
  evt.put( eta2mom,   prefix + "Eta2Moment"  );
  evt.put( phi2mom,   prefix + "Phi2Moment"  );
  BOOST_FOREACH(const std::string& btag, btagNames) 
    evt.put( std::auto_ptr<std::vector<float> >(btags[btag]), prefix + btag );

  if(pfInfo) producePF(evt,jets, all);
  if(genInfo) produceGen(evt,jets, all);
}

template<class T> void Tuple_Jet<T>::
initPF() {
  produces <std::vector<float> > (prefix + "FchargedHad" );
  produces <std::vector<float> > (prefix + "FneutralHad" );
  produces <std::vector<float> > (prefix + "FchargedEm" );
  produces <std::vector<float> > (prefix + "FneutralEm" );
  produces <std::vector<float> > (prefix + "FchargedMu" );

  produces <std::vector<unsigned> > (prefix + "Ncharged" );
  produces <std::vector<unsigned> > (prefix + "Nneutral" );
  produces <std::vector<unsigned> > (prefix + "Nmuon" );
  produces <std::vector<unsigned> > (prefix + "Nelectron" );
  produces <std::vector<unsigned> > (prefix + "Ndaughters" );
  
  produces <std::vector<bool> > ( prefix + "PFJetIDloose"   );
  produces <std::vector<bool> > ( prefix + "PFJetIDtight"   );
  produces <float> ( prefix + "FailedPtMax");

  produces <std::vector<int> > (prefix + "MuonsChargeSum" );
  produces <std::vector<int> > (prefix + "ElectronsChargeSum" );
}

template<class T> void Tuple_Jet<T>::
producePF(edm::Event& evt, const Handle_t& jets, const Handle_t& all) {
  std::auto_ptr<std::vector<float> > FchargedHad( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > FneutralHad( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > FchargedEm( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > FneutralEm( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > FchargedMu( new std::vector<float>() );
  
  std::auto_ptr<std::vector<unsigned> > Ncharged( new std::vector<unsigned>() );
  std::auto_ptr<std::vector<unsigned> > Nneutral( new std::vector<unsigned>() );
  std::auto_ptr<std::vector<unsigned> > Nmuon( new std::vector<unsigned>() );
  std::auto_ptr<std::vector<unsigned> > Nelectron( new std::vector<unsigned>() );
  std::auto_ptr<std::vector<unsigned> > Ndaughters( new std::vector<unsigned>() );

  std::auto_ptr<std::vector<bool> >  pfjetidloose  ( new std::vector<bool>()  ) ;
  std::auto_ptr<std::vector<bool> >  pfjetidtight  ( new std::vector<bool>()  ) ;
  std::auto_ptr<float>  failedPt  ( new float(-1)  )  ;

  std::auto_ptr<std::vector<int> > muonsChargeSum ( new std::vector<int>() ) ;
  std::auto_ptr<std::vector<int> > electronsChargeSum ( new std::vector<int>() ) ;

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
      Nelectron->push_back( (unsigned) jet->electronMultiplicity() );
      Ndaughters->push_back( (unsigned) jet->numberOfDaughters() );
      
      pfjetidloose->push_back(pfLooseJetID( *jet, passLooseCuts  ));
      pfjetidtight->push_back(pfTightJetID( *jet, passTightCuts  ));
      
      muonsChargeSum->push_back( 0 ) ;
      electronsChargeSum->push_back( 0 ) ;
      for(unsigned nMoreMu(Nmuon->back()),nMoreEl(Nelectron->back()), i(0); nMoreMu || nMoreEl; i++) {
	int pdgId = jet->getPFConstituent(i)->pdgId();
	if(abs(pdgId)==13) {
	  nMoreMu--;
	  muonsChargeSum->back() += (pdgId>0 ? 1 : -1) ;
	}
	if(abs(pdgId)==11) {
	  nMoreEl--;
	  electronsChargeSum->back() += (pdgId>0 ? 1 : -1) ;
	}
      }
    }
  }
  if(all.isValid()) {
    *failedPt=0;
    for(typename edm::View<T>::const_iterator jet = all->begin(); jet!=all->end(); jet++) {
      pat::strbitset passLooseCuts(   pfLooseJetID  .getBitTemplate() );
      if( !pfLooseJetID( *jet, passLooseCuts ) && jet->pt() > *failedPt) *failedPt = jet->pt();
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
  evt.put(Nelectron,   prefix + "Nelectron"   );
  evt.put(Ndaughters,  prefix + "Ndaughters"  );
 
  evt.put( pfjetidloose,  prefix + "PFJetIDloose"   );
  evt.put( pfjetidtight,  prefix + "PFJetIDtight"   );
  evt.put( failedPt,  prefix + "FailedPtMax" );

  evt.put( muonsChargeSum, prefix + "MuonsChargeSum" );
  evt.put( electronsChargeSum, prefix + "ElectronsChargeSum" );
}


template<class T> 
void Tuple_Jet<T>::
initGen() {
  produces <LorentzV> (prefix + "DeltaMETSmear");
  produces <LorentzV> (prefix + "DeltaMETSmearUp");
  produces <LorentzV> (prefix + "DeltaMETSmearDown");

  produces <std::vector<float> > (prefix + "Smear");
  produces <std::vector<float> > (prefix + "SmearUp");
  produces <std::vector<float> > (prefix + "SmearDown");
  produces <std::vector<LorentzV> > (prefix + "GenP4" );
  produces <std::vector<int> > (prefix + "GenFlavor" );
  produces <std::vector<int> > (prefix + "GenPdgId" );
  produces <std::vector<int> > (prefix + "GenMotherPdgId" );
}


template<class T> 
void Tuple_Jet<T>::
produceGen(edm::Event& evt, const Handle_t& jets, const Handle_t& all){

  std::auto_ptr<LorentzV> deltaMET( new LorentzV() );
  std::auto_ptr<LorentzV> deltaMETu( new LorentzV() );
  std::auto_ptr<LorentzV> deltaMETd( new LorentzV() );

  if(all.isValid()) {
    for (typename edm::View<T>::const_iterator jet = all->begin(); jet!=all->end(); ++jet ) {
      const reco::GenJet * gen = jet->genJet();
      if( !gen ) continue;
      const int bin( dataMcResRatio.FindFixBin(fabs(gen->eta())) );
      const double
  	c( dataMcResRatio.GetBinContent(bin) ),
  	cu( c + dataMcResRatio.GetBinError(bin) ),
  	cd( 2*c - cu ),
  	pTgOverPt( gen->pt() / jet->pt() );
      *deltaMET  += (1 - std::max(0., c  + (1-c ) * pTgOverPt ) ) * jet->p4() ;
      *deltaMETu += (1 - std::max(0., cu + (1-cu) * pTgOverPt ) ) * jet->p4() ;
      *deltaMETd += (1 - std::max(0., cd + (1-cd) * pTgOverPt ) ) * jet->p4() ;
    }
  }
  evt.put( deltaMET, prefix + "DeltaMETSmear");
  evt.put( deltaMETu, prefix + "DeltaMETSmearUp");
  evt.put( deltaMETd, prefix + "DeltaMETSmearDown");

  std::auto_ptr<std::vector<LorentzV> > p4(new std::vector<LorentzV>() );
  std::auto_ptr<std::vector<int> > flavor(new std::vector<int>() );
  std::auto_ptr<std::vector<float> > smear(new std::vector<float>() );
  std::auto_ptr<std::vector<float> > smearu(new std::vector<float>() );
  std::auto_ptr<std::vector<float> > smeard(new std::vector<float>() );
  std::auto_ptr<std::vector<int> > pdgId(new std::vector<int>() );
  std::auto_ptr<std::vector<int> > motherPdgId(new std::vector<int>() );

  if(jets.isValid()) {
    for (typename edm::View<T>::const_iterator jet = jets->begin(); jet!=jets->end(); ++jet ) {
      const reco::GenJet * gen = jet->genJet();
      const reco::GenParticle * genP = jet->genParton();
      const int bin( gen ? dataMcResRatio.FindFixBin(fabs(gen->eta())) : -1 );
      const double
	c( dataMcResRatio.GetBinContent(bin) ),
	cu( c + dataMcResRatio.GetBinError(bin) ),
	cd( 2*c - cu ),
	pTgOverPt( gen ? gen->pt() / jet->pt() : 0 );

      p4->push_back( gen ? LorentzV(gen->pt(), gen->eta(), gen->phi(), gen->mass()) : LorentzV());
      flavor->push_back(jet->partonFlavour());
      smear->push_back(  gen ? std::max(0., c  + (1-c ) * pTgOverPt ) : 1.0);
      smearu->push_back( gen ? std::max(0., cu + (1-cu) * pTgOverPt ) : 1.0);
      smeard->push_back( gen ? std::max(0., cd + (1-cd) * pTgOverPt ) : 1.0);
      pdgId->push_back( genP ? genP->pdgId() : 0.0 );
      motherPdgId->push_back( genP ? genP->mother()->pdgId() : 0.0 );
    }
  }
  evt.put(     p4, prefix + "GenP4" );
  evt.put( flavor, prefix + "GenFlavor" );
  evt.put( smear , prefix + "Smear" );
  evt.put( smearu, prefix + "SmearUp" );
  evt.put( smeard, prefix + "SmearDown" );
  evt.put( pdgId, prefix + "GenPdgId" );
  evt.put( motherPdgId, prefix + "GenMotherPdgId" );
}

#endif


