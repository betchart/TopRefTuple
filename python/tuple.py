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
        self.process.tuplePileup = cms.EDProducer("Tuple_PileupSummary",
                                                  InputTag = cms.InputTag('addPileupInfo'),
                                                  Prefix = cms.string('pileup'),
                                                  Suffix = cms.string('')
                                                  )
        return self.empty + self.process.tupleGen + self.process.tuplePileup

    def triggers(self) :
        if not self.options.isData : return self.empty
        return self.empty

    def electron(self) :
        self.process.tupleElectron = cms.EDProducer("Tuple_Electron",
                                                    electronTag = cms.InputTag('selectedPatElectrons'+self.options.postfix),
                                                    vertexTag = cms.InputTag('goodOfflinePrimaryVertices'),
                                                    prefix = cms.string('el'),
                                                    electronIDs = cms.vstring('mvaTrigV0')
                                                    )
        return self.empty + self.process.tupleElectron


    def muon(self) :
        self.process.tupleMuon = cms.EDProducer("Tuple_Muon",
                                                muonTag = cms.InputTag('selectedPatMuons'+self.options.postfix),
                                                prefix = cms.string('mu') )
        return self.empty + self.process.tupleMuon
        
    def path(self) :
        return cms.Path( self.events() *
                         self.gen() *
                         self.triggers() * # FIXME
                         self.electron() *
                         #muon
                         #jet
                         #met
                         #vertex
                         #triggers
                         self.tree() )
