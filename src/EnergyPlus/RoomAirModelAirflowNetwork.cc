// EnergyPlus, Copyright (c) 1996-2023, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/BaseboardElectric.hh>
#include <EnergyPlus/BaseboardRadiator.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataMoistureBalance.hh>
#include <EnergyPlus/DataMoistureBalanceEMPD.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSurfaceLists.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/ElectricBaseboardRadiator.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HWBaseboardRadiator.hh>
#include <EnergyPlus/HeatBalFiniteDiffManager.hh>
#include <EnergyPlus/HeatBalanceHAMTManager.hh>
#include <EnergyPlus/HighTempRadiantSystem.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/MoistureBalanceEMPDManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/RefrigeratedCase.hh>
#include <EnergyPlus/RoomAirModelAirflowNetwork.hh>
#include <EnergyPlus/SteamBaseboardRadiator.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneAirLoopEquipmentManager.hh>
#include <EnergyPlus/ZoneDehumidifier.hh>
#include <EnergyPlus/ZonePlenum.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace RoomAir {

    // MODULE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   November 2009
    //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease

    // PURPOSE OF THIS MODULE:
    // contains the RoomAir model portions of RoomAirflowNetwork modeling

    // METHODOLOGY EMPLOYED:
    // Interact with Surface HB, internal gain, HVAC system and Airflow Network Domains
    // Do heat and moisture balance calculations on roomair nodes.

    // Using/Aliasing
    using namespace DataHeatBalSurface;
    using namespace DataSurfaces;
    using namespace DataHeatBalance;

    // Object Data

    // Functions

    void SimRoomAirModelAirflowNetwork(EnergyPlusData &state, int const ZoneNum) // index number for the specified zone
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   January 2004/Aug 2005
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages RoomAirflowNetwork model simulation

        // METHODOLOGY EMPLOYED:
        // calls subroutines (LOL)

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(ZoneNum);

        // At this point, this should probably be an assert, not a ShowFatalError. This should have been trapped already.
        if (afnZoneInfo.roomAFNNodeNum == 0) {
            ShowFatalError(state,
                           format("SimRoomAirModelAirflowNetwork: Zone is not defined in the RoomAirModelAirflowNetwork model ={}",
                                  state.dataHeatBal->Zone(ZoneNum).Name));
        }

        auto &roomAFNNode = state.dataRoomAirflowNetModel->RAFN(afnZoneInfo.roomAFNNodeNum);
        roomAFNNode.ZoneNum = ZoneNum;

        // model control volume for each roomAir:node in the zone.
        for (int roomAirNode = 1; roomAirNode <= afnZoneInfo.NumOfAirNodes; ++roomAirNode) {

            roomAFNNode.RoomAirNode = roomAirNode;
            roomAFNNode.InitRoomAirModelAirflowNetwork(state, roomAirNode);
            roomAFNNode.CalcRoomAirModelAirflowNetwork(state, roomAirNode);
        }

        roomAFNNode.UpdateRoomAirModelAirflowNetwork(state);

    } // SimRoomAirModelAirflowNetwork

    //****************************************************

    void LoadPredictionRoomAirModelAirflowNetwork(EnergyPlusData &state,
                                                  int const ZoneNum,
                                                  int const RoomAirNode) // index number for the specified zone and node
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   June, 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Predict zone loads at a controlled node

        //////////// hoisted into namespace ////////////////////////////////////////////////
        // static bool OneTimeFlag_FindFirstLastPtr( true );  // one time setup flag //
        // state.dataRoomAirflowNetModel->LoadPredictionRoomAirModelAirflowNetworkOneTimeFlag
        ////////////////////////////////////////////////////////////////////////////////////
        if (state.dataRoomAirflowNetModel->LoadPredictionRoomAirModelAirflowNetworkOneTimeFlag) {
            state.dataRoomAirflowNetModel->RAFN.allocate(state.dataRoomAir->NumOfRoomAFNControl);
            state.dataRoomAirflowNetModel->LoadPredictionRoomAirModelAirflowNetworkOneTimeFlag = false;
        }

        auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(ZoneNum);

        if (afnZoneInfo.roomAFNNodeNum == 0) { // Again?
            ShowFatalError(state,
                           format("LoadPredictionRoomAirModelAirflowNetwork: Zone is not defined in the RoomAirModelAirflowNetwork model ={}",
                                  state.dataHeatBal->Zone(ZoneNum).Name));
        }
        auto &roomAFNNode = state.dataRoomAirflowNetModel->RAFN(afnZoneInfo.roomAFNNodeNum);
        roomAFNNode.ZoneNum = ZoneNum;
        roomAFNNode.InitRoomAirModelAirflowNetwork(state, RoomAirNode);

    } // LoadPredictionRoomAirModelAirflowNetwork

    //****************************************************

    void RAFNData::InitRoomAirModelAirflowNetwork(EnergyPlusData &state, int const RoomAirNode) // index number for the specified zone
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   November 2009
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 release

        // PURPOSE OF THIS SUBROUTINE:
        // Perform one-time checking and term calculations

        using InternalHeatGains::SumInternalLatentGainsByTypes;
        using Psychrometrics::PsyCpAirFnW;
        using Psychrometrics::PsyRhoAirFnPbTdbW;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        //////////// hoisted into namespace ////////////////////////////////////////////////
        // static bool MyOneTimeFlag( true );  // one time setup flag // InitRoomAirModelAirflowNetworkOneTimeFlag
        // static bool MyOneTimeFlagConf( true ); // one time setup flag for zone configuration // InitRoomAirModelAirflowNetworkOneTimeFlagConf
        // static bool MyEnvrnFlag( true ); // one time setup flag for zone configuration // InitRoomAirModelAirflowNetworkEnvrnFlag
        ////////////////////////////////////////////////////////////////////////////////////
        Real64 SumLinkMCp;
        Real64 SumLinkMCpT;
        int linkNum;
        Real64 LinkInTemp;
        Real64 CpAir;
        Real64 LinkInHumRat;
        Real64 LinkInMdot;
        Real64 SumLinkM;
        Real64 SumLinkMW;
        int LoopZone;
        int LoopAirNode;
        int NodeNum;
        int NodeIn;
        int Link;
        int IdNode;
        int EquipLoop;
        int MaxNodeNum;
        Array1D_bool NodeFound; // True if a node is found.
        int MaxEquipNum;
        Array1D_bool EquipFound;
        int ISum;
        bool ErrorsFound;
        int I;
        Array1D<Real64> SupplyFrac;
        Array1D<Real64> ReturnFrac;

        if (state.dataRoomAirflowNetModel->InitRoomAirModelAirflowNetworkOneTimeFlag) { // then do one - time setup inits

            // loop over all zones with RoomAirflowNetwork model
            for (int LoopZone = 1; LoopZone <= state.dataGlobal->NumOfZones; ++LoopZone) {
                if (!state.dataRoomAir->AFNZoneInfo(LoopZone).IsUsed) continue;
                int NumSurfs = 0;
                for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
                    auto &thisSpace = state.dataHeatBal->space(spaceNum);
                    NumSurfs += thisSpace.HTSurfaceLast - thisSpace.HTSurfaceFirst + 1;
                }

                for (LoopAirNode = 1; LoopAirNode <= state.dataRoomAir->AFNZoneInfo(LoopZone).NumOfAirNodes;
                     ++LoopAirNode) { // loop over all the modeled room air nodes
                    // calculate volume of air in node's control volume
                    state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).AirVolume =
                        state.dataHeatBal->Zone(LoopZone).Volume *
                        state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).ZoneVolumeFraction;

                    SetupOutputVariable(state,
                                        "RoomAirflowNetwork Node NonAirSystemResponse",
                                        OutputProcessor::Unit::W,
                                        state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).NonAirSystemResponse,
                                        OutputProcessor::SOVTimeStepType::HVAC,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).Name);
                    SetupOutputVariable(state,
                                        "RoomAirflowNetwork Node SysDepZoneLoadsLagged",
                                        OutputProcessor::Unit::W,
                                        state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).SysDepZoneLoadsLagged,
                                        OutputProcessor::SOVTimeStepType::HVAC,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).Name);
                    SetupOutputVariable(state,
                                        "RoomAirflowNetwork Node SumIntSensibleGain",
                                        OutputProcessor::Unit::W,
                                        state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).SumIntSensibleGain,
                                        OutputProcessor::SOVTimeStepType::HVAC,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).Name);
                    SetupOutputVariable(state,
                                        "RoomAirflowNetwork Node SumIntLatentGain",
                                        OutputProcessor::Unit::W,
                                        state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).SumIntLatentGain,
                                        OutputProcessor::SOVTimeStepType::HVAC,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).Name);
                }
            }
            state.dataRoomAirflowNetModel->InitRoomAirModelAirflowNetworkOneTimeFlag = false;
        }

        if (state.dataRoomAirflowNetModel->InitRoomAirModelAirflowNetworkOneTimeFlagConf) { // then do one - time setup inits
            if (allocated(state.dataZoneEquip->ZoneEquipConfig) && allocated(state.dataZoneEquip->ZoneEquipList)) {
                MaxNodeNum = 0;
                MaxEquipNum = 0;
                ErrorsFound = false;
                for (LoopZone = 1; LoopZone <= state.dataGlobal->NumOfZones; ++LoopZone) {
                    if (!state.dataHeatBal->Zone(LoopZone).IsControlled) continue;
                    MaxEquipNum = max(MaxEquipNum, state.dataZoneEquip->ZoneEquipList(LoopZone).NumOfEquipTypes);
                    MaxNodeNum = max(MaxNodeNum, state.dataZoneEquip->ZoneEquipConfig(LoopZone).NumInletNodes);
                }
                if (MaxNodeNum > 0) {
                    NodeFound.allocate(MaxNodeNum);
                    NodeFound = false;
                }
                if (MaxEquipNum > 0) {
                    EquipFound.allocate(MaxEquipNum);
                    SupplyFrac.allocate(MaxEquipNum);
                    ReturnFrac.allocate(MaxEquipNum);
                    EquipFound = false;
                    SupplyFrac = 0.0;
                    ReturnFrac = 0.0;
                }

                // loop over all zones with RoomAirflowNetwork model
                for (LoopZone = 1; LoopZone <= state.dataGlobal->NumOfZones; ++LoopZone) {
                    if (!state.dataHeatBal->Zone(LoopZone).IsControlled) continue;
                    if (!state.dataRoomAir->AFNZoneInfo(LoopZone).IsUsed) continue;
                    state.dataRoomAir->AFNZoneInfo(LoopZone).ActualZoneID = LoopZone;
                    SupplyFrac = 0.0;
                    ReturnFrac = 0.0;
                    NodeFound = false;
                    int numAirDistUnits = 0;

                    // find supply air node number
                    for (LoopAirNode = 1; LoopAirNode <= state.dataRoomAir->AFNZoneInfo(LoopZone).NumOfAirNodes;
                         ++LoopAirNode) { // loop over all the modeled room air nodes
                        for (EquipLoop = 1; EquipLoop <= state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).NumHVACs;
                             ++EquipLoop) { // loop over all the equip for a single room air node
                            // Check zone equipment name
                            for (I = 1; I <= state.dataZoneEquip->ZoneEquipList(LoopZone).NumOfEquipTypes; ++I) { // loop over all equip types
                                if (state.dataZoneEquip->ZoneEquipList(LoopZone).EquipType(I) == DataZoneEquipment::ZoneEquipType::AirDistributionUnit) {
                                    if (numAirDistUnits == 0)
                                        numAirDistUnits =
                                            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneHVAC:AirDistributionUnit");
                                    if (state.dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag) {
                                        ZoneAirLoopEquipmentManager::GetZoneAirLoopEquipment(state);
                                        state.dataZoneAirLoopEquipmentManager->GetAirDistUnitsFlag = false;
                                    }
                                    for (int AirDistUnitNum = 1; AirDistUnitNum <= numAirDistUnits; ++AirDistUnitNum) {
                                        if (state.dataZoneEquip->ZoneEquipList(LoopZone).EquipName(I) ==
                                            state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).Name) {
                                            if (state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).HVAC(EquipLoop).Name ==
                                                state.dataDefineEquipment->AirDistUnit(AirDistUnitNum).EquipName(1)) {
                                                if (state.dataRoomAir->AFNZoneInfo(LoopZone)
                                                        .Node(LoopAirNode)
                                                        .HVAC(EquipLoop)
                                                        .EquipConfigIndex == 0)
                                                    state.dataRoomAir->AFNZoneInfo(LoopZone)
                                                        .Node(LoopAirNode)
                                                        .HVAC(EquipLoop)
                                                        .EquipConfigIndex = I;
                                                if (!EquipFound(I)) EquipFound(I) = true;
                                                SupplyFrac(I) = SupplyFrac(I) + state.dataRoomAir->AFNZoneInfo(LoopZone)
                                                                                    .Node(LoopAirNode)
                                                                                    .HVAC(EquipLoop)
                                                                                    .SupplyFraction;
                                                ReturnFrac(I) = ReturnFrac(I) + state.dataRoomAir->AFNZoneInfo(LoopZone)
                                                                                    .Node(LoopAirNode)
                                                                                    .HVAC(EquipLoop)
                                                                                    .ReturnFraction;
                                            }
                                        }
                                    }
                                } else {
                                    if (UtilityRoutines::SameString(
                                            state.dataZoneEquip->ZoneEquipList(LoopZone).EquipName(I),
                                            state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).HVAC(EquipLoop).Name)) {
                                        if (state.dataRoomAir->AFNZoneInfo(LoopZone)
                                                .Node(LoopAirNode)
                                                .HVAC(EquipLoop)
                                                .EquipConfigIndex == 0)
                                            state.dataRoomAir->AFNZoneInfo(LoopZone)
                                                .Node(LoopAirNode)
                                                .HVAC(EquipLoop)
                                                .EquipConfigIndex = I;
                                        EquipFound(I) = true;
                                        SupplyFrac(I) = SupplyFrac(I) + state.dataRoomAir->AFNZoneInfo(LoopZone)
                                                                            .Node(LoopAirNode)
                                                                            .HVAC(EquipLoop)
                                                                            .SupplyFraction;
                                        ReturnFrac(I) = ReturnFrac(I) + state.dataRoomAir->AFNZoneInfo(LoopZone)
                                                                            .Node(LoopAirNode)
                                                                            .HVAC(EquipLoop)
                                                                            .ReturnFraction;
                                    }
                                }
                            }
                            for (IdNode = 1; IdNode <= state.dataLoopNodes->NumOfNodes; ++IdNode) { // loop over all nodes to find supply node ID
                                if (UtilityRoutines::SameString(state.dataLoopNodes->NodeID(IdNode),
                                                                state.dataRoomAir->AFNZoneInfo(LoopZone)
                                                                    .Node(LoopAirNode)
                                                                    .HVAC(EquipLoop)
                                                                    .SupplyNodeName)) {
                                    state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).HVAC(EquipLoop).SupNodeNum = IdNode;
                                    break;
                                }
                            }
                            // Verify inlet nodes
                            int inletNodeIndex = 0;
                            for (NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(LoopZone).NumInletNodes;
                                 ++NodeNum) { // loop over all supply inlet nodes in a single zone
                                // !Get node conditions
                                if (state.dataZoneEquip->ZoneEquipConfig(LoopZone).InletNode(NodeNum) == IdNode) {
                                    NodeFound(NodeNum) = true;
                                    inletNodeIndex = NodeNum;
                                    break;
                                }
                            }

                            if (state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).HVAC(EquipLoop).SupNodeNum > 0 &&
                                UtilityRoutines::SameString(
                                    state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).HVAC(EquipLoop).ReturnNodeName,
                                    "")) {
                                // Find matching return node
                                for (int retNode = 1; retNode <= state.dataZoneEquip->ZoneEquipConfig(LoopZone).NumReturnNodes; ++retNode) {
                                    if ((state.dataZoneEquip->ZoneEquipConfig(LoopZone).ReturnNodeInletNum(retNode) == inletNodeIndex) &&
                                        (state.dataZoneEquip->ZoneEquipConfig(LoopZone).ReturnNode(retNode) > 0)) {
                                        state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).HVAC(EquipLoop).RetNodeNum =
                                            state.dataZoneEquip->ZoneEquipConfig(LoopZone).ReturnNode(retNode); // Zone return node
                                        break;
                                    }
                                }
                            }

                            if (state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).HVAC(EquipLoop).RetNodeNum == 0) {
                                for (IdNode = 1; IdNode <= state.dataLoopNodes->NumOfNodes; ++IdNode) { // loop over all nodes to find return node ID
                                    if (UtilityRoutines::SameString(state.dataLoopNodes->NodeID(IdNode),
                                                                    state.dataRoomAir->AFNZoneInfo(LoopZone)
                                                                        .Node(LoopAirNode)
                                                                        .HVAC(EquipLoop)
                                                                        .ReturnNodeName)) {
                                        state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).HVAC(EquipLoop).RetNodeNum =
                                            IdNode;
                                        break;
                                    }
                                }
                            }
                            SetupOutputVariable(
                                state,
                                "RoomAirflowNetwork Node HVAC Supply Fraction",
                                OutputProcessor::Unit::None,
                                state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).HVAC(EquipLoop).SupplyFraction,
                                OutputProcessor::SOVTimeStepType::HVAC,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).HVAC(EquipLoop).Name);
                            SetupOutputVariable(
                                state,
                                "RoomAirflowNetwork Node HVAC Return Fraction",
                                OutputProcessor::Unit::None,
                                state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).HVAC(EquipLoop).ReturnFraction,
                                OutputProcessor::SOVTimeStepType::HVAC,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataRoomAir->AFNZoneInfo(LoopZone).Node(LoopAirNode).HVAC(EquipLoop).Name);
                        }
                    }
                    // Count node with.TRUE.
                    ISum = 0;
                    for (NodeNum = 1; NodeNum <= MaxNodeNum; ++NodeNum) { // loop over all supply inlet nodes in a single zone
                        if (NodeFound(NodeNum)) ISum = ISum + 1;
                    }
                    // Provide error messages with incorrect supplu node inputs
                    if (ISum != state.dataZoneEquip->ZoneEquipConfig(LoopZone).NumInletNodes) {
                        if (ISum > state.dataZoneEquip->ZoneEquipConfig(LoopZone).NumInletNodes) {
                            ShowSevereError(
                                state, "GetRoomAirflowNetworkData: The number of equipment listed in RoomAirflowNetwork:Node:HVACEquipment objects");
                            ShowContinueError(
                                state,
                                format("is greater than the number of zone configuration inlet nodes in {}", state.dataHeatBal->Zone(LoopZone).Name));
                            ShowContinueError(state, "Please check inputs of both objects.");
                            ErrorsFound = true;
                        } else {
                            ShowSevereError(
                                state, "GetRoomAirflowNetworkData: The number of equipment listed in RoomAirflowNetwork:Node:HVACEquipment objects");
                            ShowContinueError(
                                state,
                                format("is less than the number of zone configuration inlet nodes in {}", state.dataHeatBal->Zone(LoopZone).Name));
                            ShowContinueError(state, "Please check inputs of both objects.");
                            ErrorsFound = true;
                        }
                    }

                    // Check equipment names to ensure they are used in RoomAirflowNetwork : Node : HVACEquipment objects
                    for (I = 1; I <= state.dataZoneEquip->ZoneEquipList(LoopZone).NumOfEquipTypes; ++I) { // loop over all equip types
                        if (!EquipFound(I)) {
                            ShowSevereError(state,
                                            "GetRoomAirflowNetworkData: The equipment listed in ZoneEquipList is not found in the lsit of "
                                            "RoomAir:Node:AirflowNetwork:HVACEquipment objects =");
                            ShowContinueError(
                                state, format("{}. Please check inputs of both objects.", state.dataZoneEquip->ZoneEquipList(LoopZone).EquipName(I)));
                            ErrorsFound = true;
                        }
                    }

                    // Check fraction to ensure sum = 1.0 for every equipment
                    for (I = 1; I <= state.dataZoneEquip->ZoneEquipList(LoopZone).NumOfEquipTypes; ++I) { // loop over all equip types
                        if (std::abs(SupplyFrac(I) - 1.0) > 0.001) {
                            ShowSevereError(state, "GetRoomAirflowNetworkData: Invalid, zone supply fractions do not sum to 1.0");
                            ShowContinueError(state,
                                              format("Entered in {} defined in RoomAir:Node:AirflowNetwork:HVACEquipment",
                                                     state.dataZoneEquip->ZoneEquipList(LoopZone).EquipName(I)));
                            ShowContinueError(state,
                                              "The Fraction of supply fraction values across all the roomair nodes in a zone needs to sum to 1.0.");
                            ShowContinueError(state, format("The sum of fractions entered = {:.3R}", SupplyFrac(I)));
                            ErrorsFound = true;
                        }
                        if (std::abs(ReturnFrac(I) - 1.0) > 0.001) {
                            ShowSevereError(state, "GetRoomAirflowNetworkData: Invalid, zone return fractions do not sum to 1.0");
                            ShowContinueError(state,
                                              format("Entered in {} defined in RoomAir:Node:AirflowNetwork:HVACEquipment",
                                                     state.dataZoneEquip->ZoneEquipList(LoopZone).EquipName(I)));
                            ShowContinueError(state,
                                              "The Fraction of return fraction values across all the roomair nodes in a zone needs to sum to 1.0.");
                            ShowContinueError(state, format("The sum of fractions entered = {:.3R}", ReturnFrac(I)));
                            ErrorsFound = true;
                        }
                    }
                }
                state.dataRoomAirflowNetModel->InitRoomAirModelAirflowNetworkOneTimeFlagConf = false;
                if (allocated(NodeFound)) NodeFound.deallocate();
                if (ErrorsFound) {
                    ShowFatalError(state, "GetRoomAirflowNetworkData: Errors found getting air model input.  Program terminates.");
                }
            }
        } // End of InitRoomAirModelAirflowNetworkOneTimeFlagConf

        if (state.dataGlobal->BeginEnvrnFlag && state.dataRoomAirflowNetModel->InitRoomAirModelAirflowNetworkEnvrnFlag) {
            for (int LoopZone = 1; LoopZone <= state.dataGlobal->NumOfZones; ++LoopZone) {
                auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(LoopZone);
                if (!afnZoneInfo.IsUsed) continue;
                for (int LoopAirNode = 1; LoopAirNode <= afnZoneInfo.NumOfAirNodes; ++LoopAirNode) { // loop over all the modeled room air nodes
                    auto &afnNode = afnZoneInfo.Node(LoopAirNode);
                    afnNode.AirTemp = 23.0;
                    afnNode.AirTempX = {23.0, 23.0, 23.0, 23.0};
                    afnNode.AirTempDSX = {23.0, 23.0, 23.0, 23.0};
                    afnNode.AirTempT1 = 23.0;
                    afnNode.AirTempTX = 23.0;
                    afnNode.AirTempT2 = 23.0;

                    afnNode.HumRat = 0.0;
                    afnNode.HumRatX = {0.0, 0.0, 0.0, 0.0};
                    afnNode.HumRatDSX = {0.0, 0.0, 0.0, 0.0};
                    afnNode.HumRatT1 = 0.0;
                    afnNode.HumRatTX = 0.0;
                    afnNode.HumRatT2 = 0.0;

                    afnNode.SysDepZoneLoadsLagged = 0.0;
                    afnNode.SysDepZoneLoadsLaggedOld = 0.0;
                }
            }
            state.dataRoomAirflowNetModel->InitRoomAirModelAirflowNetworkEnvrnFlag = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataRoomAirflowNetModel->InitRoomAirModelAirflowNetworkEnvrnFlag = true;
        }

        // reuse code in ZoneTempPredictorCorrector for sensible components.
        CalcNodeSums(state, RoomAirNode);

        SumNonAirSystemResponseForNode(state, RoomAirNode);

        // latent gains.
        auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(ZoneNum);
        auto &afnNode = afnZoneInfo.Node(RoomAirNode);

        if (allocated(afnNode.SurfMask)) {
            CalcSurfaceMoistureSums(state, RoomAirNode, afnNode.SumHmAW, afnNode.SumHmARa, afnNode.SumHmARaW, afnNode.SurfMask);
        }

        // prepare AirflowNetwor flow rates and temperatures
        SumLinkMCp = 0.0;
        SumLinkMCpT = 0.0;
        SumLinkM = 0.0;
        SumLinkMW = 0.0;

        NodeNum = afnNode.AFNNodeID;
        if (NodeNum > 0) {
            for (linkNum = 1; linkNum <= afnNode.NumOfAirflowLinks; ++linkNum) {
                Link = afnNode.Link(linkNum).AFNSimuID;
                if (state.afn->AirflowNetworkLinkageData(Link).NodeNums[0] == NodeNum) { // incoming flow
                    NodeIn = state.afn->AirflowNetworkLinkageData(Link).NodeNums[1];
                    afnNode.Link(linkNum).TempIn = state.afn->AirflowNetworkNodeSimu(NodeIn).TZ;
                    afnNode.Link(linkNum).HumRatIn = state.afn->AirflowNetworkNodeSimu(NodeIn).WZ;
                    afnNode.Link(linkNum).MdotIn = state.afn->AirflowNetworkLinkSimu(Link).FLOW2;
                }
                if (state.afn->AirflowNetworkLinkageData(Link).NodeNums[1] == NodeNum) { // outgoing flow
                    NodeIn = state.afn->AirflowNetworkLinkageData(Link).NodeNums[0];
                    afnNode.Link(linkNum).TempIn = state.afn->AirflowNetworkNodeSimu(NodeIn).TZ;
                    afnNode.Link(linkNum).HumRatIn = state.afn->AirflowNetworkNodeSimu(NodeIn).WZ;
                    afnNode.Link(linkNum).MdotIn = state.afn->AirflowNetworkLinkSimu(Link).FLOW;
                }
            }

            for (linkNum = 1; linkNum <= afnNode.NumOfAirflowLinks; ++linkNum) {
                LinkInTemp = afnNode.Link(linkNum).TempIn;
                LinkInHumRat = afnNode.Link(linkNum).HumRatIn;
                LinkInMdot = afnNode.Link(linkNum).MdotIn;
                CpAir = PsyCpAirFnW(LinkInHumRat);
                SumLinkMCp = SumLinkMCp + CpAir * LinkInMdot;
                SumLinkMCpT = SumLinkMCpT + CpAir * LinkInMdot * LinkInTemp;
                SumLinkM = SumLinkM + LinkInMdot;
                SumLinkMW = SumLinkMW + LinkInMdot * LinkInHumRat;
            }
        }

        afnNode.SumLinkMCp = SumLinkMCp;
        afnNode.SumLinkMCpT = SumLinkMCpT;
        afnNode.SumLinkM = SumLinkM;
        afnNode.SumLinkMW = SumLinkMW;
        afnNode.SysDepZoneLoadsLagged = afnNode.SysDepZoneLoadsLaggedOld;

        afnNode.RhoAir =
            PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, afnNode.AirTemp, afnNode.HumRat, "InitRoomAirModelAirflowNetwork");

        afnNode.CpAir = PsyCpAirFnW(afnNode.HumRat);

    } // InitRoomAirModelAirflowNetwork

    //*****************************************************************************************

    void RAFNData::CalcRoomAirModelAirflowNetwork(EnergyPlusData &state, int const RoomAirNode) // index number for the specified zone and node
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   November 2009
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease
        //       RE-ENGINEERED

        // PURPOSE OF THIS SUBROUTINE:
        // calculate new values for temperature and humidity ratio for room air node

        // METHODOLOGY EMPLOYED:
        // take terms(updated in init routine) and use classic air balance equations
        // solved for state variables. Store results in structure.

        // REFERENCES:
        // na

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        using Psychrometrics::PsyHgAirFnWTdb;
        using Psychrometrics::PsyRhFnTdbWPb;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 H2OHtOfVap;
        Real64 HumRatTmp;
        std::array<Real64, 3> NodeTempX;
        std::array<Real64, 3> NodeHumRatX;
        Real64 TempDepCoef;
        Real64 TempIndCoef;
        Real64 AirCap;
        Real64 TempTmp;
        Real64 A;
        Real64 B;
        Real64 C;
        Real64 AirTempT1;
        Real64 HumRatT1;

        auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(ZoneNum);
        auto &afnNode = afnZoneInfo.Node(RoomAirNode);

        if (state.dataHVACGlobal->UseZoneTimeStepHistory) {
            NodeTempX[0] = afnNode.AirTempX[0];
            NodeTempX[1] = afnNode.AirTempX[1];
            NodeTempX[2] = afnNode.AirTempX[2];

            NodeHumRatX[0] = afnNode.HumRatX[0];
            NodeHumRatX[1] = afnNode.HumRatX[1];
            NodeHumRatX[2] = afnNode.HumRatX[2];
        } else { // use down - stepped history
            NodeTempX[0] = afnNode.AirTempDSX[0];
            NodeTempX[1] = afnNode.AirTempDSX[1];
            NodeTempX[2] = afnNode.AirTempDSX[2];

            NodeHumRatX[0] = afnNode.HumRatDSX[0];
            NodeHumRatX[1] = afnNode.HumRatDSX[1];
            NodeHumRatX[2] = afnNode.HumRatDSX[2];
        }

        if (state.dataHeatBal->ZoneAirSolutionAlgo != DataHeatBalance::SolutionAlgo::ThirdOrder) {
            AirTempT1 = afnNode.AirTempT1;
            HumRatT1 = afnNode.HumRatT1;
        }
        // solve for node drybulb temperature
        TempDepCoef = afnNode.SumHA + afnNode.SumLinkMCp + afnNode.SumSysMCp;
        TempIndCoef = afnNode.SumIntSensibleGain + afnNode.SumHATsurf - afnNode.SumHATref + afnNode.SumLinkMCpT +
                      afnNode.SumSysMCpT + afnNode.NonAirSystemResponse + afnNode.SysDepZoneLoadsLagged;
        AirCap =
            afnNode.AirVolume * state.dataHeatBal->Zone(ZoneNum).ZoneVolCapMultpSens * afnNode.RhoAir * afnNode.CpAir / TimeStepSysSec;

        if (state.dataHeatBal->ZoneAirSolutionAlgo == DataHeatBalance::SolutionAlgo::AnalyticalSolution) {
            if (TempDepCoef == 0.0) { // B=0
                TempTmp = AirTempT1 + TempIndCoef / AirCap;
            } else {
                TempTmp = (AirTempT1 - TempIndCoef / TempDepCoef) * std::exp(min(700.0, -TempDepCoef / AirCap)) + TempIndCoef / TempDepCoef;
            }
        } else if (state.dataHeatBal->ZoneAirSolutionAlgo == DataHeatBalance::SolutionAlgo::EulerMethod) {
            TempTmp = (AirCap * AirTempT1 + TempIndCoef) / (AirCap + TempDepCoef);
        } else {
            TempTmp = (TempIndCoef + AirCap * (3.0 * NodeTempX[0] - (3.0 / 2.0) * NodeTempX[1] + (1.0 / 3.0) * NodeTempX[2])) /
                      ((11.0 / 6.0) * AirCap + TempDepCoef);
        }

        afnNode.AirTemp = TempTmp;

        // solve for node humidity ratio using 3 algorithms
        H2OHtOfVap = PsyHgAirFnWTdb(afnNode.HumRat, afnNode.AirTemp);
        A = afnNode.SumLinkM + afnNode.SumHmARa + afnNode.SumSysM;
        B = (afnNode.SumIntLatentGain / H2OHtOfVap) + afnNode.SumSysMW + afnNode.SumLinkMW + afnNode.SumHmARaW;
        C = afnNode.RhoAir * afnNode.AirVolume * state.dataHeatBal->Zone(ZoneNum).ZoneVolCapMultpMoist / TimeStepSysSec;

        // Exact solution
        if (state.dataHeatBal->ZoneAirSolutionAlgo == DataHeatBalance::SolutionAlgo::AnalyticalSolution) {
            if (A == 0.0) { // B=0
                HumRatTmp = HumRatT1 + B / C;
            } else {
                HumRatTmp = (HumRatT1 - B / A) * std::exp(min(700., -A / C)) + B / A;
            }
        } else if (state.dataHeatBal->ZoneAirSolutionAlgo == DataHeatBalance::SolutionAlgo::EulerMethod) {
            HumRatTmp = (C * HumRatT1 + B) / (C + A);
        } else {
            HumRatTmp = (B + C * (3.0 * NodeHumRatX[0] - (3.0 / 2.0) * NodeHumRatX[1] + (1.0 / 3.0) * NodeHumRatX[2])) / ((11.0 / 6.0) * C + A);
        }

        afnNode.HumRat = HumRatTmp;

        afnNode.AirCap = AirCap;
        afnNode.AirHumRat = C;

        afnNode.RelHumidity = PsyRhFnTdbWPb(state, TempTmp, HumRatTmp, state.dataEnvrn->OutBaroPress, "CalcRoomAirModelAirflowNetwork") * 100.0;

    } // CalcRoomAirModelAirflowNetwork

    void RAFNData::UpdateRoomAirModelAirflowNetwork(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   November 2009
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease

        // PURPOSE OF THIS SUBROUTINE:
        // update variables

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int AirNodeNum; // nested node structure index
        int I;
        Real64 NodeMass;
        Real64 SumMass;
        Real64 SumMassT;
        Real64 SumMassW;
        int RetNodeNum;

        auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(ZoneNum);

        if (!afnZoneInfo.IsUsed) return;

        if (!state.dataGlobal->ZoneSizingCalc) SumSystemDepResponseForNode(state);

        AirNodeNum = afnZoneInfo.ControlAirNodeID;

        // Update return node conditions
        for (I = 1; I <= state.dataZoneEquip->ZoneEquipList(ZoneNum).NumOfEquipTypes; ++I) { // loop over all equip types
            SumMass = 0.0;
            SumMassT = 0.0;
            SumMassW = 0.0;
            for (int LoopAirNode = 1; LoopAirNode <= afnZoneInfo.NumOfAirNodes; ++LoopAirNode) { // loop over all the modeled room air nodes
                auto &afnNode = afnZoneInfo.Node(LoopAirNode);
                for (int EquipLoop = 1; EquipLoop <= afnNode.NumHVACs; ++EquipLoop) { // loop over all the equip for a single room air node
                    auto &afnHVAC = afnNode.HVAC(EquipLoop);
                    if (afnHVAC.EquipConfigIndex == I) {
                        if (afnHVAC.SupNodeNum > 0 && afnHVAC.RetNodeNum > 0) {
                            NodeMass = state.dataLoopNodes->Node(afnHVAC.SupNodeNum).MassFlowRate * afnHVAC.ReturnFraction;
                            SumMass += NodeMass;
                            SumMassT += NodeMass * afnNode.AirTemp;
                            SumMassW += NodeMass * afnNode.HumRat;
                            RetNodeNum = afnHVAC.RetNodeNum;
                        }
                    }
                }
            }
            if (SumMass > 0.0) {
                state.dataLoopNodes->Node(RetNodeNum).Temp = SumMassT / SumMass;
                state.dataLoopNodes->Node(RetNodeNum).HumRat = SumMassW / SumMass;
            }
        }
    } // UpdateRoomAirModelAirflowNetwork

    void RAFNData::CalcNodeSums(EnergyPlusData &state, int const RoomAirNodeNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   August 2009
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease
        //       RE - ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE :
        // This subroutine calculates the various sums that go into the zone heat balance
        // equation.This replaces the SUMC, SUMHA, and SUMHAT calculations that were
        // previously done in various places throughout the program.
        // The SumHAT portion of the code is reproduced in RadiantSystemHighTemp and
        // RadiantSystemLowTemp and should be updated accordingly.
        //
        // A reference temperature(Tref) is specified for use with the ceiling diffuser
        // convection correlation.A bogus value of Tref = -999.9 defaults to using
        // the zone air(i.e.outlet) temperature for the reference temperature.
        // If Tref is applied to all surfaces, SumHA = 0, and SumHATref /= 0.
        // If Tref is not used at all, SumHATref = 0, and SumHA /= 0.
        //

        // USE STATEMENTS:
        using InternalHeatGains::SumInternalConvectionGainsByIndices;
        using InternalHeatGains::SumInternalLatentGainsByIndices;
        using InternalHeatGains::SumReturnAirConvectionGainsByIndices;
        using InternalHeatGains::SumReturnAirConvectionGainsByTypes;
        using Psychrometrics::PsyCpAirFnW;
        using Psychrometrics::PsyRhoAirFnPbTdbW;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NodeNum;         // System node number
        Real64 NodeTemp;     // System node temperature
        Real64 NodeW;        // System node humidity ratio
        Real64 MassFlowRate; // System node mass flow rate
        int ZoneRetPlenumNum;
        int ZoneSupPlenumNum;
        bool ZoneRetPlenumAirFlag;
        bool ZoneSupPlenumAirFlag;
        Real64 CpAir;      // Specific heat of air
        Real64 HA;         //                     !Hc*Area
        Real64 Area;       //                   !Effective surface area
        Real64 RefAirTemp; //             !Reference air temperature for surface convection calculations
        Real64 ZoneMult;
        int ADUListIndex;
        int ADUNum;
        int ADUInNode;
        int ADUOutNode;
        Real64 SumIntGain; //             !node sum of convective internal gains
        Real64 SumHA;      // Zone sum of Hc*Area
        Real64 SumHATsurf; //             !Zone sum of Hc*Area*Tsurf
        Real64 SumHATref;  //              !Zone sum of Hc*Area*Tref, for ceiling diffuser convection correlation
        Real64 SumMCp;     //                !Zone sum of MassFlowRate*Cp
        Real64 SumMCpT;    //                !Zone sum of MassFlowRate*Cp*T
        Real64 SumSysMCp;  //              !Zone sum of air system MassFlowRate*Cp
        Real64 SumSysMCpT; //             !Zone sum of air system MassFlowRate*Cp*T
        Real64 SumSysM;    //                !Zone sum of air system MassFlowRate
        Real64 SumSysMW;   //               !Zone sum of air system MassFlowRate*W
        int EquipLoop;     //              !Index of equipment loop
        int Loop;          //                   !Index of RAFN node
        bool Found;        //
        Real64 SumLinkM;   //               !Zone sum of MassFlowRate from the AirflowNetwork model
        Real64 SumLinkMW;  //             !Zone sum of MassFlowRate*W from the AirflowNetwork model

        SumIntGain = 0.0;
        SumHA = 0.0;
        SumHATsurf = 0.0;
        SumHATref = 0.0;
        SumMCp = 0.0;
        SumMCpT = 0.0;
        SumSysMCp = 0.0;
        SumSysMCpT = 0.0;
        SumSysM = 0.0;
        SumSysMW = 0.0;
        SumLinkM = 0.0;
        SumLinkMW = 0.0;

        auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(ZoneNum);
        auto &afnNode = afnZoneInfo.Node(RoomAirNodeNum);
        // Sum all convective internal gains: SumIntGain
        afnNode.SumIntSensibleGain =
            SumInternalConvectionGainsByIndices(state,
                                                afnNode.NumIntGains, afnNode.intGainsDeviceSpaces, afnNode.IntGainsDeviceIndices, afnNode.IntGainsFractions);

        afnNode.SumIntLatentGain =
            SumInternalLatentGainsByIndices(state,
                                            afnNode.NumIntGains, afnNode.intGainsDeviceSpaces, afnNode.IntGainsDeviceIndices, afnNode.IntGainsFractions);
        // Add heat to return air if zonal system(no return air) or cycling system(return air frequently very low or zero)
        if (state.dataHeatBal->Zone(ZoneNum).NoHeatToReturnAir) {
            // *******************************************
            SumIntGain =
                SumReturnAirConvectionGainsByIndices(state,
                afnNode.NumIntGains, afnNode.intGainsDeviceSpaces, afnNode.IntGainsDeviceIndices, afnNode.IntGainsFractions);
            afnNode.SumIntSensibleGain += SumIntGain;
        }

        // Check to see if this is a controlled zone

        bool ControlledZoneAirFlag = state.dataHeatBal->Zone(ZoneNum).IsControlled;

        // Check to see if this is a plenum zone
        ZoneRetPlenumAirFlag = false;
        for (ZoneRetPlenumNum = 1; ZoneRetPlenumNum <= state.dataZonePlenum->NumZoneReturnPlenums; ++ZoneRetPlenumNum) {
            if (state.dataZonePlenum->ZoneRetPlenCond(ZoneRetPlenumNum).ActualZoneNum != ZoneNum) continue;
            ZoneRetPlenumAirFlag = true;
            break;
        } // ZoneRetPlenumNum
        ZoneSupPlenumAirFlag = false;
        for (ZoneSupPlenumNum = 1; ZoneSupPlenumNum <= state.dataZonePlenum->NumZoneSupplyPlenums; ++ZoneSupPlenumNum) {
            if (state.dataZonePlenum->ZoneSupPlenCond(ZoneSupPlenumNum).ActualZoneNum != ZoneNum) continue;
            ZoneSupPlenumAirFlag = true;
            break;
        } // ZoneSupPlenumNum

        // Plenum and controlled zones have a different set of inlet nodes which must be calculated.
        auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
        if (ControlledZoneAirFlag) {
            auto &thisZoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(ZoneNum);
            for (NodeNum = 1; NodeNum <= thisZoneEquipConfig.NumInletNodes; ++NodeNum) {
                // Get node conditions
                // this next block is of interest to irratic system loads... maybe nodes are not accurate at time of call ?
                // how can we tell ? predict step must be lagged ? correct step, systems have run.
                for (EquipLoop = 1; EquipLoop <= afnNode.NumHVACs;
                     ++EquipLoop) {
                    if (afnNode.HVAC(EquipLoop).SupNodeNum ==
                        thisZoneEquipConfig.InletNode(NodeNum)) {
                        NodeTemp = state.dataLoopNodes->Node(thisZoneEquipConfig.InletNode(NodeNum)).Temp;
                        NodeW = state.dataLoopNodes->Node(thisZoneEquipConfig.InletNode(NodeNum)).HumRat;
                        MassFlowRate = state.dataLoopNodes->Node(thisZoneEquipConfig.InletNode(NodeNum)).MassFlowRate *
                                       afnNode.HVAC(EquipLoop).SupplyFraction;
                        CpAir = PsyCpAirFnW(thisZoneHB.ZoneAirHumRat);
                        SumSysMCp += MassFlowRate * CpAir;
                        SumSysMCpT += MassFlowRate * CpAir * NodeTemp;
                        SumSysM += MassFlowRate;
                        SumSysMW += MassFlowRate * NodeW;
                    }
                } // EquipLoop
            }     // NodeNum
        } else if (ZoneRetPlenumAirFlag) {
            for (NodeNum = 1; NodeNum <= state.dataZonePlenum->ZoneRetPlenCond(ZoneRetPlenumNum).NumInletNodes; ++NodeNum) {
                // Get node conditions
                NodeTemp = state.dataLoopNodes->Node(state.dataZonePlenum->ZoneRetPlenCond(ZoneRetPlenumNum).InletNode(NodeNum)).Temp;
                MassFlowRate = state.dataLoopNodes->Node(state.dataZonePlenum->ZoneRetPlenCond(ZoneRetPlenumNum).InletNode(NodeNum)).MassFlowRate;
                CpAir = PsyCpAirFnW(thisZoneHB.ZoneAirHumRat);
                SumSysMCp += MassFlowRate * CpAir;
                SumSysMCpT += MassFlowRate * CpAir * NodeTemp;
            } // NodeNum
            // add in the leaks
            for (ADUListIndex = 1; ADUListIndex <= state.dataZonePlenum->ZoneRetPlenCond(ZoneRetPlenumNum).NumADUs; ++ADUListIndex) {
                ADUNum = state.dataZonePlenum->ZoneRetPlenCond(ZoneRetPlenumNum).ADUIndex(ADUListIndex);
                if (state.dataDefineEquipment->AirDistUnit(ADUNum).UpStreamLeak) {
                    ADUInNode = state.dataDefineEquipment->AirDistUnit(ADUNum).InletNodeNum;
                    NodeTemp = state.dataLoopNodes->Node(ADUInNode).Temp;
                    MassFlowRate = state.dataDefineEquipment->AirDistUnit(ADUNum).MassFlowRateUpStrLk;
                    CpAir = PsyCpAirFnW(thisZoneHB.ZoneAirHumRat);
                    SumSysMCp += MassFlowRate * CpAir;
                    SumSysMCpT += MassFlowRate * CpAir * NodeTemp;
                }
                if (state.dataDefineEquipment->AirDistUnit(ADUNum).DownStreamLeak) {
                    ADUOutNode = state.dataDefineEquipment->AirDistUnit(ADUNum).OutletNodeNum;
                    NodeTemp = state.dataLoopNodes->Node(ADUOutNode).Temp;
                    MassFlowRate = state.dataDefineEquipment->AirDistUnit(ADUNum).MassFlowRateDnStrLk;
                    CpAir = PsyCpAirFnW(thisZoneHB.ZoneAirHumRat);
                    SumSysMCp += MassFlowRate * CpAir;
                    SumSysMCpT += MassFlowRate * CpAir * NodeTemp;
                }
            } // ADUListIndex
        } else if (ZoneSupPlenumAirFlag) {
            // Get node conditions
            NodeTemp = state.dataLoopNodes->Node(state.dataZonePlenum->ZoneSupPlenCond(ZoneSupPlenumNum).InletNode).Temp;
            MassFlowRate = state.dataLoopNodes->Node(state.dataZonePlenum->ZoneSupPlenCond(ZoneSupPlenumNum).InletNode).MassFlowRate;
            CpAir = PsyCpAirFnW(thisZoneHB.ZoneAirHumRat);
            SumSysMCp += MassFlowRate * CpAir;
            SumSysMCpT += MassFlowRate * CpAir * NodeTemp;
        }

        ZoneMult = state.dataHeatBal->Zone(ZoneNum).Multiplier * state.dataHeatBal->Zone(ZoneNum).ListMultiplier;

        SumSysMCp = SumSysMCp / ZoneMult;
        SumSysMCpT = SumSysMCpT / ZoneMult;
        SumSysM = SumSysM / ZoneMult;
        SumSysMW = SumSysMW / ZoneMult;

        // Sum all surface convection : SumHA, SumHATsurf, SumHATref(and additional contributions to SumIntGain)
        // Modified by Gu to include assigned surfaces only shown in the surface lsit
        if (!afnNode.HasSurfacesAssigned) return;

        int surfCount = 0;
        for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
            auto &thisSpace = state.dataHeatBal->space(spaceNum);
            for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
                ++surfCount;
                if (afnZoneInfo.ControlAirNodeID == RoomAirNodeNum) {
                    Found = false;
                    for (Loop = 1; Loop <= afnZoneInfo.NumOfAirNodes; ++Loop) {
                        if (Loop != RoomAirNodeNum) {
                            if (afnZoneInfo.Node(Loop).SurfMask(surfCount)) {
                                Found = true;
                                break;
                            }
                        }
                    }
                    if (Found) continue;
                } else {
                    if (!afnNode.SurfMask(surfCount)) continue;
                }

                HA = 0.0;
                Area = state.dataSurface->Surface(SurfNum).Area; // For windows, this is the glazing area

                if (state.dataSurface->Surface(SurfNum).Class == DataSurfaces::SurfaceClass::Window) {

                    // Add to the convective internal gains
                    if (ANY_INTERIOR_SHADE_BLIND(state.dataSurface->SurfWinShadingFlag(SurfNum))) {
                        // The shade area covers the area of the glazing plus the area of the dividers.
                        Area += state.dataSurface->SurfWinDividerArea(SurfNum);
                        SumIntGain += state.dataSurface->SurfWinDividerHeatGain(SurfNum);
                    }

                    // Convective heat gain from natural convection in gap between glass and interior shade or blind
                    if (ANY_INTERIOR_SHADE_BLIND(state.dataSurface->SurfWinShadingFlag(SurfNum)))
                        SumIntGain += state.dataSurface->SurfWinConvHeatFlowNatural(SurfNum);

                    // Convective heat gain from airflow window
                    if (state.dataSurface->SurfWinAirflowThisTS(SurfNum) > 0.0) {
                        SumIntGain += state.dataSurface->SurfWinConvHeatGainToZoneAir(SurfNum);
                        if (state.dataHeatBal->Zone(ZoneNum).NoHeatToReturnAir) {
                            SumIntGain += state.dataSurface->SurfWinRetHeatGainToZoneAir(SurfNum);
                            state.dataSurface->SurfWinHeatGain(SurfNum) += state.dataSurface->SurfWinRetHeatGainToZoneAir(SurfNum);
                            if (state.dataSurface->SurfWinHeatGain(SurfNum) >= 0.0) {
                                state.dataSurface->SurfWinHeatGainRep(SurfNum) = state.dataSurface->SurfWinHeatGain(SurfNum);
                                state.dataSurface->SurfWinHeatGainRepEnergy(SurfNum) =
                                    state.dataSurface->SurfWinHeatGainRep(SurfNum) * state.dataGlobal->TimeStepZone * Constant::SecInHour;
                            } else {
                                state.dataSurface->SurfWinHeatLossRep(SurfNum) = -state.dataSurface->SurfWinHeatGain(SurfNum);
                                state.dataSurface->SurfWinHeatLossRepEnergy(SurfNum) =
                                    state.dataSurface->SurfWinHeatLossRep(SurfNum) * state.dataGlobal->TimeStepZone * Constant::SecInHour;
                            }
                            state.dataSurface->SurfWinHeatTransferRepEnergy(SurfNum) =
                                state.dataSurface->SurfWinHeatGain(SurfNum) * state.dataGlobal->TimeStepZone * Constant::SecInHour;
                        }
                    }

                    // Add to the surface convection sums
                    if (state.dataSurface->SurfWinFrameArea(SurfNum) > 0.0) {
                        // Window frame contribution
                        SumHATsurf += state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataSurface->SurfWinFrameArea(SurfNum) *
                                      (1.0 + state.dataSurface->SurfWinProjCorrFrIn(SurfNum)) * state.dataSurface->SurfWinFrameTempIn(SurfNum);
                        HA += state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataSurface->SurfWinFrameArea(SurfNum) *
                              (1.0 + state.dataSurface->SurfWinProjCorrFrIn(SurfNum));
                    }

                    if (state.dataSurface->SurfWinDividerArea(SurfNum) > 0.0 &&
                        !ANY_INTERIOR_SHADE_BLIND(state.dataSurface->SurfWinShadingFlag(SurfNum))) {
                        // Window divider contribution(only from shade or blind for window with divider and interior shade or blind)
                        SumHATsurf += state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataSurface->SurfWinDividerArea(SurfNum) *
                                      (1.0 + 2.0 * state.dataSurface->SurfWinProjCorrDivIn(SurfNum)) *
                                      state.dataSurface->SurfWinDividerTempIn(SurfNum);
                        HA += state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataSurface->SurfWinDividerArea(SurfNum) *
                              (1.0 + 2.0 * state.dataSurface->SurfWinProjCorrDivIn(SurfNum));
                    }

                } // End of check if window

                HA = HA + state.dataHeatBalSurf->SurfHConvInt(SurfNum) * Area;
                SumHATsurf += state.dataHeatBalSurf->SurfHConvInt(SurfNum) * Area * state.dataHeatBalSurf->SurfTempInTmp(SurfNum);

                if (state.dataSurface->SurfTAirRef(SurfNum) == DataSurfaces::RefAirTemp::ZoneMeanAirTemp) {
                    // The zone air is the reference temperature(which is to be solved for in CorrectZoneAirTemp).
                    RefAirTemp = thisZoneHB.MAT;
                    SumHA += HA;
                } else if (state.dataSurface->SurfTAirRef(SurfNum) == DataSurfaces::RefAirTemp::AdjacentAirTemp) {
                    RefAirTemp = state.dataHeatBal->SurfTempEffBulkAir(SurfNum);
                    SumHATref += HA * RefAirTemp;
                } else if (state.dataSurface->SurfTAirRef(SurfNum) == DataSurfaces::RefAirTemp::ZoneSupplyAirTemp) {
                    // check whether this zone is a controlled zone or not
                    if (!ControlledZoneAirFlag) {
                        ShowFatalError(state,
                                       format("Zones must be controlled for Ceiling-Diffuser Convection model. No system serves zone {}",
                                              state.dataHeatBal->Zone(ZoneNum).Name));
                        return;
                    }
                    // determine supply air temperature as a weighted average of the inlet temperatures.
                    RefAirTemp = SumSysMCpT / SumSysMCp;
                    SumHATref += HA * RefAirTemp;
                } else {
                    RefAirTemp = thisZoneHB.MAT;
                    SumHA = SumHA + HA;
                }

            } // SurfNum
        }
        // Assemble values
        afnNode.SumHA = SumHA;
        afnNode.SumHATsurf = SumHATsurf;
        afnNode.SumHATref = SumHATref;
        afnNode.SumSysMCp = SumSysMCp;
        afnNode.SumSysMCpT = SumSysMCpT;
        afnNode.SumSysM = SumSysM;
        afnNode.SumSysMW = SumSysMW;

    } // CalcNodeSums

    void RAFNData::CalcSurfaceMoistureSums(EnergyPlusData &state,
                                           int const RoomAirNode,
                                           Real64 &SumHmAW,
                                           Real64 &SumHmARa,
                                           Real64 &SumHmARaW,
                                           [[maybe_unused]] Array1D<bool> const &SurfMask)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B Griffith
        //                      derived from P. Biddulph-- HAMT, L. Gu -- EPMD,
        //       DATE WRITTEN   November 2009
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Breakout summation of surface moisture interaction terms

        // Using/Aliasing

        using HeatBalanceHAMTManager::UpdateHeatBalHAMT;
        using MoistureBalanceEMPDManager::UpdateMoistureBalanceEMPD;
        using Psychrometrics::PsyRhFnTdbRhov;
        using Psychrometrics::PsyRhFnTdbRhovLBnd0C;
        using Psychrometrics::PsyRhoAirFnPbTdbW;
        using Psychrometrics::PsyWFnTdbRhPb;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Loop;
        Real64 RhoAirZone;
        Real64 Wsurf;
        bool Found;

        SumHmAW = 0.0;
        SumHmARa = 0.0;
        SumHmARaW = 0.0;

        auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(ZoneNum);
        
        int surfCount = 0;
        for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
            auto &thisSpace = state.dataHeatBal->space(spaceNum);
            for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
                ++surfCount;
                if (state.dataSurface->Surface(SurfNum).Class == SurfaceClass::Window) continue;

                if (afnZoneInfo.ControlAirNodeID == RoomAirNode) {
                    Found = false;
                    for (Loop = 1; Loop <= afnZoneInfo.NumOfAirNodes; ++Loop) {
                        // None - assigned surfaces belong to the zone node
                        if (Loop != RoomAirNode) {
                            if (afnZoneInfo.Node(Loop).SurfMask(surfCount)) {
                                Found = true;
                                break;
                            }
                        }
                    }
                    if (Found) continue;
                } else {
                    if (!afnZoneInfo.Node(RoomAirNode).SurfMask(surfCount)) continue;
                }

                auto &HMassConvInFD = state.dataMstBal->HMassConvInFD;
                auto &RhoVaporSurfIn = state.dataMstBal->RhoVaporSurfIn;
                auto &RhoVaporAirIn = state.dataMstBal->RhoVaporAirIn;
                if (state.dataSurface->Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                    UpdateHeatBalHAMT(state, SurfNum);

                    SumHmAW += HMassConvInFD(SurfNum) * state.dataSurface->Surface(SurfNum).Area * (RhoVaporSurfIn(SurfNum) - RhoVaporAirIn(SurfNum));

                    RhoAirZone = PsyRhoAirFnPbTdbW(
                        state,
                        state.dataEnvrn->OutBaroPress,
                        state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataSurface->Surface(SurfNum).Zone).MAT,
                        PsyRhFnTdbRhov(state,
                                       state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataSurface->Surface(SurfNum).Zone).MAT,
                                       RhoVaporAirIn(SurfNum),
                                       "RhoAirZone"));

                    Wsurf = PsyWFnTdbRhPb(state,
                                          state.dataHeatBalSurf->SurfTempInTmp(SurfNum),
                                          PsyRhFnTdbRhov(state, state.dataHeatBalSurf->SurfTempInTmp(SurfNum), RhoVaporSurfIn(SurfNum), "Wsurf"),
                                          state.dataEnvrn->OutBaroPress);

                    SumHmARa = SumHmARa + HMassConvInFD(SurfNum) * state.dataSurface->Surface(SurfNum).Area * RhoAirZone;

                    SumHmARaW = SumHmARaW + HMassConvInFD(SurfNum) * state.dataSurface->Surface(SurfNum).Area * RhoAirZone * Wsurf;
                }

                if (state.dataSurface->Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::EMPD) {

                    UpdateMoistureBalanceEMPD(state, SurfNum);
                    RhoVaporSurfIn(SurfNum) = state.dataMstBalEMPD->RVSurface(SurfNum);

                    SumHmAW = SumHmAW +
                              HMassConvInFD(SurfNum) * state.dataSurface->Surface(SurfNum).Area * (RhoVaporSurfIn(SurfNum) - RhoVaporAirIn(SurfNum));
                    SumHmARa =
                        SumHmARa +
                        HMassConvInFD(SurfNum) * state.dataSurface->Surface(SurfNum).Area *
                            PsyRhoAirFnPbTdbW(
                                state,
                                state.dataEnvrn->OutBaroPress,
                                state.dataHeatBalSurf->SurfTempInTmp(SurfNum),
                                PsyWFnTdbRhPb(state,
                                              state.dataHeatBalSurf->SurfTempInTmp(SurfNum),
                                              PsyRhFnTdbRhovLBnd0C(state, state.dataHeatBalSurf->SurfTempInTmp(SurfNum), RhoVaporAirIn(SurfNum)),
                                              state.dataEnvrn->OutBaroPress));
                    SumHmARaW = SumHmARaW + HMassConvInFD(SurfNum) * state.dataSurface->Surface(SurfNum).Area * RhoVaporSurfIn(SurfNum);
                }
            }
        }

    } // CalcSurfaceMoistureSums

    void RAFNData::SumNonAirSystemResponseForNode(EnergyPlusData &state, int const RAFNNodeNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   June 2012
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Sum system response from none air systems

        // USE STATEMENTS:
        using BaseboardElectric::SimElectricBaseboard;
        using BaseboardRadiator::SimBaseboard;
        using ElectricBaseboardRadiator::SimElecBaseboard;
        using HighTempRadiantSystem::SimHighTempRadiantSystem;
        using HWBaseboardRadiator::SimHWBaseboard;
        using RefrigeratedCase::SimAirChillerSet;
        using SteamBaseboardRadiator::SimSteamBaseboard;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 SysOutputProvided;
        Real64 LatOutputProvided;

        // TODO
        auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(ZoneNum);
        auto &roomAFNNode = afnZoneInfo.Node(RAFNNodeNum);

        roomAFNNode.NonAirSystemResponse = 0.0;

        if (!allocated(state.dataZoneEquip->ZoneEquipConfig)) return;

        for (int I = 1; I <= roomAFNNode.NumHVACs; ++I) {
            auto &roomAFNNodeHVAC = roomAFNNode.HVAC(I);
            switch (roomAFNNodeHVAC.zoneEquipType) {

            case DataZoneEquipment::ZoneEquipType::BaseboardWater: {
                //'ZoneHVAC:Baseboard:RadiantConvective:Water' 13
                SimHWBaseboard(state, roomAFNNodeHVAC.Name, ZoneNum, false, SysOutputProvided, roomAFNNodeHVAC.CompIndex);
                roomAFNNode.NonAirSystemResponse += roomAFNNodeHVAC.SupplyFraction * SysOutputProvided;
                // LatOutputProvided = 0.0d0 !This baseboard does not add / remove any latent heat
            } break;

            case DataZoneEquipment::ZoneEquipType::BaseboardSteam: {
                // CASE(BBSteam_Num) !'ZoneHVAC:Baseboard:RadiantConvective:Steam' 14
                SimSteamBaseboard(state, roomAFNNodeHVAC.Name, ZoneNum, false, SysOutputProvided, roomAFNNodeHVAC.CompIndex);

                roomAFNNode.NonAirSystemResponse += roomAFNNodeHVAC.SupplyFraction * SysOutputProvided;
                // LatOutputProvided = 0.0d0 !This baseboard does not add / remove any latent heat
            } break;

            case DataZoneEquipment::ZoneEquipType::BaseboardConvectiveWater: {
                // CASE(BBWaterConvective_Num)  !'ZoneHVAC:Baseboard:Convective:Water' 16
                SimBaseboard(state, roomAFNNodeHVAC.Name, ZoneNum, false, SysOutputProvided, roomAFNNodeHVAC.CompIndex);
                roomAFNNode.NonAirSystemResponse += roomAFNNodeHVAC.SupplyFraction * SysOutputProvided;
                // LatOutputProvided = 0.0d0 !This baseboard does not add / remove any latent heat
            } break;

            case DataZoneEquipment::ZoneEquipType::BaseboardConvectiveElectric: {
                // CASE(BBElectricConvective_Num)  !'ZoneHVAC:Baseboard:Convective:Electric' 15
                SimElectricBaseboard(state, roomAFNNodeHVAC.Name, ZoneNum, SysOutputProvided, roomAFNNodeHVAC.CompIndex);
                roomAFNNode.NonAirSystemResponse += roomAFNNodeHVAC.SupplyFraction * SysOutputProvided;
                // LatOutputProvided = 0.0d0 !This baseboard does not add / remove any latent heat
            } break;

            case DataZoneEquipment::ZoneEquipType::RefrigerationChillerSet: {
                // CASE(RefrigerationAirChillerSet_Num)  !'ZoneHVAC:RefrigerationChillerSet' 20
                SimAirChillerSet(
                    state, roomAFNNodeHVAC.Name, ZoneNum, false, SysOutputProvided, LatOutputProvided, roomAFNNodeHVAC.CompIndex);
                roomAFNNode.NonAirSystemResponse += roomAFNNodeHVAC.SupplyFraction * SysOutputProvided;
            } break;

            case DataZoneEquipment::ZoneEquipType::BaseboardElectric: {
                // CASE(BBElectric_Num)  !'ZoneHVAC:Baseboard:RadiantConvective:Electric' 12
                SimElecBaseboard(state, roomAFNNodeHVAC.Name, ZoneNum, false, SysOutputProvided, roomAFNNodeHVAC.CompIndex);
                roomAFNNode.NonAirSystemResponse += roomAFNNodeHVAC.SupplyFraction * SysOutputProvided;
                // LatOutputProvided = 0.0d0 !This baseboard does not add / remove any latent heat
            } break;

            case DataZoneEquipment::ZoneEquipType::HighTemperatureRadiant: {
                // CASE(BBElectric_Num)  !'ZoneHVAC:HighTemperatureRadiant' 17
                SimHighTempRadiantSystem(state, roomAFNNodeHVAC.Name, false, SysOutputProvided, roomAFNNodeHVAC.CompIndex);
                roomAFNNode.NonAirSystemResponse += roomAFNNodeHVAC.SupplyFraction * SysOutputProvided;
                // LatOutputProvided = 0.0d0 !This baseboard does not add / remove any latent heat
            } break;

            default: {
            } break;
            } // switch 

            // Zone sum of system convective gains, collected via NonAirSystemResponse
        }

    } // SumNonAirSystemResponseForNode

    //*****************************************************************************************

    void RAFNData::SumSystemDepResponseForNode(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         B.Griffith
        //       DATE WRITTEN   aug 2005, Jan2004
        //       MODIFIED       Lixing Gu, Aug. 2015 for v8.4 replease
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Sum system sensible loads used at the next time step

        // USE STATEMENTS:
        using ZoneDehumidifier::SimZoneDehumidifier;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int I;
        Real64 SysOutputProvided;
        Real64 LatOutputProvided;
        int RoomAirNode;

        // TODO

        auto &afnZoneInfo = state.dataRoomAir->AFNZoneInfo(ZoneNum);

        // SysDepZoneLoads saved to be added to zone heat balance next
        SysOutputProvided = 0.0;
        for (RoomAirNode = 1; RoomAirNode <= afnZoneInfo.NumOfAirNodes; ++RoomAirNode) {
            afnZoneInfo.Node(RoomAirNode).SysDepZoneLoadsLaggedOld = 0.0;
            for (int I = 1; I <= afnZoneInfo.Node(RoomAirNode).NumHVACs; ++I) {
                if (afnZoneInfo.Node(RoomAirNode).HVAC(I).zoneEquipType == DataZoneEquipment::ZoneEquipType::DehumidifierDX) {
                    if (SysOutputProvided == 0.0)
                        SimZoneDehumidifier(state,
                                            afnZoneInfo.Node(RoomAirNode).HVAC(I).Name,
                                            ZoneNum,
                                            false,
                                            SysOutputProvided,
                                            LatOutputProvided,
                                            afnZoneInfo.Node(RoomAirNode).HVAC(I).CompIndex);
                    if (SysOutputProvided > 0.0) break;
                }
            }
        }

        if (SysOutputProvided > 0.0) {
            for (RoomAirNode = 1; RoomAirNode <= afnZoneInfo.NumOfAirNodes; ++RoomAirNode) {
                for (I = 1; I <= afnZoneInfo.Node(RoomAirNode).NumHVACs; ++I) {
                    if (afnZoneInfo.Node(RoomAirNode).HVAC(I).zoneEquipType == DataZoneEquipment::ZoneEquipType::DehumidifierDX) {
                        afnZoneInfo.Node(RoomAirNode).SysDepZoneLoadsLaggedOld +=
                            afnZoneInfo.Node(RoomAirNode).HVAC(I).SupplyFraction * SysOutputProvided;
                    }
                }
            }
        }

    } // SumSystemDepResponseForNode

    //*****************************************************************************************

} // namespace RoomAir

} // namespace EnergyPlus
