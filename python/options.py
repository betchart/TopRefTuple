from FWCore.ParameterSet.VarParsing import VarParsing as VP
from PhysicsTools.PatAlgos.tools.cmsswVersionTools import pickRelValInputFiles

def options() :
    options = VP('standard')
    options.output = "topTuple.root"
    options.maxEvents = 100
    
    options.register('isData', default = True, mytype = VP.varType.bool)
    options.register('quiet', default = True, mytype = VP.varType.bool)
    options.register('requireLepton', default = True, mytype = VP.varType.bool)
    options.register('globalTag', mytype = VP.varType.string )
    options.register('postfix','TR', mytype = VP.varType.string )
    options.parseArguments()
    options._tagOrder =[]

    defaultGT = ('GR_R_53_V15' if options.isData else 'START53_V13')

    defaultFiles = ( pickRelValInputFiles( cmsswVersion = 'CMSSW_5_3_4_cand1',
                                           dataTier     = 'RECO' if options.isData else 'AODSIM',
                                           relVal       = 'SingleMu' if options.isData else 'RelValProdTTbar',
                                           globalTag    = ( options.globalTag if options.globalTag else
                                                            'GR_R_53_V12_RelVal_mu2012A' if options.isData else
                                                            'START53_V10'),
                                           maxVersions  = 1 ) )
    
    options.files = options.files if options.files else defaultFiles[:10]
    if not options.globalTag : options.globalTag = defaultGT

    if not options.quiet : print options
    return options
