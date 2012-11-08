#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_Electron.h"

#include <boost/foreach.hpp>

#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"



Tuple_Electron::
Tuple_Electron(const edm::ParameterSet& conf) 
  : electronTag( conf.getParameter<edm::InputTag>("electronTag") ),
    vertexTag( conf.getParameter<edm::InputTag>("vertexTag") ),
    electronIDs( conf.getParameter<std::vector<std::string> >("electronIDs") ),
    prefix( conf.getParameter<std::string>("prefix") )
{
  typedef reco::Candidate::LorentzVector LorentzVector;
  produces <bool> (  prefix + "HandleValid");
  produces <std::vector<LorentzVector> > ( prefix + "P4" );
  produces <std::vector<int> > (  prefix + "Charge");

  produces <std::vector<double> > ( prefix + "RelIso" );
  produces <std::vector<double> > ( prefix + "ChIso" );
  produces <std::vector<double> > ( prefix + "NhIso" );
  produces <std::vector<double> > ( prefix + "PhIso" );
  produces <std::vector<double> > ( prefix + "PuIso" );

  produces <std::vector<double> > ( prefix + "SuperClusterEta");
  produces <std::vector<bool> >   ( prefix + "PassConversionVeto");
  produces <std::vector<double> > ( prefix + "Dxy" );
  produces <std::vector<unsigned> > (prefix + "GsfTrackInnerHits");
  
  BOOST_FOREACH(const std::string& idName, electronIDs ) {
    produces <std::vector<double> > ( prefix + idName );
  }
}

void Tuple_Electron::
produce(edm::Event &event, const edm::EventSetup&) {
  typedef reco::Candidate::LorentzVector LorentzVector;
  std::auto_ptr<std::vector<LorentzVector> > p4 ( new std::vector<LorentzVector>() );
  std::auto_ptr<std::vector<int> >           charge ( new std::vector<int>() );

  std::auto_ptr<std::vector<double> > relIso( new std::vector<double>() );
  std::auto_ptr<std::vector<double> > chIso ( new std::vector<double>() );
  std::auto_ptr<std::vector<double> > nhIso ( new std::vector<double>() );
  std::auto_ptr<std::vector<double> > phIso ( new std::vector<double>() );
  std::auto_ptr<std::vector<double> > puIso ( new std::vector<double>() );

  std::auto_ptr<std::vector<double> > scEta ( new std::vector<double>() );
  std::auto_ptr<std::vector<bool> >   passCV ( new std::vector<bool>() );
  std::auto_ptr<std::vector<double> > dxy ( new std::vector<double>() );
  std::auto_ptr<std::vector<unsigned> > nHits ( new std::vector<unsigned>() );

  std::map<std::string, std::vector<double>* > idMaps;
  BOOST_FOREACH(const std::string& idName, electronIDs ) {
    idMaps[idName] = new std::vector<double>();
  }
 
  typedef edm::View<pat::Electron> els_t;
  edm::Handle<els_t> electrons;             edm::Handle<reco::VertexCollection> vertices;
  event.getByLabel(electronTag,electrons);  event.getByLabel(vertexTag, vertices);

  const reco::Vertex* vertex = (vertices.isValid() && vertices->size() ) ? &vertices->at(0) : 0;

  if( electrons.isValid() ) {
    for(els_t::const_iterator el=electrons->begin(); el!=electrons->end(); el++) {
      p4->push_back(el->p4());
      charge->push_back(el->charge());
      chIso->push_back( el->chargedHadronIso() );
      nhIso->push_back( el->neutralHadronIso() );
      phIso->push_back( el->photonIso() );
      puIso->push_back( el->puChargedHadronIso() );
      relIso->push_back( (chIso->back() + std::max(0., nhIso->back()+phIso->back()-0.5*puIso->back()) ) / el->pt() );
      
      scEta->push_back( el->superCluster()->eta() );
      passCV->push_back( el->passConversionVeto() );
      dxy->push_back( vertex? el->gsfTrack()->dxy(vertex->position()) : -9999.9 );
      nHits->push_back( el->gsfTrack()->trackerExpectedHitsInner().numberOfHits() );
      
      BOOST_FOREACH(const std::string& idName, electronIDs) {
      	idMaps[idName]->push_back( el->electronID(idName) );
      }
    }
  }

  event.put( std::auto_ptr<bool> ( new bool(electrons.isValid() ) ), prefix + "HandleValid" );
  event.put( p4, prefix+"P4" );
  event.put( charge, prefix+"Charge");
  event.put( relIso, prefix+"RelIso");
  event.put( chIso, prefix+"ChIso");
  event.put( nhIso, prefix+"NhIso");
  event.put( phIso, prefix+"PhIso");
  event.put( puIso, prefix+"PuIso");
  event.put( scEta, prefix+"SuperClusterEta");
  event.put( passCV,prefix+"PassConversionVeto");
  event.put( dxy, prefix+"Dxy");
  event.put( nHits, prefix+"GsfTrackInnerHits");
  BOOST_FOREACH(const std::string& idName, electronIDs) {
    event.put( std::auto_ptr<std::vector<double> >(idMaps[idName]), prefix + idName);
  }
}
