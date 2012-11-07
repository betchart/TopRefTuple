import operator
from FWCore.ParameterSet import Config as cms

class Tuple(object) :

    def __init__(self,process,options) :
        self.process = process
        self.options = options
        self.empty = process.empty = cms.Sequence()
                
    def attr(self, item) : return getattr(self.process, item)

    def tree(self) :
        self.process.tree = cms.EDAnalyzer( "MakeTree", outputCommands = cms.untracked.vstring(
            'drop *',
            'keep *_tuple*_*_*',) )
        return self.process.tree
        
    def events(self) :
        self.process.tupleEvents = cms.EDProducer("Tuple_Event")
        return self.empty + self.process.tupleEvents
        
    def path(self) :
        return cms.Path( self.events() *
                         self.tree() )
