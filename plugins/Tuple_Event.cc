#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_Event.h"
#include "FWCore/Framework/interface/Event.h"

Tuple_Event::Tuple_Event(const edm::ParameterSet& iConfig) {
  produces <bool>         ( "isRealData"  );
  produces <unsigned int> ( "run"   );
  produces <unsigned int> ( "event" );
  produces <unsigned int> ( "lumiSection" );
  produces <unsigned int> ( "bunch" );
  produces <unsigned int> ( "orbit" );
  produces <double>       ( "time" );
}

void Tuple_Event::
produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {

  std::auto_ptr<bool >     isRealData ( new bool(        iEvent.isRealData()      ) );
  std::auto_ptr<unsigned int >  run   ( new unsigned int(iEvent.id().run()        ) );
  std::auto_ptr<unsigned int >  event ( new unsigned int(iEvent.id().event()      ) );
  std::auto_ptr<unsigned int >  ls    ( new unsigned int(iEvent.luminosityBlock() ) );
  std::auto_ptr<unsigned int >  bunch ( new unsigned int(iEvent.bunchCrossing()   ) );
  std::auto_ptr<unsigned int >  orbit ( new unsigned int(iEvent.orbitNumber()     ) );
  
  double sec  = iEvent.time().value() >> 32 ;
  double usec = 0xFFFFFFFF & iEvent.time().value();
  double conv = 1e6;

  std::auto_ptr<double >        time  	      ( new double(sec+usec/conv));

  iEvent.put( isRealData, "isRealData" );
  iEvent.put( run,   "run"   );
  iEvent.put( event, "event" );
  iEvent.put( ls   , "lumiSection" );
  iEvent.put( bunch, "bunch" );
  iEvent.put( orbit, "orbit" );
  iEvent.put( time,  "time"  );
}
