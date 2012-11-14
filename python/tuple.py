import operator
from FWCore.ParameterSet import Config as cms

def tags(stuff) :
    return ( cms.InputTag(stuff) if type(stuff)!=list else
             cms.VInputTag([tags(item) for item in stuff]) )

class Tuple(object) :

    def __init__(self,process,options) :
        self.process = process
        self.options = options
        self.empty = process.empty = cms.Sequence()
                
    def path(self) :
        return cms.Path( self.events() *
                         self.gen() *
                         self.triggers() *
                         self.electron() *
                         self.muon() *
                         self.met() *
                         self.jet() *
                         #vertex
                         self.tree() )

    def attr(self, item) : return getattr(self.process, item)

    def tree(self) :
        self.process.topRef = cms.EDAnalyzer( "TreeMaker", outputCommands = cms.untracked.vstring(
            'drop *',
            'keep *_tuple*_*_*',) )
        return self.process.topRef
        
    def events(self) :
        self.process.tupleEvents = cms.EDProducer("Tuple_Event")
        return self.empty + self.process.tupleEvents
        
    def gen(self) :
        if self.options.isData : return self.empty
        self.process.tupleGen = cms.EDProducer("Tuple_GenParticle",
                                               InputTag = tags('genParticles'),
                                               JetCollections = tags(["ak5GenJetsNoNu"]),
                                               Prefix = cms.string('gen'),
                                               Suffix = cms.string(''),
                                               GenStatus1PtCut = cms.double(1000.0),
                                               GenJetPtCut = cms.double(10.0),
                                               )
        self.process.tuplePileup = cms.EDProducer("Tuple_PileupSummary",
                                                  InputTag = tags('addPileupInfo'),
                                                  Prefix = cms.string('pileup'),
                                                  Suffix = cms.string('')
                                                  )
        return self.empty + self.process.tupleGen + self.process.tuplePileup

    def triggers(self) :
        if not self.options.isData : return self.empty
        self.process.tupleTriggers = cms.EDProducer("Tuple_Triggers",
                                                    InputTag = tags('TriggerResults'),
                                                    TriggerEventInputTag = tags('hltTriggerSummaryAOD'),
                                                    )
        return self.empty + self.process.tupleTriggers

    def electron(self) :
        self.process.tupleElectron = cms.EDProducer("Tuple_Electron",
                                                    electronTag = tags('selectedPatElectrons'+self.options.postfix),
                                                    vertexTag = tags('goodOfflinePrimaryVertices'),
                                                    prefix = cms.string('el'),
                                                    electronIDs = cms.vstring('mvaTrigV0')
                                                    )
        return self.empty + self.process.tupleElectron


    def muon(self) :
        self.process.tupleMuon = cms.EDProducer("Tuple_Muon",
                                                muonTag = tags('selectedPatMuons'+self.options.postfix),
                                                vertexTag = tags('goodOfflinePrimaryVertices'),
                                                prefix = cms.string('mu') )
        return self.empty + self.process.tupleMuon
        
    def met(self) :
        self.process.tupleMET = cms.EDProducer("Tuple_PatMET",
                                               metTag = tags("patMETs"+self.options.postfix),
                                               prefix = cms.string("met"),
                                               particlesTag = tags("particleFlow"),
                                               particlesPrefix = cms.string('pf')
                                               )
        return self.empty + self.process.tupleMET

    def jet(self) :
        self.process.tupleJet = cms.EDProducer("Tuple_PatJet",
                                               prefix = cms.string("jet"),
                                               jetsTag = tags("selectedPatJets"+self.options.postfix),
                                               genTag = tags("ak5GenJetsNoNu"),
                                               jecRecord = cms.string("AK5PFchs"),
                                               bTags = cms.vstring(self.options.btags),
                                               pfInfo = cms.bool(True),
                                               genInfo = cms.bool( not self.options.isData)
                                               )
        return self.empty + self.process.tupleJet
