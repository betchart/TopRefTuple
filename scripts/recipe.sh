export RELEASE=CMSSW_5_3_13

SITE=`uname -n | awk -F'.' '{print $2}'`
if [ $SITE == "fnal" ]
    then
      echo 'This is FNAL'
      source /cvmfs/cms.cern.ch/cmsset_default.sh
      source /uscmst1/prod/grid/gLite_SL5.sh
fi

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

git clone --branch V00-04-05 https://github.com/betchart/TopRefTuple.git TopQuarkAnalysis/TopRefTuple

scram b -j 8
echo "\n\n\nCheck that everything built:"
scram b

### git cms-addpkg EgammaAnalysis/ElectronTools CMSSW_5_3_13 # this command is failing in an undocumented way, doesn't like the tag specification, and fails when called from python environment.

