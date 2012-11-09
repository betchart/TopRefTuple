#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_Muon.h"
#include "TopQuarkAnalysis/TopRefTuple/interface/fTypes.h"

#include <boost/foreach.hpp>

#include "DataFormats/PatCandidates/interface/Muon.h"
//#include "DataFormats/VertexReco/interface/VertexFwd.h"
//#include "DataFormats/VertexReco/interface/Vertex.h"


Tuple_Muon::
Tuple_Muon(const edm::ParameterSet& conf) 
  : muonTag( conf.getParameter<edm::InputTag>("muonTag") ),
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

}

void Tuple_Muon::
produce(edm::Event &event, const edm::EventSetup&) {
  std::auto_ptr<std::vector<fTypes::dPolarLorentzV> > p4 ( new std::vector<fTypes::dPolarLorentzV>() );
  std::auto_ptr<std::vector<int> >           charge ( new std::vector<int>() );

  std::auto_ptr<std::vector<float> > relIso( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > chIso ( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > nhIso ( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > phIso ( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > puIso ( new std::vector<float>() );

  typedef edm::View<pat::Muon> mus_t;
  edm::Handle<mus_t> muons;
  event.getByLabel(muonTag,muons);

  if( muons.isValid() ) {
    for(mus_t::const_iterator mu=muons->begin(); mu!=muons->end(); mu++) {
      p4->push_back(fTypes::dPolarLorentzV(mu->pt(), mu->eta(), mu->phi(), mu->mass()));
      charge->push_back(mu->charge());
      chIso->push_back( mu->chargedHadronIso() );
      nhIso->push_back( mu->neutralHadronIso() );
      phIso->push_back( mu->photonIso() );
      puIso->push_back( mu->puChargedHadronIso() );
      relIso->push_back( (chIso->back() + std::max(0., nhIso->back()+phIso->back()-0.5*puIso->back()) ) / mu->pt() );
      
    }
  }

  event.put( std::auto_ptr<bool> ( new bool(muons.isValid() ) ), prefix + "HandleValid" );
  event.put( p4, prefix+"P4" );
  event.put( charge, prefix+"Charge");
  event.put( relIso, prefix+"RelIso");
  event.put( chIso, prefix+"ChIso");
  event.put( nhIso, prefix+"NhIso");
  event.put( phIso, prefix+"PhIso");
  event.put( puIso, prefix+"PuIso");
}
