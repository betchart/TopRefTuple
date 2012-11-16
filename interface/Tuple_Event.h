#ifndef TUPLE_EVENT
#define TUPLE_EVENT

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Utilities/interface/InputTag.h"

class Tuple_Event : public edm::EDProducer {
 public: 
  explicit Tuple_Event(const edm::ParameterSet&);
 private: 
  void produce( edm::Event &, const edm::EventSetup & );
  const edm::InputTag vertexTag;
};

#endif
