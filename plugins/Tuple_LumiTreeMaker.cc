#include <memory>
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include <TTree.h>

#include "DataFormats/Luminosity/interface/LumiSummary.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"

class Tuple_LumiTreeMaker : public edm::EDAnalyzer {
public:
  explicit Tuple_LumiTreeMaker(const edm::ParameterSet&);
  ~Tuple_LumiTreeMaker();
  
private:
  virtual void beginJob();
  virtual void endJob(){}
  virtual void analyze(const edm::Event&, const edm::EventSetup&){}
  virtual void beginLuminosityBlock(const edm::LuminosityBlock&, const edm::EventSetup&);

  edm::Service<TFileService> fs;
  TTree *tree;

  int run;
  int lumiSection;

  const edm::InputTag inputTag;
  bool handleIsValid;
  bool dataIsValid;
  int lsNumber;
  short lumiSecQual;
  float liveFrac;
  float avgInsRecLumi;
  float avgInsRecLumiErr;

};

Tuple_LumiTreeMaker::Tuple_LumiTreeMaker(const edm::ParameterSet& iConfig) :
  inputTag(iConfig.getParameter<edm::InputTag>("InputTag"))
{
}

Tuple_LumiTreeMaker::~Tuple_LumiTreeMaker()
{
}

void Tuple_LumiTreeMaker::beginJob()
{
  tree=fs->make<TTree>("tree", "");
  tree->Branch("handleIsValid",      &handleIsValid,        "handleIsValid/O");
  tree->Branch("run",                &run,                  "run/I");
  tree->Branch("lumiSection",        &lumiSection,          "lumiSection/I");

  tree->Branch("dataIsValid",        &dataIsValid,          "dataIsValid/O");
  tree->Branch("lsNumber",           &lsNumber,             "lsNumber/I");
  tree->Branch("lumiSecQual",        &lumiSecQual,          "lumiSecQual/S");
  tree->Branch("liveFrac",           &liveFrac,             "liveFrac/F");
  tree->Branch("avgInsRecLumi",      &avgInsRecLumi,        "avgInsRecLumi/F");
  tree->Branch("avgInsRecLumiErr",   &avgInsRecLumiErr,     "avgInsRecLumiErr/F");
}

void Tuple_LumiTreeMaker::beginLuminosityBlock(const edm::LuminosityBlock& lumiBlock, const edm::EventSetup& eventSetup)
{
  run=lumiBlock.run();
  lumiSection=lumiBlock.id().luminosityBlock();

  edm::Handle<LumiSummary> lumiSummary;
  lumiBlock.getByLabel(inputTag, lumiSummary);

  handleIsValid=lumiSummary.isValid();
  
  if (!handleIsValid) {
    dataIsValid     =false;
    lsNumber        =0;
    lumiSecQual     =0;
    liveFrac        =0.0;
    avgInsRecLumi   =0.0;
    avgInsRecLumiErr=0.0;
  }
  else {
    dataIsValid     =lumiSummary->isValid();
    lsNumber        =lumiSummary->lsNumber();
    lumiSecQual     =lumiSummary->lumiSecQual();
    liveFrac        =lumiSummary->liveFrac();
    avgInsRecLumi   =lumiSummary->avgInsRecLumi();
    avgInsRecLumiErr=lumiSummary->avgInsRecLumiErr();
  }
  
  tree->Fill();
}

DEFINE_FWK_MODULE(Tuple_LumiTreeMaker);
