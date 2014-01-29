import operator
from FWCore.ParameterSet import Config as cms

from TopQuarkAnalysis.TopRefTuple.options import options
options = options()

process = cms.Process( 'PAT' )
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.Geometry.GeometryIdeal_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('EgammaAnalysis.ElectronTools.electronIdMVAProducer_cfi')
process.load('EgammaAnalysis.ElectronTools.electronIsolatorFromEffectiveArea_cfi')
process.load('TopQuarkAnalysis.TopRefTuple.vertex_cff')
process.load('TopQuarkAnalysis.TopRefTuple.cleaning_cff')
process.source = cms.Source('PoolSource', fileNames = cms.untracked.vstring(options.files) )
process.out = cms.OutputModule( "PoolOutputModule", outputCommands = cms.untracked.vstring( 'drop *' ))
process.add_( cms.Service( "TFileService", fileName = cms.string( options.output ), closeFileFast = cms.untracked.bool(True) ) )

from TopQuarkAnalysis.TopRefTuple.pf2pat import TopRefPF2PAT
topPF2PAT = TopRefPF2PAT(process,options) #configuration object for patPF2PATSequence

process.GlobalTag.globaltag = options.globalTag + '::All'
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool( not options.quiet ))
process.MessageLogger.cerr.FwkReport.reportEvery = 1000 if options.quiet else 10
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32( options.maxEvents ))
if not options.isData : process.eventCleaning.remove( process.scrapingFilter )
if options.isFastSim :
    for item in ['HBHENoiseFilter','CSCTightHaloFilter','manystripclus53X','toomanystripclus53X','logErrorTooManyClusters'] :
        process.eventCleaning.remove( getattr(process,item) )
        delattr( process, item )
for item in ['out','mvaNonTrigV0'] : delattr( process, item )

process.patreco = cms.Path( reduce(operator.add, [getattr(process, item) for item in ['goodOfflinePrimaryVertices',
                                                                                      'eventCleaning',
                                                                                      'mvaTrigV0',
                                                                                      'patPF2PATSequence'+options.postfix]]))

process.load('TopQuarkAnalysis.TopRefTuple.lumi_cfi')
from TopQuarkAnalysis.TopRefTuple.tuple import Tuple
process.tuple = Tuple(process, options).path()
schedule = cms.Schedule( process.patreco,
                         process.tuple,
                         process.lumi )

# dump config
process.prune()
delattr(process,'PF2PAT')
with open(options.output.replace('.root','_cfg.py'),'w') as cfg : print >>cfg, process.dumpPython()
