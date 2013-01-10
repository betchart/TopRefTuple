#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_Electron.h"
#include "TopQuarkAnalysis/TopRefTuple/interface/fTypes.h"

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
  produces <bool> (  prefix + "HandleValid");
  produces <std::vector<fTypes::dPolarLorentzV> > ( prefix + "P4" );
  produces <std::vector<int> > (  prefix + "Charge");

  produces <std::vector<float> > ( prefix + "RelIso" );
  produces <std::vector<float> > ( prefix + "ChIso" );
  produces <std::vector<float> > ( prefix + "NhIso" );
  produces <std::vector<float> > ( prefix + "PhIso" );
  produces <std::vector<float> > ( prefix + "PuIso" );
  produces <std::vector<float> > ( prefix + "EAIso" );

  produces <std::vector<float> > ( prefix + "SuperClusterEta");
  produces <std::vector<bool> >   ( prefix + "PassConversionVeto");
  produces <std::vector<float> > ( prefix + "Dxy" );
  produces <std::vector<unsigned> > (prefix + "GsfTrackInnerHits");
  
  BOOST_FOREACH(const std::string& idName, electronIDs ) {
    produces <std::vector<float> > ( prefix + idName );
  }
}

void Tuple_Electron::
produce(edm::Event &event, const edm::EventSetup&) {
  std::auto_ptr<std::vector<fTypes::dPolarLorentzV> > p4 ( new std::vector<fTypes::dPolarLorentzV>() );
  std::auto_ptr<std::vector<int> >           charge ( new std::vector<int>() );

  std::auto_ptr<std::vector<float> > relIso( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > chIso ( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > nhIso ( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > phIso ( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > puIso ( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > EAIso ( new std::vector<float>() );

  std::auto_ptr<std::vector<float> > scEta ( new std::vector<float>() );
  std::auto_ptr<std::vector<bool> >  passCV ( new std::vector<bool>() );
  std::auto_ptr<std::vector<float> > dxy ( new std::vector<float>() );
  std::auto_ptr<std::vector<unsigned> > nHits ( new std::vector<unsigned>() );

  std::map<std::string, std::vector<float>* > idMaps;
  BOOST_FOREACH(const std::string& idName, electronIDs ) {
    idMaps[idName] = new std::vector<float>();
  }
 
  typedef edm::View<pat::Electron> els_t;
  edm::Handle<els_t> electrons;             edm::Handle<reco::VertexCollection> vertices;
  event.getByLabel(electronTag,electrons);  event.getByLabel(vertexTag, vertices);

  const reco::Vertex* vertex = (vertices.isValid() && vertices->size() ) ? &vertices->at(0) : 0;

  if( electrons.isValid() ) {
    for(els_t::const_iterator el=electrons->begin(); el!=electrons->end(); el++) {
      p4->push_back(fTypes::dPolarLorentzV(el->pt(), el->eta(), el->phi(), el->mass()));
      charge->push_back(el->charge());
      chIso->push_back( el->chargedHadronIso() );
      nhIso->push_back( el->neutralHadronIso() );
      phIso->push_back( el->photonIso() );
      puIso->push_back( el->puChargedHadronIso() );
      EAIso->push_back( el->userIsolation("User1Iso") );
      relIso->push_back( (chIso->back() + std::max(float(0), nhIso->back()+phIso->back()-EAIso->back()) ) / el->pt() );
      
      scEta->push_back( el->superCluster()->eta() );
      passCV->push_back( el->passConversionVeto() );
      dxy->push_back( vertex? fabs( el->gsfTrack()->dxy(vertex->position()) ) : -9999.9 );
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
  event.put( EAIso, prefix+"EAIso");
  event.put( scEta, prefix+"SuperClusterEta");
  event.put( passCV,prefix+"PassConversionVeto");
  event.put( dxy, prefix+"Dxy");
  event.put( nHits, prefix+"GsfTrackInnerHits");
  BOOST_FOREACH(const std::string& idName, electronIDs) {
    event.put( std::auto_ptr<std::vector<float> >(idMaps[idName]), prefix + idName);
  }
}
