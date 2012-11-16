#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "FWCore/Framework/interface/Event.h"

Tuple_Event::Tuple_Event(const edm::ParameterSet& cfg)
  : vertexTag(cfg.getParameter<edm::InputTag>("vertexTag"))
{
  produces <bool>         ( "isRealData"  );
  produces <unsigned int> ( "run"   );
  produces <unsigned int> ( "event" );
  produces <unsigned int> ( "lumiSection" );
  produces <unsigned int> ( "bunch" );
  produces <unsigned int> ( "orbit" );
  produces <double>       ( "time" );
  produces <unsigned>     ( "nVertex" );
}

void Tuple_Event::
produce(edm::Event& evt, const edm::EventSetup& iSetup) {
  edm::Handle<edm::View<reco::Vertex> > vertices;  evt.getByLabel(vertexTag, vertices);
  
  double 
    sec(evt.time().value() >> 32),
    usec(0xFFFFFFFF & evt.time().value()),
    conv(1e6);

  evt.put( std::auto_ptr<bool >   ( new bool( evt.isRealData()         ) ), "isRealData");
  evt.put( std::auto_ptr<unsigned>( new unsigned(evt.id().run()        ) ), "run" );
  evt.put( std::auto_ptr<unsigned>( new unsigned(evt.id().event()      ) ), "event");
  evt.put( std::auto_ptr<unsigned>( new unsigned(evt.luminosityBlock() ) ), "lumiSection");
  evt.put( std::auto_ptr<unsigned>( new unsigned(evt.bunchCrossing()   ) ), "bunch");
  evt.put( std::auto_ptr<unsigned>( new unsigned(evt.orbitNumber()     ) ), "orbit");
  evt.put( std::auto_ptr<unsigned>( new unsigned( vertices.isValid() ? vertices->size() : 0 ) ), "nVertex");
  evt.put( std::auto_ptr<double > ( new double(sec+usec/conv)), "time");
}
