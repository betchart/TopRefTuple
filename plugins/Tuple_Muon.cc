#include "TopQuarkAnalysis/TopRefTuple/interface/Tuple_Muon.h"
#include "TopQuarkAnalysis/TopRefTuple/interface/fTypes.h"

#include <boost/foreach.hpp>

#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"


Tuple_Muon::
Tuple_Muon(const edm::ParameterSet& conf) 
  : muonTag( conf.getParameter<edm::InputTag>("muonTag") ),
    vertexTag( conf.getParameter<edm::InputTag>("vertexTag") ),
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

  produces <std::vector<bool> > ( prefix + "IsPFMuon" );
  produces <std::vector<bool> > ( prefix + "IsGlobalMuon" );
  produces <std::vector<bool> > ( prefix + "IsTrackerMuon" );
  produces <std::vector<bool> > ( prefix + "IsStandAloneMuon" );
  produces <std::vector<bool> > ( prefix + "IsGoodMuon" );

  produces <std::vector<float> > ( prefix + "NormChi2" );
  produces <std::vector<unsigned> > ( prefix + "TrackingLayers" );
  produces <std::vector<unsigned> > ( prefix + "ValidMuonHits" );
  produces <std::vector<unsigned> > ( prefix + "ValidPixelHits" );
  produces <std::vector<unsigned> > ( prefix + "MatchedStations" );

  produces <std::vector<float> > ( prefix + "Dxy");
  produces <std::vector<float> > ( prefix + "Db");
  produces <std::vector<float> > ( prefix + "Dz");
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

  std::auto_ptr<std::vector<bool> > isGlobal (new std::vector<bool>() );
  std::auto_ptr<std::vector<bool> > isTracker (new std::vector<bool>() );
  std::auto_ptr<std::vector<bool> > isPF (new std::vector<bool>() );
  std::auto_ptr<std::vector<bool> > isStandAlone (new std::vector<bool>() );
  std::auto_ptr<std::vector<bool> > isGood (new std::vector<bool>() );

  std::auto_ptr<std::vector<float> > normChi2 ( new std::vector<float>() );
  std::auto_ptr<std::vector<unsigned> > trackLayers ( new std::vector<unsigned>() );
  std::auto_ptr<std::vector<unsigned> > validMuonHits ( new std::vector<unsigned>() );
  std::auto_ptr<std::vector<unsigned> > validPixelHits ( new std::vector<unsigned>() );
  std::auto_ptr<std::vector<unsigned> > matchedStations ( new std::vector<unsigned>() );

  std::auto_ptr<std::vector<float> > dxy ( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > dz ( new std::vector<float>() );
  std::auto_ptr<std::vector<float> > dB ( new std::vector<float>() );

  typedef edm::View<pat::Muon> mus_t;
  edm::Handle<mus_t> muons;         edm::Handle<reco::VertexCollection> vertices;
  event.getByLabel(muonTag,muons);  event.getByLabel(vertexTag, vertices);

  const reco::Vertex* vertex = (vertices.isValid() && vertices->size() ) ? &vertices->at(0) : 0;

  if( muons.isValid() ) {
    for(mus_t::const_iterator mu=muons->begin(); mu!=muons->end(); mu++) {
      p4->push_back(fTypes::dPolarLorentzV(mu->pt(), mu->eta(), mu->phi(), mu->mass()));
      charge->push_back(mu->charge());
      chIso->push_back( mu->chargedHadronIso() );
      nhIso->push_back( mu->neutralHadronIso() );
      phIso->push_back( mu->photonIso() );
      puIso->push_back( mu->puChargedHadronIso() );
      relIso->push_back( (chIso->back() + nhIso->back() + phIso->back() -0.5*puIso->back() ) / mu->pt() );
      
      isGlobal->push_back(mu->isGlobalMuon());
      isTracker->push_back(mu->isTrackerMuon());
      isPF->push_back(mu->isPFMuon());
      isStandAlone->push_back(mu->isStandAloneMuon());
      isGood->push_back(mu->isGood("TMOneStationTight"));

      normChi2->push_back(      isGlobal->back() ? mu->globalTrack()->normalizedChi2() : -9999.9 );
      validMuonHits->push_back( isGlobal->back() ? mu->globalTrack()->hitPattern().numberOfValidMuonHits() : 0 );
      trackLayers->push_back(  isTracker->back() ? mu->track()->hitPattern().trackerLayersWithMeasurement() : 0 );
      validPixelHits->push_back( mu->innerTrack().isNonnull() ? mu->innerTrack()->hitPattern().numberOfValidPixelHits() : 0 );
      matchedStations->push_back( mu->numberOfMatchedStations() );

      dxy->push_back( isGlobal->back() && vertex ? fabs(mu->muonBestTrack()->dxy(vertex->position())) : -9999.9 );
      dz->push_back( isGlobal->back() && vertex ? fabs(mu->muonBestTrack()->dz(vertex->position())) : -9999.9 );
      dB->push_back( mu->dB() );
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

  event.put( isPF, prefix + "IsPFMuon" );
  event.put( isGlobal, prefix + "IsGlobalMuon" );
  event.put( isTracker, prefix + "IsTrackerMuon" );
  event.put( isStandAlone, prefix + "IsStandAloneMuon" );
  event.put( isGood, prefix + "IsGoodMuon" );

  event.put( normChi2, prefix + "NormChi2" );
  event.put( trackLayers, prefix + "TrackingLayers" );
  event.put( validMuonHits, prefix + "ValidMuonHits" );
  event.put( validPixelHits, prefix + "ValidPixelHits" );
  event.put( matchedStations, prefix + "MatchedStations" );
  
  event.put( dxy, prefix + "Dxy" );
  event.put( dB, prefix + "Db" );
  event.put( dz, prefix + "Dz" );

}
