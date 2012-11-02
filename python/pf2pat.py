import operator,sys,os
from FWCore.ParameterSet import Config as cms
from PhysicsTools.PatAlgos.tools import coreTools,pfTools

def tags(stuff) :
    return ( cms.InputTag(stuff) if type(stuff)!=list else
             cms.VInputTag([tags(item) for item in stuff]) )

class TopRefPF2PAT(object) :
    '''Implement the Top Reference configuration of PF2PAT.

    https://twiki.cern.ch/twiki/bin/viewauth/CMS/TWikiTopRefEventSel
    '''

    def __init__(self, process, options) :
        self.stdout = sys.stdout
        sys.stdout = open(os.devnull, 'w')
        print >>self.stdout, "usePF2PAT() output suppressed."

        self.before = set(dir(process))
        process.load( "PhysicsTools.PatAlgos.patSequences_cff" )
        pfTools.usePF2PAT( process,
                           runPF2PAT = True,
                           postfix = options.postfix,
                           runOnMC = not options.isData,
                           pvCollection = tags( 'goodOfflinePrimaryVertices' ),
                           typeIMetCorrections = True,
                           jetAlgo = 'AK5',
                           jetCorrections = ('AK5PFchs', ['L1FastJet','L2Relative','L3Absolute','L2L3Residual'][:None if options.isData else -1] )
                           )
        getattr( process, 'pfPileUpIso'+options.postfix).checkClosestZVertex = True
        if options.isData: coreTools.runOnData( process, names = [ 'PFAll' ], postfix = options.postfix )
        coreTools.removeSpecificPATObjects( process, names = [ 'Photons', 'Taus' ], postfix = options.postfix )

        self.process = process
        self.options = options
        self.fix = options.postfix
        self.patSeq = getattr(process, 'patPF2PATSequence' + self.fix)
        self.btags = ['combinedSecondaryVertex','jetProbability']
        self.taginfos = ["impactParameter","secondaryVertex"]

        self.configTopProjections()
        self.configPfLepton('el', iso3 = True)
        self.configPfLepton('mu', iso3 = False)
        self.configPatMuons()
        self.configPatElectrons()
        self.configPatJets()
        self.removeCruft()
        sys.stdout = self.stdout
        
    def attr(self, item) : return getattr(self.process, item)
    def show(self, item) : print >>self.stdout, item, '=', self.attr(item).dumpPython()

    def configTopProjections(self) :
        '''Enable TopProjections except for Taus (do not remove tau cands from jet collection).'''

        for key,val in {'PileUp':1,'Muon':1,'Electron':1,'Jet':1,'Tau':0}.items() :
            self.attr('pfNo'+key+self.fix).enable = bool(val)

        for lep in ['Muon','Electron'] :
            sub = (lep,self.fix)
            pfNo = self.attr('pfNo%s%s'%sub)
            pfNo.topCollection = 'selectedPat%ss%s'%sub
            mods = [ self.attr( item) for item in [ lep.lower() + 'Match'+self.fix,
                                                    'pat%ss%s'%sub,
                                                    'selectedPat%ss%s'%sub]][int(self.options.isData):]
            for mod in mods : self.patSeq.remove(mod)
            self.patSeq.replace( pfNo, reduce(operator.mul, mods) * pfNo )
            pfNo.verbose = True
            self.show('pfNo%s%s'%sub)
        return

    def configPfLepton(self, lep, iso3) :
        full = {'el':'Electrons','mu':'Muons'}[lep] + self.fix
        iso = self.attr('pfIsolated'+full)
        iso.isolationCut = 0.2
        iso.doDeltaBetaCorrection = True
        iso.deltaBetaFactor = -0.5
        if iso3:
            pf = {'el':'PFId','mu':''}[lep] + self.fix
            for item in ['pf','pfIsolated'] :
                col = self.attr( item+full )
                col.deltaBetaIsolationValueMap = tags( lep+'PFIsoValuePU03'+pf )
                col.isolationValueMapsCharged  = tags( [lep+'PFIsoValueCharged03'+pf] )
                col.isolationValueMapsNeutral  = tags( [ lep+'PFIsoValueNeutral03'+pf, lep+'PFIsoValueGamma03'+pf] )
            val = self.attr( 'pat'+full ).isolationValues
            val.pfNeutralHadrons   = tags( lep+'PFIsoValueNeutral03' + pf )
            val.pfChargedAll       = tags( lep+'PFIsoValueChargedAll03' + pf )
            val.pfPUChargedHadrons = tags( lep+'PFIsoValuePU03' + pf )
            val.pfPhotons          = tags( lep+'PFIsoValueGamma03' + pf )
            val.pfChargedHadrons   = tags( lep+'PFIsoValueCharged03' + pf )
        return

    def configPatMuons(self) :
        muonCuts = ['isPFMuon',                                                                      # general reconstruction property
                    '(isGlobalMuon || isTrackerMuon)',                                               # general reconstruction property
                    'pt > 10.',                                                                      # transverse momentum
                    'abs(eta) < 2.5',                                                                # pseudo-rapisity range
                    '(chargedHadronIso+neutralHadronIso+photonIso-0.5*puChargedHadronIso)/pt < 0.20']# relative isolation w/ Delta beta corrections (factor 0.5)

        for mod,attr,val in [('patMuons','usePV',False),     # use beam spot rather than PV, which is necessary for 'dB' cut
                             ('patMuons','embedTrack',True), # embedded track needed for muon ID cuts
                             ('selectedPatMuons','cut', ' && '.join(muonCuts)),
                             ] : setattr( self.attr(mod+self.fix), attr, val )
        self.show('selectedPatMuons'+self.fix)
        return

    def configPatElectrons(self) :
        electronIDSources = cms.PSet(**dict([(i,tags(i)) for i in ['mvaTrigV0','mvaNonTrigV0']]))
        electronCuts = ['pt > 20.',                                                                              # transverse energy
                        'abs(eta) < 2.5',                                                                        # pseudo-rapidity range
                        'electronID("mvaTrigV0") > 0.',                                                          # MVA electrons ID
                        '(chargedHadronIso+max(0.,neutralHadronIso)+photonIso-0.5*puChargedHadronIso)/et < 0.15']# relative isolation with Delta beta corrections

        for mod,attr,val in [('patElectrons','electronIDSources',electronIDSources),
                             ('selectedPatElectrons','cut', ' && '.join(electronCuts)),
                             ] : setattr( self.attr( mod+self.fix), attr, val )
        self.show('selectedPatElectrons'+self.fix)
        return

    def configPatJets(self) :
        # Careful! these jet cuts affect the typeI met corrections
        jetCuts = ['abs(eta) < 2.5',
                   'pt > 15.',
                   # PF jet ID:
                   'numberOfDaughters > 1',
                   'neutralHadronEnergyFraction < 0.99',
                   'neutralEmEnergyFraction < 0.99',
                   '(chargedEmEnergyFraction < 0.99   || abs(eta) >= 2.4)',
                   '(chargedHadronEnergyFraction > 0. || abs(eta) >= 2.4)',
                   '(chargedMultiplicity > 0          || abs(eta) >= 2.4)']

        for mod,attr,val in [('patJets','discriminatorSources',[tags(tag+'BJetTagsAOD'+self.fix) for tag in self.btags]),
                             ('patJets','tagInfoSources',[tags(tag+'TagInfosAOD'+self.fix) for tag in self.taginfos]),
                             ('patJets','addTagInfos',True),
                             ('selectedPatJets','cut', ' && '.join(jetCuts)),
                             ] : setattr( self.attr(mod+self.fix), attr, val )
        self.show('selectedPatJets'+self.fix)
        return


    def removeCruft(self) :
        nothanks = [mod for mod in set(str(self.patSeq).split('+'))
                    if ( any( keyword in mod for keyword in ['count','Legacy','pfJetsPiZeros','ak7','soft','iterativeCone',
                                                             'tauIsoDeposit','tauGenJet','hpsPFTau','tauMatch','pfTau','hpsSelection',
                                                             'photonMatch','phPFIso','pfIsolatedPhotons','pfCandsNotInJet','pfCandMETcorr',
                                                             'Negative','ToVertex','particleFlowDisplacedVertex',
                                                             'ype2Corr','ype0','patType1p2CorrectedPFMet','atCandidateSummaryTR','atPFParticles'] ) or
                         ( 'JetTagsAOD' in mod and not any( (btag+'B') in mod for btag in self.btags ) ) or
                         ( 'TagInfosAOD' in mod and not any( (info+'TagInfosAOD') in mod for info in self.taginfos ) ) or
                         ( 'elPFIsoValue' in mod and ('04' in mod or 'NoPFId' in mod ) ) or
                         ( 'muPFIsoValue' in mod and ('03' in mod))) ]
        for mod in nothanks : self.patSeq.remove( self.attr(mod) )

        nothanks = ['ic5','kt4','kt6','JPT','ak7','ak5Calo']
        now = set(dir(self.process))
        for item in now - self.before - set(str(self.patSeq).split('+')) :
            if any(part in item for part in nothanks) :
                delattr(self.process,item)
        return
