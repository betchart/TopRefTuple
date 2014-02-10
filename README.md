## TopRefTuple

CMSSW usercode for producing ntuples compatible with [TOP PAG reference selection](https://twiki.cern.ch/twiki/bin/viewauth/CMS/TWikiTopRefEventSel)

### Quick Start

1. On a computer with access to CMSSW environment, follow [scripts/recipe.sh](https://github.com/betchart/TopRefTuple/blob/master/scripts/recipe.sh)
2. Compile:
        ```scram b -j<N>```
3. Make the [test/config.py](https://github.com/betchart/TopRefTuple/blob/master/test/config.py) reference ntuple:
        ```cmsRun TopQuarkAnalysis/TopRefTuple/test/config.py```

