export RELEASE=CMSSW_5_3_13
scram project CMSSW $RELEASE
cd $RELEASE/src
eval `scram runtime -sh`
export CVSROOT=:ext:bbetchar@lxplus.cern.ch:/afs/cern.ch/user/c/cvscmssw/public/CMSSW

git cms-addpkg EgammaAnalysis/ElectronTools
cd EgammaAnalysis/ElectronTools/data/
cat download.url | xargs wget
cd -

cvs co -r V00-00-13 RecoMET/METFilters
cvs co -r V00-00-08 RecoMET/METAnalyzers
cvs co -r V00-11-17 DPGAnalysis/SiStripTools

git clone --branch V00-04-02 https://github.com/betchart/TopRefTuple.git TopQuarkAnalysis/TopRefTuple

### git cms-addpkg EgammaAnalysis/ElectronTools CMSSW_5_3_13
