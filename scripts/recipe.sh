RELEASE=CMSSW_5_3_7_patch4
scram project CMSSW $RELEASE
cd $RELEASE/src
eval `scram runtime -sh`
addpkg RecoMET/METFilters V00-00-13
addpkg RecoMET/METAnalyzers  V00-00-08
addpkg DPGAnalysis/SiStripTools V00-11-17
cvs co -r V00-02-02 -d TopQuarkAnalysis/TopRefTuple UserCode/Betchart/TopRefTuple
cvs co -r V00-00-31-EA02 -d EGamma/EGammaAnalysisTools UserCode/EGamma/EGammaAnalysisTools
cd EGamma/EGammaAnalysisTools/data
cat download.url | xargs wget
cd -
