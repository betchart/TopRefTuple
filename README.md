# TopRefTuple

CMSSW usercode for producing ntuples compatible with [TOP PAG reference selection](https://twiki.cern.ch/twiki/bin/viewauth/CMS/TWikiTopRefEventSel)

## Quick Start

1. On a computer with access to CMSSW environment, follow [scripts/recipe.sh](https://github.com/betchart/TopRefTuple/blob/master/scripts/recipe.sh)
2. Set environment variables and compile:
```bash
  cd CMSSW_X_X_X/src
  cmsenv
  scram b -j<N>
```
3. Make the reference ntuple:
```bash
  cmsRun test/config.py
```
