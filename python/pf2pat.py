import operator,sys,os
from FWCore.ParameterSet import Config as cms
from PhysicsTools.PatAlgos.tools import coreTools,pfTools

def tags(stuff) :
    return ( cms.InputTag(stuff) if type(stuff)!=list else
             cms.VInputTag([tags(item) for item in stuff]) )

class TopRefPF2PAT(object) :
    '''Implement the Top Reference configuration of PF2PAT.

    https://twiki.cern.ch/twiki/bin/viewauth/CMS/TWikiTopRefEventSel
    Modify the PF selections to match the PAT selections, for consistent TopProjection cross-cleaning.
    '''

    def __init__(self, process, options) :
        self.stdout = sys.stdout
        sys.stdout = open(os.devnull, 'w')
        print >>self.stdout, "usePF2PAT() output suppressed. (%s)"%__file__

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

        self.isoValues = {'el':0.15,'mu':0.20}
        self.eleID = 'mvaTrigV0'
        self.minEleID = 0.
        self.dBFactor = -0.5
        self.cuts = {'el': ['abs(eta)<2.5',
                            'pt>20.',
                            'gsfTrackRef.isNonnull',
                            'gsfTrackRef.trackerExpectedHitsInner.numberOfLostHits<2',
                            '(chargedHadronIso+max(0.,neutralHadronIso+photonIso%+f*puChargedHadronIso))/pt < %f'%(self.dBFactor, self.isoValues['el']),
                            'electronID("%s") > %f'%(self.eleID,self.minEleID),
                            ],
                     'mu' :['abs(eta)<2.5',
                            'pt>10.',
                            '(chargedHadronIso+neutralHadronIso+photonIso%+f*puChargedHadronIso)/pt < %f'%(self.dBFactor, self.isoValues['mu']),
                            '(isPFMuon && (isGlobalMuon || isTrackerMuon) )',
                            ],
                     'jet' : ['abs(eta)<2.5', # Careful! these jet cuts affect the typeI met corrections
                              'pt > 15.',
                              # PF jet ID:
                              'numberOfDaughters > 1',
                              'neutralHadronEnergyFraction < 0.99',
                              'neutralEmEnergyFraction < 0.99',
                              '(chargedEmEnergyFraction < 0.99   || abs(eta) >= 2.4)',
                              '(chargedHadronEnergyFraction > 0. || abs(eta) >= 2.4)',
                              '(chargedMultiplicity > 0          || abs(eta) >= 2.4)']
                     }

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
        self.configLeptonFilter()
        self.removeCruft()
        sys.stdout = self.stdout
        
    def attr(self, item) : return getattr(self.process, item)
    def show(self, item) : print >> (sys.stdout if self.options.quiet else self.stdout), item, '=', self.attr(item).dumpPython() if hasattr(self.process, item) else 'Not Found.'

    def configTopProjections(self) :
        '''Enable TopProjections except for Taus (do not remove tau cands from jet collection).'''

        for key,val in {'PileUp':1,'Muon':1,'Electron':1,'Jet':1,'Tau':0}.items() :
            self.attr('pfNo'+key+self.fix).enable = bool(val)
        self.attr('pfNoElectron').verbose = True
        return

    def configPfLepton(self, lep, iso3) :
        full = {'el':'Electrons','mu':'Muons'}[lep] + self.fix
        iso = self.attr('pfIsolated'+full)
        iso.isolationCut = self.isoValues[lep]
        iso.doDeltaBetaCorrection = True
        iso.deltaBetaFactor = self.dBFactor
        sel = self.attr('pfSelected'+full)
        sel.cut = ' && '.join(self.cuts[lep][:-2])
        print >>self.stdout, ""
        if lep == 'el' :
            idName = 'pfIdentifiedElectrons'+self.fix
            id = cms.EDFilter("ElectronIDPFCandidateSelector",
                              src = cms.InputTag('pfElectronsFromVertex'+self.fix),
                              recoGsfElectrons = cms.InputTag("gsfElectrons"),
                              electronIdMap = cms.InputTag(self.eleID),
                              electronIdCut = cms.double(self.minEleID))
            setattr(self.process, idName, id )
            self.patSeq.replace( sel, id*sel)
            sel.src = idName
            self.show(idName)
        
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

        self.show('pfSelected'+full)
        self.show('pfIsolated'+full)
        return

    def configPatMuons(self) :
        for mod,attr,val in [('patMuons','usePV',False),     # use beam spot rather than PV, which is necessary for 'dB' cut
                             ('patMuons','embedTrack',True), # embedded track needed for muon ID cuts
                             ('selectedPatMuons','cut', ' && '.join(self.cuts['mu'])),
                             ] : setattr( self.attr(mod+self.fix), attr, val )
        self.show('selectedPatMuons'+self.fix)
        return

    def configPatElectrons(self) :
        electronIDSources = cms.PSet(**dict([(i,tags(i)) for i in [self.eleID]]))
        for mod,attr,val in [('patElectrons','electronIDSources', electronIDSources),
                             ('selectedPatElectrons','cut', ' && '.join(c for c in self.cuts['el'] if 'gsfTrackRef' not in c)),
                             ] : setattr( self.attr( mod+self.fix), attr, val )
        self.show('selectedPatElectrons'+self.fix)
        return

    def configPatJets(self) :
        for mod,attr,val in [('patJets','discriminatorSources',[tags(tag+'BJetTagsAOD'+self.fix) for tag in self.btags]),
                             ('patJets','tagInfoSources',[tags(tag+'TagInfosAOD'+self.fix) for tag in self.taginfos]),
                             ('patJets','addTagInfos',True),
                             ('selectedPatJets','cut', ' && '.join(self.cuts['jet'])),
                             ] : setattr( self.attr(mod+self.fix), attr, val )
        self.show('selectedPatJets'+self.fix)
        return

    def configLeptonFilter(self) :
        if not self.options.requireLepton : return
        muons20Name = 'pfIsolatedMuons20'+self.fix
        leptonsName = 'pfLeptons'+self.fix
        requireLeptonName = 'requireLepton'+self.fix

        muons20 = cms.EDFilter("GenericPFCandidateSelector", src = tags('pfIsolatedMuons'+self.fix), cut = cms.string('pt>20 && abs(eta) < 2.4'))
        leptons = cms.EDProducer("CandViewMerger", src = tags(['pfIsolated%s%s'%(lep,self.fix) for lep in ['Electrons','Muons20']]))
        requireLepton = cms.EDFilter("CandViewCountFilter", src = tags(leptonsName), minNumber = cms.uint32(1) )

        setattr(self.process, muons20Name, muons20)
        setattr(self.process, leptonsName, leptons)
        setattr(self.process, requireLeptonName, requireLepton)
        
        jets = self.attr('pfJets'+self.fix)
        self.patSeq.replace(jets, muons20*leptons*requireLepton*jets)

        self.show(muons20Name)
        self.show(leptonsName)
        self.show(requireLeptonName)
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
