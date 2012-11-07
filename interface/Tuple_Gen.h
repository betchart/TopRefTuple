#ifndef TUPLE_GEN
#define TUPLE_GEN

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"

// LHE Event headers
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"
#include "GeneratorInterface/LHEInterface/interface/LHEEvent.h"

#include <map>

template< typename T >
class Tuple_Gen : public edm::EDProducer {
 public:
  explicit Tuple_Gen(const edm::ParameterSet&);
 private:
  void produce(edm::Event &, const edm::EventSetup & );
  void produceGenJets(edm::Event &);
  static int index(const reco::Candidate*, const std::vector<const T*>&);
  typedef reco::Candidate::LorentzVector LorentzVector;
  const edm::InputTag inputTag;
  const std::vector<edm::InputTag> jetCollections;
  const std::string Prefix,Suffix;
  const double GenStatus1PtCut;
  const double GenJetPtCut;
};

template< typename T > Tuple_Gen<T>::
Tuple_Gen(const edm::ParameterSet& iConfig) :
  inputTag(iConfig.getParameter<edm::InputTag>("InputTag")),
  jetCollections(iConfig.getParameter<std::vector<edm::InputTag> >("JetCollections")),
  Prefix(iConfig.getParameter<std::string>("Prefix")),
  Suffix(iConfig.getParameter<std::string>("Suffix")),
  GenStatus1PtCut(iConfig.getParameter<double>("GenStatus1PtCut")),
  GenJetPtCut(iConfig.getParameter<double>("GenJetPtCut")) {
  produces <unsigned int> (Prefix + "signalProcessID" + Suffix);
  produces <bool>   (Prefix + "GenInfoHandleValid" + Suffix);
  produces <bool >  (Prefix + "HandleValid" + Suffix);
  produces <double> (Prefix + "pthat" + Suffix);
  produces <int> (Prefix + "id1" + Suffix);
  produces <int> (Prefix + "id2" + Suffix);
  produces <double> (Prefix + "x1" + Suffix);
  produces <double> (Prefix + "x2" + Suffix);
  produces <double> (Prefix + "pdf1" + Suffix);
  produces <double> (Prefix + "pdf2" + Suffix);
  produces <double> (Prefix + "PartonHT" + Suffix);  
  produces <std::vector<double> > (Prefix + "cteq66" + Suffix);
  produces <std::vector<double> > (Prefix + "NNPDF10" + Suffix);
  produces <std::vector<double> > (Prefix + "MRST2006nnlo" + Suffix);
  produces <std::vector<double> > (Prefix + "BinningValues" + Suffix);
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
  edm::Handle<std::vector<double> > cteqHandle;
  edm::Handle<std::vector<double> > nnpdfHandle;
  edm::Handle<std::vector<double> > mrstHandle;
  iEvent.getByLabel("pdfWeights", "cteq66", cteqHandle);
  iEvent.getByLabel("pdfWeights", "NNPDF10", nnpdfHandle);
  iEvent.getByLabel("pdfWeights", "MRST2006nnlo", mrstHandle);

  //add handle for LHE event
  edm::Handle<LHEEventProduct> product;
  iEvent.getByLabel("source", product);

  std::auto_ptr<unsigned int> signalProcessID(new unsigned int(geninfo->signalProcessID()));
  std::auto_ptr<float> Q (new float(geninfo->pdf()->scalePDF));
  std::auto_ptr<int> id1 (new int( geninfo->pdf()->id.first));
  std::auto_ptr<int> id2 (new int( geninfo->pdf()->id.second));
  std::auto_ptr<double> x1 (new double( geninfo->pdf()->x.first));
  std::auto_ptr<double> x2 (new double( geninfo->pdf()->x.second));
  std::auto_ptr<double> pdf1 (new double( geninfo->pdf()->xPDF.first));
  std::auto_ptr<double> pdf2 (new double( geninfo->pdf()->xPDF.second));

  std::auto_ptr<bool> handleValid ( new bool(collection.isValid()) );
  std::auto_ptr<bool> genInfoValid ( new bool( geninfo.isValid() && !geninfo->binningValues().empty()));
  std::auto_ptr<double> pthat (new double(*genInfoValid ? geninfo->binningValues()[0] : -1.));
  std::auto_ptr<std::vector<double> > binningValues (*genInfoValid ? new std::vector<double>(geninfo->binningValues()) : new std::vector<double>());
  std::auto_ptr<std::vector<LorentzVector> >  p4  ( new std::vector<LorentzVector>()  ) ;
  std::auto_ptr<std::vector<int> > status ( new std::vector<int>() ) ;
  std::auto_ptr<std::vector<int> > pdgId ( new std::vector<int>() ) ;
  std::auto_ptr<std::vector<int> > motherIndex ( new std::vector<int>() ) ;
  std::auto_ptr<std::vector<int> > motherPdgId ( new std::vector<int>() ) ;
  std::auto_ptr<std::vector<double> > cteq ( new std::vector<double>() ) ;
  std::auto_ptr<std::vector<double> > nnpdf ( new std::vector<double>() ) ;
  std::auto_ptr<std::vector<double> > mrst ( new std::vector<double>() ) ;
  std::auto_ptr<double> gHT ( new double(0) );

  std::vector<const T*> self;
  std::vector<const reco::Candidate*> mom;

  if(cteqHandle.isValid()){
  for(std::vector<double>::const_iterator it = cteqHandle->begin(); it != cteqHandle->end(); ++it) {
    cteq->push_back(*it);
  }}
  if(nnpdfHandle.isValid()){
  for(std::vector<double>::const_iterator it = nnpdfHandle->begin(); it != nnpdfHandle->end(); ++it) {
    nnpdf->push_back(*it);
  }}
  if(mrstHandle.isValid()){
  for(std::vector<double>::const_iterator it = mrstHandle->begin(); it != mrstHandle->end(); ++it) {
    mrst->push_back(*it);
  }}
 


  if(collection.isValid()){
    for(typename std::vector<T>::const_iterator it = collection->begin(); it != collection->end(); ++it) {
      if ( it->status() == 3         // any status 3 genParticle
	   || abs(it->pdgId()) == 11 // any electron
	   || abs(it->pdgId()) == 13 // any muon
	   || abs(it->pdgId()) == 15 // any tau
	   || ( it->status() == 1    //        status 1 particles
		&& it->pt() > GenStatus1PtCut) // above threshold
	   ) {
	p4->push_back(it->p4());
	status->push_back(it->status());
	pdgId->push_back(it->pdgId());
	motherPdgId->push_back( it->numberOfMothers() ? it->mother()->pdgId() : 0 );
	self.push_back(&*it);
	mom.push_back( it->numberOfMothers() ? it->mother(): 0);
      }
    }
  } //collection

// taken from the discussion here: https://hypernews.cern.ch/HyperNews/CMS/get/generators/1234/1/1/1.html

  if(product.isValid()){ 
    //gen Parton HT

    //make some new LHE based variables
    const lhef::HEPEUP hepeup_ = product->hepeup();
    const std::vector<lhef::HEPEUP::FiveVector> pup_ = hepeup_.PUP;

    double htEvent=0.;
    size_t iMax = hepeup_.NUP;
    for(size_t i = 2; i < iMax; ++i) {
       if( hepeup_.ISTUP[i] != 1 ) continue;
       int idabs = abs( hepeup_.IDUP[i] );
       if( idabs != 21 && (idabs<1 || idabs>6) ) continue;
       double ptPart = sqrt( pow(hepeup_.PUP[i][0],2) + pow(hepeup_.PUP[i][1],2) );
       //std::cout << ">>>>>>>> Pt Parton: " << ptPart << std::endl;
       htEvent += ptPart;
    } // iMax

    *gHT=htEvent;
  } //product
  
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
  iEvent.put( gHT,          Prefix + "PartonHT" + Suffix );
  iEvent.put( cteq,         Prefix + "cteq66" + Suffix);
  iEvent.put( nnpdf,        Prefix + "NNPDF10" + Suffix);
  iEvent.put( mrst,         Prefix + "MRST2006nnlo" + Suffix);

}

template< typename T > void Tuple_Gen<T>::
produceGenJets(edm::Event& iEvent) {
  for(unsigned i=0; i<jetCollections.size(); ++i) {
    std::auto_ptr<std::vector<LorentzVector> > p4(new std::vector<LorentzVector>());
    edm::Handle<edm::View<reco::GenJet> > genjets;
    iEvent.getByLabel(jetCollections[i], genjets);
    if(genjets.isValid()) 
      for(edm::View<reco::GenJet>::const_iterator it(genjets->begin()), end(genjets->end()); it!=end; ++it) {
	if (it->pt() >= GenJetPtCut) p4->push_back(it->p4());
      }
    iEvent.put(p4, Prefix + jetCollections[i].label() + "P4" + Suffix);
  }
}

#endif
