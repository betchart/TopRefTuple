cmsrel CMSSW_5_3_5
cd CMSSW_5_3_5/src
cmsenv
cvs co -r V00-00-08    RecoMET/METAnalyzers
cvs co -r V15-02-06    RecoParticleFlow/PFProducer
cvs co              -d TopQuarkAnalysis/TopRefTuple UserCode/Betchart/TopRefTuple
cvs co -r V00-00-13 -d EGamma/EGammaAnalysisTools UserCode/EGamma/EGammaAnalysisTools
cd EGamma/EGammaAnalysisTools/data
cat download.url | xargs wget
cd -
