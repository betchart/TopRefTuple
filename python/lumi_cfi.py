import FWCore.ParameterSet.Config as cms

lumiTree = cms.EDAnalyzer("Tuple_LumiTreeMaker",
                          InputTag = cms.InputTag("lumiProducer")
                          )

lumi = cms.Path( lumiTree )
