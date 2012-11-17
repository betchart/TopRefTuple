RELEASE=CMSSW_5_3_6_patch1
scram project CMSSW $RELEASE
cd $RELEASE/src
eval `scram runtime -sh`
addpkg RecoMET/METAnalyzers  V00-00-08
addpkg RecoParticleFlow/PFProducer V15-02-06
cvs co -r V00-01-05 -d TopQuarkAnalysis/TopRefTuple UserCode/Betchart/TopRefTuple
cvs co -r V00-00-13 -d EGamma/EGammaAnalysisTools UserCode/EGamma/EGammaAnalysisTools
cd EGamma/EGammaAnalysisTools/data
cat download.url | xargs wget
cd -
