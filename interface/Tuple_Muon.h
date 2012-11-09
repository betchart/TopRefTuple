#ifndef TUPLE_MUON
#define TUPLE_MUON

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

class Tuple_Muon : public edm::EDProducer {
 public:
  explicit Tuple_Muon(const edm::ParameterSet&);

 private:
  void produce(edm::Event &, const edm::EventSetup & );

  const edm::InputTag muonTag,vertexTag;
  const std::string prefix;
};


#endif
