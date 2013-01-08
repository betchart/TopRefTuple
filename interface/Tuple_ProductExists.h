#ifndef TUPLE_PRODUCTEXISTS
#define TUPLE_PRODUCTEXISTS

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/Provenance/interface/Provenance.h"


class Tuple_ProductExists : public edm::EDProducer 
{
  typedef std::vector<edm::InputTag> VInputTag;
  typedef std::vector<edm::Provenance const*> Provenances;

 public:
  explicit Tuple_ProductExists(const edm::ParameterSet& conf)
    : inputTags ( conf.getParameter<VInputTag>("products") )
    {
      for(VInputTag::const_iterator tag=inputTags.begin(); tag!=inputTags.end(); tag++)
	produces <bool> ( tag->label() + "Exists");
    }

 private:
  const VInputTag inputTags;

  void produce( edm::Event& event, const edm::EventSetup& setup ) {
    Provenances provenances;
    event.getAllProvenance(provenances);
    for(VInputTag::const_iterator tag=inputTags.begin(); tag!=inputTags.end(); tag++) {
      bool exists=false;
      for(Provenances::const_iterator prov=provenances.begin(); prov!=provenances.end(); prov++) {
	if( tag->instance()==""
	    ? (*prov)->moduleLabel() == tag->label() 
	    : (*prov)->productInstanceName() == tag->instance() ) {
	  exists=true;
	  break;
	}
      }
      event.put( std::auto_ptr<bool>(new bool(exists)), tag->label() + "Exists");
    }
  }
};

#endif
