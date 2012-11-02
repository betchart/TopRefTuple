import operator
from FWCore.ParameterSet import Config as cms

scrapingFilter = cms.EDFilter( "FilterOutScraping",
                               applyfilter = cms.untracked.bool( True ),
                               debugOn     = cms.untracked.bool( False ),
                               numtrack    = cms.untracked.uint32( 10 ),
                               thresh      = cms.untracked.double( 0.25 ) )

from RecoMET.METAnalyzers.CSCHaloFilter_cfi import CSCTightHaloFilter
from RecoMET.METFilters.eeBadScFilter_cfi import eeBadScFilter

from CommonTools.RecoAlgos.HBHENoiseFilter_cfi import HBHENoiseFilter
# s. https://hypernews.cern.ch/HyperNews/CMS/get/JetMET/1196.html
for attr in ['minIsolatedNoiseSumE','minNumIsolatedNoiseChannels','minIsolatedNoiseSumEt'] : setattr( HBHENoiseFilter, attr, 999999 )

from RecoMET.METFilters.hcalLaserEventFilter_cfi import hcalLaserEventFilter
hcalLaserEventFilter.vetoByRunEventNumber = cms.untracked.bool( False )
hcalLaserEventFilter.vetoByHBHEOccupancy = cms.untracked.bool( True )

from RecoMET.METFilters.EcalDeadCellTriggerPrimitiveFilter_cfi import EcalDeadCellTriggerPrimitiveFilter
EcalDeadCellTriggerPrimitiveFilter.tpDigiCollection = cms.InputTag( 'ecalTPSkimNA' )

from RecoMET.METFilters.trackingFailureFilter_cfi import trackingFailureFilter
trackingFailureFilter.VertexSource = cms.InputTag( 'goodOfflinePrimaryVertices' )

cleaningMods = ['HBHENoiseFilter',
                'CSCTightHaloFilter',
                'hcalLaserEventFilter',
                'EcalDeadCellTriggerPrimitiveFilter',
                'eeBadScFilter',
                'trackingFailureFilter',
                'scrapingFilter']
eventCleaning = cms.Sequence( reduce(operator.add,[eval(mod) for mod in cleaningMods]) )
