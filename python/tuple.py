import operator
from FWCore.ParameterSet import Config as cms

class Tuple(object) :

    def __init__(self,process,options) :
        self.process = process
        self.options = options
        self.empty = process.empty = cms.Sequence()
                
    def attr(self, item) : return getattr(self.process, item)

    def tree(self) :
        self.process.topRef = cms.EDAnalyzer( "MakeTree", outputCommands = cms.untracked.vstring(
            'drop *',
            'keep *_tuple*_*_*',) )
        return self.process.topRef
        
    def events(self) :
        self.process.tupleEvents = cms.EDProducer("Tuple_Event")
        return self.empty + self.process.tupleEvents
        
    def gen(self) :
        if self.options.isData : return self.empty
        self.process.tupleGen = cms.EDProducer("Tuple_GenParticle",
                                               InputTag = cms.InputTag('genParticles'),
                                               JetCollections = cms.VInputTag("ak5GenJetsNoNu"),
                                               Prefix = cms.string('gen'),
                                               Suffix = cms.string(''),
                                               GenStatus1PtCut = cms.double(10.0),
                                               GenJetPtCut = cms.double(10.0),
                                               )
        return self.empty + self.process.tupleGen

    def path(self) :
        return cms.Path( self.events() *
                         self.gen() *
                         self.tree() )
