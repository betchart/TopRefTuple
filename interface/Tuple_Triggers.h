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
    , sourceName(conf.getParameter<std::string>  ("SourceName"))
    , sourceType(NOT_APPLICABLE)
    , tag_( conf.getParameter<edm::InputTag>("TriggerEventInputTag")) 
    , run_(-1)
    {
    // Source is either a stream or a dataset (mutually exclusive)
    if (sourceName.length() > 0) {
      if (sourceName.length() >= 2 && sourceName[0]=='S' && sourceName[1]==':') {
        sourceType      = STREAM;
        sourceName      = sourceName.substr(2);
      }
      else if (sourceName.length() >= 3 && sourceName[0]=='D' && sourceName[1]=='S' && sourceName[2]==':') {
        sourceType      = DATASET;
        sourceName      = sourceName.substr(3);
      }
      else throw edm::Exception(edm::errors::Configuration)
        << "Invalid SourceName = '" << sourceName 
        << "' -- must start with either 'S:' for streams or 'DS:' for datasets."
        << std::endl;
    }

    produces <bool> ( "hltHandleValid");
    produces <std::string> ( "hltKey");
    produces <std::map<std::string,bool> >("triggered");
    produces <std::map<std::string,bool> >("parasiticTrigger");
    produces <std::map<std::string,int> > ("prescaled");
    produces <std::map<std::string,std::string> >("hltL1Seeds");
  }

private: 
  enum DataSource { NOT_APPLICABLE, STREAM, DATASET };
  edm::InputTag         inputTag;
  std::string           sourceName;
  DataSource            sourceType;
  HLTConfigProvider     hltConfig;
  edm::InputTag         tag_;
  int                   run_;
  
  // Stored per run/lumisection to save time
  std::vector<std::string>    dataSource;

  void printNames(const std::vector<std::string>& names) {
    for (unsigned int i = 0; i < names.size(); ++i)
      edm::LogProblem( "Tuple_Triggers" ) << "  " << names[i] << std::endl;
  }

  void getDataSource() {
    dataSource.clear();
    if (sourceType == NOT_APPLICABLE) return;

    if (sourceType == STREAM) {
      unsigned int  index   = hltConfig.streamIndex(sourceName);
      if (index >= hltConfig.streamNames().size()) {
        edm::LogError( "Tuple_Triggers" ) << "Streams in '" << inputTag.process() << "' HLT menu:";
        printNames(hltConfig.streamNames());
        throw edm::Exception(edm::errors::Configuration) << "Stream with name '" << sourceName << "' does not exist." << std::endl;
      }
      dataSource    = hltConfig.streamContent(sourceName);
    }
    else {
      unsigned int  index   = hltConfig.datasetIndex(sourceName);
      if (index >= hltConfig.datasetNames().size()) {
        edm::LogError( "Tuple_Triggers" ) << "Datasets in '" << inputTag.process() << "' HLT menu:";
        printNames(hltConfig.datasetNames());
        throw edm::Exception(edm::errors::Configuration) << "Dataset with name '" << sourceName << "' does not exist." << std::endl;
      }
      dataSource    = hltConfig.datasetContent(sourceName);
    }
  }

  void produce( edm::Event& event, const edm::EventSetup& setup) {

    if ( int(event.getRun().run()) != run_ ) {
      // Set process name using method here: https://hypernews.cern.ch/HyperNews/CMS/get/physTools/1791/1/1/1/1/1/2.html
      if ( inputTag.process().empty() ) { 
        edm::Handle<trigger::TriggerEvent> temp;
        event.getByLabel( tag_, temp );
        if( temp.isValid() ) { inputTag = edm::InputTag( inputTag.label(), inputTag.instance(), temp.provenance()->processName() ); }
        else { edm::LogError( "Tuple_Triggers" ) << "[SusyCAF::produce] Cannot retrieve TriggerEvent product for " << tag_; }
      }
      // Initialise HLTConfigProvider
      bool  hltChanged = false;
      if (!hltConfig.init(event.getRun(), setup, inputTag.process(), hltChanged) ) {
        edm::LogError( "Tuple_Triggers" ) << "HLT config initialization error with process name \"" << inputTag.process() << "\".";
      } else if ( hltConfig.size() < 1 ) {
        edm::LogError( "Tuple_Triggers" ) << "HLT config has zero size.";
      }
      getDataSource();
    }

    // Retrieve TriggerResults with appropriate InputTag
    edm::Handle<edm::TriggerResults> results;
    event.getByLabel( inputTag, results ); 
    //std::cout << "Retrieving TriggerResults with: " << inputTag << std::endl;

    std::auto_ptr<std::map<std::string,bool> > triggered(new std::map<std::string,bool>());
    std::auto_ptr<std::map<std::string,bool> > parasiticTrigger(new std::map<std::string,bool>());
    std::auto_ptr<std::map<std::string,int> >  prescaled(new std::map<std::string,int>());
    std::auto_ptr<std::map<std::string,std::string> > hltL1Seeds(new std::map<std::string,std::string>());

    if(results.isValid()) {
      const edm::TriggerNames& names = event.triggerNames(*results);
      for(unsigned i=0; i < results->size(); i++) {
        (*prescaled)[names.triggerName(i)] = hltConfig.prescaleValue(event,setup,names.triggerName(i));

        if (dataSource.empty() || std::find(dataSource.begin(), dataSource.end(), names.triggerName(i)) != dataSource.end()) {
          (*triggered)[names.triggerName(i)] = results->accept(i) ;
          const std::vector<std::pair<bool,std::string> >&  l1Seeds = hltConfig.hltL1GTSeeds(i);
          if (l1Seeds.size() == 1) {
            (*hltL1Seeds)[names.triggerName(i)]  = l1Seeds.front().second;
          }
          else if (l1Seeds.size() > 1) {
            std::string   combined;
            for (unsigned iSeed = 0; iSeed < l1Seeds.size(); ++iSeed) {
              if (combined.length())  combined += " OR ";
              combined   += "("+l1Seeds[iSeed].second+")";
            } // end loop over L1 seeds
            (*hltL1Seeds)[names.triggerName(i)]  = combined;
          }
        } else
          (*parasiticTrigger)[names.triggerName(i)] = results->accept(i) ;
      }
    } else { edm::LogError( "Tuple_Triggers" ) << "[SusyCAF::produce] Cannot retrieve TriggerResults product for " << inputTag; }

    std::auto_ptr<std::vector<std::string> >  source(new std::vector<std::string>(dataSource));

    event.put( std::auto_ptr<bool>(new bool(results.isValid())), "hltHandleValid");
    event.put( std::auto_ptr<std::string>(new std::string(hltConfig.tableName())), "hltKey");
    event.put( triggered,"triggered");
    event.put( parasiticTrigger,"parasiticTrigger");
    event.put( prescaled,"prescaled");
    event.put( hltL1Seeds,"hltL1Seeds");
  }

};

#endif
