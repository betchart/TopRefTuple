from FWCore.ParameterSet import Config as cms

goodOfflinePrimaryVertices = cms.EDFilter( "PrimaryVertexObjectFilter", # checks for fake PVs automatically
                                           src = cms.InputTag( 'offlinePrimaryVertices' ),
                                           filter = cms.bool( True ), # at least 1 good vertex
                                           filterParams = cms.PSet( minNdof = cms.double( 4. ),
                                                                    maxZ    = cms.double( 24. ),
                                                                    maxRho  = cms.double( 2. )))
