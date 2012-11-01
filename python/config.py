import operator
from FWCore.ParameterSet import VarParsing, Config as cms
options = VarParsing.VarParsing('standard')
options.register('isData', True, VarParsing.VarParsing.multiplicity.singleton, VarParsing.VarParsing.varType.bool, "decide if run on MC or data")
options.register('verbose', True, VarParsing.VarParsing.multiplicity.singleton, VarParsing.VarParsing.varType.bool, "be verbose")
options.parseArguments()

### Basic configuration
process = cms.Process( 'PAT' )
process.load('FWCore.MessageService.MessageLogger_cfi')
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool( options.verbose ))
process.MessageLogger.cerr.FwkReport.reportEvery = 100 if options.verbose else 1000
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32( 1000 ))
process.load('Configuration.Geometry.GeometryIdeal_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = ('GR_R_53_V13::All' if options.isData else
                               'START53_V11::All')

### Input configuration
from PhysicsTools.PatAlgos.tools.cmsswVersionTools import pickRelValInputFiles
inputFiles = ( pickRelValInputFiles( cmsswVersion = 'CMSSW_5_3_4_cand1',
                                     dataTier     = 'RECO' if options.isData else 'AODSIM',
                                     relVal       = 'SingleMu' if options.isData else 'RelValProdTTbar',
                                     globalTag    = 'GR_R_53_V12_RelVal_mu2012A' if options.isData else 'START53_V10',
                                     maxVersions  = 1 ) )
process.source = cms.Source( "PoolSource",
                             noEventSort        = cms.untracked.bool( True ),
                             duplicateCheckMode = cms.untracked.string( 'noDuplicateCheck' ),
                             skipBadFiles       = cms.untracked.bool( True ),
                             fileNames          = cms.untracked.vstring(inputFiles))
#### Output configuration
process.out = cms.OutputModule( "PoolOutputModule", outputCommands = cms.untracked.vstring( 'drop *' ))

### Cleaning
if True :
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
                  'scrapingFilter'][:None if options.isData else -1]
  for mod in cleaningMods : setattr( process, mod, eval(mod) )
  process.eventCleaning = cms.Sequence( reduce(operator.add,[eval(mod) for mod in cleaningMods]) )


# PAT/PF2PAT configuration
postfix = 'TR'
process.goodOfflinePrimaryVertices = cms.EDFilter( "PrimaryVertexObjectFilter", # checks for fake PVs automatically
                                                   src = cms.InputTag( 'offlinePrimaryVertices' ),
                                                   filter = cms.bool( False ), # use only as producer
                                                   filterParams = cms.PSet( minNdof = cms.double( 4. ),
                                                                            maxZ    = cms.double( 24. ),
                                                                            maxRho  = cms.double( 2. )))
process.load( "PhysicsTools.PatAlgos.patSequences_cff" )
from PhysicsTools.PatAlgos.tools.pfTools import usePF2PAT
usePF2PAT( process,  runPF2PAT = True,  postfix = postfix,  runOnMC = not options.isData,
           pvCollection = cms.InputTag( 'goodOfflinePrimaryVertices' ),
           typeIMetCorrections = True, jetAlgo = 'AK5',
           jetCorrections = ( 'AK5PFchs', ['L1FastJet','L2Relative','L3Absolute','L2L3Residual','L5Flavor','L7Parton'][:-2 if options.isData else -3] )
           )
# remove MC matching, object cleaning, objects etc.
import PhysicsTools.PatAlgos.tools.coreTools as coreTools
if options.isData:
  coreTools.runOnData( process, names = [ 'PFAll' ], postfix = postfix )
coreTools.removeSpecificPATObjects( process, names = [ 'Photons', 'Taus' ], postfix = postfix ) # includes 'removeCleaning'

patSeq = getattr(process, 'patPF2PATSequence' + postfix)

# switch to new PF isolation with L1Fastjet CHS
getattr( process, 'pfPileUpIso'+postfix).checkClosestZVertex = True
# Enable TopProjections except for Taus ( do not remove tau candidates from jet collection )
getattr( process, 'pfNoPileUp'+postfix).enable = True
getattr( process, 'pfNoJet'   +postfix).enable = True
getattr( process, 'pfNoTau'   +postfix).enable = False
# Use selected PAT leptons as topCollections
for lep in ['Muon','Electron'] :
  pfNo = getattr( process, 'pfNo%s%s'%(lep,postfix))
  pfNo.enable = True
  pfNo.topCollection = 'selectedPat%ss%s'%(lep,postfix)
  mods = [ getattr(process, item) for item in [ lep.lower() + 'Match'+postfix,
                                                'pat%ss%s'%(lep,postfix),
                                                'selectedPat%ss%s'%(lep,postfix)]][int(options.isData):]
  for mod in mods : patSeq.remove(mod)
  patSeq.replace( pfNo, reduce(operator.mul, mods) * pfNo )

# pfLepton config
lepIso3 = {'mu':False, 'el':True}
for lep in ['el','mu'] :
  full = {'el':'Electrons','mu':'Muons'}[lep] + postfix
  iso = getattr(process, 'pfIsolated'+full)
  iso.isolationCut = 0.2
  iso.doDeltaBetaCorrection = True
  iso.deltaBetaFactor = -0.5
  if lepIso3[lep]:
    pf = {'el':'PFId','mu':''}[lep] + postfix
    for item in ['pf','pfIsolated'] :
      col = getattr( process, item+full )
      col.deltaBetaIsolationValueMap = cms.InputTag( lep+'PFIsoValuePU03' + pf )
      col.isolationValueMapsCharged  = cms.VInputTag( cms.InputTag( lep+'PFIsoValueCharged03' + pf ) )
      col.isolationValueMapsNeutral  = cms.VInputTag( cms.InputTag( lep+'PFIsoValueNeutral03' + pf ),
                                                      cms.InputTag( lep+'PFIsoValueGamma03' + pf ) )
    val = getattr( process, 'pat'+full ).isolationValues
    val.pfNeutralHadrons   = cms.InputTag( lep+'PFIsoValueNeutral03' + pf )
    val.pfChargedAll       = cms.InputTag( lep+'PFIsoValueChargedAll03' + pf )
    val.pfPUChargedHadrons = cms.InputTag( lep+'PFIsoValuePU03' + pf )
    val.pfPhotons          = cms.InputTag( lep+'PFIsoValueGamma03' + pf )
    val.pfChargedHadrons   = cms.InputTag( lep+'PFIsoValueCharged03' + pf )

# PAT configuration
### Top Reference selection
muonCuts = ['isPFMuon',                                                                      # general reconstruction property
            '(isGlobalMuon || isTrackerMuon)',                                               # general reconstruction property
            'pt > 10.',                                                                      # transverse momentum
            'abs(eta) < 2.5',                                                                # pseudo-rapisity range
            '(chargedHadronIso+neutralHadronIso+photonIso-0.5*puChargedHadronIso)/pt < 0.2'] # relative isolation w/ Delta beta corrections (factor 0.5)

electronCuts = ['pt > 20.',                                                                              # transverse energy
                'abs(eta) < 2.5',                                                                        # pseudo-rapisity range
                'electronID("mvaTrigV0") > 0.',                                                          # MVA electrons ID
                '(chargedHadronIso+max(0.,neutralHadronIso)+photonIso-0.5*puChargedHadronIso)/et < 0.2'] # relative isolation with Delta beta corrections
electronIDSources = cms.PSet(**dict([(i,cms.InputTag(i)) for i in ['mvaTrigV0','mvaNonTrigV0']]))

# Careful! these jet cuts affect the typeI met corrections
jetCuts = ['abs(eta) < 2.5',
           'pt > 15.',
           # PF jet ID:
           'numberOfDaughters > 1',
           'neutralHadronEnergyFraction < 0.99',
           'neutralEmEnergyFraction < 0.99',
           '(chargedEmEnergyFraction < 0.99   || abs(eta) >= 2.4)',
           '(chargedHadronEnergyFraction > 0. || abs(eta) >= 2.4)',
           '(chargedMultiplicity > 0          || abs(eta) >= 2.4)']
btags = ['combinedSecondaryVertex','jetProbability']
taginfos = ["impactParameter","secondaryVertex"]

for mod,attr,val in [('patMuons','usePV',False),     # use beam spot rather than PV, which is necessary for 'dB' cut
                     ('patMuons','embedTrack',True), # embedded track needed for muon ID cuts
                     ('patElectrons','electronIDSources',electronIDSources),
                     ('patJets','discriminatorSources',[cms.InputTag(tag+'BJetTagsAOD'+postfix) for tag in btags]),
                     ('patJets','tagInfoSources',[cms.InputTag(tag+'TagInfosAOD'+postfix) for tag in taginfos]),
                     ('patJets','addTagInfos',True),
                     ('selectedPatMuons','cut', ' && '.join(muonCuts)),
                     ('selectedPatElectrons','cut', ' && '.join(electronCuts)),
                     ('selectedPatJets','cut', ' && '.join(jetCuts)),
                     ] : setattr( getattr( process, mod+postfix), attr, val )
# Drop cruft
nothanks = [mod for mod in set(str(patSeq).split('+'))
            if ( any( keyword in mod for keyword in ['count','Legacy','pfJetsPiZeros','ak7','soft','iterativeCone',
                                                     'tauIsoDeposit','tauGenJet','hpsPFTau','tauMatch','pfTau','hpsSelection',
                                                     'photonMatch','phPFIso','pfIsolatedPhotons','pfCandsNotInJet','pfCandMETcorr','Negative','ToVertex','particleFlowDisplacedVertex',
                                                     'ype2Corr','ype0','patType1p2CorrectedPFMet','atCandidateSummaryTR','atPFParticles'] ) or
                 ( 'JetTagsAOD' in mod and not any( (btag+'B') in mod for btag in btags ) ) or
                 ( 'TagInfosAOD' in mod and not any( (info+'TagInfosAOD') in mod for info in taginfos ) ) or
                 ( 'elPFIsoValue' in mod and ('04' in mod or 'NoPFId' in mod ) ) or
                 ( 'muPFIsoValue' in mod and ('03' in mod))) ]
for mod in nothanks : patSeq.remove( getattr(process, mod) )

# MVA electron ID
process.load( "EGamma.EGammaAnalysisTools.electronIdMVAProducer_cfi" )
process.eidMVASequence = cms.Sequence( process.mvaTrigV0 + process.mvaNonTrigV0 )

# Scheduling
process.p = cms.Path( reduce(operator.add, [getattr(process, item) for item in ['goodOfflinePrimaryVertices',
                                                                                'eventCleaning',
                                                                                'eidMVASequence',
                                                                                'patPF2PATSequence'+postfix]]))
schedule = cms.Schedule( process.p )

# dump config
mods = str(patSeq).split('+')
mods = [mod for i,mod in enumerate(mods) if not mods[:i].count(mod)]
print '\n'.join(mods)
with open('configInfo.txt','w') as F :
  for mod in mods :
    print >>F, mod, getattr(process, mod).dumpPython()
