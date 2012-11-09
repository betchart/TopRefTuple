#ifndef TREEMAKER_H
#define TREEMAKER_H

/** \class TreeMaker
 * 
 *  Makes a tree out of C++ standard types and vectors of C++ standard types
 *
 *  This class, which is an EDAnalyzer, takes the same "keep" and
 *  "drop" outputCommands parameter as the PoolOutputSource, making a
 *  tree of the selected variables, which it obtains from the EDM
 *  tree.  
 *
 *  $Date: 2012/11/07 21:53:09 $
 *  $Revision: 1.1 $
 *  \author Burt Betchart - University of Rochester <burton.andrew.betchart@cern.ch>
 */

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include <string>
#include <vector>
#include <TTree.h>

class TreeMaker : public edm::EDAnalyzer {
private:    
  virtual void beginJob();
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob(){}

  class BranchConnector {
  public:
    virtual ~BranchConnector() {};
    virtual void connect(const edm::Event&) = 0;
  };
  
  template <class T>
  class TypedBranchConnector : public BranchConnector {
  private:
    std::string ml;   //module label
    std::string pin;  //product instance name
    T object_;
    T* object_ptr_;
  public:
    TypedBranchConnector(edm::BranchDescription const*, std::string, TTree*);
    void connect(const edm::Event&);
  };

  edm::Service<TFileService> fs;
  TTree * tree;
  std::vector<BranchConnector*> connectors;
  edm::ParameterSet pset;

public:
  explicit TreeMaker(const edm::ParameterSet& iConfig) : pset(iConfig) {}
};

#endif
