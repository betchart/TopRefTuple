#ifndef TUPLE_ELECTRON
#define TUPLE_ELECTRON

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

class Tuple_Electron : public edm::EDProducer {
 public:
  explicit Tuple_Electron(const edm::ParameterSet&);

 private:
  void produce(edm::Event &, const edm::EventSetup & );

  const edm::InputTag electronTag,vertexTag;
  const std::vector<std::string> electronIDs;
  const std::string prefix;
};


#endif
