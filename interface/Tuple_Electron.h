#ifndef TUPLE_ELECTRON
#define TUPLE_ELECTRON

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/BeamSpot/interface/BeamSpot.h"
//#include "SUSYBSMAnalysis/SusyCAF/interface/Tuple_functions.h"
#include <string>

// Conversion variables
#include "RecoEgamma/EgammaTools/interface/ConversionTools.h"
#include "RecoEgamma/EgammaTools/interface/ConversionFinder.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"

// ID computation
#include "DataFormats/RecoCandidate/interface/IsoDeposit.h"
#include "EGamma/EGammaAnalysisTools/interface/EGammaCutBasedEleId.h"


template< typename T >
class Tuple_Electron : public edm::EDProducer {
public:
  explicit Tuple_Electron(const edm::ParameterSet&);
private:
  void initTemplate();
  void initRECO();
  void initPAT();
  std::string underscoreless(std::string);
  void produce(edm::Event &, const edm::EventSetup & );
  void produceTemplate(edm::Event &, const edm::EventSetup &, edm::Handle<std::vector<T> > &);
  void produceRECO(edm::Event &, const edm::EventSetup &, edm::Handle<std::vector<T> > &);
  void producePAT(edm::Event &, const edm::EventSetup &, edm::Handle<std::vector<T> > &);
  bool isInCollection(const T&, const std::vector<T>&);
  void pogIdReco(edm::Event& iEvent, const edm::EventSetup& iSetup, reco::GsfElectronRef ele,
		 bool& veto, bool& loose, bool& medium, bool& tight);
  void pogIdPat(edm::Event& iEvent, const edm::EventSetup& iSetup, const T& ele,
		bool& veto, bool& loose, bool& medium, bool& tight);
  
  typedef reco::Candidate::LorentzVector LorentzVector;
  
  // input tags
  const edm::InputTag inputTag,selectedTag;
  const std::string Prefix,Suffix;
  const bool StoreConversionInfo,IdFromReco;
  const std::vector<std::string> IdFlagsOldStyle;

  const edm::InputTag conversionsInputTag_, beamSpotInputTag_, rhoIsoInputTag_, primaryVertexInputTag_;
  const std::vector<edm::InputTag> isoValInputTags_;
};

template< typename T >
Tuple_Electron<T>::Tuple_Electron(const edm::ParameterSet& iConfig) :
 inputTag(iConfig.getParameter<edm::InputTag>("InputTag")),
 selectedTag(iConfig.getParameter<edm::InputTag>("SelectedElectrons")),
 Prefix(iConfig.getParameter<std::string>("Prefix")),
 Suffix(iConfig.getParameter<std::string>("Suffix")),
 StoreConversionInfo(iConfig.getParameter<bool>("StoreConversionInfo")),
 IdFromReco(iConfig.getParameter<bool>("IdFromReco")),
 IdFlagsOldStyle(iConfig.getParameter<std::vector< std::string> >("IdFlagsOldStyle")),

 conversionsInputTag_(iConfig.getParameter<edm::InputTag>("conversionsInputTag")),
 beamSpotInputTag_(iConfig.getParameter<edm::InputTag>("beamSpotInputTag")),
 rhoIsoInputTag_(iConfig.getParameter<edm::InputTag>("rhoIsoInputTag")),
 primaryVertexInputTag_(iConfig.getParameter<edm::InputTag>("primaryVertexInputTag")),
 isoValInputTags_(iConfig.getParameter<std::vector<edm::InputTag> >("isoValInputTags"))
{
 initTemplate();
}


template< typename T >
void Tuple_Electron<T>::initRECO()
{
 produces <bool> (  Prefix + "HandleValid" + Suffix);
 produces <std::vector<reco::Candidate::LorentzVector> > ( Prefix + "P4" + Suffix );
 produces <std::vector<int> > (  Prefix + "Charge" + Suffix);
 produces <std::vector<double> > (  Prefix + "GsfTracknormalizedChi2" + Suffix);
 produces <std::vector<unsigned> > (  Prefix + "GsfTracknumberOfValidHits" + Suffix);

 produces <std::vector<int> > (Prefix + "Selected" + Suffix );
 produces <std::vector<float> > (  Prefix + "GsfTrackChargeMode" + Suffix);
 produces <std::vector<float> > (  Prefix + "GsfTrackPtMode" + Suffix);
 produces <std::vector<float> > (  Prefix + "GsfTrackQoverPErrorMode" + Suffix);
 produces <std::vector<float> > (  Prefix + "GsfTrackCharge" + Suffix);
 produces <std::vector<float> > (  Prefix + "GsfTrackPt" + Suffix);
 produces <std::vector<float> > (  Prefix + "GsfTrackQoverPError" + Suffix);
 produces <std::vector<float> > (  Prefix + "GsfTrackLostHits" + Suffix);
 produces <std::vector<int> > ( Prefix + "HasValidHitInFirstPixelBarrel" + Suffix);
 produces <std::vector<int> > ( Prefix +  "GsfTrackTrackerExpectedHitsInner" + Suffix);
 produces <std::vector<double> > (  Prefix + "GsfTrackDxy" + Suffix);
 produces <std::vector<double> > (  Prefix + "GsfTrackDz" + Suffix);
 produces <std::vector<double> > (  Prefix + "GsfTrackDxyBS" + Suffix);
 produces <std::vector<double> > (  Prefix + "GsfTrackDzBS" + Suffix);
 produces <std::vector<double> > (  Prefix + "GsfTrackVertexz" + Suffix);
 produces <std::vector<double> > (  Prefix + "GsfTrackDxyError" + Suffix);
 produces <std::vector<double> > (  Prefix + "GsfTrackDzError" + Suffix);

 produces <std::vector<int> > (  Prefix + "IsEB" + Suffix);
 produces <std::vector<int> > (  Prefix + "IsEE" + Suffix);
 produces <std::vector<float> > (  Prefix + "E1x5" + Suffix);
 produces <std::vector<float> > (  Prefix + "E5x5" + Suffix);
 produces <std::vector<float> > (  Prefix + "E2x5Max" + Suffix);
 produces <std::vector<float> > (  Prefix + "Fbrem" + Suffix);
 produces <std::vector<float> > (  Prefix + "HcalOverEcal" + Suffix);
 produces <std::vector<float> > (  Prefix + "HcalDepth1OverEcal" + Suffix);
 produces <std::vector<float> > (  Prefix + "HcalDepth2OverEcal" + Suffix);
 produces <std::vector<float> > (  Prefix + "EEleClusterOverPout" + Suffix);
 produces <std::vector<float> > (  Prefix + "ESeedClusterOverPout" + Suffix);
 produces <std::vector<float> > (  Prefix + "ESeedClusterOverP" + Suffix);
 produces <std::vector<float> > (  Prefix + "ESuperClusterOverP" + Suffix);
 produces <std::vector<float> > (  Prefix + "ESuperClusterEta" + Suffix);
 produces <std::vector<float> > (  Prefix + "ESuperClusterPhi" + Suffix); 
 produces <std::vector<float> > (  Prefix + "DeltaPhiSuperClusterTrackAtVtx" + Suffix);
 produces <std::vector<float> > (  Prefix + "DeltaEtaSuperClusterTrackAtVtx" + Suffix);
 produces <std::vector<float> > (  Prefix + "DeltaPhiSeedClusterTrackAtCalo" + Suffix);
 produces <std::vector<float> > (  Prefix + "DeltaEtaSeedClusterTrackAtCalo" + Suffix);
 produces <std::vector<float> > (  Prefix + "DeltaEtaEleClusterTrackAtCalo" + Suffix);
 produces <std::vector<float> > (  Prefix + "DeltaPhiEleClusterTrackAtCalo" + Suffix);
 produces <std::vector<float> > (  Prefix + "SigmaEtaEta" + Suffix);
 produces <std::vector<float> > (  Prefix + "SigmaIetaIeta" + Suffix);
 produces <std::vector<float> > (  Prefix + "Classification" + Suffix);
 produces <std::vector<float> > (  Prefix + "Mva" + Suffix);
 produces <std::vector<float> > (  Prefix + "Dr03TkSumPt" + Suffix);
 produces <std::vector<float> > (  Prefix + "Dr03EcalRecHitSumEt" + Suffix);
 produces <std::vector<float> > (  Prefix + "Dr03HcalTowerSumEt" + Suffix);
 produces <std::vector<float> > (  Prefix + "Dr03HcalDepth1TowerSumEt" + Suffix);
 produces <std::vector<float> > (  Prefix + "Dr03HcalDepth2TowerSumEt" + Suffix);
 produces <std::vector<float> > (  Prefix + "CaloEnergy" + Suffix);
 produces <std::vector<float> > (  Prefix + "EcalEnergy" + Suffix);
 produces <std::vector<float> > (  Prefix + "EcalEnergyError" + Suffix);
 produces <std::vector<float> > (  Prefix + "ElectronMomentumError" + Suffix);
 produces <std::vector<float> > (  Prefix + "NumberOfTracks" + Suffix);
 produces <std::vector<int> >   (  Prefix + "NumberOfBrems" + Suffix);
 produces <std::vector<float> > (  Prefix + "ShFracInnerHits" + Suffix);
 produces <std::vector<math::XYZPoint> > (  Prefix + "Vertex" + Suffix);
 produces <std::vector<double> > (  Prefix + "VertexChi2" + Suffix);
 produces <std::vector<double> > (  Prefix + "VertexNdof" + Suffix);
 produces <bool> ( Prefix + "ConversionInfoStored" + Suffix);
 if(StoreConversionInfo){
   produces <std::vector<double> > (  Prefix + "ConversionDCot" + Suffix );
   produces <std::vector<double> > (  Prefix + "ConversionDist" + Suffix );
   produces <std::vector<int> > (  Prefix + "ConversionMissingHits" + Suffix );
   produces <std::vector<int> > ( Prefix + "ConversionPartnerCtfTrackTrackerExpectedHitsInner" + Suffix);
   produces <std::vector<int> > ( Prefix + "ConversionPartnerGsfTrackTrackerExpectedHitsInner" + Suffix);
 }
 produces <std::vector<float> > ( Prefix + "KfTrackCharge" + Suffix );
 produces <std::vector<int> > ( Prefix + "ScPixCharge" + Suffix );
 produces <std::vector<int> > ( Prefix + "ClosestCtfTrackCharge" + Suffix );

 if(IdFromReco){
   produces <std::vector<int> >   (Prefix + "IdVeto"          + Suffix);
   produces <std::vector<int> >   (Prefix + "IdLoose"         + Suffix);
   produces <std::vector<int> >   (Prefix + "IdMedium"        + Suffix);
   produces <std::vector<int> >   (Prefix + "IdTight"         + Suffix);
 }
}

template< typename T >
std::string Tuple_Electron<T>::underscoreless(std::string s) {
  while(s.find("_")!=std::string::npos) s.replace(s.find("_"), 1, "");
  return s;
}

// extra information stored for PAT data
template< typename T >
void Tuple_Electron<T>::initPAT()
{
 produces <std::vector<float> > (Prefix + "EcalIso"             + Suffix);
 produces <std::vector<float> > (Prefix + "HcalIso"             + Suffix);
 produces <std::vector<float> > (Prefix + "TrackIso"            + Suffix);
 produces <std::vector<float> > (Prefix + "EcalIsoDep"          + Suffix);
 produces <std::vector<float> > (Prefix + "HcalIsoDep"          + Suffix);
 produces <std::vector<int> >   (Prefix + "ProducedFromPF"      + Suffix);
 produces <std::vector<float> > (Prefix + "ParticleIso"         + Suffix);
 produces <std::vector<float> > (Prefix + "ChargedHadronIso"    + Suffix);
 produces <std::vector<float> > (Prefix + "NeutralHadronIso"    + Suffix);
 produces <std::vector<float> > (Prefix + "PhotonIso"           + Suffix);
 produces <std::vector<float> > (Prefix + "ChargedHadronIsoRA4" + Suffix);
 produces <std::vector<float> > (Prefix + "NeutralHadronIsoRA4" + Suffix);
 produces <std::vector<float> > (Prefix + "PhotonIsoRA4"        + Suffix);
 produces <std::vector<int> > (  Prefix + "ConversionRejection" + Suffix);
 if(!IdFromReco) {
   produces <std::vector<int> >   (Prefix + "IdVeto"          + Suffix);
   produces <std::vector<int> >   (Prefix + "IdLoose"         + Suffix);
   produces <std::vector<int> >   (Prefix + "IdMedium"        + Suffix);
   produces <std::vector<int> >   (Prefix + "IdTight"         + Suffix);
 }

 for(std::vector<std::string>::const_iterator name=IdFlagsOldStyle.begin(); name!=IdFlagsOldStyle.end(); ++name) {
   produces <std::vector<int> > (Prefix + underscoreless(*name) + Suffix);
 }

}

template< typename T >
void Tuple_Electron<T>::
produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
 edm::Handle<std::vector<T> > collection;
 iEvent.getByLabel(inputTag,collection);

 produceTemplate(iEvent, iSetup, collection);
}

template< typename T >
void Tuple_Electron<T>::
produceRECO(edm::Event& iEvent, const edm::EventSetup& iSetup, edm::Handle<std::vector<T> >& collection) {

 std::auto_ptr<bool> isHandleValid ( new bool(collection.isValid()) );
 std::auto_ptr<std::vector<reco::Candidate::LorentzVector> > p4 ( new std::vector<reco::Candidate::LorentzVector>() );
 std::auto_ptr<std::vector<int> >  charge   ( new std::vector<int>()  ) ;
 std::auto_ptr<std::vector<double> >  gsfTrack_normalizedChi2   ( new std::vector<double>()  ) ;
 std::auto_ptr<std::vector<unsigned> >  gsfTrack_numberOfValidHits   ( new std::vector<unsigned>()  ) ;

 std::auto_ptr<std::vector<int> >  selected     ( new std::vector<int>()  ) ;

 std::auto_ptr<std::vector<double> >  gsfTrack_dxy     ( new std::vector<double>()  ) ;
 std::auto_ptr<std::vector<double> >  gsfTrack_dz      ( new std::vector<double>()  ) ;
 std::auto_ptr<std::vector<double> >  gsfTrack_dxyBS   ( new std::vector<double>()  ) ;
 std::auto_ptr<std::vector<double> >  gsfTrack_dzBS    ( new std::vector<double>()  ) ;
 std::auto_ptr<std::vector<double> >  gsfTrack_vertexz ( new std::vector<double>()  ) ;
 std::auto_ptr<std::vector<double> >  gsfTrack_dxyError( new std::vector<double>()  ) ;
 std::auto_ptr<std::vector<double> >  gsfTrack_dzError ( new std::vector<double>()  ) ;

 std::auto_ptr<std::vector<int> > isEB (new std::vector<int>() );
 std::auto_ptr<std::vector<int> > isEE (new std::vector<int>() );
 std::auto_ptr<std::vector<float> > gsfTrkChargeMode (new std::vector<float>() );
 std::auto_ptr<std::vector<float> > gsfTrkPtMode (new std::vector<float>() );
 std::auto_ptr<std::vector<float> > gsfTrkQoverPErrMode (new std::vector<float>() );
 std::auto_ptr<std::vector<float> > gsfTrkCharge (new std::vector<float>() );
 std::auto_ptr<std::vector<float> > gsfTrkPt (new std::vector<float>() );
 std::auto_ptr<std::vector<float> > gsfTrkQoverPErr (new std::vector<float>() );
 std::auto_ptr<std::vector<float> > gsfTrkLostHits (new std::vector<float>() );
 std::auto_ptr<std::vector<int> > hasValHitinFirstPixBarrel (new std::vector<int>() );
 std::auto_ptr<std::vector<int> > gsfTrkTrackerExpectedHitsInner (new std::vector<int>() );
 std::auto_ptr<std::vector<float> >  e1x5   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  e5x5   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  e2x5Max   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  fbrem   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  hcalOverEcal   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  hcalDepth1OverEcal   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  hcalDepth2OverEcal   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  eEleClusterOverPout   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  eSeedClusterOverPout   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  eSeedClusterOverP   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  eSuperClusterOverP   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  eSuperClusterEta  ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  eSuperClusterPhi   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  deltaPhiSuperClusterTrackAtVtx   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  deltaEtaSuperClusterTrackAtVtx   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  deltaPhiSeedClusterTrackAtCalo   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  deltaEtaSeedClusterTrackAtCalo   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  deltaEtaEleClusterTrackAtCalo   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  deltaPhiEleClusterTrackAtCalo   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  sigmaEtaEta   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  sigmaIetaIeta   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  classification   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  mva   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  dr03TkSumPt   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  dr03EcalRecHitSumEt   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  dr03HcalTowerSumEt   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  dr03HcalDepth1TowerSumEt   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  dr03HcalDepth2TowerSumEt   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  caloEnergy   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  ecalEnergy   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  ecalEnergyError   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  electronMomentumError   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<float> >  numberOfTracks   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<int> >    numberOfBrems   ( new std::vector<int>()  ) ;
 std::auto_ptr<std::vector<float> >  shFracInnerHits   ( new std::vector<float>()  ) ;
 std::auto_ptr<std::vector<math::XYZPoint> >  vertex   ( new std::vector<math::XYZPoint>()  ) ;
 std::auto_ptr<std::vector<double> >  vertexChi2   ( new std::vector<double>()  ) ;
 std::auto_ptr<std::vector<double> >  vertexNdof   ( new std::vector<double>()  ) ;
 std::auto_ptr<bool> conversionInfoStored ( new bool() );
 std::auto_ptr<std::vector<double> > dcot ( new std::vector<double>() ) ;
 std::auto_ptr<std::vector<double> > dist ( new std::vector<double>() ) ;
 std::auto_ptr<std::vector<int> > missingHits ( new std::vector<int>() ) ;
 std::auto_ptr<std::vector<int> > conversionPartnerCtfTrackTrackerExpectedHitsInner ( new std::vector<int>() ) ;
 std::auto_ptr<std::vector<int> > conversionPartnerGsfTrackTrackerExpectedHitsInner ( new std::vector<int>() ) ;
 std::auto_ptr<std::vector<float> > kfcharge ( new std::vector<float>() );
 std::auto_ptr<std::vector<int> > scPixCharge ( new std::vector<int>() );
 std::auto_ptr<std::vector<int> > closestCtfTrackCharge( new std::vector<int>() );

 std::auto_ptr<std::vector<int> > idVeto(   new std::vector<int>() );
 std::auto_ptr<std::vector<int> > idLoose(  new std::vector<int>() );
 std::auto_ptr<std::vector<int> > idMedium( new std::vector<int>() );
 std::auto_ptr<std::vector<int> > idTight(  new std::vector<int>() );

 math::XYZPoint bs = math::XYZPoint(0.,0.,0.);
 math::XYZPoint vx = math::XYZPoint(0.,0.,0.);
 edm::Handle<reco::BeamSpot> beamspots;        iEvent.getByLabel(beamSpotInputTag_, beamspots);
 edm::Handle<reco::VertexCollection> vertices; iEvent.getByLabel(primaryVertexInputTag_, vertices);

 if (beamspots.isValid()) bs = beamspots->position();

 const  MagneticField *mField = 0;
 edm::Handle<reco::TrackCollection> ctfTracks;
 edm::Handle<reco::GsfTrackCollection> gsfTracks;
 ConversionFinder cf;
 if(StoreConversionInfo){
   edm::ESHandle<MagneticField> magneticField;
   iSetup.get<IdealMagneticFieldRecord>().get(magneticField);
   mField = magneticField.product();
   iEvent.getByLabel("generalTracks", ctfTracks);
   iEvent.getByLabel("electronGsfTracks", gsfTracks);
 }

  edm::Handle<std::vector<T> > selectedHandle;
  iEvent.getByLabel(selectedTag,selectedHandle);

 if (collection.isValid()){
   for(typename std::vector<T>::const_iterator it = collection->begin(); it!=collection->end(); it++) {

     p4->push_back(it->p4());
     charge->push_back(it->charge());
     gsfTrack_normalizedChi2->push_back(it->gsfTrack()->normalizedChi2());
     gsfTrack_numberOfValidHits->push_back(it->gsfTrack()->numberOfValidHits());

     isEB->push_back(it->isEB());
     isEE->push_back(it->isEE());

     selected->push_back(selectedHandle.isValid() && isInCollection(*it, *selectedHandle) ) ;

     //if(vertices.isValid() && vertices->size()) vx = Tuple_functions::closestDzPrimaryVertexPosition(it->gsfTrack().get(),*vertices);
     gsfTrack_dxy->push_back(it->gsfTrack()->dxy(vx));
     gsfTrack_dz->push_back(it->gsfTrack()->dz(vx));
     gsfTrack_dxyBS->push_back(it->gsfTrack()->dxy(bs));
     gsfTrack_dzBS->push_back(it->gsfTrack()->dz(bs));
     gsfTrack_vertexz->push_back(it->gsfTrack()->vertex().z());
     gsfTrack_dxyError->push_back(it->gsfTrack()->dxyError());
     gsfTrack_dzError->push_back(it->gsfTrack()->dzError());

     gsfTrkChargeMode->push_back(it->gsfTrack()->chargeMode());
     gsfTrkPtMode->push_back(it->gsfTrack()->ptMode());
     gsfTrkQoverPErrMode->push_back(it->gsfTrack()->qoverpModeError());
     gsfTrkCharge->push_back(it->gsfTrack()->charge());
     gsfTrkPt->push_back(it->gsfTrack()->pt());
     gsfTrkQoverPErr->push_back(it->gsfTrack()->qoverpError());
     gsfTrkLostHits->push_back(it->gsfTrack()->lost());
     hasValHitinFirstPixBarrel->push_back(it->gsfTrack()->hitPattern().hasValidHitInFirstPixelBarrel());
     gsfTrkTrackerExpectedHitsInner->push_back(it->gsfTrack()->trackerExpectedHitsInner().numberOfHits());
     e1x5->push_back(it->e1x5());
     e5x5->push_back(it->e5x5());
     e2x5Max->push_back(it->e2x5Max());
     fbrem->push_back(it->fbrem());
     hcalOverEcal->push_back(it->hcalOverEcal());
     hcalDepth1OverEcal->push_back(it->hcalDepth1OverEcal());
     hcalDepth2OverEcal->push_back(it->hcalDepth2OverEcal());
     eEleClusterOverPout->push_back(it->eEleClusterOverPout());
     eSeedClusterOverPout->push_back(it->eSeedClusterOverPout());
     eSeedClusterOverP->push_back(it->eSeedClusterOverP());
     eSuperClusterOverP->push_back(it->eSuperClusterOverP());
     eSuperClusterEta->push_back(it->superClusterPosition().Eta());
     eSuperClusterPhi->push_back(it->superClusterPosition().Phi());      
     deltaPhiSuperClusterTrackAtVtx->push_back(it->deltaPhiSuperClusterTrackAtVtx());
     deltaEtaSuperClusterTrackAtVtx->push_back(it->deltaEtaSuperClusterTrackAtVtx());
     deltaPhiSeedClusterTrackAtCalo->push_back(it->deltaPhiSeedClusterTrackAtCalo());
     deltaEtaSeedClusterTrackAtCalo->push_back(it->deltaEtaSeedClusterTrackAtCalo());
     deltaEtaEleClusterTrackAtCalo->push_back(it->deltaEtaEleClusterTrackAtCalo());
     deltaPhiEleClusterTrackAtCalo->push_back(it->deltaPhiEleClusterTrackAtCalo());
     sigmaEtaEta->push_back(it->sigmaEtaEta());
     sigmaIetaIeta->push_back(it->sigmaIetaIeta());
     classification->push_back(it->classification());
     mva->push_back(it->mva());
     dr03TkSumPt->push_back(it->dr03TkSumPt());
     dr03EcalRecHitSumEt->push_back(it->dr03EcalRecHitSumEt());
     dr03HcalTowerSumEt->push_back(it->dr03HcalTowerSumEt());
     dr03HcalDepth1TowerSumEt->push_back(it->dr03HcalDepth1TowerSumEt());
     dr03HcalDepth2TowerSumEt->push_back(it->dr03HcalDepth2TowerSumEt());
     caloEnergy->push_back(it->caloEnergy());
     ecalEnergy->push_back(it->ecalEnergy());
     ecalEnergyError->push_back(it->ecalEnergyError());
     electronMomentumError->push_back(it->p4Error(it->candidateP4Kind()));
     numberOfTracks->push_back(it->numberOfTracks());
     numberOfBrems->push_back(it->numberOfBrems());
     shFracInnerHits->push_back(it->shFracInnerHits());
     vertex->push_back(it->vertex());
     vertexChi2->push_back(it->vertexChi2());
     vertexNdof->push_back(it->vertexNdof());

     if(StoreConversionInfo && ctfTracks.isValid() && gsfTracks.isValid()){
	*conversionInfoStored = true;
	
       const math::XYZPoint tpoint = it->gsfTrack()->referencePoint();
       const GlobalPoint gp(tpoint.x(), tpoint.y(), tpoint.z());
       double bfield = mField->inTesla(gp).mag();
       ConversionInfo info = cf.getConversionInfo(*it, ctfTracks, gsfTracks, bfield);
       dist->push_back(info.dist());
       dcot->push_back(info.dcot());
	missingHits->push_back(it->gsfTrack()->trackerExpectedHitsInner().numberOfHits());
	conversionPartnerCtfTrackTrackerExpectedHitsInner->push_back(info.conversionPartnerCtfTk().isAvailable() ? 
								     info.conversionPartnerCtfTk()->trackerExpectedHitsInner().numberOfHits() : -1);
	conversionPartnerGsfTrackTrackerExpectedHitsInner->push_back(info.conversionPartnerGsfTk().isAvailable() ? 
								     info.conversionPartnerGsfTk()->trackerExpectedHitsInner().numberOfHits() : -1);
     }
     kfcharge->push_back(it->track().isAvailable() ? it->track()->charge() : 0);
     scPixCharge->push_back(it->scPixCharge());
     closestCtfTrackCharge->push_back(it->closestCtfTrackRef().isAvailable() ? it->closestCtfTrackRef()->charge() : 0) ;

     //not yet working
     if(IdFromReco) {
       //bool veto,loose,medium,tight;
       //pogIdReco(iEvent, iSetup, reco::GsfElectronRef(*it), veto, loose, medium, tight);
       //idVeto->push_back(veto);
       //idLoose->push_back(loose);
       //idMedium->push_back(medium);
       //idTight->push_back(tight);
     }
   }
 }

 iEvent.put( isHandleValid,  Prefix + "HandleValid" + Suffix );
 iEvent.put( p4,  Prefix + "P4" + Suffix );
 iEvent.put( charge,  Prefix + "Charge" + Suffix );

 iEvent.put( selected, Prefix + "Selected" + Suffix );
 iEvent.put( gsfTrack_normalizedChi2,  Prefix + "GsfTracknormalizedChi2" + Suffix );
 iEvent.put( gsfTrack_numberOfValidHits,  Prefix + "GsfTracknumberOfValidHits" + Suffix );

 iEvent.put( isEB,  Prefix + "IsEB" + Suffix );
 iEvent.put( isEE,  Prefix + "IsEE" + Suffix );

 iEvent.put( gsfTrack_dxy,  Prefix + "GsfTrackDxy" + Suffix );
 iEvent.put( gsfTrack_dz,   Prefix + "GsfTrackDz" + Suffix );
 iEvent.put( gsfTrack_dxyBS,  Prefix + "GsfTrackDxyBS" + Suffix );
 iEvent.put( gsfTrack_dzBS,   Prefix + "GsfTrackDzBS" + Suffix );
 iEvent.put( gsfTrack_vertexz,   Prefix + "GsfTrackVertexz" + Suffix );
 iEvent.put( gsfTrack_dxyError,  Prefix + "GsfTrackDxyError" + Suffix );
 iEvent.put( gsfTrack_dzError,   Prefix + "GsfTrackDzError" + Suffix );

 iEvent.put( gsfTrkChargeMode,  Prefix + "GsfTrackChargeMode" + Suffix );
 iEvent.put( gsfTrkPtMode,  Prefix + "GsfTrackPtMode" + Suffix );
 iEvent.put( gsfTrkQoverPErrMode,  Prefix + "GsfTrackQoverPErrorMode" + Suffix );
 iEvent.put( gsfTrkCharge,  Prefix + "GsfTrackCharge" + Suffix );
 iEvent.put( gsfTrkPt, Prefix+ "GsfTrackPt" + Suffix);
 iEvent.put( gsfTrkQoverPErr,  Prefix + "GsfTrackQoverPError" + Suffix );
 iEvent.put( gsfTrkLostHits,  Prefix + "GsfTrackLostHits" + Suffix );
 iEvent.put( hasValHitinFirstPixBarrel, Prefix + "HasValidHitInFirstPixelBarrel" + Suffix);
 iEvent.put( gsfTrkTrackerExpectedHitsInner, Prefix + "GsfTrackTrackerExpectedHitsInner" + Suffix);
 iEvent.put( e1x5,  Prefix + "E1x5" + Suffix );
 iEvent.put( e5x5,  Prefix + "E5x5" + Suffix );
 iEvent.put( e2x5Max,  Prefix + "E2x5Max" + Suffix );
 iEvent.put( fbrem,  Prefix + "Fbrem" + Suffix );
 iEvent.put( hcalOverEcal,  Prefix + "HcalOverEcal" + Suffix );
 iEvent.put( hcalDepth1OverEcal,  Prefix + "HcalDepth1OverEcal" + Suffix );
 iEvent.put( hcalDepth2OverEcal,  Prefix + "HcalDepth2OverEcal" + Suffix );
 iEvent.put( eEleClusterOverPout,  Prefix + "EEleClusterOverPout" + Suffix );
 iEvent.put( eSeedClusterOverPout,  Prefix + "ESeedClusterOverPout" + Suffix );
 iEvent.put( eSeedClusterOverP,  Prefix + "ESeedClusterOverP" + Suffix );
 iEvent.put (eSuperClusterEta, Prefix + "ESuperClusterEta" + Suffix);
 iEvent.put (eSuperClusterPhi, Prefix + "ESuperClusterPhi" + Suffix);
 iEvent.put( eSuperClusterOverP,  Prefix + "ESuperClusterOverP" + Suffix );
 iEvent.put( deltaPhiSuperClusterTrackAtVtx,  Prefix + "DeltaPhiSuperClusterTrackAtVtx" + Suffix );
 iEvent.put( deltaEtaSuperClusterTrackAtVtx,  Prefix + "DeltaEtaSuperClusterTrackAtVtx" + Suffix );
 iEvent.put( deltaPhiSeedClusterTrackAtCalo,  Prefix + "DeltaPhiSeedClusterTrackAtCalo" + Suffix );
 iEvent.put( deltaEtaSeedClusterTrackAtCalo,  Prefix + "DeltaEtaSeedClusterTrackAtCalo" + Suffix );
 iEvent.put( deltaEtaEleClusterTrackAtCalo,  Prefix + "DeltaEtaEleClusterTrackAtCalo" + Suffix );
 iEvent.put( deltaPhiEleClusterTrackAtCalo,  Prefix + "DeltaPhiEleClusterTrackAtCalo" + Suffix );
 iEvent.put( sigmaEtaEta,  Prefix + "SigmaEtaEta" + Suffix );
 iEvent.put( sigmaIetaIeta,  Prefix + "SigmaIetaIeta" + Suffix );
 iEvent.put( classification,  Prefix + "Classification" + Suffix );
 iEvent.put( mva,  Prefix + "Mva" + Suffix );
 iEvent.put( dr03TkSumPt,  Prefix + "Dr03TkSumPt" + Suffix );
 iEvent.put( dr03EcalRecHitSumEt,  Prefix + "Dr03EcalRecHitSumEt" + Suffix );
 iEvent.put( dr03HcalTowerSumEt,  Prefix + "Dr03HcalTowerSumEt" + Suffix );
 iEvent.put( dr03HcalDepth1TowerSumEt,  Prefix + "Dr03HcalDepth1TowerSumEt" + Suffix );
 iEvent.put( dr03HcalDepth2TowerSumEt,  Prefix + "Dr03HcalDepth2TowerSumEt" + Suffix );
 iEvent.put( caloEnergy,  Prefix + "CaloEnergy" + Suffix );
 iEvent.put( ecalEnergy,  Prefix + "EcalEnergy" + Suffix );
 iEvent.put( ecalEnergyError,  Prefix + "EcalEnergyError" + Suffix );
 iEvent.put( electronMomentumError,  Prefix + "ElectronMomentumError" + Suffix );
 iEvent.put( numberOfTracks,  Prefix + "NumberOfTracks" + Suffix );
 iEvent.put( numberOfBrems,  Prefix + "NumberOfBrems" + Suffix );
 iEvent.put( shFracInnerHits,  Prefix + "ShFracInnerHits" + Suffix );
 iEvent.put( vertex,  Prefix + "Vertex" + Suffix );
 iEvent.put( vertexChi2,  Prefix + "VertexChi2" + Suffix );
 iEvent.put( vertexNdof,  Prefix + "VertexNdof" + Suffix );
 if(StoreConversionInfo) {
   iEvent.put( dist, Prefix + "ConversionDist" + Suffix );
   iEvent.put( dcot, Prefix + "ConversionDCot" + Suffix );
   iEvent.put( missingHits, Prefix + "ConversionMissingHits" + Suffix );
   iEvent.put( conversionPartnerCtfTrackTrackerExpectedHitsInner, Prefix + "ConversionPartnerCtfTrackTrackerExpectedHitsInner" + Suffix );
   iEvent.put( conversionPartnerGsfTrackTrackerExpectedHitsInner, Prefix + "ConversionPartnerGsfTrackTrackerExpectedHitsInner" + Suffix );
 }
 iEvent.put( conversionInfoStored , Prefix + "ConversionInfoStored" + Suffix );
 iEvent.put(kfcharge, Prefix + "KfTrackCharge" + Suffix);
 iEvent.put(scPixCharge, Prefix + "ScPixCharge" + Suffix);
 iEvent.put(closestCtfTrackCharge, Prefix + "ClosestCtfTrackCharge" + Suffix);

 if(IdFromReco) {
   iEvent.put(idVeto,     Prefix + "IdVeto"    + Suffix);
   iEvent.put(idLoose,    Prefix + "IdLoose"   + Suffix);
   iEvent.put(idMedium,   Prefix + "IdMedium"  + Suffix);
   iEvent.put(idTight,    Prefix + "IdTight"   + Suffix);
 }

}

// extra information stored for PAT data
template< typename T >
void Tuple_Electron<T>::
producePAT(edm::Event& iEvent, const edm::EventSetup& iSetup, edm::Handle<std::vector<T> >& collection) {

  std::map< std::string, std::vector<int> > idsOld;
  
  for(std::vector<std::string>::const_iterator name=IdFlagsOldStyle.begin(); name!=IdFlagsOldStyle.end(); ++name) {
    idsOld[*name] = std::vector<int>();
  }

  std::auto_ptr<std::vector<int> >   idVeto(   new std::vector<int>() );
  std::auto_ptr<std::vector<int> >   idLoose(  new std::vector<int>() );
  std::auto_ptr<std::vector<int> >   idMedium( new std::vector<int>() );
  std::auto_ptr<std::vector<int> >   idTight(  new std::vector<int>() );

  std::auto_ptr<std::vector<float> >  ecalIso   ( new std::vector<float>() );
  std::auto_ptr<std::vector<float> >  hcalIso   ( new std::vector<float>() );
  std::auto_ptr<std::vector<float> >  trackIso  ( new std::vector<float>() );
  std::auto_ptr<std::vector<float> >  ecalIsoDep( new std::vector<float>() );
  std::auto_ptr<std::vector<float> >  hcalIsoDep( new std::vector<float>() );
  
  std::auto_ptr<std::vector<int> > ispf (new std::vector<int>() );
  std::auto_ptr<std::vector<float> > partIso (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > charHadIso (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > neutHadIso (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > photIso (new std::vector<float>() ); 
  std::auto_ptr<std::vector<float> > charHadIsoRA4 (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > neutHadIsoRA4 (new std::vector<float>() );
  std::auto_ptr<std::vector<float> > photIsoRA4 (new std::vector<float>() ); 
  std::auto_ptr<std::vector<int> >  conversionRejection ( new std::vector<int>() );

  if (collection.isValid()){
    for(std::vector<pat::Electron>::const_iterator it = collection->begin(); it!=collection->end(); it++) {


     // -- converion rejection store bool
      // beam spot
      edm::Handle<reco::BeamSpot> beamspot_h;
      iEvent.getByLabel(beamSpotInputTag_, beamspot_h);
      const reco::BeamSpot &beamSpot = *(beamspot_h.product());

     // conversions
     edm::Handle<reco::ConversionCollection> conversions_h;
     iEvent.getByLabel(conversionsInputTag_, conversions_h);
     
     //     bool tmpConvRej = ConversionTools::hasMatchedConversion((const reco::GsfElectron)*it, conversions_h, beamSpot.position());
     //     conversionRejection->push_back(tmpConvRej);
     conversionRejection->push_back(ConversionTools::hasMatchedConversion((const reco::GsfElectron)*it, conversions_h, beamSpot.position()));

      // -- store pfIso values as described below Twiki apply 2012 electron id by hand
      edm::Ptr< reco::GsfElectron > ele = (edm::Ptr< reco::GsfElectron >) it->originalObjectRef();

      std::vector< edm::Handle< edm::ValueMap<double> > > isoVals(isoValInputTags_.size());
      for (size_t j = 0; j < isoValInputTags_.size(); ++j) {
	iEvent.getByLabel(isoValInputTags_[j], isoVals[j]);
      }

      charHadIsoRA4->push_back((*(isoVals)[0])[ele]);
      photIsoRA4->push_back((*(isoVals)[1])[ele]);
      neutHadIsoRA4->push_back((*(isoVals)[2])[ele]);



      //old-style IDs
      for(std::map< std::string, std::vector<int> >::const_iterator id=idsOld.begin(); id!=idsOld.end(); ++id) {
	idsOld[id->first].push_back(it->electronID(id->first));
      }

      if(!IdFromReco) {
	bool veto,loose,medium,tight;
	pogIdPat(iEvent, iSetup, *it, veto, loose, medium, tight);

	idVeto->push_back(veto);
	idLoose->push_back(loose);
	idMedium->push_back(medium);
	idTight->push_back(tight);
      }

      ecalIso ->push_back(it->ecalIso());
      hcalIso ->push_back(it->hcalIso());
      trackIso->push_back(it->trackIso());
      ecalIsoDep->push_back(it->ecalIsoDeposit() ? it->ecalIsoDeposit()->candEnergy() : -999.9);
      hcalIsoDep->push_back(it->hcalIsoDeposit() ? it->hcalIsoDeposit()->candEnergy() : -999.9);

      ispf->push_back(it->pfCandidateRef().isAvailable());
      partIso->push_back(it->particleIso());
      charHadIso->push_back(it->chargedHadronIso());
      neutHadIso->push_back(it->neutralHadronIso());
      photIso->push_back(it->photonIso());
    }
   } // end loop over electrons

  //store old-style IDs
  for(std::map< std::string, std::vector<int> >::const_iterator id=idsOld.begin(); id!=idsOld.end(); ++id) {
    std::auto_ptr<std::vector<int> > ptr( new std::vector<int>(id->second) );
    iEvent.put(ptr, Prefix + underscoreless(id->first) + Suffix);
  }

  if(!IdFromReco) {
    iEvent.put(idVeto,     Prefix + "IdVeto"    + Suffix);
    iEvent.put(idLoose,    Prefix + "IdLoose"   + Suffix);
    iEvent.put(idMedium,   Prefix + "IdMedium"  + Suffix);
    iEvent.put(idTight,    Prefix + "IdTight"   + Suffix);
  }

  iEvent.put(ecalIso,    Prefix + "EcalIso"    + Suffix);
  iEvent.put(hcalIso,    Prefix + "HcalIso"    + Suffix);
  iEvent.put(trackIso,   Prefix + "TrackIso"   + Suffix);
  iEvent.put(ecalIsoDep, Prefix + "EcalIsoDep" + Suffix);
  iEvent.put(hcalIsoDep, Prefix + "HcalIsoDep" + Suffix);

  iEvent.put(ispf, Prefix + "ProducedFromPF" + Suffix);
  iEvent.put(partIso, Prefix + "ParticleIso" + Suffix);
  iEvent.put(charHadIso, Prefix + "ChargedHadronIso" + Suffix);
  iEvent.put(neutHadIso, Prefix + "NeutralHadronIso" + Suffix);
  iEvent.put(photIso, Prefix + "PhotonIso" + Suffix);
  iEvent.put(charHadIsoRA4, Prefix + "ChargedHadronIsoRA4" + Suffix);
  iEvent.put(neutHadIsoRA4, Prefix + "NeutralHadronIsoRA4" + Suffix);
  iEvent.put(photIsoRA4, Prefix + "PhotonIsoRA4" + Suffix);
  iEvent.put(conversionRejection, Prefix + "ConversionRejection" + Suffix );
}

//2012 ID helper functions
template< typename T >
void Tuple_Electron<T>::
pogIdReco(edm::Event& iEvent, const edm::EventSetup& iSetup, reco::GsfElectronRef ele,
      bool& veto, bool& loose, bool& medium, bool& tight)
{
  //http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/EGamma/EGammaAnalysisTools/src/EGammaCutBasedEleIdAnalyzer.cc?revision=1.2&view=markup

  // conversions
  edm::Handle<reco::ConversionCollection> conversions_h;
  iEvent.getByLabel(conversionsInputTag_, conversions_h);

  // iso deposits
  std::vector< edm::Handle< edm::ValueMap<double> > > isoVals(isoValInputTags_.size());
  for (size_t j = 0; j < isoValInputTags_.size(); ++j) {
    iEvent.getByLabel(isoValInputTags_[j], isoVals[j]);
  }

  // beam spot
  edm::Handle<reco::BeamSpot> beamspot_h;
  iEvent.getByLabel(beamSpotInputTag_, beamspot_h);
  const reco::BeamSpot &beamSpot = *(beamspot_h.product());

  // vertices
  edm::Handle<reco::VertexCollection> vtx_h;
  iEvent.getByLabel(primaryVertexInputTag_, vtx_h);

  // rho for isolation
  edm::Handle<double> rhoIso_h;
  iEvent.getByLabel(rhoIsoInputTag_, rhoIso_h);
  double rhoIso = *(rhoIso_h.product());


  // particle flow isolation
  double iso_ch =  (*(isoVals)[0])[ele];
  double iso_em = (*(isoVals)[1])[ele];
  double iso_nh = (*(isoVals)[2])[ele];

  
  // working points
  veto   = EgammaCutBasedEleId::PassWP(EgammaCutBasedEleId::VETO,   ele, conversions_h, beamSpot, vtx_h, iso_ch, iso_em, iso_nh, rhoIso);
  loose  = EgammaCutBasedEleId::PassWP(EgammaCutBasedEleId::LOOSE,  ele, conversions_h, beamSpot, vtx_h, iso_ch, iso_em, iso_nh, rhoIso);
  medium = EgammaCutBasedEleId::PassWP(EgammaCutBasedEleId::MEDIUM, ele, conversions_h, beamSpot, vtx_h, iso_ch, iso_em, iso_nh, rhoIso);
  tight  = EgammaCutBasedEleId::PassWP(EgammaCutBasedEleId::TIGHT,  ele, conversions_h, beamSpot, vtx_h, iso_ch, iso_em, iso_nh, rhoIso);

 }

template< typename T >
void Tuple_Electron<T>::
pogIdPat(edm::Event& iEvent, const edm::EventSetup& iSetup, const T& ele,
      bool& veto, bool& loose, bool& medium, bool& tight)
{
  //http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/EGamma/EGammaAnalysisTools/src/EGammaCutBasedEleId.cc?revision=1.7&view=markup

  // conversions
  edm::Handle<reco::ConversionCollection> conversions_h;
  iEvent.getByLabel(conversionsInputTag_, conversions_h);

  // beam spot
  edm::Handle<reco::BeamSpot> beamspot_h;
  iEvent.getByLabel(beamSpotInputTag_, beamspot_h);
  const reco::BeamSpot &beamSpot = *(beamspot_h.product());

  // vertices
  edm::Handle<reco::VertexCollection> vtxs;
  iEvent.getByLabel(primaryVertexInputTag_, vtxs);

  // kinematic variables
  bool isEB           = ele.isEB() ? true : false;
  float pt            = ele.pt();
  float eta           = ele.superCluster()->eta();
  
  // id variables
  float dEtaIn        = ele.deltaEtaSuperClusterTrackAtVtx();
  float dPhiIn        = ele.deltaPhiSuperClusterTrackAtVtx();
  float sigmaIEtaIEta = ele.sigmaIetaIeta();
  float hoe           = ele.hadronicOverEm();
  float ooemoop       = (1.0/ele.ecalEnergy() - ele.eSuperClusterOverP()/ele.ecalEnergy());
  
  // impact parameter variables
  float d0vtx         = 0.0;
  float dzvtx         = 0.0;
  if (vtxs->size() > 0) {
    reco::VertexRef vtx(vtxs, 0);    
    d0vtx = ele.gsfTrack()->dxy(vtx->position());
    dzvtx = ele.gsfTrack()->dz(vtx->position());
  } else {
    d0vtx = ele.gsfTrack()->dxy();
    dzvtx = ele.gsfTrack()->dz();
  }

  double iso_ch = ele.chargedHadronIso();
  double iso_nh = ele.neutralHadronIso();
  double iso_em = ele.photonIso();

  // conversion rejection variables
  bool vtxFitConversion = ConversionTools::hasMatchedConversion((const reco::GsfElectron)ele, conversions_h, beamSpot.position());
  float mHits = ele.gsfTrack()->trackerExpectedHitsInner().numberOfHits();

  // rho for isolation
  edm::Handle<double> rhoIso_h;
  iEvent.getByLabel(rhoIsoInputTag_, rhoIso_h);
  double rho = *(rhoIso_h.product());

  veto   = EgammaCutBasedEleId::PassWP(EgammaCutBasedEleId::VETO, isEB, pt, eta,
				       dEtaIn, dPhiIn, sigmaIEtaIEta, hoe,
				       ooemoop, d0vtx, dzvtx, iso_ch, iso_em, iso_nh, 
				       vtxFitConversion, mHits, rho);
  loose  = EgammaCutBasedEleId::PassWP(EgammaCutBasedEleId::LOOSE, isEB, pt, eta,
				       dEtaIn, dPhiIn, sigmaIEtaIEta, hoe,
				       ooemoop, d0vtx, dzvtx, iso_ch, iso_em, iso_nh, 
				       vtxFitConversion, mHits, rho);
  medium = EgammaCutBasedEleId::PassWP(EgammaCutBasedEleId::MEDIUM, isEB, pt, eta,
				       dEtaIn, dPhiIn, sigmaIEtaIEta, hoe,
				       ooemoop, d0vtx, dzvtx, iso_ch, iso_em, iso_nh, 
				       vtxFitConversion, mHits, rho);
  tight  = EgammaCutBasedEleId::PassWP(EgammaCutBasedEleId::TIGHT, isEB, pt, eta,
				       dEtaIn, dPhiIn, sigmaIEtaIEta, hoe,
				       ooemoop, d0vtx, dzvtx, iso_ch, iso_em, iso_nh, 
				       vtxFitConversion, mHits, rho);
}

#endif
