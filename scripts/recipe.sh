cmsrel CMSSW_5_3_5
cd CMSSW_5_3_5/src
cmsenv
cvs co -r V07-00-03    TopQuarkAnalysis/Configuration
cvs co -r V07-01-04    TopQuarkAnalysis/TopSkimming
cvs co -r V06-07-13    TopQuarkAnalysis/TopTools
cvs co              -d TopQuarkAnalysis/TopRefTuple UserCode/Betchart/TopRefTuple
cvs co -r V00-00-08    RecoMET/METAnalyzers
cvs co -r V15-02-06    RecoParticleFlow/PFProducer
cvs co -r V00-00-13 -d EGamma/EGammaAnalysisTools UserCode/EGamma/EGammaAnalysisTools
cd EGamma/EGammaAnalysisTools/data
cat download.url | xargs wget
cd -
