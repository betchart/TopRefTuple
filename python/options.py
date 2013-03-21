from FWCore.ParameterSet.VarParsing import VarParsing as VP
from PhysicsTools.PatAlgos.tools.cmsswVersionTools import pickRelValInputFiles

def options() :
    options = VP('standard')
    options.output = "topTuple.root"
    options.maxEvents = 100
    
    options.register('isData', default = True, mytype = VP.varType.bool)
    options.register('isFastSim', default = False, mytype = VP.varType.bool)
    options.register('quiet', default = True, mytype = VP.varType.bool)
    options.register('requireLepton', default = True, mytype = VP.varType.bool)
    options.register('globalTag', mytype = VP.varType.string )
    options.register('postfix','TR', mytype = VP.varType.string )
    options.register('btags', mytype = VP.varType.string, mult = VP.multiplicity.list )
    options.register('doElectronEA', default = True, mytype = VP.varType.bool)
    options.parseArguments()
    options._tagOrder =[]

    defaultGT = ('GR_R_53_V18' if options.isData else 'START53_V15')

    sync53 = '/store/mc/Summer12_DR53X/TTJets_MassiveBinDECAY_TuneZ2star_8TeV-madgraph-tauola/AODSIM/PU_S10_START53_V7A-v1/0000/FE4C2F81-D0E1-E111-9080-0030487E0A2D.root'
    defaultFiles = [sync53] if not options.isData else pickRelValInputFiles( cmsswVersion = 'CMSSW_5_3_6',
                                                                             dataTier     = 'RECO',
                                                                             relVal       = 'SingleMu',
                                                                             globalTag    = 'GR_R_53_V15_RelVal_mu2012A',
                                                                             numberOfFiles = 10
                                                                             )
    
    options.files = options.files if options.files else defaultFiles
    if not options.globalTag : options.globalTag = defaultGT

    if not options.quiet : print options
    options.btags = ['combinedSecondaryVertex','jetProbability']
    return options
