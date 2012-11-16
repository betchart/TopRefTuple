RELEASE=CMSSW_5_3_6_patch1
cmsrel $RELEASE
cd $RELEASE/src
cmsenv
addpkg RecoMET/METAnalyzers  V00-00-08
addpkg RecoParticleFlow/PFProducer V15-02-06
cvs co              -d TopQuarkAnalysis/TopRefTuple UserCode/Betchart/TopRefTuple
cvs co -r V00-00-13 -d EGamma/EGammaAnalysisTools UserCode/EGamma/EGammaAnalysisTools
cd EGamma/EGammaAnalysisTools/data
cat download.url | xargs wget
cd -
