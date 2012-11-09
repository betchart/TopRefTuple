#ifndef TUPLE_TRIGGERS
#define TUPLE_TRIGGERS

#include <algorithm>

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "FWCore/Utilities/interface/InputTag.h"

class Tuple_Triggers : public edm::EDProducer 
{
public: 
  explicit Tuple_Triggers(const edm::ParameterSet& conf)
    : inputTag  (conf.getParameter<edm::InputTag>("InputTag"))
    , tag_( conf.getParameter<edm::InputTag>("TriggerEventInputTag")) 
    , run_(-1)
    {
    produces <bool> ( "hltHandleValid");
    produces <std::string> ( "hltKey");
    produces <std::map<std::string,int> > ("prescaled");
  }

private: 
  edm::InputTag         inputTag;
  HLTConfigProvider     hltConfig;
  const edm::InputTag   tag_;
  int                   run_;
  
  void produce( edm::Event& event, const edm::EventSetup& setup) {

    if ( int(event.getRun().run()) != run_ ) {
      // Set process name using method here: https://hypernews.cern.ch/HyperNews/CMS/get/physTools/1791/1/1/1/1/1/2.html
      if ( inputTag.process().empty() ) { 
        edm::Handle<trigger::TriggerEvent> tmp;
        event.getByLabel( tag_, tmp );
        if( tmp.isValid() ) { inputTag = edm::InputTag( inputTag.label(), inputTag.instance(), tmp.provenance()->processName() ); }
        else { edm::LogError( "Tuple_Triggers" ) << "[SusyCAF::produce] Cannot retrieve TriggerEvent product for " << tag_; }
      }
      // Initialise HLTConfigProvider
      bool  hltChanged = false;
      if (!hltConfig.init(event.getRun(), setup, inputTag.process(), hltChanged) ) {
        edm::LogError( "Tuple_Triggers" ) << "HLT config initialization error with process name \"" << inputTag.process() << "\".";
      } else if ( hltConfig.size() < 1 ) {
        edm::LogError( "Tuple_Triggers" ) << "HLT config has zero size.";
      }
    }
    //run_ = event.getRun().run() // Can this save time, and is it safe?

    edm::Handle<edm::TriggerResults> results;
    event.getByLabel( inputTag, results ); 

    std::auto_ptr<std::map<std::string,int> >  prescaled(new std::map<std::string,int>());

    if(results.isValid()) {
      const edm::TriggerNames& names = event.triggerNames(*results);
      for(unsigned i=0; i < results->size(); i++) 
	if (results->accept(i)) 
	  (*prescaled)[names.triggerName(i)] = hltConfig.prescaleValue(event,setup,names.triggerName(i));
    }
    
    event.put( std::auto_ptr<bool>(new bool(results.isValid())), "hltHandleValid");
    event.put( std::auto_ptr<std::string>(new std::string(hltConfig.tableName())), "hltKey");
    event.put( prescaled,"prescaled");
  }

};

#endif
