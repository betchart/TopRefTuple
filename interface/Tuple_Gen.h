#ifndef TUPLE_GEN
#define TUPLE_GEN

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "fTypes.h"

#include <map>

template< typename T >
class Tuple_Gen : public edm::EDProducer {
 public:
  explicit Tuple_Gen(const edm::ParameterSet&);
 private:
  void produce(edm::Event &, const edm::EventSetup & );
  void produceGenJets(edm::Event &);
  static int index(const reco::Candidate*, const std::vector<const T*>&);
  typedef fTypes::dPolarLorentzV LorentzVector;
  const edm::InputTag inputTag;
  const std::vector<edm::InputTag> jetCollections;
  const std::string Prefix,Suffix;
  const double GenStatus1PtCut;
  const double GenJetPtCut;
};

template< typename T > Tuple_Gen<T>::
Tuple_Gen(const edm::ParameterSet& conf) 
  : inputTag(conf.getParameter<edm::InputTag>("InputTag")),
    jetCollections(conf.getParameter<std::vector<edm::InputTag> >("JetCollections")),
    Prefix(conf.getParameter<std::string>("Prefix")),
    Suffix(conf.getParameter<std::string>("Suffix")),
    GenStatus1PtCut(conf.getParameter<double>("GenStatus1PtCut")),
    GenJetPtCut(conf.getParameter<double>("GenJetPtCut")) 
{
  produces <unsigned int> (Prefix + "signalProcessID" + Suffix);
  produces <bool>   (Prefix + "GenInfoHandleValid" + Suffix);
  produces <bool >  (Prefix + "HandleValid" + Suffix);
  produces <float> (Prefix + "pthat" + Suffix);
  produces <int> (Prefix + "id1" + Suffix);
  produces <int> (Prefix + "id2" + Suffix);
  produces <float> (Prefix + "x1" + Suffix);
  produces <float> (Prefix + "x2" + Suffix);
  produces <float> (Prefix + "pdf1" + Suffix);
  produces <float> (Prefix + "pdf2" + Suffix);
  produces <std::vector<float> > (Prefix + "BinningValues" + Suffix);
  produces <float> (Prefix + "Q" + Suffix);
  produces <std::vector<LorentzVector> > ( Prefix + "P4"  + Suffix );
  produces <std::vector<int> > (Prefix + "PdgId" + Suffix);
  produces <std::vector<int> > (Prefix + "Status" + Suffix);
  produces <std::vector<int> > (Prefix + "MotherIndex" + Suffix);
  produces <std::vector<int> > (Prefix + "MotherPdgId" + Suffix);
  

  for(unsigned i=0; i<jetCollections.size(); ++i)
    produces<std::vector<LorentzVector> >(Prefix + jetCollections[i].label() +"P4" + Suffix);
}

template< typename T > int Tuple_Gen<T>::
index(const reco::Candidate* item, const typename std::vector<const T*>& collection) {
  typename std::vector<const T*>::const_iterator it(collection.begin()), begin(collection.begin()), end(collection.end());
  for(; it!=end; it++) if ((*it)==item) return it-begin; //Compare addresses
  return -1;
}

template< typename T > void Tuple_Gen<T>::
produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  produceGenJets(iEvent);

  edm::Handle<std::vector<T> > collection;   iEvent.getByLabel(inputTag,collection);
  edm::Handle<GenEventInfoProduct> geninfo;  iEvent.getByLabel("generator",geninfo);

  std::auto_ptr<unsigned int> signalProcessID(new unsigned int(geninfo->signalProcessID()));
  std::auto_ptr<float> Q (new float(geninfo->pdf()->scalePDF));
  std::auto_ptr<int> id1 (new int( geninfo->pdf()->id.first));
  std::auto_ptr<int> id2 (new int( geninfo->pdf()->id.second));
  std::auto_ptr<float> x1 (new float( geninfo->pdf()->x.first));
  std::auto_ptr<float> x2 (new float( geninfo->pdf()->x.second));
  std::auto_ptr<float> pdf1 (new float( geninfo->pdf()->xPDF.first));
  std::auto_ptr<float> pdf2 (new float( geninfo->pdf()->xPDF.second));

  std::auto_ptr<bool> handleValid ( new bool(collection.isValid()) );
  std::auto_ptr<bool> genInfoValid ( new bool( geninfo.isValid() && !geninfo->binningValues().empty()));
  std::auto_ptr<float> pthat (new float(*genInfoValid ? geninfo->binningValues()[0] : -1.));
  std::auto_ptr<std::vector<float> > binningValues (*genInfoValid ? new std::vector<float>(geninfo->binningValues().begin(),
											   geninfo->binningValues().end()) : new std::vector<float>());
  std::auto_ptr<std::vector<LorentzVector> >  p4  ( new std::vector<LorentzVector>()  ) ;
  std::auto_ptr<std::vector<int> > status ( new std::vector<int>() ) ;
  std::auto_ptr<std::vector<int> > pdgId ( new std::vector<int>() ) ;
  std::auto_ptr<std::vector<int> > motherIndex ( new std::vector<int>() ) ;
  std::auto_ptr<std::vector<int> > motherPdgId ( new std::vector<int>() ) ;

  std::vector<const T*> self;
  std::vector<const reco::Candidate*> mom;

  if(collection.isValid()){
    for(typename std::vector<T>::const_iterator it = collection->begin(); it != collection->end(); ++it) {
      if ( it->status() == 3         // any status 3 genParticle
	   || abs(it->pdgId()) == 11 // any electron
	   || abs(it->pdgId()) == 13 // any muon
	   || abs(it->pdgId()) == 15 // any tau
	   || ( it->status() == 1    //        status 1 particles
		&& it->pt() > GenStatus1PtCut) // above threshold
	   ) {
	p4->push_back(LorentzVector(it->pt(),it->eta(),it->phi(),it->mass()));
	status->push_back(it->status());
	pdgId->push_back(it->pdgId());
	motherPdgId->push_back( it->numberOfMothers() ? it->mother()->pdgId() : 0 );
	self.push_back(&*it);
	mom.push_back( it->numberOfMothers() ? it->mother(): 0);
      }
    }
  } //collection


  for(typename std::vector<const reco::Candidate*>::const_iterator it = mom.begin(); it!=mom.end(); ++it)
    motherIndex->push_back( index(*it,self) );

  iEvent.put( handleValid,  Prefix + "HandleValid"        + Suffix);
  iEvent.put( genInfoValid, Prefix + "GenInfoHandleValid" + Suffix);
  iEvent.put( pthat,        Prefix + "pthat"  + Suffix);
  iEvent.put( binningValues,Prefix + "BinningValues" + Suffix);
  iEvent.put( p4,           Prefix + "P4"     + Suffix );
  iEvent.put( status,       Prefix + "Status" + Suffix );
  iEvent.put( pdgId,        Prefix + "PdgId"  + Suffix );
  iEvent.put( motherIndex,  Prefix + "MotherIndex" + Suffix );
  iEvent.put( motherPdgId,  Prefix + "MotherPdgId" + Suffix );
  iEvent.put( signalProcessID, Prefix + "signalProcessID" + Suffix );
  iEvent.put( Q,            Prefix + "Q" + Suffix );
  iEvent.put( x1,           Prefix + "x1" + Suffix );
  iEvent.put( x2,           Prefix + "x2" + Suffix );
  iEvent.put( pdf1,         Prefix + "pdf1" + Suffix );
  iEvent.put( pdf2,         Prefix + "pdf2" + Suffix );
  iEvent.put( id1,          Prefix + "id1" + Suffix );
  iEvent.put( id2,          Prefix + "id2" + Suffix );
}

template< typename T > void Tuple_Gen<T>::
produceGenJets(edm::Event& iEvent) {
  for(unsigned i=0; i<jetCollections.size(); ++i) {
    std::auto_ptr<std::vector<LorentzVector> > p4(new std::vector<LorentzVector>());
    edm::Handle<edm::View<reco::GenJet> > genjets;
    iEvent.getByLabel(jetCollections[i], genjets);
    if(genjets.isValid()) 
      for(edm::View<reco::GenJet>::const_iterator it(genjets->begin()), end(genjets->end()); it!=end; ++it) {
	if (it->pt() >= GenJetPtCut) p4->push_back(LorentzVector(it->pt(),it->eta(),it->phi(),it->mass()));
      }
    iEvent.put(p4, Prefix + jetCollections[i].label() + "P4" + Suffix);
  }
}

#endif
