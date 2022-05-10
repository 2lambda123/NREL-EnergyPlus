// EnergyPlus, Copyright (c) 1996-2022, The Board of Trustees of the University of Illinois,
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

// C++ Headers
#include <algorithm>
#include <cmath>
#include <set>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <AirflowNetwork/Elements.hpp>
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/AirflowNetworkBalanceManager.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataBranchNodeConnections.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACFan.hh>
#include <EnergyPlus/HVACHXAssistedCoolingCoil.hh>
#include <EnergyPlus/HVACStandAloneERV.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/RoomAirModelManager.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SingleDuct.hh>
#include <EnergyPlus/SplitterComponent.hh>
#include <EnergyPlus/ThermalComfort.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterThermalTanks.hh>
#include <EnergyPlus/ZoneDehumidifier.hh>

namespace EnergyPlus {

namespace AirflowNetwork {

    // MODULE INFORMATION:
    //       AUTHOR         Lixing Gu, Don Shirey, and Muthusamy V. Swami
    //       DATE WRITTEN   July 28, 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // This module is used to simulate performance of air distribution system with a single HVAC system and a constant
    // volume supply fan.

    // Using/Aliasing
    using CurveManager::CurveValue;
    using CurveManager::GetCurveIndex;
    using DataEnvironment::OutDryBulbTempAt;
    using DataHVACGlobals::ContFanCycCoil;
    using DataHVACGlobals::CycFanCycCoil;
    using DataHVACGlobals::FanType_SimpleConstVolume;
    using DataHVACGlobals::FanType_SimpleOnOff;
    using DataHVACGlobals::FanType_SimpleVAV;
    using DataHVACGlobals::FanType_ZoneExhaust;
    using DataSurfaces::cExtBoundCondition;
    using DataSurfaces::ExternalEnvironment;
    using DataSurfaces::OtherSideCoefNoCalcExt;
    using DataSurfaces::SurfaceClass;
    using Fans::GetFanIndex;
    using Fans::GetFanInletNode;
    using Fans::GetFanOutletNode;
    using Fans::GetFanType;
    using Fans::GetFanVolFlow;
    using Psychrometrics::PsyCpAirFnW;
    using Psychrometrics::PsyHFnTdbW;
    using Psychrometrics::PsyRhoAirFnPbTdbW;
    using ScheduleManager::GetCurrentScheduleValue;
    using ScheduleManager::GetScheduleIndex;

    // AirflowNetwork::Solver solver;

    // Functions

    int constexpr NumOfVentCtrTypes(6); // Number of zone level venting control types

    void ManageAirflowNetworkBalance(EnergyPlusData &state,
                                     Optional_bool_const FirstHVACIteration, // True when solution technique on first iteration
                                     Optional_int_const Iter,                // Iteration number
                                     Optional_bool ResimulateAirZone         // True when solution technique on third iteration
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   July 28, 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs simulation of air distribution system.

        // Using/Aliasing
        auto &TurnFansOn = state.dataHVACGlobal->TurnFansOn;
        using DataHVACGlobals::VerySmallMassFlow;

        // Locals
        int i;
        int AFNSupplyFanType = 0;

        if (state.afn->AirflowNetworkGetInputFlag) {
            GetAirflowNetworkInput(state);
            state.afn->AirflowNetworkGetInputFlag = false;
            return;
        }

        if (present(ResimulateAirZone)) {
            ResimulateAirZone = false;
        }

        if (state.afn->SimulateAirflowNetwork < AirflowNetworkControlMultizone) return;

        if (state.dataGlobal->BeginEnvrnFlag) {
            TurnFansOn = false; // The FAN should be off when BeginEnvrnFlag = .True.
        }

        state.afn->initialize(state);

        auto &NetworkNumOfNodes = state.afn->ActualNumOfNodes;
        auto &NetworkNumOfLinks = state.afn->ActualNumOfLinks;

        NetworkNumOfNodes = state.afn->NumOfNodesMultiZone;
        NetworkNumOfLinks = state.afn->NumOfLinksMultiZone;

        state.afn->AirflowNetworkFanActivated = false;

        if (present(FirstHVACIteration) && state.afn->SimulateAirflowNetwork >= AirflowNetworkControlSimpleADS) {
            if (FirstHVACIteration) {
                if (allocated(state.dataAirLoop->AirLoopAFNInfo)) {
                    for (i = 1; i <= state.afn->DisSysNumOfCVFs; i++) {
                        state.dataAirLoop->AirLoopAFNInfo(i).AFNLoopHeatingCoilMaxRTF = 0.0;
                        state.dataAirLoop->AirLoopAFNInfo(i).AFNLoopOnOffFanRTF = 0.0;
                        state.dataAirLoop->AirLoopAFNInfo(i).AFNLoopDXCoilRTF = 0.0;
                        state.dataAirLoop->AirLoopAFNInfo(i).LoopOnOffFanPartLoadRatio = 0.0;
                    }
                }
            }
            Real64 FanMassFlowRate = 0.0;
            int FanOperModeCyc = 0;
            AFNSupplyFanType = 0;

            for (i = 1; i <= state.afn->DisSysNumOfCVFs; i++) {
                AFNSupplyFanType = state.afn->DisSysCompCVFData(i).FanTypeNum;
                FanMassFlowRate =
                    max(FanMassFlowRate, state.dataLoopNodes->Node(state.afn->DisSysCompCVFData(i).OutletNode).MassFlowRate);
                // VAV take high priority
                if (state.afn->DisSysCompCVFData(i).FanTypeNum == FanType_SimpleVAV) {
                    AFNSupplyFanType = state.afn->DisSysCompCVFData(i).FanTypeNum;
                    break;
                }
                if (FanMassFlowRate > VerySmallMassFlow && state.dataAirLoop->AirLoopAFNInfo(i).LoopFanOperationMode == CycFanCycCoil &&
                    state.dataAirLoop->AirLoopAFNInfo(i).LoopSystemOnMassFlowrate > 0.0) {
                    FanOperModeCyc = CycFanCycCoil;
                    AFNSupplyFanType = state.afn->DisSysCompCVFData(i).FanTypeNum;
                    if (AFNSupplyFanType == FanType_SimpleOnOff) {
                        break;
                    }
                }
            }
            //            Revised to meet heat exchanger requirement
            if ((FanMassFlowRate > VerySmallMassFlow) && (!FirstHVACIteration)) {
                if (AFNSupplyFanType == FanType_SimpleOnOff && FanOperModeCyc == CycFanCycCoil) {
                    state.afn->AirflowNetworkFanActivated = true;
                } else if (AFNSupplyFanType == FanType_SimpleVAV) {
                    if (present(Iter) && Iter > 1) state.afn->AirflowNetworkFanActivated = true;
                } else if (state.afn->AirflowNetworkUnitarySystem) {
                    if (present(Iter) && Iter > 1) state.afn->AirflowNetworkFanActivated = true;
                } else {
                    state.afn->AirflowNetworkFanActivated = true;
                }
            }
        }
        if (allocated(state.dataZoneEquip->ZoneEquipConfig) && state.dataHVACGlobal->NumHybridVentSysAvailMgrs > 0 &&
            allocated(state.dataAirSystemsData->PrimaryAirSystems))
            HybridVentilationControl(state);
        if (state.afn->VentilationCtrl == 1 && state.dataHVACGlobal->NumHybridVentSysAvailMgrs > 0)
            state.afn->AirflowNetworkFanActivated = false;

        if (present(Iter) && present(ResimulateAirZone) && state.afn->SimulateAirflowNetwork >= AirflowNetworkControlSimpleADS) {
            if (state.afn->AirflowNetworkFanActivated && Iter < 3 && AFNSupplyFanType == FanType_SimpleOnOff) {
                ResimulateAirZone = true;
            }
            if (AFNSupplyFanType == FanType_SimpleVAV) {
                if (!state.afn->AirflowNetworkFanActivated && Iter < 3) ResimulateAirZone = true;
            }
            if (state.afn->AirflowNetworkUnitarySystem) {
                if (!state.afn->AirflowNetworkFanActivated && Iter < 3) ResimulateAirZone = true;
            }
        }
        if (state.afn->AirflowNetworkFanActivated &&
            state.afn->SimulateAirflowNetwork > AirflowNetworkControlMultizone) {
            NetworkNumOfNodes = state.afn->AirflowNetworkNumOfNodes;
            NetworkNumOfLinks = state.afn->AirflowNetworkNumOfLinks;
        }

        if (allocated(state.dataZoneEquip->ZoneEquipConfig)) ValidateExhaustFanInput(state);

        // VAV terminal set only
        if (present(FirstHVACIteration) && FirstHVACIteration) state.afn->VAVTerminalRatio = 0.0;

        // Set AirLoop Number for fans
        if (FirstHVACIteration && state.afn->AssignFanAirLoopNumFlag) {
            AssignFanAirLoopNum(state);
            state.afn->AssignFanAirLoopNumFlag = false;
        }

        if (state.afn->AirflowNetworkFanActivated &&
            state.afn->SimulateAirflowNetwork > AirflowNetworkControlMultizone) {
            if (state.afn->ValidateDistributionSystemFlag) {
                ValidateDistributionSystem(state);
                ValidateFanFlowRate(state);
                state.afn->ValidateDistributionSystemFlag = false;
            }
        }
        CalcAirflowNetworkAirBalance(state);

        if (state.afn->AirflowNetworkFanActivated &&
            state.afn->SimulateAirflowNetwork > AirflowNetworkControlMultizone) {

            state.afn->LoopOnOffFlag = false;
            for (i = 1; i <= state.afn->DisSysNumOfCVFs; i++) {
                if (state.afn->DisSysCompCVFData(i).AirLoopNum > 0) {
                    if (state.dataLoopNodes->Node(state.afn->DisSysCompCVFData(i).InletNode).MassFlowRate > 0.0) {
                        state.afn->LoopOnOffFlag(state.afn->DisSysCompCVFData(i).AirLoopNum) = true;
                    }
                }
            }

            CalcAirflowNetworkHeatBalance(state);
            CalcAirflowNetworkMoisBalance(state);
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) CalcAirflowNetworkCO2Balance(state);
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) CalcAirflowNetworkGCBalance(state);
        }

        UpdateAirflowNetwork(state, FirstHVACIteration);
    }

    static bool getAirflowElementInput(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jason DeGraw
        //       DATE WRITTEN   Oct. 2018
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reads airflow element inputs (eventually)

        static constexpr std::string_view RoutineName{"getAirflowElementInput"};
        std::string CurrentModuleObject;
        bool success{true};

        // *** Read AirflowNetwork simulation reference crack conditions
        std::unordered_map<std::string, ReferenceConditions> referenceConditions; // Map for lookups
        ReferenceConditions defaultReferenceConditions("Default");                // Defaulted conditions
        bool conditionsAreDefaulted(true);                                        // Conditions are defaulted?
        CurrentModuleObject = "AirflowNetwork:MultiZone:ReferenceCrackConditions";
        auto instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            // globalSolverObject.referenceConditions.clear();
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                Real64 temperature(20.0);
                if (fields.find("reference_temperature") != fields.end()) { // required field, has default value
                    temperature = fields.at("reference_temperature").get<Real64>();
                }
                Real64 pressure(101325.0);
                if (fields.find("reference_barometric_pressure") != fields.end()) { // not required field, has default value
                    pressure = fields.at("reference_barometric_pressure").get<Real64>();
                    if (std::abs((pressure - state.dataEnvrn->StdBaroPress) / state.dataEnvrn->StdBaroPress) > 0.1) { // 10% off
                        ShowWarningError(state,
                                         format("{}: {}: Pressure = {:.0R} differs by more than 10% from Standard Barometric Pressure = {:.0R}.",
                                                RoutineName,
                                                CurrentModuleObject,
                                                pressure,
                                                state.dataEnvrn->StdBaroPress));
                        ShowContinueError(state, "...occurs in " + CurrentModuleObject + " = " + thisObjectName);
                    }
                    if (pressure <= 31000.0) {
                        ShowSevereError(state,
                                        format(RoutineName) + ": " + CurrentModuleObject + ": " + thisObjectName +
                                            ". Reference Barometric Pressure must be greater than 31000 Pa.");
                        success = false;
                    }
                }
                Real64 humidity(0.0);
                if (fields.find("reference_humidity_ratio") != fields.end()) { // not required field, has default value
                    humidity = fields.at("reference_humidity_ratio").get<Real64>();
                }
                // globalSolverObject.referenceConditions.emplace_back(thisObjectName, temperature, pressure, humidity);
                referenceConditions.emplace(std::piecewise_construct,
                                            std::forward_as_tuple(thisObjectName),
                                            std::forward_as_tuple(instance.key(), temperature, pressure, humidity));
            }
            // Check that there is more than one
            if (referenceConditions.size() == 1) {
                state.dataInputProcessing->inputProcessor->markObjectAsUsed("AirflowNetwork:MultiZone:ReferenceCrackConditions",
                                                                            referenceConditions.begin()->second.name);
                defaultReferenceConditions = referenceConditions.begin()->second;

            } else {
                conditionsAreDefaulted = false;
            }
        }
        if (!success) {
            return false;
        }

        auto &solver = state.afn;

        // *** Read AirflowNetwork simulation surface crack component
        CurrentModuleObject = "AirflowNetwork:MultiZone:Surface:Crack";
        state.afn->AirflowNetworkNumOfSurCracks =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1; // Temporary workaround
            state.afn->MultizoneSurfaceCrackData.allocate(
                state.afn->AirflowNetworkNumOfSurCracks); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 coeff{fields.at("air_mass_flow_coefficient_at_reference_conditions")}; // Required field
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent") != fields.end()) { // not required field, has default value
                    expnt = fields.at("air_mass_flow_exponent").get<Real64>();
                }
                Real64 refT = defaultReferenceConditions.temperature;
                Real64 refP = defaultReferenceConditions.pressure;
                Real64 refW = defaultReferenceConditions.humidity_ratio;
                if (!conditionsAreDefaulted) {
                    if (fields.find("reference_crack_conditions") != fields.end()) { // not required field, *should* have default value
                        auto refCrackCondName = fields.at("reference_crack_conditions").get<std::string>();
                        auto result = referenceConditions.find(UtilityRoutines::MakeUPPERCase(refCrackCondName));
                        if (result == referenceConditions.end()) {
                            ShowSevereError(state,
                                            format(RoutineName) + CurrentModuleObject + ": " + thisObjectName +
                                                ". Cannot find reference crack conditions object \"" +
                                                fields.at("reference_crack_conditions").get<std::string>() + "\".");
                            success = false;
                        } else {
                            refT = result->second.temperature;
                            refP = result->second.pressure;
                            refW = result->second.humidity_ratio;
                            state.dataInputProcessing->inputProcessor->markObjectAsUsed("AirflowNetwork:MultiZone:ReferenceCrackConditions",
                                                                                        result->second.name);
                        }
                    }
                }
                // globalSolverObject.cracks[thisObjectName] = SurfaceCrack(coeff, expnt, refT, refP, refW);
                state.afn->MultizoneSurfaceCrackData(i).name = thisObjectName; // Name of surface crack component
                state.afn->MultizoneSurfaceCrackData(i).coefficient = coeff;   // Air Mass Flow Coefficient
                state.afn->MultizoneSurfaceCrackData(i).exponent = expnt;      // Air Mass Flow exponent
                state.afn->MultizoneSurfaceCrackData(i).reference_density = AIRDENSITY(state, refP, refT, refW);
                state.afn->MultizoneSurfaceCrackData(i).reference_viscosity = AIRDYNAMICVISCOSITY(refT);

                // This is the first element that is being added to the lookup table, so no check of naming overlaps
                solver->elements[thisObjectName] = &state.afn->MultizoneSurfaceCrackData(i); // Yet another workaround

                ++i;
            }
        }

        // *** Read AirflowNetwork simulation zone exhaust fan component
        CurrentModuleObject = "AirflowNetwork:MultiZone:Component:ZoneExhaustFan";
        state.afn->AirflowNetworkNumOfExhFan =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        state.afn->NumOfExhaustFans =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Fan:ZoneExhaust"); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1; // Temporary workaround
            state.afn->MultizoneCompExhaustFanData.allocate(
                state.afn->AirflowNetworkNumOfExhFan); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 coeff{fields.at("air_mass_flow_coefficient_when_the_zone_exhaust_fan_is_off_at_reference_conditions")}; // Required field
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent_when_the_zone_exhaust_fan_is_off") != fields.end()) { // not required field, has default value
                    expnt = fields.at("air_mass_flow_exponent_when_the_zone_exhaust_fan_is_off").get<Real64>();
                }

                // This breaks the component model, need to fix
                bool fanErrorFound = false;
                int fanIndex;
                GetFanIndex(state, thisObjectName, fanIndex, fanErrorFound);
                if (fanErrorFound) {
                    ShowSevereError(state,
                                    format(RoutineName) + ": " + CurrentModuleObject + " = " + thisObjectName +
                                        " is not found in Fan:ZoneExhaust objects.");
                    success = false;
                }
                Real64 flowRate;

                GetFanVolFlow(state, fanIndex, flowRate);
                flowRate *= state.dataEnvrn->StdRhoAir;
                bool nodeErrorsFound{false};
                int inletNode = GetFanInletNode(state, "Fan:ZoneExhaust", thisObjectName, nodeErrorsFound);
                int outletNode = GetFanOutletNode(state, "Fan:ZoneExhaust", thisObjectName, nodeErrorsFound);
                if (nodeErrorsFound) {
                    success = false;
                }
                int fanType_Num;
                GetFanType(state, thisObjectName, fanType_Num, fanErrorFound);
                if (fanType_Num != FanType_ZoneExhaust) {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + " = " + thisObjectName + ". The specified " + "Name" +
                                        " is not found as a valid Fan:ZoneExhaust object.");
                    success = false;
                }

                Real64 refT = defaultReferenceConditions.temperature;
                Real64 refP = defaultReferenceConditions.pressure;
                Real64 refW = defaultReferenceConditions.humidity_ratio;
                if (!conditionsAreDefaulted) {
                    if (fields.find("reference_crack_conditions") != fields.end()) { // not required field, *should* have default value
                        auto refCrackCondName = fields.at("reference_crack_conditions").get<std::string>();
                        auto result = referenceConditions.find(UtilityRoutines::MakeUPPERCase(refCrackCondName));
                        if (result == referenceConditions.end()) {
                            ShowSevereError(state,
                                            format(RoutineName) + CurrentModuleObject + ": " + thisObjectName +
                                                ". Cannot find reference crack conditions object \"" +
                                                fields.at("reference_crack_conditions").get<std::string>() + "\".");
                            success = false;
                        } else {
                            refT = result->second.temperature;
                            refP = result->second.pressure;
                            refW = result->second.humidity_ratio;
                            state.dataInputProcessing->inputProcessor->markObjectAsUsed("AirflowNetwork:MultiZone:ReferenceCrackConditions",
                                                                                        result->second.name);
                        }
                    }
                }

                state.afn->MultizoneCompExhaustFanData(i).name = thisObjectName; // Name of zone exhaust fan component
                state.afn->MultizoneCompExhaustFanData(i).FlowCoef = coeff;      // flow coefficient
                state.afn->MultizoneCompExhaustFanData(i).FlowExpo = expnt;      // Flow exponent

                state.afn->MultizoneCompExhaustFanData(i).FlowRate = flowRate;
                state.afn->MultizoneCompExhaustFanData(i).InletNode = inletNode;
                state.afn->MultizoneCompExhaustFanData(i).OutletNode = outletNode;

                state.afn->MultizoneCompExhaustFanData(i).StandardT = refT;
                state.afn->MultizoneCompExhaustFanData(i).StandardP = refP;
                state.afn->MultizoneCompExhaustFanData(i).StandardW = refW;

                // Add the element to the lookup table, check for name overlaps
                if (solver->elements.find(thisObjectName) == solver->elements.end()) {
                    solver->elements[thisObjectName] = &state.afn->MultizoneCompExhaustFanData(i); // Yet another workaround
                } else {
                    ShowSevereError(state, format(RoutineName) + "Duplicated airflow element names are found = " + thisObjectName);
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read Outdoor Airflow object
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:OutdoorAirFlow";
        state.afn->NumOfOAFans =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                                                                                        // Temporary workaround
            state.afn->DisSysCompOutdoorAirData.allocate(state.afn->NumOfOAFans); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                std::string mixer_name = UtilityRoutines::MakeUPPERCase(fields.at("outdoor_air_mixer_name").get<std::string>());
                Real64 coeff{fields.at("air_mass_flow_coefficient_when_no_outdoor_air_flow_at_reference_conditions")};
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent_when_no_outdoor_air_flow") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent_when_no_outdoor_air_flow").get<Real64>();
                }

                int OAMixerNum = MixedAir::GetOAMixerNumber(state, mixer_name);
                if (OAMixerNum == 0) {
                    ShowSevereError(state,
                                    format(RoutineName) + ": " + CurrentModuleObject + " object " + thisObjectName + ". Invalid " +
                                        "Outdoor Air Mixer Name" + " \"" + mixer_name + "\" given.");
                    success = false;
                }

                Real64 refT = defaultReferenceConditions.temperature;
                Real64 refP = defaultReferenceConditions.pressure;
                Real64 refW = defaultReferenceConditions.humidity_ratio;
                if (!conditionsAreDefaulted) {
                    if (fields.find("reference_crack_conditions") != fields.end()) { // not required field, *should* have default value
                        auto refCrackCondName = fields.at("reference_crack_conditions").get<std::string>();
                        auto result = referenceConditions.find(UtilityRoutines::MakeUPPERCase(refCrackCondName));
                        if (result == referenceConditions.end()) {
                            ShowSevereError(state,
                                            format(RoutineName) + CurrentModuleObject + ": " + thisObjectName +
                                                ". Cannot find reference crack conditions object \"" +
                                                fields.at("reference_crack_conditions").get<std::string>() + "\".");
                            success = false;
                        } else {
                            refT = result->second.temperature;
                            refP = result->second.pressure;
                            refW = result->second.humidity_ratio;
                            state.dataInputProcessing->inputProcessor->markObjectAsUsed("AirflowNetwork:MultiZone:ReferenceCrackConditions",
                                                                                        result->second.name);
                        }
                    }
                }

                state.afn->DisSysCompOutdoorAirData(i).name = thisObjectName; // Name of zone exhaust fan component
                state.afn->DisSysCompOutdoorAirData(i).FlowCoef = coeff;      // flow coefficient
                state.afn->DisSysCompOutdoorAirData(i).FlowExpo = expnt;      // Flow exponent

                state.afn->DisSysCompOutdoorAirData(i).OAMixerNum = OAMixerNum;

                state.afn->DisSysCompOutdoorAirData(i).StandardT = refT;
                state.afn->DisSysCompOutdoorAirData(i).StandardP = refP;
                state.afn->DisSysCompOutdoorAirData(i).StandardW = refW;

                // Add the element to the lookup table, check for name overlaps
                if (solver->elements.find(thisObjectName) == solver->elements.end()) {
                    solver->elements[thisObjectName] = &state.afn->DisSysCompOutdoorAirData(i); // Yet another workaround
                } else {
                    ShowSevereError(state, format(RoutineName) + "Duplicated airflow element names are found = " + thisObjectName);
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read Relief Airflow object
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:ReliefAirFlow";
        state.afn->NumOfReliefFans =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1; // Temporary workaround
            state.afn->DisSysCompReliefAirData.allocate(
                state.afn->NumOfReliefFans); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                std::string mixer_name = UtilityRoutines::MakeUPPERCase(fields.at("outdoor_air_mixer_name").get<std::string>());
                Real64 coeff{fields.at("air_mass_flow_coefficient_when_no_outdoor_air_flow_at_reference_conditions")};
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent_when_no_outdoor_air_flow") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent_when_no_outdoor_air_flow").get<Real64>();
                }

                int OAMixerNum{MixedAir::GetOAMixerNumber(state, mixer_name)};
                if (OAMixerNum == 0) {
                    ShowSevereError(state,
                                    format(RoutineName) + ": " + CurrentModuleObject + " object " + thisObjectName + ". Invalid " +
                                        "Outdoor Air Mixer Name" + " \"" + mixer_name + "\" given.");
                    success = false;
                }

                Real64 refT = defaultReferenceConditions.temperature;
                Real64 refP = defaultReferenceConditions.pressure;
                Real64 refW = defaultReferenceConditions.humidity_ratio;
                if (!conditionsAreDefaulted) {
                    if (fields.find("reference_crack_conditions") != fields.end()) { // not required field, *should* have default value
                        auto refCrackCondName = fields.at("reference_crack_conditions").get<std::string>();
                        auto result = referenceConditions.find(UtilityRoutines::MakeUPPERCase(refCrackCondName));
                        if (result == referenceConditions.end()) {
                            ShowSevereError(state,
                                            format(RoutineName) + CurrentModuleObject + ": " + thisObjectName +
                                                ". Cannot find reference crack conditions object \"" +
                                                fields.at("reference_crack_conditions").get<std::string>() + "\".");
                            success = false;
                        } else {
                            refT = result->second.temperature;
                            refP = result->second.pressure;
                            refW = result->second.humidity_ratio;
                            state.dataInputProcessing->inputProcessor->markObjectAsUsed("AirflowNetwork:MultiZone:ReferenceCrackConditions",
                                                                                        result->second.name);
                        }
                    }
                }

                state.afn->DisSysCompReliefAirData(i).name = thisObjectName; // Name of zone exhaust fan component
                state.afn->DisSysCompReliefAirData(i).FlowCoef = coeff;      // flow coefficient
                state.afn->DisSysCompReliefAirData(i).FlowExpo = expnt;      // Flow exponent
                state.afn->DisSysCompReliefAirData(i).OAMixerNum = OAMixerNum;
                state.afn->DisSysCompReliefAirData(i).StandardT = refT;
                state.afn->DisSysCompReliefAirData(i).StandardP = refP;
                state.afn->DisSysCompReliefAirData(i).StandardW = refW;

                // Add the element to the lookup table, check for name overlaps
                if (solver->elements.find(thisObjectName) == solver->elements.end()) {
                    solver->elements[thisObjectName] = &state.afn->DisSysCompReliefAirData(i); // Yet another workaround
                } else {
                    ShowSevereError(state, format(RoutineName) + "Duplicated airflow element names are found = " + thisObjectName);
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork simulation detailed openings
        CurrentModuleObject = "AirflowNetwork:MultiZone:Component:DetailedOpening";
        state.afn->AirflowNetworkNumOfDetOpenings =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1; // Temporary workaround
            state.afn->MultizoneCompDetOpeningData.allocate(
                state.afn->AirflowNetworkNumOfDetOpenings); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 coeff{fields.at("air_mass_flow_coefficient_when_opening_is_closed")};
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent_when_opening_is_closed") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent_when_opening_is_closed").get<Real64>();
                }

                int LVOtype{1};
                std::string LVOstring;
                if (fields.find("type_of_rectangular_large_vertical_opening_lvo_") != fields.end()) {
                    LVOstring = fields.at("type_of_rectangular_large_vertical_opening_lvo_").get<std::string>();
                    if (UtilityRoutines::SameString(LVOstring, "NonPivoted") || UtilityRoutines::SameString(LVOstring, "1")) {
                        LVOtype = 1; // Large vertical opening type number
                    } else if (UtilityRoutines::SameString(LVOstring, "HorizontallyPivoted") || UtilityRoutines::SameString(LVOstring, "2")) {
                        LVOtype = 2; // Large vertical opening type number
                    } else {
                        ShowSevereError(state,
                                        format(RoutineName) + "Invalid Type of Rectangular Large Vertical Opening (LVO) = " + LVOstring + "in " +
                                            CurrentModuleObject + " = " + thisObjectName);
                        ShowContinueError(state, "Valid choices are NonPivoted and HorizontallyPivoted.");
                        success = false;
                    }
                }

                Real64 extra{0.0};
                if (fields.find("extra_crack_length_or_height_of_pivoting_axis") != fields.end()) {
                    extra = fields.at("extra_crack_length_or_height_of_pivoting_axis").get<Real64>();
                }

                Real64 N{fields.at("number_of_sets_of_opening_factor_data")};

                std::vector<Real64> factors(N);
                std::vector<Real64> cds(N);
                std::vector<Real64> width_factors(N);
                std::vector<Real64> height_factors(N);
                std::vector<Real64> start_height_factors(N);

                // Real64 factor{0.0};
                // if (fields.find("opening_factor_1") != fields.end()) {
                //    factor = fields.at("opening_factor_1");
                //}
                Real64 cd{0.001};
                if (fields.find("discharge_coefficient_for_opening_factor_1") != fields.end()) {
                    cd = fields.at("discharge_coefficient_for_opening_factor_1").get<Real64>();
                }
                Real64 width_factor{0.0};
                if (fields.find("width_factor_for_opening_factor_1") != fields.end()) {
                    width_factor = fields.at("width_factor_for_opening_factor_1").get<Real64>();
                }
                Real64 height_factor{0.0};
                if (fields.find("height_factor_for_opening_factor_1") != fields.end()) {
                    height_factor = fields.at("height_factor_for_opening_factor_1").get<Real64>();
                }
                Real64 start_height_factor{0.0};
                if (fields.find("start_height_factor_for_opening_factor_1") != fields.end()) {
                    start_height_factor = fields.at("start_height_factor_for_opening_factor_1").get<Real64>();
                }

                factors[0] = 0.0; // factor; // This factor must be zero
                cds[0] = cd;
                width_factors[0] = width_factor;
                height_factors[0] = height_factor;
                start_height_factors[0] = start_height_factor;

                Real64 factor{fields.at("opening_factor_2")};
                cd = 1.0;
                if (fields.find("discharge_coefficient_for_opening_factor_2") != fields.end()) {
                    cd = fields.at("discharge_coefficient_for_opening_factor_2").get<Real64>();
                }
                width_factor = 1.0;
                if (fields.find("width_factor_for_opening_factor_2") != fields.end()) {
                    width_factor = fields.at("width_factor_for_opening_factor_2").get<Real64>();
                }
                height_factor = 1.0;
                if (fields.find("height_factor_for_opening_factor_2") != fields.end()) {
                    height_factor = fields.at("height_factor_for_opening_factor_2").get<Real64>();
                }
                start_height_factor = 0.0;
                if (fields.find("start_height_factor_for_opening_factor_2") != fields.end()) {
                    start_height_factor = fields.at("start_height_factor_for_opening_factor_2").get<Real64>();
                }

                factors[1] = factor;
                cds[1] = cd;
                width_factors[1] = width_factor;
                height_factors[1] = height_factor;
                start_height_factors[1] = start_height_factor;

                if (N >= 3) {
                    factor = fields.at("opening_factor_3").get<Real64>();
                    cd = 0.0;
                    if (fields.find("discharge_coefficient_for_opening_factor_3") != fields.end()) {
                        cd = fields.at("discharge_coefficient_for_opening_factor_3").get<Real64>();
                    }
                    width_factor = 0.0;
                    if (fields.find("width_factor_for_opening_factor_3") != fields.end()) {
                        width_factor = fields.at("width_factor_for_opening_factor_3").get<Real64>();
                    }
                    height_factor = 0.0;
                    if (fields.find("height_factor_for_opening_factor_3") != fields.end()) {
                        height_factor = fields.at("height_factor_for_opening_factor_3").get<Real64>();
                    }
                    start_height_factor = 0.0;
                    if (fields.find("start_height_factor_for_opening_factor_3") != fields.end()) {
                        start_height_factor = fields.at("start_height_factor_for_opening_factor_3").get<Real64>();
                    }

                    factors[2] = factor;
                    cds[2] = cd;
                    width_factors[2] = width_factor;
                    height_factors[2] = height_factor;
                    start_height_factors[2] = start_height_factor;

                    if (N >= 4) {
                        factor = fields.at("opening_factor_4").get<Real64>();
                        cd = 0.0;
                        if (fields.find("discharge_coefficient_for_opening_factor_4") != fields.end()) {
                            cd = fields.at("discharge_coefficient_for_opening_factor_4").get<Real64>();
                        }
                        width_factor = 0.0;
                        if (fields.find("width_factor_for_opening_factor_4") != fields.end()) {
                            width_factor = fields.at("width_factor_for_opening_factor_4").get<Real64>();
                        }
                        height_factor = 0.0;
                        if (fields.find("height_factor_for_opening_factor_4") != fields.end()) {
                            height_factor = fields.at("height_factor_for_opening_factor_4").get<Real64>();
                        }
                        start_height_factor = 0.0;
                        if (fields.find("start_height_factor_for_opening_factor_4") != fields.end()) {
                            start_height_factor = fields.at("start_height_factor_for_opening_factor_4").get<Real64>();
                        }

                        factors[3] = factor;
                        cds[3] = cd;
                        width_factors[3] = width_factor;
                        height_factors[3] = height_factor;
                        start_height_factors[3] = start_height_factor;
                    }
                }

                state.afn->MultizoneCompDetOpeningData(i).name = thisObjectName; // Name of large detailed opening component
                state.afn->MultizoneCompDetOpeningData(i).FlowCoef = coeff; // Air Mass Flow Coefficient When Window or Door Is Closed
                state.afn->MultizoneCompDetOpeningData(i).FlowExpo = expnt; // Air Mass Flow exponent When Window or Door Is Closed
                state.afn->MultizoneCompDetOpeningData(i).TypeName = LVOstring; // Large vertical opening type
                state.afn->MultizoneCompDetOpeningData(i).LVOType = LVOtype;    // Large vertical opening type number
                state.afn->MultizoneCompDetOpeningData(i).LVOValue = extra; // Extra crack length for LVO type 1 with multiple openable
                                                                                           // parts, or Height of pivoting axis for LVO type 2

                state.afn->MultizoneCompDetOpeningData(i).NumFac = N; // Number of Opening Factor Values

                state.afn->MultizoneCompDetOpeningData(i).OpenFac1 = factors[0];        // Opening factor #1
                state.afn->MultizoneCompDetOpeningData(i).DischCoeff1 = cds[0];         // Discharge coefficient for opening factor #1
                state.afn->MultizoneCompDetOpeningData(i).WidthFac1 = width_factors[0]; // Width factor for for Opening factor #1
                state.afn->MultizoneCompDetOpeningData(i).HeightFac1 = height_factors[0]; // Height factor for opening factor #1
                state.afn->MultizoneCompDetOpeningData(i).StartHFac1 =
                    start_height_factors[0];                                                           // Start height factor for opening factor #1
                state.afn->MultizoneCompDetOpeningData(i).OpenFac2 = factors[1];        // Opening factor #2
                state.afn->MultizoneCompDetOpeningData(i).DischCoeff2 = cds[1];         // Discharge coefficient for opening factor #2
                state.afn->MultizoneCompDetOpeningData(i).WidthFac2 = width_factors[1]; // Width factor for for Opening factor #2
                state.afn->MultizoneCompDetOpeningData(i).HeightFac2 = height_factors[1]; // Height factor for opening factor #2
                state.afn->MultizoneCompDetOpeningData(i).StartHFac2 =
                    start_height_factors[1]; // Start height factor for opening factor #2

                state.afn->MultizoneCompDetOpeningData(i).OpenFac3 = 0.0;    // Opening factor #3
                state.afn->MultizoneCompDetOpeningData(i).DischCoeff3 = 0.0; // Discharge coefficient for opening factor #3
                state.afn->MultizoneCompDetOpeningData(i).WidthFac3 = 0.0;   // Width factor for for Opening factor #3
                state.afn->MultizoneCompDetOpeningData(i).HeightFac3 = 0.0;  // Height factor for opening factor #3
                state.afn->MultizoneCompDetOpeningData(i).StartHFac3 = 0.0;  // Start height factor for opening factor #3
                state.afn->MultizoneCompDetOpeningData(i).OpenFac4 = 0.0;    // Opening factor #4
                state.afn->MultizoneCompDetOpeningData(i).DischCoeff4 = 0.0; // Discharge coefficient for opening factor #4
                state.afn->MultizoneCompDetOpeningData(i).WidthFac4 = 0.0;   // Width factor for for Opening factor #4
                state.afn->MultizoneCompDetOpeningData(i).HeightFac4 = 0.0;  // Height factor for opening factor #4
                state.afn->MultizoneCompDetOpeningData(i).StartHFac4 = 0.0;  // Start height factor for opening factor #4
                if (N == 2) {
                    if (factors[1] != 1.0) {
                        ShowWarningError(state, format(RoutineName) + ": " + CurrentModuleObject + " = " + thisObjectName);
                        ShowContinueError(
                            state,
                            "..This object specifies that only 3 opening factors will be used. So, the value of Opening Factor #2 is set to 1.0.");
                        ShowContinueError(state,
                                          format("..Input value was {:.2R}", state.afn->MultizoneCompDetOpeningData(i).OpenFac2));
                        state.afn->MultizoneCompDetOpeningData(i).OpenFac2 = 1.0;
                    }
                } else if (N >= 3) {
                    state.afn->MultizoneCompDetOpeningData(i).OpenFac3 = factors[2]; // Opening factor #3
                    state.afn->MultizoneCompDetOpeningData(i).DischCoeff3 = cds[2];  // Discharge coefficient for opening factor #3
                    state.afn->MultizoneCompDetOpeningData(i).WidthFac3 = width_factors[2];   // Width factor for for Opening factor #3
                    state.afn->MultizoneCompDetOpeningData(i).HeightFac3 = height_factors[2]; // Height factor for opening factor #3
                    state.afn->MultizoneCompDetOpeningData(i).StartHFac3 =
                        start_height_factors[2]; // Start height factor for opening factor #3
                    if (N >= 4) {
                        state.afn->MultizoneCompDetOpeningData(i).OpenFac4 = factors[3]; // Opening factor #4
                        if (factors[3] != 1.0) {
                            ShowWarningError(state, format(RoutineName) + ": " + CurrentModuleObject + " = " + thisObjectName);
                            ShowContinueError(state,
                                              "..This object specifies that 4 opening factors will be used. So, the value of Opening Factor #4 "
                                              "is set to 1.0.");
                            ShowContinueError(state,
                                              format("..Input value was {:.2R}", state.afn->MultizoneCompDetOpeningData(i).OpenFac4));
                            state.afn->MultizoneCompDetOpeningData(i).OpenFac4 = 1.0;
                        }
                        state.afn->MultizoneCompDetOpeningData(i).DischCoeff4 = cds[3]; // Discharge coefficient for opening factor #4
                        state.afn->MultizoneCompDetOpeningData(i).WidthFac4 =
                            width_factors[3]; // Width factor for for Opening factor #4
                        state.afn->MultizoneCompDetOpeningData(i).HeightFac4 =
                            height_factors[3]; // Height factor for opening factor #4
                        state.afn->MultizoneCompDetOpeningData(i).StartHFac4 =
                            start_height_factors[3]; // Start height factor for opening factor #4
                    } else {
                        if (factors[2] != 1.0) {
                            ShowWarningError(state, format(RoutineName) + ": " + CurrentModuleObject + " = " + thisObjectName);
                            ShowContinueError(state,
                                              "..This object specifies that only 3 opening factors will be used. So, the value of Opening Factor #3 "
                                              "is set to 1.0.");
                            ShowContinueError(state,
                                              format("..Input value was {:.2R}", state.afn->MultizoneCompDetOpeningData(i).OpenFac3));
                            state.afn->MultizoneCompDetOpeningData(i).OpenFac3 = 1.0;
                        }
                    }
                }

                // Sanity checks, check sum of Height Factor and the Start Height Factor
                if (state.afn->MultizoneCompDetOpeningData(i).HeightFac1 +
                        state.afn->MultizoneCompDetOpeningData(i).StartHFac1 >
                    1.0) {
                    ShowSevereError(state, format(RoutineName) + ": " + CurrentModuleObject + " = " + thisObjectName);
                    ShowContinueError(
                        state, "..The sum of Height Factor for Opening Factor 1 and Start Height Factor for Opening Factor 1 is greater than 1.0");
                    success = false;
                }
                if (state.afn->MultizoneCompDetOpeningData(i).HeightFac2 +
                        state.afn->MultizoneCompDetOpeningData(i).StartHFac2 >
                    1.0) {
                    ShowSevereError(state, format(RoutineName) + ": " + CurrentModuleObject + " = " + thisObjectName);
                    ShowContinueError(
                        state, "..The sum of Height Factor for Opening Factor 2 and Start Height Factor for Opening Factor 2 is greater than 1.0");
                    success = false;
                }
                if (state.afn->MultizoneCompDetOpeningData(i).NumFac > 2) {
                    if (state.afn->MultizoneCompDetOpeningData(i).OpenFac2 >=
                        state.afn->MultizoneCompDetOpeningData(i).OpenFac3) {
                        ShowSevereError(state, format(RoutineName) + ": " + CurrentModuleObject + " = " + thisObjectName);
                        ShowContinueError(state, "..The value of Opening Factor #2 >= the value of Opening Factor #3");
                        success = false;
                    }
                    if (state.afn->MultizoneCompDetOpeningData(i).HeightFac3 +
                            state.afn->MultizoneCompDetOpeningData(i).StartHFac3 >
                        1.0) {
                        ShowSevereError(state, format(RoutineName) + ": " + CurrentModuleObject + " = " + thisObjectName);
                        ShowContinueError(
                            state,
                            "..The sum of Height Factor for Opening Factor 3 and Start Height Factor for Opening Factor 3 is greater than 1.0");
                        success = false;
                    }
                    if (state.afn->MultizoneCompDetOpeningData(i).NumFac == 4) {
                        if (state.afn->MultizoneCompDetOpeningData(i).OpenFac3 >=
                            state.afn->MultizoneCompDetOpeningData(i).OpenFac4) {
                            ShowSevereError(state, format(RoutineName) + ": " + CurrentModuleObject + " = " + thisObjectName);
                            ShowContinueError(state, "..The value of Opening Factor #3 >= the value of Opening Factor #4");
                            success = false;
                        }
                        if (state.afn->MultizoneCompDetOpeningData(i).HeightFac4 +
                                state.afn->MultizoneCompDetOpeningData(i).StartHFac4 >
                            1.0) {
                            ShowSevereError(state, format(RoutineName) + ": " + CurrentModuleObject + " = " + thisObjectName);
                            ShowContinueError(
                                state,
                                "..The sum of Height Factor for Opening Factor 4 and Start Height Factor for Opening Factor 4 is greater than 1.0");
                            success = false;
                        }
                    }
                }

                // Add the element to the lookup table, check for name overlaps
                if (solver->elements.find(thisObjectName) == solver->elements.end()) {
                    solver->elements[thisObjectName] = &state.afn->MultizoneCompDetOpeningData(i); // Yet another workaround
                } else {
                    ShowSevereError(state, format(RoutineName) + "Duplicated airflow element names are found = " + thisObjectName);
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork simulation simple openings
        CurrentModuleObject = "AirflowNetwork:MultiZone:Component:SimpleOpening";
        state.afn->AirflowNetworkNumOfSimOpenings =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1; // Temporary workaround
            state.afn->MultizoneCompSimpleOpeningData.allocate(
                state.afn->AirflowNetworkNumOfSimOpenings); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 coeff{fields.at("air_mass_flow_coefficient_when_opening_is_closed")};
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent_when_opening_is_closed") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent_when_opening_is_closed").get<Real64>();
                }
                Real64 diff{fields.at("minimum_density_difference_for_two_way_flow")};
                Real64 dischargeCoeff{fields.at("discharge_coefficient")};

                state.afn->MultizoneCompSimpleOpeningData(i).name = thisObjectName; // Name of large simple opening component
                state.afn->MultizoneCompSimpleOpeningData(i).FlowCoef =
                    coeff; // Air Mass Flow Coefficient When Window or Door Is Closed
                state.afn->MultizoneCompSimpleOpeningData(i).FlowExpo = expnt;  // Air Mass Flow exponent When Window or Door Is Closed
                state.afn->MultizoneCompSimpleOpeningData(i).MinRhoDiff = diff; // Minimum density difference for two-way flow
                state.afn->MultizoneCompSimpleOpeningData(i).DischCoeff = dischargeCoeff; // Discharge coefficient at full opening

                // Add the element to the lookup table, check for name overlaps
                if (solver->elements.find(thisObjectName) == solver->elements.end()) {
                    solver->elements[thisObjectName] = &state.afn->MultizoneCompSimpleOpeningData(i); // Yet another workaround
                } else {
                    ShowSevereError(state, format(RoutineName) + "Duplicated airflow element names are found = " + thisObjectName);
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork simulation horizontal openings
        CurrentModuleObject = "AirflowNetwork:MultiZone:Component:HorizontalOpening";
        state.afn->AirflowNetworkNumOfHorOpenings =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1; // Temporary workaround
            state.afn->MultizoneCompHorOpeningData.allocate(
                state.afn->AirflowNetworkNumOfHorOpenings); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 coeff{fields.at("air_mass_flow_coefficient_when_opening_is_closed")};
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent_when_opening_is_closed") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent_when_opening_is_closed").get<Real64>();
                }
                Real64 angle{90.0};
                if (fields.find("sloping_plane_angle") != fields.end()) {
                    angle = fields.at("sloping_plane_angle").get<Real64>();
                }
                Real64 dischargeCoeff{fields.at("discharge_coefficient")};

                state.afn->MultizoneCompHorOpeningData(i).name = thisObjectName; // Name of large simple opening component
                state.afn->MultizoneCompHorOpeningData(i).FlowCoef = coeff; // Air Mass Flow Coefficient When Window or Door Is Closed
                state.afn->MultizoneCompHorOpeningData(i).FlowExpo = expnt; // Air Mass Flow exponent When Window or Door Is Closed
                state.afn->MultizoneCompHorOpeningData(i).Slope = angle;    // Sloping plane angle
                state.afn->MultizoneCompHorOpeningData(i).DischCoeff = dischargeCoeff; // Discharge coefficient at full opening

                // Add the element to the lookup table, check for name overlaps
                if (solver->elements.find(thisObjectName) == solver->elements.end()) {
                    solver->elements[thisObjectName] = &state.afn->MultizoneCompHorOpeningData(i); // Yet another workaround
                } else {
                    ShowSevereError(state, format(RoutineName) + "Duplicated airflow element names are found = " + thisObjectName);
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // *** Read AirflowNetwork simulation surface effective leakage area component
        CurrentModuleObject = "AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea";
        state.afn->AirflowNetworkNumOfSurELA =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1; // Temporary workaround
            state.afn->MultizoneSurfaceELAData.allocate(
                state.afn->AirflowNetworkNumOfSurELA); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 ela{fields.at("effective_leakage_area")};
                Real64 cd{1.0};
                if (fields.find("discharge_coefficient") != fields.end()) {
                    cd = fields.at("discharge_coefficient").get<Real64>();
                }
                Real64 dp{4.0};
                if (fields.find("reference_pressure_difference") != fields.end()) {
                    dp = fields.at("reference_pressure_difference").get<Real64>();
                }
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent").get<Real64>();
                }

                state.afn->MultizoneSurfaceELAData(i).name = thisObjectName; // Name of surface effective leakage area component
                state.afn->MultizoneSurfaceELAData(i).ELA = ela;             // Effective leakage area
                state.afn->MultizoneSurfaceELAData(i).DischCoeff = cd;       // Discharge coefficient
                state.afn->MultizoneSurfaceELAData(i).RefDeltaP = dp;        // Reference pressure difference
                state.afn->MultizoneSurfaceELAData(i).FlowExpo = expnt;      // Air Mass Flow exponent
                state.afn->MultizoneSurfaceELAData(i).TestDeltaP = 0.0;      // Testing pressure difference
                state.afn->MultizoneSurfaceELAData(i).TestDisCoef = 0.0;     // Testing Discharge coefficient

                // Add the element to the lookup table, check for name overlaps
                if (solver->elements.find(thisObjectName) == solver->elements.end()) {
                    solver->elements[thisObjectName] = &state.afn->MultizoneSurfaceELAData(i); // Yet another workaround
                } else {
                    ShowSevereError(state, format(RoutineName) + "Duplicated airflow element names are found = " + thisObjectName);
                    success = false;
                }

                ++i;
            }
        }

        // *** Read AirflowNetwork simulation specified flow components
        CurrentModuleObject = "AirflowNetwork:MultiZone:SpecifiedFlowRate";
        state.afn->AirflowNetworkNumOfSFR =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i_mass = 0; // Temporary workaround that increasingly looks like the long term solution
            int i_vol = 0;
            auto &instancesValue = instances.value();

            instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 flow_rate{fields.at("air_flow_value")};
                bool is_mass_flow = true;
                if (fields.find("air_flow_units") != fields.end()) {
                    if (fields.at("air_flow_units") != "MassFlow") {
                        is_mass_flow = false;
                    }
                }

                // Check for name overlaps
                if (solver->elements.find(thisObjectName) != solver->elements.end()) {
                    ShowSevereError(state, format(RoutineName) + "Duplicated airflow element names are found = " + thisObjectName);
                    success = false;
                }

                if (is_mass_flow) {
                    state.afn->SpecifiedMassFlowData.emplace_back();
                    state.afn->SpecifiedMassFlowData[i_mass].name = thisObjectName;
                    state.afn->SpecifiedMassFlowData[i_mass].mass_flow = flow_rate;
                    solver->elements[thisObjectName] = &state.afn->SpecifiedMassFlowData[i_mass]; // Yet another workaround
                    ++i_mass;
                } else {
                    state.afn->SpecifiedVolumeFlowData.emplace_back();
                    state.afn->SpecifiedVolumeFlowData[i_vol].name = thisObjectName;
                    state.afn->SpecifiedVolumeFlowData[i_vol].volume_flow = flow_rate;
                    solver->elements[thisObjectName] = &state.afn->SpecifiedVolumeFlowData[i_vol]; // Yet another workaround
                    ++i_vol;
                }
            }
        }

        // Read AirflowNetwork Distribution system component: duct leakage
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:Leak";
        state.afn->DisSysNumOfLeaks =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                                                                                       // Temporary workaround
            state.afn->DisSysCompLeakData.allocate(state.afn->DisSysNumOfLeaks); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 coeff{fields.at("air_mass_flow_coefficient")};
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent").get<Real64>();
                }

                state.afn->DisSysCompLeakData(i).name = thisObjectName; // Name of duct leak component
                state.afn->DisSysCompLeakData(i).FlowCoef = coeff;      // Air Mass Flow Coefficient
                state.afn->DisSysCompLeakData(i).FlowExpo = expnt;      // Air Mass Flow exponent

                // Add the element to the lookup table, check for name overlaps
                if (solver->elements.find(thisObjectName) == solver->elements.end()) {
                    solver->elements[thisObjectName] = &state.afn->DisSysCompLeakData(i); // Yet another workaround
                } else {
                    ShowSevereError(state, format(RoutineName) + "Duplicated airflow element names are found = " + thisObjectName);
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork Distribution system component: duct effective leakage ratio
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:LeakageRatio";
        state.afn->DisSysNumOfELRs =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                                                                                     // Temporary workaround
            state.afn->DisSysCompELRData.allocate(state.afn->DisSysNumOfELRs); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 elr{fields.at("effective_leakage_ratio")};
                Real64 maxflow{fields.at("maximum_flow_rate")};
                Real64 dp{fields.at("reference_pressure_difference")};
                Real64 expnt{0.65};
                if (fields.find("air_mass_flow_exponent") != fields.end()) {
                    expnt = fields.at("air_mass_flow_exponent").get<Real64>();
                }

                state.afn->DisSysCompELRData(i).name = thisObjectName; // Name of duct effective leakage ratio component
                state.afn->DisSysCompELRData(i).ELR = elr;             // Value of effective leakage ratio
                state.afn->DisSysCompELRData(i).FlowRate = maxflow * state.dataEnvrn->StdRhoAir; // Maximum airflow rate
                state.afn->DisSysCompELRData(i).RefPres = dp;                                    // Reference pressure difference
                state.afn->DisSysCompELRData(i).FlowExpo = expnt;                                // Air Mass Flow exponent

                // Add the element to the lookup table, check for name overlaps
                if (solver->elements.find(thisObjectName) == solver->elements.end()) {
                    solver->elements[thisObjectName] = &state.afn->DisSysCompELRData(i); // Yet another workaround
                } else {
                    ShowSevereError(state, format(RoutineName) + "Duplicated airflow element names are found = " + thisObjectName);
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork Distribution system component: duct
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:Duct";
        state.afn->DisSysNumOfDucts =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                                                                                       // Temporary workaround
            state.afn->DisSysCompDuctData.allocate(state.afn->DisSysNumOfDucts); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 L{fields.at("duct_length")};
                Real64 D{fields.at("hydraulic_diameter")};
                Real64 A{fields.at("cross_section_area")};
                Real64 e{0.0009};
                if (fields.find("surface_roughness") != fields.end()) {
                    e = fields.at("surface_roughness").get<Real64>();
                }
                Real64 dlc{0.0};
                if (fields.find("coefficient_for_local_dynamic_loss_due_to_fitting") != fields.end()) {
                    dlc = fields.at("coefficient_for_local_dynamic_loss_due_to_fitting").get<Real64>();
                }
                Real64 U{0.943};
                if (fields.find("heat_transmittance_coefficient_u_factor_for_duct_wall_construction") != fields.end()) {
                    U = fields.at("heat_transmittance_coefficient_u_factor_for_duct_wall_construction").get<Real64>();
                }
                Real64 Um{0.001};
                if (fields.find("overall_moisture_transmittance_coefficient_from_air_to_air") != fields.end()) {
                    Um = fields.at("overall_moisture_transmittance_coefficient_from_air_to_air").get<Real64>();
                }
                Real64 hout{0.0};
                if (fields.find("outside_convection_coefficient") != fields.end()) {
                    hout = fields.at("outside_convection_coefficient").get<Real64>();
                }
                Real64 hin{0.0};
                if (fields.find("inside_convection_coefficient") != fields.end()) {
                    hin = fields.at("inside_convection_coefficient").get<Real64>();
                }

                state.afn->DisSysCompDuctData(i).name = thisObjectName;   // Name of duct effective leakage ratio component
                state.afn->DisSysCompDuctData(i).L = L;                   // Duct length [m]
                state.afn->DisSysCompDuctData(i).hydraulicDiameter = D;   // Hydraulic diameter [m]
                state.afn->DisSysCompDuctData(i).A = A;                   // Cross section area [m2]
                state.afn->DisSysCompDuctData(i).roughness = e;           // Surface roughness [m]
                state.afn->DisSysCompDuctData(i).TurDynCoef = dlc;        // Turbulent dynamic loss coefficient
                state.afn->DisSysCompDuctData(i).UThermConduct = U;       // Conduction heat transmittance [W/m2.K]
                state.afn->DisSysCompDuctData(i).UMoisture = Um;          // Overall moisture transmittance [kg/m2]
                state.afn->DisSysCompDuctData(i).OutsideConvCoeff = hout; // Outside convection coefficient [W/m2.K]
                state.afn->DisSysCompDuctData(i).InsideConvCoeff = hin;   // Inside convection coefficient [W/m2.K]
                state.afn->DisSysCompDuctData(i).MThermal = 0.0;          // Thermal capacity [J/K]
                state.afn->DisSysCompDuctData(i).MMoisture = 0.0;         // Moisture capacity [kg]
                state.afn->DisSysCompDuctData(i).LamDynCoef = 64.0;       // Laminar dynamic loss coefficient
                state.afn->DisSysCompDuctData(i).LamFriCoef = dlc;        // Laminar friction loss coefficient
                state.afn->DisSysCompDuctData(i).InitLamCoef = 128.0;     // Coefficient of linear initialization
                state.afn->DisSysCompDuctData(i).RelRough = e / D;        // e/D: relative roughness
                state.afn->DisSysCompDuctData(i).RelL = L / D;            // L/D: relative length
                state.afn->DisSysCompDuctData(i).A1 =
                    1.14 - 0.868589 * std::log(state.afn->DisSysCompDuctData(i).RelRough); // 1.14 - 0.868589*ln(e/D)
                state.afn->DisSysCompDuctData(i).g =
                    state.afn->DisSysCompDuctData(i).A1; // 1/sqrt(Darcy friction factor)

                // Add the element to the lookup table, check for name overlaps
                if (solver->elements.find(thisObjectName) == solver->elements.end()) {
                    solver->elements[thisObjectName] = &state.afn->DisSysCompDuctData(i); // Yet another workaround
                } else {
                    ShowSevereError(state, format(RoutineName) + "Duplicated airflow element names are found = " + thisObjectName);
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork Distribution system component: constant volume fan
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:Fan";
        state.afn->DisSysNumOfCVFs =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        if (state.afn->DisSysNumOfCVFs > 0 &&
            state.afn->DisSysNumOfCVFs !=
                state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC")) {
            ShowSevereError(state,
                            format("The number of entered AirflowNetwork:Distribution:Component:Fan objects is {}",
                                   state.afn->DisSysNumOfCVFs));
            ShowSevereError(state,
                            format("The number of entered AirLoopHVAC objects is {}",
                                   state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC")));
            ShowContinueError(state, "Both numbers should be equal. Please check your inputs.");
            success = false;
        }

        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                                                                                     // Temporary workaround
            state.afn->DisSysCompCVFData.allocate(state.afn->DisSysNumOfCVFs); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                std::string fan_name = UtilityRoutines::MakeUPPERCase(fields.at("fan_name").get<std::string>());
                std::string fan_type = fields.at("supply_fan_object_type").get<std::string>();

                bool FanErrorFound = false;
                int fanIndex;
                Real64 flowRate = 0.0;
                int fanType_Num = 0;
                int inletNode;
                int outletNode;

                if (UtilityRoutines::SameString(UtilityRoutines::MakeUPPERCase(fan_type), "FAN:SYSTEMMODEL")) {
                    state.dataHVACFan->fanObjs.emplace_back(new HVACFan::FanSystem(state, fan_name));
                    fanIndex = HVACFan::getFanObjectVectorIndex(state, fan_name);
                    if (fanIndex < 0) {
                        ShowSevereError(state, "...occurs in " + CurrentModuleObject + " = " + state.afn->DisSysCompCVFData(i).name);
                        success = false;
                    } else {
                        flowRate = state.dataHVACFan->fanObjs[fanIndex]->designAirVolFlowRate;
                        flowRate *= state.dataEnvrn->StdRhoAir;
                        state.afn->DisSysCompCVFData(i).FanModelFlag = true;
                        inletNode = state.dataHVACFan->fanObjs[fanIndex]->inletNodeNum;
                        outletNode = state.dataHVACFan->fanObjs[fanIndex]->outletNodeNum;
                        if (state.dataHVACFan->fanObjs[fanIndex]->speedControl == HVACFan::FanSystem::SpeedControlMethod::Continuous) {
                            fanType_Num = FanType_SimpleVAV;
                            state.afn->VAVSystem = true;
                        } else {
                            fanType_Num = FanType_SimpleOnOff;
                        }
                        state.afn->SupplyFanType = fanType_Num;
                    }

                } else {

                    GetFanIndex(state, fan_name, fanIndex, FanErrorFound);

                    if (FanErrorFound) {
                        ShowSevereError(state, "...occurs in " + CurrentModuleObject + " = " + state.afn->DisSysCompCVFData(i).name);
                        success = false;
                    }

                    GetFanVolFlow(state, fanIndex, flowRate);
                    flowRate *= state.dataEnvrn->StdRhoAir;

                    GetFanType(state, fan_name, fanType_Num, FanErrorFound);
                    state.afn->SupplyFanType = fanType_Num;
                }

                if (!(fanType_Num == FanType_SimpleConstVolume || fanType_Num == FanType_SimpleOnOff || fanType_Num == FanType_SimpleVAV)) {
                    ShowSevereError(state,
                                    format(RoutineName) + "The Supply Fan Object Type in " + CurrentModuleObject + " = " + thisObjectName +
                                        " is not a valid fan type.");
                    ShowContinueError(state, "Valid fan types are  Fan:ConstantVolume, Fan:OnOff, Fan:VariableVolume, or Fan:SystemModel.");
                    success = false;
                } else {
                    if (UtilityRoutines::SameString(fan_type, "Fan:ConstantVolume") && fanType_Num == FanType_SimpleOnOff) {
                        ShowSevereError(state, "The Supply Fan Object Type defined in " + CurrentModuleObject + " is " + fan_type);
                        ShowContinueError(state, "The Supply Fan Object Type defined in an AirLoopHVAC is Fan:OnOff");
                        success = false;
                    }
                    if (UtilityRoutines::SameString(fan_type, "Fan:OnOff") && fanType_Num == FanType_SimpleConstVolume) {
                        ShowSevereError(state, "The Supply Fan Object Type defined in " + CurrentModuleObject + " is " + fan_type);
                        ShowContinueError(state, "The Supply Fan Object Type defined in an AirLoopHVAC is Fan:ConstantVolume");
                        success = false;
                    }
                }
                bool ErrorsFound{false};
                if (fanType_Num == FanType_SimpleConstVolume) {
                    inletNode = GetFanInletNode(state, "Fan:ConstantVolume", fan_name, ErrorsFound);
                    outletNode = GetFanOutletNode(state, "Fan:ConstantVolume", fan_name, ErrorsFound);
                }
                if (fanType_Num == FanType_SimpleOnOff && !state.afn->DisSysCompCVFData(i).FanModelFlag) {
                    inletNode = GetFanInletNode(state, "Fan:OnOff", fan_name, ErrorsFound);
                    outletNode = GetFanOutletNode(state, "Fan:OnOff", fan_name, ErrorsFound);
                }
                if (fanType_Num == FanType_SimpleVAV && !state.afn->DisSysCompCVFData(i).FanModelFlag) {
                    inletNode = GetFanInletNode(state, "Fan:VariableVolume", fan_name, ErrorsFound);
                    outletNode = GetFanOutletNode(state, "Fan:VariableVolume", fan_name, ErrorsFound);
                    state.afn->VAVSystem = true;
                }

                if (ErrorsFound) {
                    success = false;
                }

                state.afn->DisSysCompCVFData(i).name = fan_name; // Name of duct effective leakage ratio component
                state.afn->DisSysCompCVFData(i).Ctrl = 1.0;      // Control ratio
                state.afn->DisSysCompCVFData(i).FanIndex = fanIndex;
                state.afn->DisSysCompCVFData(i).FlowRate = flowRate;
                state.afn->DisSysCompCVFData(i).FanTypeNum = fanType_Num;
                state.afn->DisSysCompCVFData(i).InletNode = inletNode;
                state.afn->DisSysCompCVFData(i).OutletNode = outletNode;

                // Add the element to the lookup table, check for name overlaps
                if (solver->elements.find(fan_name) == solver->elements.end()) {
                    solver->elements[fan_name] = &state.afn->DisSysCompCVFData(i); // Yet another workaround
                } else {
                    ShowSevereError(state, format(RoutineName) + "Duplicated airflow element names are found = " + fan_name);
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork Distribution system component: coil
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:Coil";
        state.afn->DisSysNumOfCoils =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                                                                                       // Temporary workaround
            state.afn->DisSysCompCoilData.allocate(state.afn->DisSysNumOfCoils); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                // auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                std::string coil_name = fields.at("coil_name").get<std::string>();
                std::string coil_type = fields.at("coil_object_type").get<std::string>();
                Real64 L{fields.at("air_path_length")};
                Real64 D{fields.at("air_path_hydraulic_diameter")};

                state.afn->DisSysCompCoilData(i).name =
                    UtilityRoutines::MakeUPPERCase(coil_name);                         // Name of associated EPlus coil component
                state.afn->DisSysCompCoilData(i).EPlusType = coil_type; // coil type
                state.afn->DisSysCompCoilData(i).L = L;                 // Air path length
                state.afn->DisSysCompCoilData(i).hydraulicDiameter = D; // Air path hydraulic diameter

                // Add the element to the lookup table, check for name overlaps
                if (solver->elements.find(state.afn->DisSysCompCoilData(i).name) == solver->elements.end()) {
                    solver->elements[state.afn->DisSysCompCoilData(i).name] =
                        &state.afn->DisSysCompCoilData(i); // Yet another workaround
                } else {
                    ShowSevereError(state,
                                    format(RoutineName) +
                                        "Duplicated airflow element names are found = " + state.afn->DisSysCompCoilData(i).name);
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Read AirflowNetwork Distribution system component: heat exchanger
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:HeatExchanger";
        state.afn->DisSysNumOfHXs =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                                                                                   // Temporary workaround
            state.afn->DisSysCompHXData.allocate(state.afn->DisSysNumOfHXs); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                // auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                std::string hx_name = fields.at("heatexchanger_name").get<std::string>();
                std::string hx_type = fields.at("heatexchanger_object_type").get<std::string>();
                Real64 L{fields.at("air_path_length")};
                Real64 D{fields.at("air_path_hydraulic_diameter")};

                state.afn->DisSysCompHXData(i).name =
                    UtilityRoutines::MakeUPPERCase(hx_name);                         // Name of associated EPlus heat exchange component
                state.afn->DisSysCompHXData(i).EPlusType = hx_type;   // coil type
                state.afn->DisSysCompHXData(i).L = L;                 // Air path length
                state.afn->DisSysCompHXData(i).hydraulicDiameter = D; // Air path hydraulic diameter
                state.afn->DisSysCompHXData(i).CoilParentExists =
                    HVACHXAssistedCoolingCoil::VerifyHeatExchangerParent(state, hx_type, hx_name);

                // Add the element to the lookup table, check for name overlaps
                if (solver->elements.find(state.afn->DisSysCompHXData(i).name) == solver->elements.end()) {
                    solver->elements[state.afn->DisSysCompHXData(i).name] =
                        &state.afn->DisSysCompHXData(i); // Yet another workaround
                } else {
                    ShowSevereError(state,
                                    format(RoutineName) +
                                        "Duplicated airflow element names are found = " + state.afn->DisSysCompHXData(i).name);
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }
                ++i;
            }
        }

        // Read AirflowNetwork Distribution system component: terminal unit
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:TerminalUnit";
        state.afn->DisSysNumOfTermUnits =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1; // Temporary workaround
            state.afn->DisSysCompTermUnitData.allocate(
                state.afn->DisSysNumOfTermUnits); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                // auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                std::string tu_name = fields.at("terminal_unit_name").get<std::string>();
                std::string tu_type = fields.at("terminal_unit_object_type").get<std::string>();
                Real64 L{fields.at("air_path_length")};
                Real64 D{fields.at("air_path_hydraulic_diameter")};

                state.afn->DisSysCompTermUnitData(i).name =
                    UtilityRoutines::MakeUPPERCase(tu_name);                               // Name of associated EPlus coil component
                state.afn->DisSysCompTermUnitData(i).EPlusType = tu_type;   // Terminal unit type
                state.afn->DisSysCompTermUnitData(i).L = L;                 // Air path length
                state.afn->DisSysCompTermUnitData(i).hydraulicDiameter = D; // Air path hydraulic diameter

                // Add the element to the lookup table, check for name overlaps
                if (solver->elements.find(state.afn->DisSysCompTermUnitData(i).name) == solver->elements.end()) {
                    solver->elements[state.afn->DisSysCompTermUnitData(i).name] =
                        &state.afn->DisSysCompTermUnitData(i); // Yet another workaround
                } else {
                    ShowSevereError(state,
                                    format(RoutineName) +
                                        "Duplicated airflow element names are found = " + state.afn->DisSysCompTermUnitData(i).name);
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        // Get input data of constant pressure drop component
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:ConstantPressureDrop";
        state.afn->DisSysNumOfCPDs =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject); // Temporary workaround
        instances = state.dataInputProcessing->inputProcessor->epJSON.find(CurrentModuleObject);
        if (instances != state.dataInputProcessing->inputProcessor->epJSON.end()) {
            int i = 1;                                                                                                     // Temporary workaround
            state.afn->DisSysCompCPDData.allocate(state.afn->DisSysNumOfCPDs); // Temporary workaround
            auto &instancesValue = instances.value();
            for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
                auto const &fields = instance.value();
                auto const &thisObjectName = UtilityRoutines::MakeUPPERCase(instance.key());
                state.dataInputProcessing->inputProcessor->markObjectAsUsed(CurrentModuleObject, instance.key()); // Temporary workaround

                Real64 dp{fields.at("pressure_difference_across_the_component")};

                state.afn->DisSysCompCPDData(i).name = thisObjectName; // Name of constant pressure drop component
                state.afn->DisSysCompCPDData(i).A = 1.0;               // cross section area
                state.afn->DisSysCompCPDData(i).DP = dp;               // Pressure difference across the component

                // Add the element to the lookup table, check for name overlaps
                if (solver->elements.find(thisObjectName) == solver->elements.end()) {
                    solver->elements[thisObjectName] = &state.afn->DisSysCompCPDData(i); // Yet another workaround
                } else {
                    ShowSevereError(state, format(RoutineName) + "Duplicated airflow element names are found = " + thisObjectName);
                    // ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2));
                    success = false;
                }

                ++i;
            }
        }

        return success;
    }

    void GetAirflowNetworkInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Aug. 2003
        //       MODIFIED       Aug. 2005
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reads inputs of air distribution system

        // Using/Aliasing
        using CurveManager::GetCurveIndex;
        using DataLoopNode::ObjectIsParent;
        using HVACHXAssistedCoolingCoil::VerifyHeatExchangerParent;
        using MixedAir::GetOAMixerNumber;
        using NodeInputManager::GetOnlySingleNode;
        using OutAirNodeManager::SetOutAirNodes;
        using RoomAirModelManager::GetRAFNNodeNum;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetAirflowNetworkInput: "); // include trailing blank space

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // int i;
        int n;
        int j;
        int k;
        int m;
        int count;
        bool NodeFound;
        bool CompFound;
        bool ErrorsFound;
        bool found;
        bool NodeFound1;
        bool NodeFound2;
        int NumAPL;
        Array1D_string CompName(2);
        std::string SimAirNetworkKey;
        bool SimObjectError;
        std::string StringOut;
        int ZoneNum;
        int NodeNum;

        // Declare variables used in this subroutine for debug purpose
        bool AirflowNetworkInitFlag;
        Array1D_int ZoneCheck;
        Array1D_int ZoneBCCheck;
        bool SurfaceFound;

        int NumAlphas;  // Number of Alphas for each GetObjectItem call
        int NumNumbers; // Number of Numbers for each GetObjectItem call
        int IOStatus;   // Used in GetObjectItem
        std::string CurrentModuleObject;
        Array1D_string Alphas;         // Alpha input items for object
        Array1D_string cAlphaFields;   // Alpha field names
        Array1D_string cNumericFields; // Numeric field names
        Array1D<Real64> Numbers;       // Numeric input items for object
        Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.
        int MaxNums(0);                // Maximum number of numeric input fields
        int MaxAlphas(0);              // Maximum number of alpha input fields
        int TotalArgs(0);              // Total number of alpha and numeric arguments (max) for a
        bool Errorfound1;
        Real64 minHeight;
        Real64 maxHeight;
        Real64 baseratio;

        auto &Node(state.dataLoopNodes->Node);

        // Formats
        static constexpr std::string_view Format_110(
            "! <AirflowNetwork Model:Control>, No Multizone or Distribution/Multizone with Distribution/Multizone "
            "without Distribution/Multizone with Distribution only during Fan Operation\n");
        static constexpr std::string_view Format_120("AirflowNetwork Model:Control,{}\n");

        // Set the maximum numbers of input fields
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "AirflowNetwork:SimulationControl", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "AirflowNetwork:MultiZone:Zone", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "AirflowNetwork:MultiZone:Surface", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "AirflowNetwork:MultiZone:Component:DetailedOpening", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "AirflowNetwork:MultiZone:ExternalNode", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "AirflowNetwork:MultiZone:WindPressureCoefficientArray", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "AirflowNetwork:MultiZone:WindPressureCoefficientValues", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "AirflowNetwork:Distribution:Node", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "AirflowNetwork:Distribution:DuctViewFactors", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "AirflowNetwork:Distribution:Linkage", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "AirflowNetwork:OccupantVentilationControl", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "AirflowNetwork:IntraZone:Node", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "AirflowNetwork:IntraZone:Linkage", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "AirflowNetwork:ZoneControl:PressureController", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        Alphas.allocate(MaxAlphas);
        cAlphaFields.allocate(MaxAlphas);
        cNumericFields.allocate(MaxNums);
        Numbers.dimension(MaxNums, 0.0);
        lAlphaBlanks.dimension(MaxAlphas, true);
        lNumericBlanks.dimension(MaxNums, true);

        ErrorsFound = false;
        AirflowNetworkInitFlag = false;

        auto &Zone(state.dataHeatBal->Zone);

        // Read AirflowNetwork OccupantVentilationControl before reading other AirflowNetwork objects, so that this object can be called by other
        // simple ventilation objects
        CurrentModuleObject = "AirflowNetwork:OccupantVentilationControl";
        state.afn->AirflowNetworkNumOfOccuVentCtrls =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        if (state.afn->AirflowNetworkNumOfOccuVentCtrls > 0) {
            state.afn->OccupantVentilationControl.allocate(
                state.afn->AirflowNetworkNumOfOccuVentCtrls);
            for (int i = 1; i <= state.afn->AirflowNetworkNumOfOccuVentCtrls; ++i) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         i,
                                                                         Alphas,
                                                                         NumAlphas,
                                                                         Numbers,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericBlanks,
                                                                         lAlphaBlanks,
                                                                         cAlphaFields,
                                                                         cNumericFields);
                UtilityRoutines::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);
                state.afn->OccupantVentilationControl(i).Name = Alphas(1); // Name of object
                state.afn->OccupantVentilationControl(i).MinOpeningTime = Numbers(1);
                if (state.afn->OccupantVentilationControl(i).MinOpeningTime < 0.0) {
                    ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(1) + " < 0.0");
                    ShowContinueError(state,
                                      format("..Input value = {:.1R}, Value will be reset to 0.0",
                                             state.afn->OccupantVentilationControl(i).MinOpeningTime));
                    ShowContinueError(
                        state, "..for " + cAlphaFields(1) + " = \"" + state.afn->OccupantVentilationControl(i).Name);
                    state.afn->OccupantVentilationControl(i).MinOpeningTime = 0.0;
                }
                state.afn->OccupantVentilationControl(i).MinClosingTime = Numbers(2);
                if (state.afn->OccupantVentilationControl(i).MinClosingTime < 0.0) {
                    ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(2) + " < 0.0");
                    ShowContinueError(state,
                                      format("..Input value = {:.1R}, Value will be reset to 0.0",
                                             state.afn->OccupantVentilationControl(i).MinClosingTime));
                    ShowContinueError(
                        state, "..for " + cAlphaFields(1) + " = \"" + state.afn->OccupantVentilationControl(i).Name);
                    state.afn->OccupantVentilationControl(i).MinClosingTime = 0.0;
                }
                if (NumAlphas == 1 && NumNumbers == 2) {
                    state.afn->OccupantVentilationControl(i).MinTimeControlOnly = true;
                }
                if (!lAlphaBlanks(2)) {
                    state.afn->OccupantVentilationControl(i).ComfortLowTempCurveName = Alphas(2);
                    state.afn->OccupantVentilationControl(i).ComfortLowTempCurveNum =
                        GetCurveIndex(state, Alphas(2)); // convert curve name to number
                    if (state.afn->OccupantVentilationControl(i).ComfortLowTempCurveNum == 0) {
                        state.afn->OccupantVentilationControl(i).MinTimeControlOnly = true;
                        ShowWarningError(state,
                                         format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(2) + " not found = " +
                                             state.afn->OccupantVentilationControl(i).ComfortLowTempCurveName);
                        ShowContinueError(state, "..for specified " + cAlphaFields(1) + " = " + Alphas(1));
                        ShowContinueError(
                            state,
                            "Thermal comfort will not be performed and minimum opening and closing times are checked only. Simulation continues.");
                    } else {
                        ErrorsFound |= CurveManager::CheckCurveDims(
                            state,
                            state.afn->OccupantVentilationControl(i).ComfortLowTempCurveNum, // Curve index
                            {1},                                                                                          // Valid dimensions
                            RoutineName,                                                                                  // Routine name
                            CurrentModuleObject,                                                                          // Object Type
                            state.afn->OccupantVentilationControl(i).Name,                   // Object Name
                            cAlphaFields(2));                                                                             // Field Name
                    }
                }
                if (!lAlphaBlanks(3)) {
                    state.afn->OccupantVentilationControl(i).ComfortHighTempCurveName = Alphas(3);
                    state.afn->OccupantVentilationControl(i).ComfortHighTempCurveNum =
                        GetCurveIndex(state, Alphas(3)); // convert curve name to number
                    if (state.afn->OccupantVentilationControl(i).ComfortHighTempCurveNum > 0) {
                        ErrorsFound |= CurveManager::CheckCurveDims(
                            state,
                            state.afn->OccupantVentilationControl(i).ComfortHighTempCurveNum, // Curve index
                            {1},                                                                                           // Valid dimensions
                            RoutineName,                                                                                   // Routine name
                            CurrentModuleObject,                                                                           // Object Type
                            state.afn->OccupantVentilationControl(i).Name,                    // Object Name
                            cAlphaFields(3));                                                                              // Field Name
                    } else {
                        ShowWarningError(state,
                                         format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(3) + " not found = " +
                                             state.afn->OccupantVentilationControl(i).ComfortHighTempCurveName);
                        ShowContinueError(state, "..for specified " + cAlphaFields(1) + " = " + Alphas(1));
                        ShowContinueError(state, "A single curve of thermal comfort low temperature is used only. Simulation continues.");
                    }
                }
                if (state.afn->OccupantVentilationControl(i).ComfortHighTempCurveNum > 0) {
                    state.afn->OccupantVentilationControl(i).ComfortBouPoint = Numbers(3);
                    if (state.afn->OccupantVentilationControl(i).ComfortBouPoint < 0.0) {
                        ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(3) + " < 0.0");
                        ShowContinueError(state,
                                          format("..Input value = {:.1R}, Value will be reset to 10.0 as default",
                                                 state.afn->OccupantVentilationControl(i).ComfortBouPoint));
                        ShowContinueError(
                            state, "..for " + cAlphaFields(1) + " = \"" + state.afn->OccupantVentilationControl(i).Name);
                        state.afn->OccupantVentilationControl(i).ComfortBouPoint = 10.0;
                    }
                }
                // Check continuity of both curves at boundary point
                if (state.afn->OccupantVentilationControl(i).ComfortLowTempCurveNum > 0 &&
                    state.afn->OccupantVentilationControl(i).ComfortHighTempCurveNum) {
                    if (std::abs(CurveValue(state,
                                            state.afn->OccupantVentilationControl(i).ComfortLowTempCurveNum,
                                            Numbers(3)) -
                                 CurveValue(state,
                                            state.afn->OccupantVentilationControl(i).ComfortHighTempCurveNum,
                                            Numbers(3))) > 0.1) {
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject +
                                            " object: The difference of both curve values at boundary point > 0.1");
                        ShowContinueError(state, "Both curve names are = " + cAlphaFields(2) + " and " + cAlphaFields(3));
                        ShowContinueError(state,
                                          format("The input value of {} = {:.1R}",
                                                 cNumericFields(3),
                                                 state.afn->OccupantVentilationControl(i).ComfortBouPoint));
                        ErrorsFound = true;
                    }
                }
                if (!lNumericBlanks(4)) {
                    state.afn->OccupantVentilationControl(i).MaxPPD = Numbers(4);
                    if (state.afn->OccupantVentilationControl(i).MaxPPD < 0.0 ||
                        state.afn->OccupantVentilationControl(i).MaxPPD > 100.0) {
                        ShowWarningError(state,
                                         format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(4) + " beyond 0.0 and 100.0");
                        ShowContinueError(state,
                                          format("..Input value = {:.1R}, Value will be reset to 10.0 as default",
                                                 state.afn->OccupantVentilationControl(i).MaxPPD));
                        ShowContinueError(
                            state, "..for " + cAlphaFields(1) + " = \"" + state.afn->OccupantVentilationControl(i).Name);
                        state.afn->OccupantVentilationControl(i).MaxPPD = 10.0;
                    }
                }
                if (!lAlphaBlanks(4)) {
                    if (UtilityRoutines::SameString(Alphas(4), "Yes")) {
                        state.afn->OccupantVentilationControl(i).OccupancyCheck = true;
                    } else if (UtilityRoutines::SameString(Alphas(4), "No")) {
                        state.afn->OccupantVentilationControl(i).OccupancyCheck = false;
                    } else {
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject + "=\"" + Alphas(1) + "\" invalid " + cAlphaFields(2) + "=\"" +
                                            Alphas(2) + "\" illegal key.");
                        ShowContinueError(state, "Valid keys are: Yes or No");
                        ErrorsFound = true;
                    }
                }
                if (!lAlphaBlanks(5)) {
                    state.afn->OccupantVentilationControl(i).OpeningProbSchName =
                        Alphas(5); // a schedule name for opening probability
                    state.afn->OccupantVentilationControl(i).OpeningProbSchNum =
                        GetScheduleIndex(state, state.afn->OccupantVentilationControl(i).OpeningProbSchName);
                    if (state.afn->OccupantVentilationControl(i).OpeningProbSchNum == 0) {
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(5) + " not found = " +
                                            state.afn->OccupantVentilationControl(i).OpeningProbSchName);
                        ShowContinueError(state, "..for specified " + cAlphaFields(1) + " = " + Alphas(1));
                        ErrorsFound = true;
                    }
                }
                if (!lAlphaBlanks(6)) {
                    state.afn->OccupantVentilationControl(i).ClosingProbSchName =
                        Alphas(6); // a schedule name for closing probability
                    state.afn->OccupantVentilationControl(i).ClosingProbSchNum =
                        GetScheduleIndex(state, state.afn->OccupantVentilationControl(i).ClosingProbSchName);
                    if (state.afn->OccupantVentilationControl(i).OpeningProbSchNum == 0) {
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(6) + " not found = " +
                                            state.afn->OccupantVentilationControl(i).ClosingProbSchName);
                        ShowContinueError(state, "..for specified " + cAlphaFields(1) + " = " + Alphas(1));
                        ErrorsFound = true;
                    }
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, format(RoutineName) + "Errors found getting inputs. Previous error(s) cause program termination.");
        }

        // *** Read AirflowNetwork simulation parameters
        CurrentModuleObject = "AirflowNetwork:SimulationControl";
        state.afn->NumAirflowNetwork =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        if (state.afn->NumAirflowNetwork == 0) {
            if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirflowNetwork:MultiZone:Zone") >= 1 &&
                state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirflowNetwork:MultiZone:Surface") >= 2) {
                state.afn->AFNDefaultControlFlag = true;
                state.afn->AirflowNetworkSimu.AirflowNetworkSimuName = "AFNDefaultControl";
                state.afn->AirflowNetworkSimu.Control = "MULTIZONEWITHOUTDISTRIBUTION";
                state.afn->AirflowNetworkSimu.WPCCntr = "SURFACEAVERAGECALCULATION";
                state.afn->AirflowNetworkSimu.HeightOption = "OPENINGHEIGHT";
                state.afn->AirflowNetworkSimu.BldgType = "LOWRISE";
                state.afn->AirflowNetworkSimu.InitType = "ZERONODEPRESSURES";
                state.afn->AirflowNetworkSimu.TExtHeightDep = false;
                state.afn->AirflowNetworkSimu.solver = AirflowNetworkSimuProp::Solver::SkylineLU;
                // Use default values for numerical fields
                state.afn->AirflowNetworkSimu.MaxIteration = 500;
                state.afn->AirflowNetworkSimu.RelTol = 1.E-4;
                state.afn->AirflowNetworkSimu.AbsTol = 1.E-6;
                state.afn->AirflowNetworkSimu.ConvLimit = -0.5;
                state.afn->AirflowNetworkSimu.Azimuth = 0.0;
                state.afn->AirflowNetworkSimu.AspectRatio = 1.0;
                state.afn->AirflowNetworkSimu.MaxPressure = 500.0; // Maximum pressure difference by default
                state.afn->SimulateAirflowNetwork = AirflowNetworkControlMultizone;
                SimAirNetworkKey = "MultizoneWithoutDistribution";
                state.afn->AirflowNetworkSimu.InitFlag = 1;
                ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object is not found ");
                ShowContinueError(state, "..The default behaviour values are assigned. Please see details in Input Output Reference.");
            } else {
                state.afn->SimulateAirflowNetwork = AirflowNetworkControlSimple;
                print(state.files.eio, Format_110);
                print(state.files.eio, Format_120, "NoMultizoneOrDistribution");
                return;
            }
        }
        if (state.afn->NumAirflowNetwork > 1) {
            ShowFatalError(state, format(RoutineName) + "Only one (\"1\") " + CurrentModuleObject + " object per simulation is allowed.");
        }

        SimObjectError = false;
        if (!state.afn->AFNDefaultControlFlag) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     state.afn->NumAirflowNetwork,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            state.afn->AirflowNetworkSimu.AirflowNetworkSimuName = Alphas(1);
            state.afn->AirflowNetworkSimu.Control = Alphas(2);
            state.afn->AirflowNetworkSimu.WPCCntr = Alphas(3);
            state.afn->AirflowNetworkSimu.HeightOption = Alphas(4);
            state.afn->AirflowNetworkSimu.BldgType = Alphas(5);

            // Retrieve flag allowing the support of zone equipment
            state.afn->AirflowNetworkSimu.AllowSupportZoneEqp = false;
            if (UtilityRoutines::SameString(Alphas(9), "Yes")) {
                state.afn->AirflowNetworkSimu.AllowSupportZoneEqp = true;
            }

            // Find a flag for possible combination of vent and distribution system
            {
                auto const SELECT_CASE_var(UtilityRoutines::MakeUPPERCase(state.afn->AirflowNetworkSimu.Control));
                if (SELECT_CASE_var == "NOMULTIZONEORDISTRIBUTION") {
                    state.afn->SimulateAirflowNetwork = AirflowNetworkControlSimple;
                    SimAirNetworkKey = "NoMultizoneOrDistribution";
                } else if (SELECT_CASE_var == "MULTIZONEWITHOUTDISTRIBUTION") {
                    state.afn->SimulateAirflowNetwork = AirflowNetworkControlMultizone;
                    SimAirNetworkKey = "MultizoneWithoutDistribution";
                } else if (SELECT_CASE_var == "MULTIZONEWITHDISTRIBUTIONONLYDURINGFANOPERATION") {
                    state.afn->SimulateAirflowNetwork = AirflowNetworkControlSimpleADS;
                    SimAirNetworkKey = "MultizoneWithDistributionOnlyDuringFanOperation";
                } else if (SELECT_CASE_var == "MULTIZONEWITHDISTRIBUTION") {
                    state.afn->SimulateAirflowNetwork = AirflowNetworkControlMultiADS;
                    SimAirNetworkKey = "MultizoneWithDistribution";
                } else { // Error
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + " object, The entered choice for " + cAlphaFields(2) +
                                        " is not valid = \"" + state.afn->AirflowNetworkSimu.Control + "\"");
                    ShowContinueError(state,
                                      "Valid choices are \"NO MULTIZONE OR DISTRIBUTION\",\"MULTIZONE WITH DISTRIBUTION ONLY DURING FAN OPERATION\"");
                    ShowContinueError(state, "\"MULTIZONE WITH DISTRIBUTION\", or \"MULTIZONE WITHOUT DISTRIBUTION\"");
                    ShowContinueError(state,
                                      "..specified in " + CurrentModuleObject + ' ' + cAlphaFields(1) + " = " +
                                          state.afn->AirflowNetworkSimu.AirflowNetworkSimuName);
                    ErrorsFound = true;
                }
            }
        }

        // Check the number of primary air loops
        if (state.afn->SimulateAirflowNetwork == AirflowNetworkControlSimpleADS ||
            state.afn->SimulateAirflowNetwork == AirflowNetworkControlMultiADS) {
            NumAPL = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC");
            if (NumAPL > 0) {
                state.afn->LoopPartLoadRatio.allocate(NumAPL);
                state.afn->LoopOnOffFanRunTimeFraction.allocate(NumAPL);
                state.afn->LoopOnOffFlag.allocate(NumAPL);
                state.afn->LoopPartLoadRatio = 0.0;
                state.afn->LoopOnOffFanRunTimeFraction = 0.0;
                state.afn->LoopOnOffFlag = false;
            }
        }
        print(state.files.eio, Format_110);
        print(state.files.eio, Format_120, SimAirNetworkKey);

        if (state.afn->AFNDefaultControlFlag) {
            cAlphaFields(2) = "AirflowNetwork Control";
        }

        // Check whether there are any objects from infiltration, ventilation, mixing and cross mixing
        if (state.afn->SimulateAirflowNetwork == AirflowNetworkControlSimple ||
            state.afn->SimulateAirflowNetwork == AirflowNetworkControlSimpleADS) {
            if (state.dataHeatBal->TotInfiltration + state.dataHeatBal->TotVentilation + state.dataHeatBal->TotMixing +
                    state.dataHeatBal->TotCrossMixing + state.dataHeatBal->TotZoneAirBalance +
                    state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneEarthtube") +
                    state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneThermalChimney") +
                    state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneCoolTower:Shower") ==
                0) {
                ShowWarningError(state, format(RoutineName) + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\".");
                ShowContinueError(
                    state,
                    "..but there are no Infiltration, Ventilation, Mixing, Cross Mixing or ZoneAirBalance objects. The simulation continues...");
            }
        }

        // Check whether a user wants to perform SIMPLE calculation only or not
        if (state.afn->SimulateAirflowNetwork == AirflowNetworkControlSimple) return;

        if (state.afn->SimulateAirflowNetwork == AirflowNetworkControlMultizone ||
            state.afn->SimulateAirflowNetwork == AirflowNetworkControlMultiADS) {
            if (state.dataHeatBal->TotInfiltration > 0) {
                ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, ");
                ShowContinueError(state,
                                  "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\" and ZoneInfiltration:* objects are present.");
                ShowContinueError(state, "..ZoneInfiltration objects will not be simulated.");
            }
            if (state.dataHeatBal->TotVentilation > 0) {
                ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, ");
                ShowContinueError(state,
                                  "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\" and ZoneVentilation:* objects are present.");
                ShowContinueError(state, "..ZoneVentilation objects will not be simulated.");
            }
            if (state.dataHeatBal->TotMixing > 0) {
                ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, ");
                ShowContinueError(state, "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\" and ZoneMixing objects are present.");
                ShowContinueError(state, "..ZoneMixing objects will not be simulated.");
            }
            if (state.dataHeatBal->TotCrossMixing > 0) {
                ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, ");
                ShowContinueError(state,
                                  "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\" and ZoneCrossMixing objects are present.");
                ShowContinueError(state, "..ZoneCrossMixing objects will not be simulated.");
            }
            if (state.dataHeatBal->TotZoneAirBalance > 0) {
                ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, ");
                ShowContinueError(
                    state, "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\" and ZoneAirBalance:OutdoorAir objects are present.");
                ShowContinueError(state, "..ZoneAirBalance:OutdoorAir objects will not be simulated.");
            }
            if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneEarthtube") > 0) {
                ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, ");
                ShowContinueError(state, "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\" and ZoneEarthtube objects are present.");
                ShowContinueError(state, "..ZoneEarthtube objects will not be simulated.");
            }
            if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneThermalChimney") > 0) {
                ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, ");
                ShowContinueError(state,
                                  "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\" and ZoneThermalChimney objects are present.");
                ShowContinueError(state, "..ZoneThermalChimney objects will not be simulated.");
            }
            if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneCoolTower:Shower") > 0) {
                ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, ");
                ShowContinueError(state,
                                  "..Specified " + cAlphaFields(2) + " = \"" + SimAirNetworkKey + "\" and ZoneCoolTower:Shower objects are present.");
                ShowContinueError(state, "..ZoneCoolTower:Shower objects will not be simulated.");
            }
        }

        SetOutAirNodes(state);
        if (!state.afn->AFNDefaultControlFlag) {
            if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.WPCCntr, "Input")) {
                state.afn->AirflowNetworkSimu.iWPCCnt = iWPCCntr::Input;
                if (lAlphaBlanks(4)) {
                    ShowSevereError(state, format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(3) + " = INPUT.");
                    ShowContinueError(state, ".." + cAlphaFields(4) + " was not entered.");
                    ErrorsFound = true;
                    SimObjectError = true;
                } else {
                    if (!(UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.HeightOption, "ExternalNode") ||
                          UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.HeightOption, "OpeningHeight"))) {
                        ShowSevereError(
                            state, format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(4) + " = " + Alphas(4) + " is invalid.");
                        ShowContinueError(state,
                                          "Valid choices are ExternalNode or OpeningHeight. " + CurrentModuleObject + ": " + cAlphaFields(1) + " = " +
                                              state.afn->AirflowNetworkSimu.AirflowNetworkSimuName);
                        ErrorsFound = true;
                        SimObjectError = true;
                    }
                }
            } else if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.WPCCntr, "SurfaceAverageCalculation")) {
                state.afn->AirflowNetworkSimu.iWPCCnt = iWPCCntr::SurfAvg;
                if (!(UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.BldgType, "LowRise") ||
                      UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.BldgType, "HighRise"))) {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(5) + " = " + Alphas(5) + " is invalid.");
                    ShowContinueError(state,
                                      "Valid choices are LowRise or HighRise. " + CurrentModuleObject + ": " + cAlphaFields(1) + " = " +
                                          state.afn->AirflowNetworkSimu.AirflowNetworkSimuName);
                    ErrorsFound = true;
                    SimObjectError = true;
                }
                for (k = 1; k <= state.dataLoopNodes->NumOfNodes; ++k) {
                    if (Node(k).IsLocalNode) {
                        ShowSevereError(state, format(RoutineName) + "Invalid " + cAlphaFields(3) + "=" + Alphas(3));
                        ShowContinueError(state,
                                          "A local air node is defined to INPUT the wind pressure coefficient curve, while Wind Pressure Coefficient "
                                          "Type is set to SurfaceAverageCalculation.");
                        ShowContinueError(state, "It requires  the Wind Pressure Coefficient Type be set to INPUT to use the local air node.");
                        ErrorsFound = true;
                        SimObjectError = true;
                        break;
                    }
                }
            } else {
                ShowSevereError(state,
                                format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(3) + " = " +
                                    state.afn->AirflowNetworkSimu.WPCCntr + " is not valid.");
                ShowContinueError(state,
                                  "Valid choices are Input or SurfaceAverageCalculation. " + CurrentModuleObject + " = " +
                                      state.afn->AirflowNetworkSimu.AirflowNetworkSimuName);
                ErrorsFound = true;
                SimObjectError = true;
            }

            state.afn->AirflowNetworkSimu.InitType = Alphas(6);
            if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.InitType, "LinearInitializationMethod")) {
                state.afn->AirflowNetworkSimu.InitFlag = 0;
            } else if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.InitType, "ZeroNodePressures")) {
                state.afn->AirflowNetworkSimu.InitFlag = 1;
            } else if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.InitType, "0")) {
                state.afn->AirflowNetworkSimu.InitFlag = 0;
            } else if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.InitType, "1")) {
                state.afn->AirflowNetworkSimu.InitFlag = 1;
            } else {
                ShowSevereError(state,
                                format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(6) + " = " + Alphas(6) + " is invalid.");
                ShowContinueError(state,
                                  "Valid choices are LinearInitializationMethod or ZeroNodePressures. " + CurrentModuleObject + " = " +
                                      state.afn->AirflowNetworkSimu.AirflowNetworkSimuName);
                ErrorsFound = true;
                SimObjectError = true;
            }

            if (!lAlphaBlanks(7) && UtilityRoutines::SameString(Alphas(7), "Yes")) state.afn->AirflowNetworkSimu.TExtHeightDep = true;

            if (lAlphaBlanks(8)) {
                state.afn->AirflowNetworkSimu.solver = AirflowNetworkSimuProp::Solver::SkylineLU;
            } else if (UtilityRoutines::SameString(Alphas(8), "SkylineLU")) {
                state.afn->AirflowNetworkSimu.solver = AirflowNetworkSimuProp::Solver::SkylineLU;
            } else if (UtilityRoutines::SameString(Alphas(8), "ConjugateGradient")) {
                state.afn->AirflowNetworkSimu.solver = AirflowNetworkSimuProp::Solver::ConjugateGradient;
            } else {
                state.afn->AirflowNetworkSimu.solver = AirflowNetworkSimuProp::Solver::SkylineLU;
                ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, ");
                ShowContinueError(state, "..Specified " + cAlphaFields(8) + " = \"" + Alphas(8) + "\" is unrecognized.");
                ShowContinueError(state, "..Default value \"SkylineLU\" will be used.");
            }

            if (SimObjectError) {
                ShowFatalError(state,
                               format(RoutineName) + "Errors found getting " + CurrentModuleObject +
                                   " object. Previous error(s) cause program termination.");
            }

            state.afn->AirflowNetworkSimu.MaxIteration = Numbers(1);
            state.afn->AirflowNetworkSimu.RelTol = Numbers(2);
            state.afn->AirflowNetworkSimu.AbsTol = Numbers(3);
            state.afn->AirflowNetworkSimu.ConvLimit = Numbers(4);
            state.afn->AirflowNetworkSimu.Azimuth = Numbers(5);
            state.afn->AirflowNetworkSimu.AspectRatio = Numbers(6);
            state.afn->AirflowNetworkSimu.MaxPressure = 500.0; // Maximum pressure difference by default
        }

        // *** Read AirflowNetwork simulation zone data
        CurrentModuleObject = "AirflowNetwork:MultiZone:Zone";
        state.afn->AirflowNetworkNumOfZones =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        if (state.afn->AirflowNetworkNumOfZones > 0) {
            state.afn->MultizoneZoneData.allocate(state.afn->AirflowNetworkNumOfZones);
            state.afn->AirflowNetworkZoneFlag.dimension(state.dataGlobal->NumOfZones, false); // AirflowNetwork zone flag
            for (int i = 1; i <= state.afn->AirflowNetworkNumOfZones; ++i) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         i,
                                                                         Alphas,
                                                                         NumAlphas,
                                                                         Numbers,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericBlanks,
                                                                         lAlphaBlanks,
                                                                         cAlphaFields,
                                                                         cNumericFields);
                UtilityRoutines::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);
                state.afn->MultizoneZoneData(i).ZoneName = Alphas(1); // Name of Associated EnergyPlus Thermal Zone
                if (!lAlphaBlanks(2))
                    state.afn->MultizoneZoneData(i).VentControl = Alphas(2); // Ventilation Control Mode: "Temperature", "Enthalpy",
                // "ASHRAE55ADAPTIVE", "CEN15251AdaptiveComfort,
                // "Constant", or "NoVent"
                state.afn->MultizoneZoneData(i).VentSchName = Alphas(3); // Name of ventilation temperature control schedule
                state.afn->MultizoneZoneData(i).OpenFactor =
                    Numbers(1); // Limit Value on Multiplier for Modulating Venting Open Factor,
                // Not applicable if Vent Control Mode = CONSTANT or NOVENT
                state.afn->MultizoneZoneData(i).LowValueTemp = Numbers(2); // Lower Value on Inside/Outside Temperature Difference
                // for Modulating the Venting Open Factor with temp control
                state.afn->MultizoneZoneData(i).UpValueTemp = Numbers(3); // Upper Value on Inside/Outside Temperature Difference
                // for Modulating the Venting Open Factor with temp control
                state.afn->MultizoneZoneData(i).LowValueEnth = Numbers(4); // Lower Value on Inside/Outside Temperature Difference
                // for Modulating the Venting Open Factor with Enthalpy control
                state.afn->MultizoneZoneData(i).UpValueEnth = Numbers(5); // Upper Value on Inside/Outside Temperature Difference
                // for Modulating the Venting Open Factor with Enthalpy control
                state.afn->MultizoneZoneData(i).VentCtrNum = VentControlType::None;
                state.afn->MultizoneZoneData(i).SingleSidedCpType = Alphas(5);
                state.afn->MultizoneZoneData(i).BuildWidth = Numbers(6);

                if (!lAlphaBlanks(6)) {
                    state.afn->MultizoneZoneData(i).OccupantVentilationControlName = Alphas(6);
                    state.afn->MultizoneZoneData(i).OccupantVentilationControlNum =
                        UtilityRoutines::FindItemInList(state.afn->MultizoneZoneData(i).OccupantVentilationControlName,
                                                        state.afn->OccupantVentilationControl);
                    if (state.afn->MultizoneZoneData(i).OccupantVentilationControlNum == 0) {
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(6) +
                                            " not found = " + state.afn->MultizoneZoneData(i).OccupantVentilationControlName);
                        ShowContinueError(state, "..for specified " + cAlphaFields(1) + " = " + Alphas(1));
                        ErrorsFound = true;
                    }
                }
                if (UtilityRoutines::SameString(state.afn->MultizoneZoneData(i).VentControl, "Temperature"))
                    state.afn->MultizoneZoneData(i).VentCtrNum = VentControlType::Temp;
                if (UtilityRoutines::SameString(state.afn->MultizoneZoneData(i).VentControl, "Enthalpy"))
                    state.afn->MultizoneZoneData(i).VentCtrNum = VentControlType::Enth;
                if (UtilityRoutines::SameString(state.afn->MultizoneZoneData(i).VentControl, "Constant"))
                    state.afn->MultizoneZoneData(i).VentCtrNum = VentControlType::Const;
                if (UtilityRoutines::SameString(state.afn->MultizoneZoneData(i).VentControl, "ASHRAE55Adaptive"))
                    state.afn->MultizoneZoneData(i).VentCtrNum = VentControlType::ASH55;
                if (UtilityRoutines::SameString(state.afn->MultizoneZoneData(i).VentControl, "CEN15251Adaptive"))
                    state.afn->MultizoneZoneData(i).VentCtrNum = VentControlType::CEN15251;
                if (UtilityRoutines::SameString(state.afn->MultizoneZoneData(i).VentControl, "NoVent"))
                    state.afn->MultizoneZoneData(i).VentCtrNum = VentControlType::NoVent;

                if (state.afn->MultizoneZoneData(i).VentCtrNum < NumOfVentCtrTypes) {
                    if (NumAlphas >= 4 && (!lAlphaBlanks(4))) {
                        state.afn->MultizoneZoneData(i).VentingSchName = Alphas(4);
                        state.afn->MultizoneZoneData(i).VentingSchNum =
                            GetScheduleIndex(state, state.afn->MultizoneZoneData(i).VentingSchName);
                        if (state.afn->MultizoneZoneData(i).VentingSchNum == 0) {
                            ShowSevereError(state,
                                            format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(4) +
                                                " not found = " + state.afn->MultizoneZoneData(i).VentingSchName);
                            ShowContinueError(state, "..for specified " + cAlphaFields(1) + " = " + Alphas(1));
                            ErrorsFound = true;
                        }
                    }
                } else {
                    state.afn->MultizoneZoneData(i).VentingSchName = std::string();
                    state.afn->MultizoneZoneData(i).VentingSchNum = 0;
                }
            }
        } else {
            ShowSevereError(state,
                            format(RoutineName) + "For an AirflowNetwork Simulation, at least one " + CurrentModuleObject +
                                " object is required but none were found.");
            ShowFatalError(
                state, format(RoutineName) + "Errors found getting " + CurrentModuleObject + " object. Previous error(s) cause program termination.");
        }

        // ==> Zone data validation
        for (int i = 1; i <= state.afn->AirflowNetworkNumOfZones; ++i) {
            // Zone name validation
            state.afn->MultizoneZoneData(i).ZoneNum =
                UtilityRoutines::FindItemInList(state.afn->MultizoneZoneData(i).ZoneName, Zone);
            if (state.afn->MultizoneZoneData(i).ZoneNum == 0) {
                ShowSevereError(state, format(RoutineName) + CurrentModuleObject + " object, invalid " + cAlphaFields(1) + " given.");
                ShowContinueError(state, "..invalid " + cAlphaFields(1) + " = \"" + state.afn->MultizoneZoneData(i).ZoneName + "\"");
                ErrorsFound = true;
            } else {
                state.afn->AirflowNetworkZoneFlag(state.afn->MultizoneZoneData(i).ZoneNum) = true;
                state.afn->MultizoneZoneData(i).Height =
                    Zone(state.afn->MultizoneZoneData(i).ZoneNum).Centroid.z; // Nodal height
            }
            if (state.afn->MultizoneZoneData(i).VentCtrNum == VentControlType::None) {
                ShowSevereError(state,
                                format(RoutineName) + CurrentModuleObject + " object, invalid " + cAlphaFields(2) + " = " +
                                    state.afn->MultizoneZoneData(i).VentControl);
                ShowContinueError(state, "Valid choices are Temperature, Enthalpy, Constant, or NoVent");
                ShowContinueError(state, ".. in " + cAlphaFields(1) + " = \"" + state.afn->MultizoneZoneData(i).ZoneName + "\"");
                ErrorsFound = true;
            }
            if (UtilityRoutines::SameString(state.afn->MultizoneZoneData(i).VentControl, "Temperature") ||
                UtilityRoutines::SameString(state.afn->MultizoneZoneData(i).VentControl, "Enthalpy")) {
                // .or. &
                // UtilityRoutines::SameString(state.afn->MultizoneZoneData(i)%VentControl,'ASHRAE55Adaptive') .or. &
                // UtilityRoutines::SameString(state.afn->MultizoneZoneData(i)%VentControl,'CEN15251Adaptive')) then
                state.afn->MultizoneZoneData(i).VentSchNum =
                    GetScheduleIndex(state, state.afn->MultizoneZoneData(i).VentSchName);
                if (state.afn->MultizoneZoneData(i).VentSchName == std::string()) {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + " object, No " + cAlphaFields(3) +
                                        " was found, but is required when " + cAlphaFields(2) + " is Temperature or Enthalpy.");
                    ShowContinueError(state,
                                      "..for " + cAlphaFields(1) + " = \"" + state.afn->MultizoneZoneData(i).ZoneName + "\", with " +
                                          cAlphaFields(2) + " = \"" + state.afn->MultizoneZoneData(i).VentControl + "\"");
                    ErrorsFound = true;
                } else if (state.afn->MultizoneZoneData(i).VentSchNum == 0) {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + " object, invalid " + cAlphaFields(3) + ", required when " +
                                        cAlphaFields(2) + " is Temperature or Enthalpy.");
                    ShowContinueError(state, ".." + cAlphaFields(3) + " in error = " + state.afn->MultizoneZoneData(i).VentSchName);
                    ShowContinueError(state,
                                      "..for " + cAlphaFields(1) + " = \"" + state.afn->MultizoneZoneData(i).ZoneName + "\", with " +
                                          cAlphaFields(2) + " = \"" + state.afn->MultizoneZoneData(i).VentControl + "\"");
                    ErrorsFound = true;
                }
            } else {
                state.afn->MultizoneZoneData(i).VentSchNum =
                    GetScheduleIndex(state, state.afn->MultizoneZoneData(i).VentSchName);
                if (state.afn->MultizoneZoneData(i).VentSchNum > 0) {
                    ShowWarningError(state,
                                     format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(3) + " not required, when " +
                                         cAlphaFields(2) + " is neither Temperature nor Enthalpy.");
                    ShowContinueError(state, ".." + cAlphaFields(3) + " specified = " + state.afn->MultizoneZoneData(i).VentSchName);
                    ShowContinueError(state,
                                      "..for " + cAlphaFields(1) + " = \"" + state.afn->MultizoneZoneData(i).ZoneName + "\", with " +
                                          cAlphaFields(2) + " = \"" + state.afn->MultizoneZoneData(i).VentControl + "\"");
                    state.afn->MultizoneZoneData(i).VentSchNum = 0;
                    state.afn->MultizoneZoneData(i).VentSchName = std::string();
                }
            }
            if (state.afn->MultizoneZoneData(i).OpenFactor > 1.0 || state.afn->MultizoneZoneData(i).OpenFactor < 0.0) {
                ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(1) + " is out of range [0.0,1.0]");
                ShowContinueError(
                    state, format("..Input value = {:.2R}, Value will be set to 1.0", state.afn->MultizoneZoneData(i).OpenFactor));
                state.afn->MultizoneZoneData(i).OpenFactor = 1.0;
            }

            {
                auto const SELECT_CASE_var(UtilityRoutines::MakeUPPERCase(state.afn->MultizoneZoneData(i).VentControl));
                if (SELECT_CASE_var == "TEMPERATURE") { // checks on Temperature control
                    if (state.afn->MultizoneZoneData(i).LowValueTemp < 0.0) {
                        ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(2) + " < 0.0");
                        ShowContinueError(
                            state,
                            format("..Input value = {:.1R}, Value will be set to 0.0", state.afn->MultizoneZoneData(i).LowValueTemp));
                        ShowContinueError(state, "..for " + cAlphaFields(1) + " = \"" + state.afn->MultizoneZoneData(i).ZoneName);
                        state.afn->MultizoneZoneData(i).LowValueTemp = 0.0;
                    }
                    if (state.afn->MultizoneZoneData(i).LowValueTemp >= 100.0) {
                        ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(2) + " >= 100.0");
                        ShowContinueError(state,
                                          format("..Input value = {:.1R}, Value will be reset to 0.0",
                                                 state.afn->MultizoneZoneData(i).LowValueTemp));
                        ShowContinueError(state, "..for " + cAlphaFields(1) + " = \"" + state.afn->MultizoneZoneData(i).ZoneName);
                        state.afn->MultizoneZoneData(i).LowValueTemp = 0.0;
                    }
                    if (state.afn->MultizoneZoneData(i).UpValueTemp <= state.afn->MultizoneZoneData(i).LowValueTemp) {
                        ShowWarningError(state,
                                         format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(3) + " <= " + cNumericFields(2));
                        ShowContinueError(state,
                                          format("..Input value for {} = {:.1R}, Value will be reset to 100.0",
                                                 cNumericFields(3),
                                                 state.afn->MultizoneZoneData(i).UpValueTemp));
                        ShowContinueError(state, "..for " + cAlphaFields(1) + " = \"" + state.afn->MultizoneZoneData(i).ZoneName);
                        state.afn->MultizoneZoneData(i).UpValueTemp = 100.0;
                    }

                } else if (SELECT_CASE_var == "ENTHALPY") { // checks for Enthalpy control
                    if (state.afn->MultizoneZoneData(i).LowValueEnth < 0.0) {
                        ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(4) + " < 0.0");
                        ShowContinueError(state,
                                          format("..Input value = {:.1R}, Value will be reset to 0.0",
                                                 state.afn->MultizoneZoneData(i).LowValueEnth));
                        ShowContinueError(state, "..for " + cAlphaFields(1) + " = \"" + state.afn->MultizoneZoneData(i).ZoneName);
                        state.afn->MultizoneZoneData(i).LowValueEnth = 0.0;
                    }
                    if (state.afn->MultizoneZoneData(i).LowValueEnth >= 300000.0) {
                        ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(4) + " >= 300000.0");
                        ShowContinueError(state,
                                          format("..Input value = {:.1R}, Value will be reset to 0.0.",
                                                 state.afn->MultizoneZoneData(i).LowValueEnth));
                        ShowContinueError(state, "..for " + cAlphaFields(1) + " = \"" + state.afn->MultizoneZoneData(i).ZoneName);
                        state.afn->MultizoneZoneData(i).LowValueEnth = 0.0;
                    }
                    if (state.afn->MultizoneZoneData(i).UpValueEnth <= state.afn->MultizoneZoneData(i).LowValueEnth) {
                        ShowWarningError(state,
                                         format(RoutineName) + CurrentModuleObject + " object, " + cNumericFields(5) + " <= " + cNumericFields(4));
                        ShowContinueError(state,
                                          format("..Input value for {}= {:.1R}, Value will be reset to 300000.0",
                                                 cNumericFields(5),
                                                 state.afn->MultizoneZoneData(i).UpValueEnth));
                        ShowContinueError(state, "..for " + cAlphaFields(1) + " = \"" + state.afn->MultizoneZoneData(i).ZoneName);
                        state.afn->MultizoneZoneData(i).UpValueEnth = 300000.0;
                    }
                } else if (SELECT_CASE_var == "ASHRAE55ADAPTIVE") {
                    // Check that for the given zone, there is a people object for which ASHRAE 55 calculations are carried out
                    ZoneNum = state.afn->MultizoneZoneData(i).ZoneNum;
                    for (j = 1; j <= state.dataHeatBal->TotPeople; ++j) {
                        if (ZoneNum == state.dataHeatBal->People(j).ZonePtr && state.dataHeatBal->People(j).AdaptiveASH55) {
                            state.afn->MultizoneZoneData(i).ASH55PeopleInd = j;
                        }
                    }
                    if (state.afn->MultizoneZoneData(i).ASH55PeopleInd == 0) {
                        ShowFatalError(state,
                                       "ASHRAE55 ventilation control for zone " + state.afn->MultizoneZoneData(i).ZoneName +
                                           " requires a people object with respective model calculations.");
                    }
                } else if (SELECT_CASE_var == "CEN15251ADAPTIVE") {
                    // Check that for the given zone, there is a people object for which CEN-15251 calculations are carried out
                    ZoneNum = state.afn->MultizoneZoneData(i).ZoneNum;
                    for (j = 1; j <= state.dataHeatBal->TotPeople; ++j) {
                        if (ZoneNum == state.dataHeatBal->People(j).ZonePtr && state.dataHeatBal->People(j).AdaptiveCEN15251) {
                            state.afn->MultizoneZoneData(i).CEN15251PeopleInd = j;
                            break;
                        }
                    }
                    if (state.afn->MultizoneZoneData(i).CEN15251PeopleInd == 0) {
                        ShowFatalError(state,
                                       "CEN15251 ventilation control for zone " + state.afn->MultizoneZoneData(i).ZoneName +
                                           " requires a people object with respective model calculations.");
                    }
                } else {
                }
            }
        }

        // *** Read AirflowNetwork external node
        if (state.afn->AirflowNetworkSimu.iWPCCnt == iWPCCntr::Input) {
            // Wind coefficient == Surface-Average does not need inputs of external nodes
            state.afn->AirflowNetworkNumOfExtNode =
                state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirflowNetwork:MultiZone:ExternalNode");
            if (state.dataGlobal->AnyLocalEnvironmentsInModel) {
                state.afn->AirflowNetworkNumOfOutAirNode =
                    state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "OutdoorAir:Node");
                state.afn->AirflowNetworkNumOfExtNode +=
                    state.afn->AirflowNetworkNumOfOutAirNode;
            }

            if (state.afn->AirflowNetworkNumOfExtNode > 0) {
                state.afn->MultizoneExternalNodeData.allocate(state.afn->AirflowNetworkNumOfExtNode);
                CurrentModuleObject = "AirflowNetwork:MultiZone:ExternalNode";
                for (int i = 1; i <= state.afn->AirflowNetworkNumOfExtNode -
                                         state.afn->AirflowNetworkNumOfOutAirNode;
                     ++i) {
                    state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                             CurrentModuleObject,
                                                                             i,
                                                                             Alphas,
                                                                             NumAlphas,
                                                                             Numbers,
                                                                             NumNumbers,
                                                                             IOStatus,
                                                                             lNumericBlanks,
                                                                             lAlphaBlanks,
                                                                             cAlphaFields,
                                                                             cNumericFields);
                    UtilityRoutines::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);
                    state.afn->MultizoneExternalNodeData(i).Name = Alphas(1);    // Name of external node
                    state.afn->MultizoneExternalNodeData(i).height = Numbers(1); // Nodal height
                    if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.HeightOption, "ExternalNode") && lNumericBlanks(1)) {
                        ShowWarningError(state,
                                         format(RoutineName) + CurrentModuleObject + " object =" + Alphas(1) + ". The input of " + cNumericFields(1) +
                                             " is required, but a blank is found.");
                        ShowContinueError(state, format("The default value is assigned as {:.1R}", Numbers(1)));
                    }
                    state.afn->MultizoneExternalNodeData(i).ExtNum =
                        state.afn->AirflowNetworkNumOfZones + i; // External node number
                    state.afn->MultizoneExternalNodeData(i).curve =
                        CurveManager::GetCurveIndex(state, Alphas(2)); // Wind pressure curve
                    if (state.afn->MultizoneExternalNodeData(i).curve == 0) {
                        ShowSevereError(state, format(RoutineName) + "Invalid " + cAlphaFields(2) + "=" + Alphas(2));
                        ShowContinueError(state, "Entered in " + CurrentModuleObject + '=' + Alphas(1));
                        ErrorsFound = true;
                    }
                    if (NumAlphas >= 3 && !lAlphaBlanks(3)) { // Symmetric curve
                        if (UtilityRoutines::SameString(Alphas(3), "Yes")) {
                            state.afn->MultizoneExternalNodeData(i).symmetricCurve = true;
                        } else if (!UtilityRoutines::SameString(Alphas(3), "No")) {
                            ShowWarningError(
                                state, format(RoutineName) + CurrentModuleObject + " object, Invalid input " + cAlphaFields(3) + " = " + Alphas(3));
                            ShowContinueError(state, "The default value is assigned as No.");
                        }
                    }
                    if (NumAlphas == 4 && !lAlphaBlanks(4)) { // Relative or absolute wind angle
                        if (UtilityRoutines::SameString(Alphas(4), "Relative")) {
                            state.afn->MultizoneExternalNodeData(i).useRelativeAngle = true;
                        } else if (!UtilityRoutines::SameString(Alphas(4), "Absolute")) {
                            ShowWarningError(
                                state, format(RoutineName) + CurrentModuleObject + " object, Invalid input " + cAlphaFields(4) + " = " + Alphas(4));
                            ShowContinueError(state, "The default value is assigned as Absolute.");
                        }
                    }
                }
                if (state.dataGlobal->AnyLocalEnvironmentsInModel) {

                    CurrentModuleObject = "OutdoorAir:Node";
                    for (int i = state.afn->AirflowNetworkNumOfExtNode -
                                 state.afn->AirflowNetworkNumOfOutAirNode + 1;
                         i <= state.afn->AirflowNetworkNumOfExtNode;
                         ++i) {
                        state.dataInputProcessing->inputProcessor->getObjectItem(
                            state,
                            CurrentModuleObject,
                            i - (state.afn->AirflowNetworkNumOfExtNode -
                                 state.afn->AirflowNetworkNumOfOutAirNode),
                            Alphas,
                            NumAlphas,
                            Numbers,
                            NumNumbers,
                            IOStatus,
                            lNumericBlanks,
                            lAlphaBlanks,
                            cAlphaFields,
                            cNumericFields);
                        UtilityRoutines::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);
                        // HACK: Need to verify name is unique between "OutdoorAir:Node" and "AirflowNetwork:MultiZone:ExternalNode"

                        if (NumAlphas > 5 && !lAlphaBlanks(6)) { // Wind pressure curve
                            state.afn->MultizoneExternalNodeData(i).curve = GetCurveIndex(state, Alphas(6));
                            if (state.afn->MultizoneExternalNodeData(i).curve == 0) {
                                ShowSevereError(state, format(RoutineName) + "Invalid " + cAlphaFields(6) + "=" + Alphas(6));
                                ShowContinueError(state, "Entered in " + CurrentModuleObject + '=' + Alphas(1));
                                ErrorsFound = true;
                            }
                        }

                        if (NumAlphas > 6 && !lAlphaBlanks(7)) { // Symmetric curve
                            if (UtilityRoutines::SameString(Alphas(7), "Yes")) {
                                state.afn->MultizoneExternalNodeData(i).symmetricCurve = true;
                            } else if (!UtilityRoutines::SameString(Alphas(7), "No")) {
                                ShowWarningError(state,
                                                 format(RoutineName) + CurrentModuleObject + " object, Invalid input " + cAlphaFields(7) + " = " +
                                                     Alphas(7));
                                ShowContinueError(state, "The default value is assigned as No.");
                            }
                        }

                        if (NumAlphas > 7 && !lAlphaBlanks(8)) { // Relative or absolute wind angle
                            if (UtilityRoutines::SameString(Alphas(8), "Relative")) {
                                state.afn->MultizoneExternalNodeData(i).useRelativeAngle = true;
                            } else if (!UtilityRoutines::SameString(Alphas(8), "Absolute")) {
                                ShowWarningError(state,
                                                 format(RoutineName) + CurrentModuleObject + " object, Invalid input " + cAlphaFields(8) + " = " +
                                                     Alphas(8));
                                ShowContinueError(state, "The default value is assigned as Absolute.");
                            }
                        }

                        state.afn->MultizoneExternalNodeData(i).Name = Alphas(1); // Name of external node
                        NodeNum = GetOnlySingleNode(state,
                                                    Alphas(1),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::OutdoorAirNode,
                                                    "AirflowNetwork:Multizone:Surface",
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    ObjectIsParent);
                        state.afn->MultizoneExternalNodeData(i).OutAirNodeNum = NodeNum;       // Name of outdoor air node
                        state.afn->MultizoneExternalNodeData(i).height = Node(NodeNum).Height; // Nodal height
                        state.afn->MultizoneExternalNodeData(i).ExtNum =
                            state.afn->AirflowNetworkNumOfZones + i; // External node number
                    }
                }
            } else {
                ShowSevereError(state,
                                format(RoutineName) + "An " + CurrentModuleObject +
                                    " object is required but not found when Wind Pressure Coefficient Type = Input.");
                ErrorsFound = true;
            }
        }

        // *** Read AirflowNetwork element data
        ErrorsFound = ErrorsFound || !getAirflowElementInput(state);

        // *** Read AirflowNetwork simulation surface data
        CurrentModuleObject = "AirflowNetwork:MultiZone:Surface";
        state.afn->AirflowNetworkNumOfSurfaces =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        if (state.afn->AirflowNetworkNumOfSurfaces > 0) {
            state.afn->MultizoneSurfaceData.allocate(state.afn->AirflowNetworkNumOfSurfaces);
            for (int i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         i,
                                                                         Alphas,
                                                                         NumAlphas,
                                                                         Numbers,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericBlanks,
                                                                         lAlphaBlanks,
                                                                         cAlphaFields,
                                                                         cNumericFields);
                UtilityRoutines::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);
                state.afn->MultizoneSurfaceData(i).SurfName = Alphas(1);    // Name of Associated EnergyPlus surface
                state.afn->MultizoneSurfaceData(i).OpeningName = Alphas(2); // Name of crack or opening component,
                // either simple or detailed large opening, or crack
                state.afn->MultizoneSurfaceData(i).ExternalNodeName = Alphas(3); // Name of external node, but not used at WPC="INPUT"
                if (UtilityRoutines::FindItemInList(Alphas(3), state.afn->MultizoneExternalNodeData) &&
                    state.afn
                            ->MultizoneExternalNodeData(
                                UtilityRoutines::FindItemInList(Alphas(3), state.afn->MultizoneExternalNodeData))
                            .curve == 0) {
                    ShowSevereError(state, format(RoutineName) + "Invalid " + cAlphaFields(3) + "=" + Alphas(3));
                    ShowContinueError(state,
                                      "A valid wind pressure coefficient curve name is required but not found when Wind Pressure "
                                      "Coefficient Type = Input.");
                    ErrorsFound = true;
                }
                state.afn->MultizoneSurfaceData(i).Factor = Numbers(1); // Crack Actual Value or Window Open Factor for Ventilation
                if (state.afn->MultizoneSurfaceData(i).Factor > 1.0 ||
                    state.afn->MultizoneSurfaceData(i).Factor <= 0.0) {
                    ShowWarningError(state,
                                     format(RoutineName) + CurrentModuleObject +
                                         " object=" + state.afn->MultizoneSurfaceData(i).SurfName + ", " + cNumericFields(1) +
                                         " is out of range (0.0,1.0]");
                    ShowContinueError(
                        state, format("..Input value = {:.2R}, Value will be set to 1.0", state.afn->MultizoneSurfaceData(i).Factor));
                    state.afn->MultizoneSurfaceData(i).Factor = 1.0;
                }
                // Get input of ventilation control and associated data
                if (NumAlphas >= 4) {
                    // Ventilation Control Mode: "TEMPERATURE", "ENTHALPY",
                    //   "CONSTANT", "ZONELEVEL", "NOVENT", "ADJACENTTEMPERATURE",
                    //   or "ADJACENTENTHALPY"
                    if (!lAlphaBlanks(4)) state.afn->MultizoneSurfaceData(i).VentControl = Alphas(4);
                    // Name of ventilation temperature control schedule
                    if (!lAlphaBlanks(5)) state.afn->MultizoneSurfaceData(i).VentSchName = Alphas(5);
                    {
                        auto const SELECT_CASE_var(UtilityRoutines::MakeUPPERCase(state.afn->MultizoneSurfaceData(i).VentControl));
                        if (SELECT_CASE_var == "TEMPERATURE") {
                            state.afn->MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::Temp;
                            state.afn->MultizoneSurfaceData(i).IndVentControl = true;
                        } else if (SELECT_CASE_var == "ENTHALPY") {
                            state.afn->MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::Enth;
                            state.afn->MultizoneSurfaceData(i).IndVentControl = true;
                        } else if (SELECT_CASE_var == "CONSTANT") {
                            state.afn->MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::Const;
                            state.afn->MultizoneSurfaceData(i).IndVentControl = true;
                        } else if (SELECT_CASE_var == "ASHRAE55ADAPTIVE") {
                            state.afn->MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::ASH55;
                            state.afn->MultizoneSurfaceData(i).IndVentControl = true;
                        } else if (SELECT_CASE_var == "CEN15251ADAPTIVE") {
                            state.afn->MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::CEN15251;
                            state.afn->MultizoneSurfaceData(i).IndVentControl = true;
                        } else if (SELECT_CASE_var == "NOVENT") {
                            state.afn->MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::NoVent;
                            state.afn->MultizoneSurfaceData(i).IndVentControl = true;
                        } else if (SELECT_CASE_var == "ZONELEVEL") {
                            state.afn->MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::ZoneLevel;
                            state.afn->MultizoneSurfaceData(i).IndVentControl = false;
                        } else if (SELECT_CASE_var == "ADJACENTTEMPERATURE") {
                            state.afn->MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::AdjTemp;
                            state.afn->MultizoneSurfaceData(i).IndVentControl = true;
                        } else if (SELECT_CASE_var == "ADJACENTENTHALPY") {
                            state.afn->MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::AdjEnth;
                            state.afn->MultizoneSurfaceData(i).IndVentControl = true;
                        } else {
                            ShowSevereError(state, format(RoutineName) + CurrentModuleObject + " object, Invalid " + cAlphaFields(4));
                            ShowContinueError(state,
                                              ".." + cAlphaFields(1) + " = " + state.afn->MultizoneSurfaceData(i).SurfName +
                                                  ", Specified " + cAlphaFields(4) + " = " + Alphas(4));
                            ShowContinueError(state,
                                              "..The valid choices are \"Temperature\", \"Enthalpy\", \"Constant\", \"NoVent\", \"ZoneLevel\", "
                                              "\"AdjancentTemperature\" or \"AdjacentEnthalpy\"");
                            ErrorsFound = true;
                        }
                    }
                }
                state.afn->MultizoneSurfaceData(i).ModulateFactor =
                    Numbers(2); // Limit Value on Multiplier for Modulating Venting Open Factor
                state.afn->MultizoneSurfaceData(i).LowValueTemp =
                    Numbers(3); // Lower temperature value for modulation of temperature control
                state.afn->MultizoneSurfaceData(i).UpValueTemp =
                    Numbers(4); // Upper temperature value for modulation of temperature control
                state.afn->MultizoneSurfaceData(i).LowValueEnth =
                    Numbers(5);                                                             // Lower Enthalpy value for modulation of Enthalpy control
                state.afn->MultizoneSurfaceData(i).UpValueEnth = Numbers(6); // Lower Enthalpy value for modulation of Enthalpy control
                if (state.afn->MultizoneSurfaceData(i).VentSurfCtrNum < 4 ||
                    state.afn->MultizoneSurfaceData(i).VentSurfCtrNum == VentControlType::AdjTemp ||
                    state.afn->MultizoneSurfaceData(i).VentSurfCtrNum == VentControlType::AdjEnth) {
                    if (!lAlphaBlanks(6)) {
                        state.afn->MultizoneSurfaceData(i).VentingSchName = Alphas(6); // Name of ventilation availability schedule
                    }
                }
                if (!lAlphaBlanks(7)) {
                    state.afn->MultizoneSurfaceData(i).OccupantVentilationControlName = Alphas(7);
                    state.afn->MultizoneSurfaceData(i).OccupantVentilationControlNum =
                        UtilityRoutines::FindItemInList(state.afn->MultizoneSurfaceData(i).OccupantVentilationControlName,
                                                        state.afn->OccupantVentilationControl);
                    if (state.afn->MultizoneSurfaceData(i).OccupantVentilationControlNum == 0) {
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(7) +
                                            " not found = " + state.afn->MultizoneSurfaceData(i).OccupantVentilationControlName);
                        ShowContinueError(state, "..for specified " + cAlphaFields(1) + " = " + Alphas(1));
                        ErrorsFound = true;
                    }
                }
                // Get data of polygonal surface
                if (!lAlphaBlanks(8)) {
                    if (Alphas(8) == "POLYGONHEIGHT") {
                        state.afn->MultizoneSurfaceData(i).EquivRecMethod = EquivRec::Height;
                    } else if (Alphas(8) == "BASESURFACEASPECTRATIO") {
                        state.afn->MultizoneSurfaceData(i).EquivRecMethod = EquivRec::BaseAspectRatio;
                    } else if (Alphas(8) == "USERDEFINEDASPECTRATIO") {
                        state.afn->MultizoneSurfaceData(i).EquivRecMethod = EquivRec::UserAspectRatio;
                    } else {
                        ShowSevereError(state, format(RoutineName) + CurrentModuleObject + " object, Invalid " + cAlphaFields(8));
                        ShowContinueError(state,
                                          ".." + cAlphaFields(1) + " = " + state.afn->MultizoneSurfaceData(i).SurfName +
                                              ", Specified " + cAlphaFields(8) + " = " + Alphas(8));
                        ShowContinueError(state,
                                          "..The valid choices are \"PolygonHeight\", \"BaseSurfaceAspectRatio\", or \"UserDefinedAspectRatio\"");
                        ErrorsFound = true;
                    }
                } else {
                    state.afn->MultizoneSurfaceData(i).EquivRecMethod = EquivRec::Height;
                }
                if (!lNumericBlanks(7)) {
                    state.afn->MultizoneSurfaceData(i).EquivRecUserAspectRatio = Numbers(7);
                } else {
                    state.afn->MultizoneSurfaceData(i).EquivRecUserAspectRatio = 1.0;
                }
            }
        } else {
            ShowSevereError(state, format(RoutineName) + "An " + CurrentModuleObject + " object is required but not found.");
            ErrorsFound = true;
        }

        // remove extra OutdoorAir:Node, not assigned to External Node Name
        if (state.dataGlobal->AnyLocalEnvironmentsInModel && state.afn->AirflowNetworkNumOfOutAirNode > 0) {
            for (int i = state.afn->AirflowNetworkNumOfExtNode -
                         state.afn->AirflowNetworkNumOfOutAirNode + 1;
                 i <= state.afn->AirflowNetworkNumOfExtNode;
                 ++i) {
                found = false;
                for (j = 1; j <= state.afn->AirflowNetworkNumOfSurfaces; ++j) {
                    if (UtilityRoutines::SameString(state.afn->MultizoneSurfaceData(j).ExternalNodeName,
                                                    state.afn->MultizoneExternalNodeData(i).Name)) {
                        found = true;
                    }
                }
                if (!found) {
                    if (i < state.afn->AirflowNetworkNumOfExtNode) {
                        for (k = i; k <= state.afn->AirflowNetworkNumOfExtNode - 1; ++k) {
                            state.afn->MultizoneExternalNodeData(k).Name =
                                state.afn->MultizoneExternalNodeData(k + 1).Name;
                            state.afn->MultizoneExternalNodeData(k).OutAirNodeNum =
                                state.afn->MultizoneExternalNodeData(k + 1).OutAirNodeNum;
                            state.afn->MultizoneExternalNodeData(k).height =
                                state.afn->MultizoneExternalNodeData(k + 1).height;
                            state.afn->MultizoneExternalNodeData(k).ExtNum =
                                state.afn->MultizoneExternalNodeData(k + 1).ExtNum - 1;
                        }
                        i -= 1;
                    }
                    state.afn->AirflowNetworkNumOfOutAirNode -= 1;
                    state.afn->AirflowNetworkNumOfExtNode -= 1;
                    state.afn->MultizoneExternalNodeData.redimension(
                        state.afn->AirflowNetworkNumOfExtNode);
                }
            }
        }

        // ==> Validate AirflowNetwork simulation surface data
        state.afn->NumOfExtNodes = 0;
        for (int i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) {
            // Check a valid surface defined earlier
            state.afn->MultizoneSurfaceData(i).SurfNum =
                UtilityRoutines::FindItemInList(state.afn->MultizoneSurfaceData(i).SurfName, state.dataSurface->Surface);
            if (state.afn->MultizoneSurfaceData(i).SurfNum == 0) {
                ShowSevereError(state,
                                format(RoutineName) + CurrentModuleObject + " object, Invalid " + cAlphaFields(1) +
                                    " given = " + state.afn->MultizoneSurfaceData(i).SurfName);
                ShowFatalError(state, format(RoutineName) + "Errors found getting inputs. Previous error(s) cause program termination.");
            }
            if (!state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).HeatTransSurf &&
                !state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).IsAirBoundarySurf) {
                ShowSevereError(state, format(RoutineName) + CurrentModuleObject + " object");
                ShowContinueError(state,
                                  "..The surface specified must be a heat transfer surface. Invalid " + cAlphaFields(1) + " = " +
                                      state.afn->MultizoneSurfaceData(i).SurfName);
                ErrorsFound = true;
                continue;
            }
            // Ensure an interior surface does not face itself
            if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtBoundCond >= 1) {
                // Check the surface is a subsurface or not
                if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).BaseSurf ==
                    state.afn->MultizoneSurfaceData(i).SurfNum) {
                    if (state.afn->MultizoneSurfaceData(i).SurfNum ==
                        state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtBoundCond) {
                        ShowSevereError(state, format(RoutineName) + CurrentModuleObject + " object");
                        ShowContinueError(state,
                                          "..The surface facing itself is not allowed. Invalid " + cAlphaFields(1) + " = " +
                                              state.afn->MultizoneSurfaceData(i).SurfName);
                        ErrorsFound = true;
                    }
                } else {
                    if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).BaseSurf ==
                        state.dataSurface->Surface(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).BaseSurf)
                            .ExtBoundCond) {
                        ShowSevereError(state, format(RoutineName) + CurrentModuleObject + " object");
                        ShowContinueError(state,
                                          "..The base surface facing itself is not allowed. Invalid " + cAlphaFields(1) + " = " +
                                              state.afn->MultizoneSurfaceData(i).SurfName);
                        ErrorsFound = true;
                    }
                }
            }
            // Ensure zones defined in inside and outside environment are used in the object of AIRFLOWNETWORK:MULTIZONE:ZONE
            found = false;
            n = state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Zone;
            for (j = 1; j <= state.afn->AirflowNetworkNumOfZones; ++j) {
                if (state.afn->MultizoneZoneData(j).ZoneNum == n) {
                    found = true;
                    break;
                }
            }
            // find a surface geometry
            state.afn->MultizoneSurfaceData(i).Height =
                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Height;
            state.afn->MultizoneSurfaceData(i).Width =
                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Width;
            state.afn->MultizoneSurfaceData(i).CHeight =
                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Centroid.z;
            if (found) {
                state.afn->MultizoneSurfaceData(i).NodeNums[0] = j;
            } else {
                ShowSevereError(state,
                                format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(1) + " = " +
                                    state.afn->MultizoneSurfaceData(i).SurfName);
                ShowContinueError(state,
                                  "..Zone for inside surface must be defined in a AirflowNetwork:MultiZone:Zone object.  Could not find Zone = " +
                                      Zone(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Zone).Name);
                ShowFatalError(state, format(RoutineName) + "Errors found getting inputs. Previous error(s) cause program termination.");
            }

            // Calculate equivalent width and height
            if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Sides != 4) {
                state.afn->MultizoneSurfaceData(i).NonRectangular = true;
                if (state.afn->MultizoneSurfaceData(i).EquivRecMethod == EquivRec::Height) {
                    if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Tilt < 1.0 ||
                        state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Tilt > 179.0) { // horizontal surface
                        // check base surface shape
                        if (state.dataSurface->Surface(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).BaseSurf)
                                .Sides == 4) {
                            baseratio = state.dataSurface
                                            ->Surface(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).BaseSurf)
                                            .Width /
                                        state.dataSurface
                                            ->Surface(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).BaseSurf)
                                            .Height;
                            state.afn->MultizoneSurfaceData(i).Width =
                                sqrt(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Area * baseratio);
                            state.afn->MultizoneSurfaceData(i).Height =
                                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Area /
                                state.afn->MultizoneSurfaceData(i).Width;
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                ShowWarningError(state,
                                                 format(RoutineName) + CurrentModuleObject +
                                                     " object = " + state.afn->MultizoneSurfaceData(i).SurfName);
                                ShowContinueError(state,
                                                  "The entered choice of Equivalent Rectangle Method is PolygonHeight. This choice is not valid for "
                                                  "a horizontal surface.");
                                ShowContinueError(state, "The BaseSurfaceAspectRatio choice is used. Simulation continues.");
                            }
                        } else {
                            state.afn->MultizoneSurfaceData(i).Width =
                                sqrt(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Area *
                                     state.afn->MultizoneSurfaceData(i).EquivRecUserAspectRatio);
                            state.afn->MultizoneSurfaceData(i).Height =
                                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Area /
                                state.afn->MultizoneSurfaceData(i).Width;
                            // add warning
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                ShowWarningError(state,
                                                 format(RoutineName) + CurrentModuleObject +
                                                     " object = " + state.afn->MultizoneSurfaceData(i).SurfName);
                                ShowContinueError(state,
                                                  "The entered choice of Equivalent Rectangle Method is PolygonHeight. This choice is not valid for "
                                                  "a horizontal surface with a polygonal base surface.");
                                ShowContinueError(state, "The default aspect ratio at 1 is used. Simulation continues.");
                            }
                        }
                    } else {
                        minHeight = min(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(1).z,
                                        state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(2).z);
                        maxHeight = max(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(1).z,
                                        state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(2).z);
                        for (j = 3; j <= state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Sides; ++j) {
                            minHeight = min(minHeight,
                                            min(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(j - 1).z,
                                                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(j).z));
                            maxHeight = max(maxHeight,
                                            max(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(j - 1).z,
                                                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(j).z));
                        }
                        if (maxHeight > minHeight) {
                            state.afn->MultizoneSurfaceData(i).Height = maxHeight - minHeight;
                            state.afn->MultizoneSurfaceData(i).Width =
                                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Area / (maxHeight - minHeight);
                        }
                    }
                }
                if (state.afn->MultizoneSurfaceData(i).EquivRecMethod == EquivRec::BaseAspectRatio) {
                    if (state.dataSurface->Surface(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).BaseSurf)
                            .Sides == 4) {
                        baseratio =
                            state.dataSurface->Surface(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).BaseSurf)
                                .Width /
                            state.dataSurface->Surface(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).BaseSurf)
                                .Height;
                        state.afn->MultizoneSurfaceData(i).Width =
                            sqrt(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Area * baseratio);
                        state.afn->MultizoneSurfaceData(i).Height =
                            state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Area /
                            state.afn->MultizoneSurfaceData(i).Width;
                    } else {
                        minHeight = min(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(1).z,
                                        state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(2).z);
                        maxHeight = max(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(1).z,
                                        state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(2).z);
                        for (j = 3; j <= state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Sides; ++j) {
                            minHeight = min(minHeight,
                                            min(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(j - 1).z,
                                                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(j).z));
                            maxHeight = max(maxHeight,
                                            max(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(j - 1).z,
                                                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Vertex(j).z));
                        }
                        if (maxHeight > minHeight) {
                            state.afn->MultizoneSurfaceData(i).Height = maxHeight - minHeight;
                            state.afn->MultizoneSurfaceData(i).Width =
                                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Area / (maxHeight - minHeight);
                            // add warning
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                ShowWarningError(state,
                                                 format(RoutineName) + CurrentModuleObject +
                                                     " object = " + state.afn->MultizoneSurfaceData(i).SurfName);
                                ShowContinueError(state,
                                                  "The entered choice of Equivalent Rectangle Method is BaseSurfaceAspectRatio. This choice is not "
                                                  "valid for a polygonal base surface.");
                                ShowContinueError(state, "The PolygonHeight choice is used. Simulation continues.");
                            }
                        } else {
                            state.afn->MultizoneSurfaceData(i).Width =
                                sqrt(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Area *
                                     state.afn->MultizoneSurfaceData(i).EquivRecUserAspectRatio);
                            state.afn->MultizoneSurfaceData(i).Height =
                                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Area /
                                state.afn->MultizoneSurfaceData(i).Width;
                            // add warning
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                ShowWarningError(state,
                                                 format(RoutineName) + CurrentModuleObject +
                                                     " object = " + state.afn->MultizoneSurfaceData(i).SurfName);
                                ShowContinueError(state,
                                                  "The entered choice of Equivalent Rectangle Method is BaseSurfaceAspectRatio. This choice is not "
                                                  "valid for a horizontal surface with a polygonal base surface.");
                                ShowContinueError(state, "The default aspect ratio at 1 is used. Simulation continues.");
                            }
                        }
                    }
                }
                if (state.afn->MultizoneSurfaceData(i).EquivRecMethod == EquivRec::UserAspectRatio) {
                    state.afn->MultizoneSurfaceData(i).Width =
                        sqrt(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Area *
                             state.afn->MultizoneSurfaceData(i).EquivRecUserAspectRatio);
                    state.afn->MultizoneSurfaceData(i).Height =
                        state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Area /
                        state.afn->MultizoneSurfaceData(i).Width;
                }
            }

            // Get the number of external surfaces
            if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtBoundCond == ExternalEnvironment ||
                (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtBoundCond == OtherSideCoefNoCalcExt &&
                 state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtWind)) {
                ++state.afn->AirflowNetworkNumOfExtSurfaces;
            }

            // Outside face environment
            if (state.afn->AirflowNetworkSimu.iWPCCnt == iWPCCntr::Input) {
                n = state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtBoundCond;
                if (n == ExternalEnvironment ||
                    (n == OtherSideCoefNoCalcExt && state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtWind)) {
                    ++state.afn->NumOfExtNodes;
                    if (state.afn->AirflowNetworkNumOfExtNode > 0) {
                        found = false;
                        for (j = 1; j <= state.afn->AirflowNetworkNumOfExtNode; ++j) {
                            if (UtilityRoutines::SameString(state.afn->MultizoneSurfaceData(i).ExternalNodeName,
                                                            state.afn->MultizoneExternalNodeData(j).Name)) {
                                state.afn->MultizoneSurfaceData(i).NodeNums[1] =
                                    state.afn->MultizoneExternalNodeData(j).ExtNum;
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            ShowSevereError(state,
                                            format(RoutineName) + CurrentModuleObject + ": Invalid " + cAlphaFields(3) + " = " +
                                                state.afn->MultizoneSurfaceData(i).ExternalNodeName);
                            ShowContinueError(state, "A valid " + cAlphaFields(3) + " is required when Wind Pressure Coefficient Type = Input");
                            ErrorsFound = true;
                        }
                    } else {
                        //          state.afn->MultizoneSurfaceData(i)%NodeNums[1] =
                        //          state.afn->AirflowNetworkNumOfZones+NumOfExtNodes
                    }
                    continue;
                } else {
                    if (n < ExternalEnvironment &&
                        !(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtBoundCond ==
                              OtherSideCoefNoCalcExt &&
                          state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtWind)) {
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject + ": Invalid " + cAlphaFields(1) + " = " +
                                            state.afn->MultizoneSurfaceData(i).SurfName);
                        ShowContinueError(state, "This type of surface (has ground, etc exposure) cannot be used in the AiflowNetwork model.");
                        ErrorsFound = true;
                    }
                }
                found = false;
                for (j = 1; j <= state.afn->AirflowNetworkNumOfZones; ++j) {
                    if (state.afn->MultizoneZoneData(j).ZoneNum == state.dataSurface->Surface(n).Zone) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    state.afn->MultizoneSurfaceData(i).NodeNums[1] = j;
                } else {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(1) + " = " +
                                        state.afn->MultizoneSurfaceData(i).SurfName);
                    ShowContinueError(
                        state,
                        "..Zone for outside surface must be defined in a AirflowNetwork:MultiZone:Zone object.  Could not find Zone = " +
                            Zone(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Zone).Name);
                    ErrorsFound = true;
                    continue;
                }
            }
            if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.WPCCntr, "SurfaceAverageCalculation")) {
                n = state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtBoundCond;
                if (n >= 1) { // exterior boundary condition is a surface
                    found = false;
                    for (j = 1; j <= state.afn->AirflowNetworkNumOfZones; ++j) {
                        if (state.afn->MultizoneZoneData(j).ZoneNum == state.dataSurface->Surface(n).Zone) {
                            found = true;
                            break;
                        }
                    }
                    if (found) {
                        state.afn->MultizoneSurfaceData(i).NodeNums[1] = j;
                    } else {
                        ShowSevereError(
                            state, format(RoutineName) + CurrentModuleObject + " = " + state.afn->MultizoneSurfaceData(i).SurfName);
                        ShowContinueError(state,
                                          "An adjacent zone = " + Zone(state.dataSurface->Surface(n).Zone).Name +
                                              " is not described in AIRFLOWNETWORK:MULTIZONE:ZONE");
                        ErrorsFound = true;
                        continue;
                    }
                }
            }
            if (!(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtBoundCond == -2 &&
                  state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtWind)) {
                if (state.afn->MultizoneSurfaceData(i).NodeNums[1] == 0 &&
                    state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtBoundCond < 0) {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + " = " + state.afn->MultizoneSurfaceData(i).SurfName);
                    ShowContinueError(
                        state,
                        "Outside boundary condition and object are " +
                            cExtBoundCondition(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtBoundCond) +
                            " and " + state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtBoundCondName + ".");
                    ShowContinueError(state, "The outside boundary condition must be exposed to either the outside or an adjacent zone.");
                    ErrorsFound = true;
                    continue;
                }
            }
        }

        // write outputs in eio file
        found = true;
        for (int i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) {
            if (state.afn->MultizoneSurfaceData(i).NonRectangular) {
                if (found) {
                    print(state.files.eio,
                          "! <AirflowNetwork Model:Equivalent Rectangle Surface>, Name, Equivalent Height {{m}}, Equivalent Width {{m}} "
                          "AirflowNetwork "
                          "Model:Equivalent Rectangle\n");
                    found = false;
                }
                print(state.files.eio,
                      "AirflowNetwork Model:Equivalent Rectangle Surface, {}, {:.2R},{:.2R}\n",
                      state.afn->MultizoneSurfaceData(i).SurfName,
                      state.afn->MultizoneSurfaceData(i).Height,
                      state.afn->MultizoneSurfaceData(i).Width);
            }
        }

        // Validate adjacent temperature and Enthalpy control for an interior surface only
        for (int i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) {
            if (state.afn->MultizoneSurfaceData(i).VentSurfCtrNum == VentControlType::AdjTemp) {
                if (!(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtBoundCond >= 1)) {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(1) + " = " +
                                        state.afn->MultizoneSurfaceData(i).SurfName);
                    ShowContinueError(state, "..AdjacentTemperature venting control must be defined for an interzone surface.");
                    ErrorsFound = true;
                }
            }
            if (state.afn->MultizoneSurfaceData(i).VentSurfCtrNum == VentControlType::AdjEnth) {
                if (!(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtBoundCond >= 1)) {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + " object, " + cAlphaFields(1) + " = " +
                                        state.afn->MultizoneSurfaceData(i).SurfName);
                    ShowContinueError(state, "..AdjacentEnthalpy venting control must be defined for an interzone surface.");
                    ErrorsFound = true;
                }
            }
        }

        // Ensure the number of external node = the number of external surface with HeightOption choice = OpeningHeight
        if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.HeightOption, "OpeningHeight") &&
            state.afn->AirflowNetworkSimu.iWPCCnt == iWPCCntr::Input) {
            if (state.afn->AirflowNetworkNumOfExtSurfaces !=
                state.afn->AirflowNetworkNumOfExtNode) {
                ShowSevereError(state,
                                format(RoutineName) +
                                    "When the choice of Height Selection for Local Wind Speed Calculation is OpeningHeight, the number of external "
                                    "surfaces defined in " +
                                    CurrentModuleObject + " objects ");
                ShowContinueError(state, "has to be equal to the number of AirflowNetwork:MultiZone:ExternalNode objects.");
                ShowContinueError(state,
                                  format("The entered number of external nodes is {}. The entered number of external surfaces is {}.",
                                         state.afn->AirflowNetworkNumOfExtNode,
                                         state.afn->AirflowNetworkNumOfExtSurfaces));
                ErrorsFound = true;
            }
        }

        // Read AirflowNetwork simulation detailed openings
        // Moved into getAirflowElementInput

        // Validate opening component and assign opening dimension
        if (state.afn->AirflowNetworkNumOfDetOpenings > 0) {
            for (int i = 1; i <= state.afn->AirflowNetworkNumOfDetOpenings; ++i) {
                found = false;
                for (j = 1; j <= state.afn->AirflowNetworkNumOfSurfaces; ++j) {
                    if (state.afn->MultizoneCompDetOpeningData(i).name ==
                        state.afn->MultizoneSurfaceData(j).OpeningName) {
                        //           state.afn->MultizoneCompDetOpeningData(i)%Width =
                        //           Surface(state.afn->MultizoneSurfaceData(j)%SurfNum)%Width
                        //           state.afn->MultizoneCompDetOpeningData(i)%Height =
                        //           Surface(state.afn->MultizoneSurfaceData(j)%SurfNum)%Height
                        found = true;
                    }
                }
            }
        }

        // Read AirflowNetwork simulation simple openings
        // Moved into getAirflowElementInput

        // Read AirflowNetwork simulation horizontal openings
        // Moved into getAirflowElementInput
        auto &solver = state.afn;

        // Check status of control level for each surface with an opening
        j = 0;
        CurrentModuleObject = "AirflowNetwork:MultiZone:Surface";
        for (int i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) {
            if (state.afn->MultizoneSurfaceData(i).SurfNum == 0) continue;
            bool has_Opening{false};
            // This is terrible, should not do it this way
            auto afe = solver->elements.find(state.afn->MultizoneSurfaceData(i).OpeningName);
            if (afe != solver->elements.end()) {
                auto type = afe->second->type();
                has_Opening = (type == ComponentType::DOP) || (type == ComponentType::SOP) || (type == ComponentType::HOP);
            }
            // Obtain schedule number and check surface shape
            if (has_Opening) {
                if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).Sides == 3) {
                    ShowWarningError(state,
                                     format(RoutineName) + CurrentModuleObject + "=\"" + state.afn->MultizoneSurfaceData(i).SurfName +
                                         "\".");
                    ShowContinueError(state,
                                      "The opening is a Triangular subsurface. A rectangular subsurface will be used with equivalent "
                                      "width and height.");
                }
                // Venting controls are not allowed for an air boundary surface
                if ((state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).IsAirBoundarySurf) &&
                    (state.afn->MultizoneSurfaceData(i).VentSurfCtrNum != VentControlType::Const)) {
                    ShowWarningError(state,
                                     format(RoutineName) + CurrentModuleObject + "=\"" + state.afn->MultizoneSurfaceData(i).SurfName +
                                         "\" is an air boundary surface.");
                    ShowContinueError(state, "Ventilation Control Mode = " + Alphas(4) + " is not valid. Resetting to Constant.");
                    state.afn->MultizoneSurfaceData(i).VentSurfCtrNum = VentControlType::Const;
                    state.afn->MultizoneSurfaceData(i).IndVentControl = true;
                }
                if (!state.afn->MultizoneSurfaceData(i).VentingSchName.empty()) {
                    state.afn->MultizoneSurfaceData(i).VentingSchNum =
                        GetScheduleIndex(state, state.afn->MultizoneSurfaceData(i).VentingSchName);
                    if (state.afn->MultizoneSurfaceData(i).VentingSchNum == 0) {
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject + "=\"" +
                                            state.afn->MultizoneSurfaceData(i).SurfName + "\", invalid schedule.");
                        ShowContinueError(state,
                                          "Venting Schedule not found=\"" + state.afn->MultizoneSurfaceData(i).VentingSchName + "\".");
                        ErrorsFound = true;
                    } else if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).IsAirBoundarySurf) {
                        ShowWarningError(state,
                                         format(RoutineName) + CurrentModuleObject + "=\"" +
                                             state.afn->MultizoneSurfaceData(i).SurfName + "\" is an air boundary surface.");
                        ShowContinueError(state, "Venting Availability Schedule will be ignored, venting is always available.");
                        state.afn->MultizoneSurfaceData(i).VentingSchName = "";
                        state.afn->MultizoneSurfaceData(i).VentingSchNum = 0;
                    }
                } else {
                    state.afn->MultizoneSurfaceData(i).VentingSchName = "";
                    state.afn->MultizoneSurfaceData(i).VentingSchNum = 0;
                }
                switch (state.afn->MultizoneSurfaceData(i).VentSurfCtrNum) {
                case VentControlType::Temp:
                case VentControlType::AdjTemp: {
                    state.afn->MultizoneSurfaceData(i).VentSchNum =
                        GetScheduleIndex(state, state.afn->MultizoneSurfaceData(i).VentSchName);
                    if (state.afn->MultizoneSurfaceData(i).VentSchName == std::string()) {
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject +
                                            " object, No Ventilation Schedule was found, but is required when ventilation control is "
                                            "Temperature.");
                        ShowContinueError(state, "..for Surface = \"" + state.afn->MultizoneSurfaceData(i).SurfName + "\"");
                        ErrorsFound = true;
                    } else if (state.afn->MultizoneSurfaceData(i).VentSchNum == 0) {
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject +
                                            " object, Invalid Ventilation Schedule, required when ventilation control is Temperature.");
                        ShowContinueError(state, "..Schedule name in error = " + state.afn->MultizoneSurfaceData(i).VentSchName);
                        ShowContinueError(state, "..for Surface = \"" + state.afn->MultizoneSurfaceData(i).SurfName + "\"");
                        ErrorsFound = true;
                    }
                    if (state.afn->MultizoneSurfaceData(i).LowValueTemp < 0.0) {
                        ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, Low Temperature difference value < 0.0d0");
                        ShowContinueError(state,
                                          format("..Input value={:.1R}, Value will be reset to 0.0.",
                                                 state.afn->MultizoneSurfaceData(i).LowValueTemp));
                        ShowContinueError(state, "..for Surface = \"" + state.afn->MultizoneSurfaceData(i).SurfName + "\"");
                        state.afn->MultizoneSurfaceData(i).LowValueTemp = 0.0;
                    }
                    if (state.afn->MultizoneSurfaceData(i).LowValueTemp >= 100.0) {
                        ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, Low Temperature difference value >= 100.0d0");
                        ShowContinueError(state,
                                          format("..Input value = {:.1R}, Value will be reset to 0.0",
                                                 state.afn->MultizoneSurfaceData(i).LowValueTemp));
                        ShowContinueError(state, "..for Surface = \"" + state.afn->MultizoneSurfaceData(i).SurfName + "\"");
                        state.afn->MultizoneZoneData(i).LowValueTemp = 0.0;
                    }
                    if (state.afn->MultizoneSurfaceData(i).UpValueTemp <=
                        state.afn->MultizoneSurfaceData(i).LowValueTemp) {
                        ShowWarningError(
                            state, format(RoutineName) + CurrentModuleObject + " object, Upper Temperature <= Lower Temperature difference value.");
                        ShowContinueError(state,
                                          format("..Input value = {:.1R}, Value will be reset to 100.0",
                                                 state.afn->MultizoneSurfaceData(i).UpValueTemp));
                        ShowContinueError(state, "..for Surface = \"" + state.afn->MultizoneSurfaceData(i).SurfName + "\"");
                        state.afn->MultizoneSurfaceData(i).UpValueTemp = 100.0;
                    }

                } break;
                case VentControlType::Enth:
                case VentControlType::AdjEnth: {
                    state.afn->MultizoneSurfaceData(i).VentSchNum =
                        GetScheduleIndex(state, state.afn->MultizoneSurfaceData(i).VentSchName);
                    if (state.afn->MultizoneSurfaceData(i).VentSchName == std::string()) {
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject +
                                            " object, No Ventilation Schedule was found, but is required when ventilation control is Enthalpy.");
                        ShowContinueError(state, "..for Surface = \"" + state.afn->MultizoneSurfaceData(i).SurfName + "\"");
                        ErrorsFound = true;
                    } else if (state.afn->MultizoneSurfaceData(i).VentSchNum == 0) {
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject +
                                            " object, Invalid Ventilation Schedule, required when ventilation control is Enthalpy.");
                        ShowContinueError(state, "..Schedule name in error = " + state.afn->MultizoneSurfaceData(i).VentSchName);
                        ShowContinueError(state, "..for Surface = \"" + state.afn->MultizoneSurfaceData(i).SurfName + "\"");
                        ErrorsFound = true;
                    }
                    if (state.afn->MultizoneSurfaceData(i).LowValueEnth < 0.0) {
                        ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, Low Enthalpy difference value < 0.0d0");
                        ShowContinueError(state,
                                          format("..Input value = {:.1R}, Value will be reset to 0.0",
                                                 state.afn->MultizoneSurfaceData(i).LowValueEnth));
                        ShowContinueError(state, "..for Surface = \"" + state.afn->MultizoneSurfaceData(i).SurfName + "\"");
                        state.afn->MultizoneSurfaceData(i).LowValueEnth = 0.0;
                    }
                    if (state.afn->MultizoneSurfaceData(i).LowValueEnth >= 300000.0) {
                        ShowWarningError(state, format(RoutineName) + CurrentModuleObject + " object, Low Enthalpy difference value >= 300000.0");
                        ShowContinueError(state,
                                          format("..Input value = {:.1R}, Value will be reset to 0.0",
                                                 state.afn->MultizoneSurfaceData(i).LowValueEnth));
                        ShowContinueError(state, "..for Surface = \"" + state.afn->MultizoneSurfaceData(i).SurfName + "\"");
                        state.afn->MultizoneZoneData(i).LowValueEnth = 0.0;
                    }
                    if (state.afn->MultizoneSurfaceData(i).UpValueEnth <=
                        state.afn->MultizoneSurfaceData(i).LowValueEnth) {
                        ShowWarningError(state,
                                         format(RoutineName) + CurrentModuleObject + " object, Upper Enthalpy <= Lower Enthalpy difference value.");
                        ShowContinueError(state,
                                          format("..Input value = {:.1R}, Value will be set to 300000.0",
                                                 state.afn->MultizoneSurfaceData(i).UpValueEnth));
                        ShowContinueError(state, "..for Surface = \"" + state.afn->MultizoneSurfaceData(i).SurfName + "\"");
                        state.afn->MultizoneSurfaceData(i).UpValueEnth = 300000.0;
                    }

                } break;
                case VentControlType::Const:
                case VentControlType::ASH55:
                case VentControlType::CEN15251:
                case VentControlType::NoVent:
                case VentControlType::ZoneLevel: {
                    state.afn->MultizoneSurfaceData(i).VentSchNum = 0;
                    state.afn->MultizoneSurfaceData(i).VentSchName = "";
                } break;
                default:
                    break;
                }
            }
        }

        // Validate opening component and assign opening dimension
        if (state.afn->AirflowNetworkNumOfSimOpenings > 0) {
            for (int i = 1; i <= state.afn->AirflowNetworkNumOfSimOpenings; ++i) {
                found = false;
                for (j = 1; j <= state.afn->AirflowNetworkNumOfSurfaces; ++j) {
                    if (state.afn->MultizoneCompSimpleOpeningData(i).name ==
                        state.afn->MultizoneSurfaceData(j).OpeningName) {
                        //           state.afn->MultizoneCompSimpleOpeningData(i)%Width =
                        //           Surface(state.afn->MultizoneSurfaceData(j)%SurfNum)%Width
                        //           state.afn->MultizoneCompSimpleOpeningData(i)%Height =
                        //           Surface(state.afn->MultizoneSurfaceData(j)%SurfNum)%Height
                        found = true;
                    }
                }
            }
        }

        // Calculate CP values
        if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.WPCCntr, "SurfaceAverageCalculation")) {
            state.afn->calculateWindPressureCoeffs(state);
            // Ensure automatic generation is OK
            n = 0;
            for (j = 1; j <= 5; ++j) {
                found = false;
                for (int i = 1; i <= state.afn->AirflowNetworkNumOfExtNode; ++i) {
                    if (state.afn->MultizoneExternalNodeData(i).facadeNum == j) {
                        found = true;
                        break;
                    }
                }
                if (found) ++n;
                if (j == 5 && (!found)) {
                    found = true;
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        ShowWarningError(state,
                                         format(RoutineName) +
                                             "SurfaceAverageCalculation is entered for field = Wind Pressure Coefficient Type, but no roof "
                                             "surface is defined using an AirflowNetwork:MultiZone:Surface object.");
                        ShowContinueError(state, "Reconsider if this is your modeling intent. Simulation continues.");
                    }
                }
            }
            if (n < 5 && state.dataGlobal->DisplayExtraWarnings) {
                ShowWarningError(state, format(RoutineName) + "SurfaceAverageCalculation is entered for field = Wind Pressure Coefficient Type.");
                ShowContinueError(state,
                                  "The AirflowNetwork model provides wind pressure coefficients for 4 vertical exterior orientations and "
                                  "1 horizontal roof.");
                ShowContinueError(state,
                                  format(" There are only {} exterior surface orientations defined in this input file using "
                                         "AirflowNetwork:MultiZone:Surface objects.",
                                         n));
                ShowContinueError(state, "Reconsider if this is your modeling intent. Simulation continues.");
            }
        }

        // Assign external node height
        if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.WPCCntr, "SurfaceAverageCalculation") ||
            UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.HeightOption, "OpeningHeight")) {
            for (int i = 1; i <= state.afn->AirflowNetworkNumOfExtNode; ++i) {
                for (j = 1; j <= state.afn->AirflowNetworkNumOfSurfaces; ++j) {
                    if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(j).SurfNum).ExtBoundCond == ExternalEnvironment ||
                        (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(j).SurfNum).ExtBoundCond ==
                             OtherSideCoefNoCalcExt &&
                         state.dataSurface->Surface(state.afn->MultizoneSurfaceData(j).SurfNum).ExtWind)) {
                        if (UtilityRoutines::SameString(state.afn->MultizoneSurfaceData(j).ExternalNodeName,
                                                        state.afn->MultizoneExternalNodeData(i).Name)) {
                            state.afn->MultizoneExternalNodeData(i).height =
                                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(j).SurfNum).Centroid.z;
                            break;
                        }
                    }
                }
            }
        }

        // Assign external node azimuth, should consider combining this with the above to avoid the repeated search
        for (int i = 1; i <= state.afn->AirflowNetworkNumOfExtNode; ++i) {
            for (j = 1; j <= state.afn->AirflowNetworkNumOfSurfaces; ++j) {
                if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(j).SurfNum).ExtBoundCond == ExternalEnvironment ||
                    (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(j).SurfNum).ExtBoundCond == OtherSideCoefNoCalcExt &&
                     state.dataSurface->Surface(state.afn->MultizoneSurfaceData(j).SurfNum).ExtWind)) {
                    if (UtilityRoutines::SameString(state.afn->MultizoneSurfaceData(j).ExternalNodeName,
                                                    state.afn->MultizoneExternalNodeData(i).Name)) {
                        state.afn->MultizoneExternalNodeData(i).azimuth =
                            state.dataSurface->Surface(state.afn->MultizoneSurfaceData(j).SurfNum).Azimuth;
                        break;
                    }
                }
            }
        }

        if (ErrorsFound) ShowFatalError(state, format(RoutineName) + "Errors found getting inputs. Previous error(s) cause program termination.");

        // Write wind pressure coefficients in the EIO file
        print(state.files.eio, "! <AirflowNetwork Model:Wind Direction>, Wind Direction #1 to n (degree)\n");
        print(state.files.eio, "AirflowNetwork Model:Wind Direction, ");

        int numWinDirs = 11;
        Real64 angleDelta = 30.0;
        if (state.afn->AirflowNetworkNumOfSingleSideZones > 0) {
            numWinDirs = 35;
            angleDelta = 10.0;
        }

        for (int i = 0; i < numWinDirs; ++i) {
            print(state.files.eio, "{:.1R},", i * angleDelta);
        }
        print(state.files.eio, "{:.1R}\n", numWinDirs * angleDelta);

        print(state.files.eio, "! <AirflowNetwork Model:Wind Pressure Coefficients>, Name, Wind Pressure Coefficients #1 to n (dimensionless)\n");

        // The old version used to write info with single-sided natural ventilation specific labeling, this version no longer does that.
        std::set<int> curves;
        for (int i = 1; i <= state.afn->AirflowNetworkNumOfExtNode; ++i) {
            curves.insert(state.afn->MultizoneExternalNodeData(i).curve);
        }
        for (auto index : curves) {
            print(state.files.eio, "AirflowNetwork Model:Wind Pressure Coefficients, {}, ", CurveManager::GetCurveName(state, index));

            for (j = 0; j < numWinDirs; ++j) {
                print(state.files.eio, "{:.2R},", CurveManager::CurveValue(state, index, j * angleDelta));
            }
            print(state.files.eio, "{:.2R}\n", CurveManager::CurveValue(state, index, numWinDirs * angleDelta));
        }

        if (state.afn->AirflowNetworkNumOfSingleSideZones > 0) {
            for (int i = 1; i <= state.afn->AirflowNetworkNumOfZones; ++i) {
                if (state.afn->MultizoneZoneData(i).SingleSidedCpType == "ADVANCED") {
                    print(state.files.eio,
                          "AirflowNetwork: Advanced Single-Sided Model: Difference in Opening Wind Pressure Coefficients (DeltaCP), ");
                    print(state.files.eio, "{}, ", state.afn->MultizoneZoneData(i).ZoneName);
                    for (unsigned j = 1; j <= state.afn->EPDeltaCP(i).WindDir.size() - 1; ++j) {
                        print(state.files.eio, "{:.2R},", state.afn->EPDeltaCP(i).WindDir(j));
                    }
                    print(state.files.eio,
                          "{:.2R}\n",
                          state.afn->EPDeltaCP(i).WindDir(state.afn->EPDeltaCP(i).WindDir.size()));
                }
            }
        }

        // If no zone object, exit
        if (state.afn->AirflowNetworkNumOfZones == 0) {
            ShowFatalError(state, format(RoutineName) + "Errors found getting inputs. Previous error(s) cause program termination.");
        }
        // If zone node number =0, exit.
        for (j = 1; j <= state.afn->AirflowNetworkNumOfSurfaces; ++j) {
            if (state.afn->MultizoneSurfaceData(j).NodeNums[0] == 0 && ErrorsFound) {
                ShowFatalError(state, format(RoutineName) + "Errors found getting inputs. Previous error(s) cause program termination.");
            }
            if (state.afn->MultizoneSurfaceData(j).NodeNums[1] == 0 && ErrorsFound) {
                ShowFatalError(state, format(RoutineName) + "Errors found getting inputs. Previous error(s) cause program termination.");
            }
        }

        // Ensure at least two surfaces are exposed to a zone
        ZoneCheck.allocate(state.afn->AirflowNetworkNumOfZones);
        ZoneBCCheck.allocate(state.afn->AirflowNetworkNumOfZones);
        ZoneCheck = 0;
        ZoneBCCheck = 0;
        CurrentModuleObject = "AirflowNetwork:MultiZone:Surface";
        for (j = 1; j <= state.afn->AirflowNetworkNumOfSurfaces; ++j) {
            if (state.afn->MultizoneSurfaceData(j).NodeNums[0] <= state.afn->AirflowNetworkNumOfZones) {
                ++ZoneCheck(state.afn->MultizoneSurfaceData(j).NodeNums[0]);
                ZoneBCCheck(state.afn->MultizoneSurfaceData(j).NodeNums[0]) =
                    state.afn->MultizoneSurfaceData(j).NodeNums[1];
            }
            if (state.afn->MultizoneSurfaceData(j).NodeNums[1] <= state.afn->AirflowNetworkNumOfZones) {
                ++ZoneCheck(state.afn->MultizoneSurfaceData(j).NodeNums[1]);
                ZoneBCCheck(state.afn->MultizoneSurfaceData(j).NodeNums[1]) =
                    state.afn->MultizoneSurfaceData(j).NodeNums[0];
            }
        }
        for (int i = 1; i <= state.afn->AirflowNetworkNumOfZones; ++i) {
            if (ZoneCheck(i) == 0) {
                ShowSevereError(state,
                                format(RoutineName) + "AirflowNetwork:Multizone:Zone = " + state.afn->MultizoneZoneData(i).ZoneName);
                ShowContinueError(state, " does not have any surfaces defined in " + CurrentModuleObject);
                ShowContinueError(state, "Each zone should have at least two surfaces defined in " + CurrentModuleObject);
                ErrorsFound = true;
            }
            if (ZoneCheck(i) == 1) {
                ShowSevereError(state,
                                format(RoutineName) + "AirflowNetwork:Multizone:Zone = " + state.afn->MultizoneZoneData(i).ZoneName);
                ShowContinueError(state, " has only one surface defined in " + CurrentModuleObject);
                ShowContinueError(state, " Each zone should have at least two surfaces defined in " + CurrentModuleObject);
                ErrorsFound = true;
            }
            if (ZoneCheck(i) > 1) {
                SurfaceFound = false;
                for (j = 1; j <= state.afn->AirflowNetworkNumOfSurfaces; ++j) {
                    if (state.afn->MultizoneSurfaceData(j).NodeNums[0] == i) {
                        if (ZoneBCCheck(i) != state.afn->MultizoneSurfaceData(j).NodeNums[1]) {
                            SurfaceFound = true;
                            break;
                        }
                    }
                    if (state.afn->MultizoneSurfaceData(j).NodeNums[1] == i) {
                        if (ZoneBCCheck(i) != state.afn->MultizoneSurfaceData(j).NodeNums[0]) {
                            SurfaceFound = true;
                            break;
                        }
                    }
                }
                if (!SurfaceFound) {
                    ShowWarningError(
                        state, format(RoutineName) + "AirflowNetwork:Multizone:Zone = " + state.afn->MultizoneZoneData(i).ZoneName);
                    ShowContinueError(state,
                                      "has more than one surface defined in " + CurrentModuleObject + ", but has the same boundary conditions");
                    ShowContinueError(state, "Please check inputs of " + CurrentModuleObject);
                }
            }
        }
        ZoneCheck.deallocate();
        ZoneBCCheck.deallocate();

        // Validate CP Value number
        if (state.afn->AirflowNetworkSimu.iWPCCnt == iWPCCntr::Input) { // Surface-Average does not need inputs of external nodes
            // Ensure different curve is used to avoid a single side boundary condition
            found = false;
            bool differentAngle = false;
            for (j = 2; j <= state.afn->AirflowNetworkNumOfExtNode; ++j) {
                if (state.afn->MultizoneExternalNodeData(j - 1).curve !=
                    state.afn->MultizoneExternalNodeData(j).curve) {
                    found = true;
                    break;
                } else {
                    // If the curves are the same, then check to see if the azimuths are different
                    if (state.afn->MultizoneExternalNodeData(j - 1).azimuth !=
                        state.afn->MultizoneExternalNodeData(j).azimuth) {
                        differentAngle = state.afn->MultizoneExternalNodeData(j - 1).symmetricCurve ||
                                         state.afn->MultizoneExternalNodeData(j).symmetricCurve;
                    }
                }
            }
            if (!found && !differentAngle) {
                ShowSevereError(state, "The same Wind Pressure Coefficient Curve name is used in all AirflowNetwork:MultiZone:ExternalNode objects.");
                ShowContinueError(
                    state, "Please input at least two different Wind Pressure Coefficient Curve names to avoid single side boundary condition.");
                ErrorsFound = true;
            }
        }

        // Assign occupant ventilation control number from zone to surface
        for (int i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) {
            j = state.afn->MultizoneSurfaceData(i).SurfNum;
            if (state.dataSurface->SurfWinOriginalClass(j) == SurfaceClass::Window ||
                state.dataSurface->SurfWinOriginalClass(j) == SurfaceClass::Door ||
                state.dataSurface->SurfWinOriginalClass(j) == SurfaceClass::GlassDoor) {
                for (n = 1; n <= state.afn->AirflowNetworkNumOfZones; ++n) {
                    if (state.afn->MultizoneZoneData(n).ZoneNum == state.dataSurface->Surface(j).Zone) {
                        if (state.afn->MultizoneZoneData(n).OccupantVentilationControlNum > 0 &&
                            state.afn->MultizoneSurfaceData(i).OccupantVentilationControlNum == 0) {
                            state.afn->MultizoneSurfaceData(i).OccupantVentilationControlNum =
                                state.afn->MultizoneZoneData(n).OccupantVentilationControlNum;
                        }
                    }
                }
            }
        }

        // Read AirflowNetwork Intra zone node
        CurrentModuleObject = "AirflowNetwork:IntraZone:Node";
        state.afn->IntraZoneNumOfNodes =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        if (state.afn->IntraZoneNumOfNodes > 0) {
            state.afn->IntraZoneNodeData.allocate(state.afn->IntraZoneNumOfNodes);
            for (int i = 1; i <= state.afn->IntraZoneNumOfNodes; ++i) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         i,
                                                                         Alphas,
                                                                         NumAlphas,
                                                                         Numbers,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericBlanks,
                                                                         lAlphaBlanks,
                                                                         cAlphaFields,
                                                                         cNumericFields);
                UtilityRoutines::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);
                state.afn->IntraZoneNodeData(i).Name = Alphas(1);         // Name of node
                state.afn->IntraZoneNodeData(i).RAFNNodeName = Alphas(2); // Name of RoomAir node
                state.afn->IntraZoneNodeData(i).Height = Numbers(1);      // Nodal height
                // verify RoomAir model node names(May be too early to check and move to another subroutine)
                GetRAFNNodeNum(state,
                               state.afn->IntraZoneNodeData(i).RAFNNodeName,
                               state.afn->IntraZoneNodeData(i).ZoneNum,
                               state.afn->IntraZoneNodeData(i).RAFNNodeNum,
                               Errorfound1);
                if (Errorfound1) ErrorsFound = true;
                if (state.afn->IntraZoneNodeData(i).RAFNNodeNum == 0) {
                    ShowSevereError(
                        state, format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + "' invalid name " + cAlphaFields(2) + "='" + Alphas(2));
                    ErrorsFound = true;
                }
                state.afn->IntraZoneNodeData(i).AFNZoneNum =
                    UtilityRoutines::FindItemInList(Alphas(3),
                                                    state.afn->MultizoneZoneData,
                                                    &MultizoneZoneProp::ZoneName,
                                                    state.afn->AirflowNetworkNumOfZones);
                if (state.afn->MultizoneZoneData(state.afn->IntraZoneNodeData(i).AFNZoneNum).RAFNNodeNum == 0) {
                    GetRAFNNodeNum(state,
                                   state.afn->MultizoneZoneData(state.afn->IntraZoneNodeData(i).AFNZoneNum).ZoneName,
                                   state.afn->IntraZoneNodeData(i).ZoneNum,
                                   state.afn->MultizoneZoneData(state.afn->IntraZoneNodeData(i).AFNZoneNum).RAFNNodeNum,
                                   Errorfound1);
                }
                if (state.afn->IntraZoneNodeData(i).ZoneNum == 0) {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + "' the Zone is not defined for " +
                                        cAlphaFields(3) + "='" + Alphas(3));
                    ErrorsFound = true;
                }
            }
        }

        // check model compatibility
        if (state.afn->IntraZoneNumOfNodes > 0) {
            if (!UtilityRoutines::SameString(SimAirNetworkKey, "MultizoneWithoutDistribution")) {
                ShowSevereError(state,
                                format(RoutineName) + CurrentModuleObject +
                                    " model requires Simulation Control = MultizoneWithoutDistribution, while the input choice is " +
                                    SimAirNetworkKey + ".");
                ErrorsFound = true;
                ShowFatalError(state,
                               format(RoutineName) + "Errors found getting " + CurrentModuleObject +
                                   " object."
                                   " Previous error(s) cause program termination.");
            }
        }

        state.afn->NumOfNodesIntraZone = state.afn->IntraZoneNumOfNodes;
        // check zone node
        state.afn->IntraZoneNumOfZones = 0;
        for (int i = 1; i <= state.afn->AirflowNetworkNumOfZones; ++i) {
            if (state.afn->MultizoneZoneData(i).RAFNNodeNum > 0) {
                state.afn->IntraZoneNumOfZones += 1;
            }
        }

        // Override error check due to RoomAirNode for the time being

        // Read AirflowNetwork Intra linkage
        CurrentModuleObject = "AirflowNetwork:IntraZone:Linkage";
        state.afn->IntraZoneNumOfLinks =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        if (state.afn->IntraZoneNumOfLinks > 0) {
            state.afn->IntraZoneLinkageData.allocate(state.afn->IntraZoneNumOfLinks);
            state.afn->UniqueAirflowNetworkSurfaceName.reserve(
                state.afn->IntraZoneNumOfLinks);
            for (int i = 1; i <= state.afn->IntraZoneNumOfLinks; ++i) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         i,
                                                                         Alphas,
                                                                         NumAlphas,
                                                                         Numbers,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericBlanks,
                                                                         lAlphaBlanks,
                                                                         cAlphaFields,
                                                                         cNumericFields);
                UtilityRoutines::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);
                state.afn->IntraZoneLinkageData(i).Name = Alphas(1); // Name of linkage
                state.afn->IntraZoneLinkageData(i).NodeNames[0] = Alphas(2);
                state.afn->IntraZoneLinkageData(i).NodeHeights[0] = 0.0;
                state.afn->IntraZoneLinkageData(i).NodeNames[1] = Alphas(3);
                state.afn->IntraZoneLinkageData(i).NodeHeights[1] = 0.0;
                state.afn->IntraZoneLinkageData(i).CompName = Alphas(4);
                if (!lAlphaBlanks(5)) {
                    // Perform simple test first.The comprehensive input validation will occur later
                    // Check valid surface name
                    state.afn->IntraZoneLinkageData(i).SurfaceName = Alphas(5);
                    state.afn->IntraZoneLinkageData(i).LinkNum =
                        UtilityRoutines::FindItemInList(Alphas(5),
                                                        state.afn->MultizoneSurfaceData,
                                                        &MultizoneSurfaceProp::SurfName,
                                                        state.afn->AirflowNetworkNumOfSurfaces);
                    if (state.afn->IntraZoneLinkageData(i).LinkNum == 0) {
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + "': Invalid " + cAlphaFields(5) +
                                            " given = " + Alphas(5) + " in AirflowNetwork:MultiZone:Surface objects");
                        ErrorsFound = true;
                    }
                    GlobalNames::VerifyUniqueInterObjectName(state,
                                                             state.afn->UniqueAirflowNetworkSurfaceName,
                                                             Alphas(5),
                                                             CurrentModuleObject,
                                                             cAlphaFields(5),
                                                             ErrorsFound);
                }
                if (UtilityRoutines::SameString(Alphas(2), Alphas(3))) {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + "': Invalid inputs of both node name with " +
                                        Alphas(2) + " = " + Alphas(3));
                    ErrorsFound = true;
                }
                // Check valid node names
                state.afn->IntraZoneLinkageData(i).NodeNums[0] = UtilityRoutines::FindItemInList(
                    Alphas(2), state.afn->IntraZoneNodeData, state.afn->IntraZoneNumOfNodes);
                if (state.afn->IntraZoneLinkageData(i).NodeNums[0] == 0) {
                    state.afn->IntraZoneLinkageData(i).NodeNums[0] =
                        UtilityRoutines::FindItemInList(Alphas(2),
                                                        state.afn->MultizoneZoneData,
                                                        &MultizoneZoneProp::ZoneName,
                                                        state.afn->AirflowNetworkNumOfZones);
                    state.afn->IntraZoneLinkageData(i).NodeHeights[0] =
                        Zone(state.afn->MultizoneZoneData(state.afn->IntraZoneLinkageData(i).NodeNums[0]).ZoneNum)
                            .Centroid.z;
                    if (state.afn->IntraZoneLinkageData(i).NodeNums[0] == 0) {
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + "': Invalid " + cAlphaFields(2) +
                                            " given = " + Alphas(2) + " in AirflowNetwork:IntraZone:Node and AirflowNetwork:MultiZone:Zone objects");
                        ErrorsFound = true;
                    }
                } else {
                    state.afn->IntraZoneLinkageData(i).NodeHeights[0] =
                        state.afn->IntraZoneNodeData(state.afn->IntraZoneLinkageData(i).NodeNums[0]).Height;
                    state.afn->IntraZoneLinkageData(i).NodeNums[0] =
                        state.afn->IntraZoneLinkageData(i).NodeNums[0] + state.afn->AirflowNetworkNumOfZones +
                        state.afn->AirflowNetworkNumOfExtNode;
                }
                state.afn->IntraZoneLinkageData(i).NodeNums[1] = UtilityRoutines::FindItemInList(
                    Alphas(3), state.afn->IntraZoneNodeData, state.afn->IntraZoneNumOfNodes);
                if (state.afn->IntraZoneLinkageData(i).NodeNums[1] == 0) {
                    state.afn->IntraZoneLinkageData(i).NodeNums[1] =
                        UtilityRoutines::FindItemInList(Alphas(3),
                                                        state.afn->MultizoneZoneData,
                                                        &MultizoneZoneProp::ZoneName,
                                                        state.afn->AirflowNetworkNumOfZones);
                    if (state.afn->IntraZoneLinkageData(i).NodeNums[1] > 0) {
                        state.afn->IntraZoneLinkageData(i).NodeHeights[1] =
                            Zone(state.afn->MultizoneZoneData(state.afn->IntraZoneLinkageData(i).NodeNums[1]).ZoneNum)
                                .Centroid.z;
                    } else {
                        if (state.afn->AirflowNetworkSimu.iWPCCnt ==
                            iWPCCntr::Input) { // Surface-Average does not need inputs of external nodes
                            state.afn->IntraZoneLinkageData(i).NodeNums[1] =
                                state.afn->MultizoneSurfaceData(state.afn->IntraZoneLinkageData(i).LinkNum).NodeNums[1];
                            if (state.afn->IntraZoneLinkageData(i).NodeNums[1] == 0) {
                                ShowSevereError(state,
                                                format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + "': Invalid " + cAlphaFields(3) +
                                                    " given = " + Alphas(3) +
                                                    " in AirflowNetwork:IntraZone:Node or AirflowNetwork:MultiZone:Zone or "
                                                    "AirflowNetwork:MultiZone:ExternalNode objects");
                                ErrorsFound = true;
                            }
                        }
                        if (state.afn->AirflowNetworkSimu.iWPCCnt == iWPCCntr::SurfAvg) {
                            if (!lAlphaBlanks(3)) {
                                ShowWarningError(state,
                                                 format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + " The input of " + cAlphaFields(3) +
                                                     " is not needed, ");
                                ShowContinueError(state,
                                                  " since AirflowNetwork Wind Pressure Coefficient Type = SURFACE-AVERAGE CALCULATION. The "
                                                  "simulation continues...");
                            }
                            state.afn->IntraZoneLinkageData(i).NodeNums[1] =
                                state.afn->MultizoneSurfaceData(state.afn->IntraZoneLinkageData(i).LinkNum).NodeNums[1];
                        }
                    }
                } else {
                    state.afn->IntraZoneLinkageData(i).NodeHeights[1] =
                        state.afn->IntraZoneNodeData(state.afn->IntraZoneLinkageData(i).NodeNums[1]).Height;
                    state.afn->IntraZoneLinkageData(i).NodeNums[1] =
                        state.afn->IntraZoneLinkageData(i).NodeNums[1] + state.afn->AirflowNetworkNumOfZones +
                        state.afn->AirflowNetworkNumOfExtNode;
                }
                // Ensure the both linked nodes for a surface are not zone nodes.One of nodes has to be an intrazone node
                if (state.afn->IntraZoneLinkageData(i).NodeNums[1] <= state.afn->AirflowNetworkNumOfZones &&
                    state.afn->IntraZoneLinkageData(i).NodeNums[0] <= state.afn->AirflowNetworkNumOfZones) {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + "': Invalid node inputs " + Alphas(2) + " and " +
                                        Alphas(3) + " are zone nodes");
                    ErrorsFound = true;
                }
                if (state.afn->IntraZoneLinkageData(i).NodeNums[0] <= state.afn->AirflowNetworkNumOfZones &&
                    state.afn->IntraZoneLinkageData(i).NodeNums[1] >
                        state.afn->AirflowNetworkNumOfZones + state.afn->AirflowNetworkNumOfExtNode &&
                    lAlphaBlanks(5)) {
                    if (state.afn->IntraZoneLinkageData(i).NodeNums[0] !=
                        state.afn
                            ->IntraZoneNodeData(state.afn->IntraZoneLinkageData(i).NodeNums[1] -
                                                state.afn->AirflowNetworkNumOfZones -
                                                state.afn->AirflowNetworkNumOfExtNode)
                            .AFNZoneNum) {
                        ShowSevereError(
                            state,
                            format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + ": Invalid zone inputs between Node and Link " +
                                Alphas(2) + " and " +
                                state.afn
                                    ->MultizoneZoneData(
                                        state.afn->IntraZoneNodeData(state.afn->IntraZoneLinkageData(i).NodeNums[0])
                                            .AFNZoneNum)
                                    .ZoneName);
                        ErrorsFound = true;
                    }
                }
                if (state.afn->IntraZoneLinkageData(i).NodeNums[1] <= state.afn->AirflowNetworkNumOfZones &&
                    state.afn->IntraZoneLinkageData(i).NodeNums[0] >
                        state.afn->AirflowNetworkNumOfZones + state.afn->AirflowNetworkNumOfExtNode &&
                    lAlphaBlanks(5)) {
                    if (state.afn->IntraZoneLinkageData(i).NodeNums[1] !=
                        state.afn
                            ->IntraZoneNodeData(state.afn->IntraZoneLinkageData(i).NodeNums[0] -
                                                state.afn->AirflowNetworkNumOfZones -
                                                state.afn->AirflowNetworkNumOfExtNode)
                            .AFNZoneNum) {
                        ShowSevereError(
                            state,
                            format(RoutineName) + CurrentModuleObject + "='" + Alphas(1) + ": Invalid zone inputs between Node and Link " +
                                Alphas(3) + " and " +
                                state.afn
                                    ->MultizoneZoneData(
                                        state.afn->IntraZoneNodeData(state.afn->IntraZoneLinkageData(i).NodeNums[1])
                                            .AFNZoneNum)
                                    .ZoneName);
                        ErrorsFound = true;
                    }
                }
            }

            // Reset the number of intrazone links for a given surface
            state.afn->NumOfLinksIntraZone = state.afn->IntraZoneNumOfLinks;
            for (int i = 1; i <= state.afn->IntraZoneNumOfLinks; ++i) {
                j = state.afn->IntraZoneLinkageData(i).LinkNum;
                if (j > 0) {
                    // Revise data in multizone object
                    state.afn->NumOfLinksIntraZone = state.afn->NumOfLinksIntraZone - 1;
                    if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(j).SurfNum).ExtBoundCond == 0) {
                        // Exterior surface NodeNums[1] should be equal
                        if (state.afn->IntraZoneLinkageData(i).NodeNums[0] >
                            state.afn->AirflowNetworkNumOfZones + state.afn->AirflowNetworkNumOfExtNode) {
                            state.afn->MultizoneSurfaceData(j).RAFNflag = true;
                            state.afn->MultizoneSurfaceData(j).ZonePtr = state.afn->MultizoneSurfaceData(j).NodeNums[0];
                            state.afn->MultizoneSurfaceData(j).NodeNums[0] =
                                state.afn->IntraZoneLinkageData(i).NodeNums[0];
                        } else if (state.afn->IntraZoneLinkageData(i).NodeNums[1] >
                                   state.afn->AirflowNetworkNumOfZones +
                                       state.afn->AirflowNetworkNumOfExtNode) {
                            state.afn->MultizoneSurfaceData(j).RAFNflag = true;
                            state.afn->MultizoneSurfaceData(j).ZonePtr = state.afn->MultizoneSurfaceData(j).NodeNums[0];
                            state.afn->MultizoneSurfaceData(j).NodeNums[0] =
                                state.afn->IntraZoneLinkageData(i).NodeNums[1];
                        } else {
                            ShowSevereError(state,
                                            format(RoutineName) + "The InterZone link is not found between AirflowNetwork:IntraZone:Linkage =" +
                                                state.afn->IntraZoneLinkageData(i).Name + " and AirflowNetwork:Multizone:Surface = " +
                                                state.afn->MultizoneSurfaceData(j).SurfName);
                            ErrorsFound = true;
                        }
                    } else {
                        // Interior surface
                        if (state.afn->IntraZoneLinkageData(i).NodeNums[0] >
                                state.afn->AirflowNetworkNumOfZones +
                                    state.afn->AirflowNetworkNumOfExtNode &&
                            state.afn->IntraZoneLinkageData(i).NodeNums[1] >
                                state.afn->AirflowNetworkNumOfZones +
                                    state.afn->AirflowNetworkNumOfExtNode) {
                            state.afn->MultizoneSurfaceData(j).RAFNflag = true;
                            if (state.afn->MultizoneZoneData(state.afn->MultizoneSurfaceData(j).NodeNums[0]).ZoneNum ==
                                state.afn
                                    ->IntraZoneNodeData(state.afn->IntraZoneLinkageData(i).NodeNums[0] -
                                                        state.afn->AirflowNetworkNumOfZones -
                                                        state.afn->AirflowNetworkNumOfExtNode)
                                    .ZoneNum) {
                                state.afn->MultizoneSurfaceData(j).ZonePtr =
                                    state.afn->MultizoneSurfaceData(j).NodeNums[0];
                                state.afn->MultizoneSurfaceData(j).NodeNums[0] =
                                    state.afn->IntraZoneLinkageData(i).NodeNums[0];
                                state.afn->MultizoneSurfaceData(j).NodeNums[1] =
                                    state.afn->IntraZoneLinkageData(i).NodeNums[1];
                            } else {
                                state.afn->MultizoneSurfaceData(j).ZonePtr =
                                    state.afn->MultizoneSurfaceData(j).NodeNums[0];
                                state.afn->MultizoneSurfaceData(j).NodeNums[0] =
                                    state.afn->IntraZoneLinkageData(i).NodeNums[1];
                                state.afn->MultizoneSurfaceData(j).NodeNums[1] =
                                    state.afn->IntraZoneLinkageData(i).NodeNums[0];
                            }
                        } else if (state.afn->IntraZoneLinkageData(i).NodeNums[0] >
                                   state.afn->AirflowNetworkNumOfZones +
                                       state.afn->AirflowNetworkNumOfExtNode) {
                            state.afn->MultizoneSurfaceData(j).RAFNflag = true;
                            if (state.afn->IntraZoneLinkageData(i).NodeNums[1] ==
                                state.afn->MultizoneSurfaceData(j).NodeNums[0]) {
                                state.afn->MultizoneSurfaceData(j).NodeNums[1] =
                                    state.afn->IntraZoneLinkageData(i).NodeNums[0];
                            } else if (state.afn->IntraZoneLinkageData(i).NodeNums[1] ==
                                       state.afn->MultizoneSurfaceData(j).NodeNums[1]) {
                                state.afn->MultizoneSurfaceData(j).ZonePtr =
                                    state.afn->MultizoneSurfaceData(j).NodeNums[0];
                                state.afn->MultizoneSurfaceData(j).NodeNums[0] =
                                    state.afn->IntraZoneLinkageData(i).NodeNums[0];
                            } else {
                                ShowSevereError(
                                    state,
                                    format(RoutineName) + "The InterZone link is not found between AirflowNetwork:IntraZone:Linkage =" +
                                        state.afn->IntraZoneLinkageData(i).Name +
                                        " and AirflowNetwork:Multizone:Surface = " + state.afn->MultizoneSurfaceData(j).SurfName);
                                ErrorsFound = true;
                            }
                        } else if (state.afn->IntraZoneLinkageData(i).NodeNums[1] >
                                   state.afn->AirflowNetworkNumOfZones +
                                       state.afn->AirflowNetworkNumOfExtNode) {
                            state.afn->MultizoneSurfaceData(j).RAFNflag = true;
                            if (state.afn->IntraZoneLinkageData(i).NodeNums[0] ==
                                state.afn->MultizoneSurfaceData(j).NodeNums[0]) {
                                state.afn->MultizoneSurfaceData(j).NodeNums[1] =
                                    state.afn->IntraZoneLinkageData(i).NodeNums[1];
                            } else if (state.afn->IntraZoneLinkageData(i).NodeNums[0] ==
                                       state.afn->MultizoneSurfaceData(j).NodeNums[1]) {
                                state.afn->MultizoneSurfaceData(j).ZonePtr =
                                    state.afn->MultizoneSurfaceData(j).NodeNums[0];
                                state.afn->MultizoneSurfaceData(j).NodeNums[0] =
                                    state.afn->IntraZoneLinkageData(i).NodeNums[1];
                            } else {
                                ShowSevereError(
                                    state,
                                    format(RoutineName) + "The InterZone link is not found between AirflowNetwork:IntraZone:Linkage =" +
                                        state.afn->IntraZoneLinkageData(i).Name +
                                        " and AirflowNetwork:Multizone:Surface = " + state.afn->MultizoneSurfaceData(j).SurfName);
                                ErrorsFound = true;
                            }
                        }
                    }
                }
            }
            // Remove links with surface defined in Multizone : Surface objects
            int link = 1;
            if (state.afn->NumOfLinksIntraZone < state.afn->IntraZoneNumOfLinks) {
                while (link <= state.afn->NumOfLinksIntraZone) {
                    if (state.afn->IntraZoneLinkageData(link).LinkNum > 0) {
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            ShowWarningError(state,
                                             format(RoutineName) + CurrentModuleObject + "='" +
                                                 state.afn->IntraZoneLinkageData(link).Name +
                                                 " is reomoved from the list due to the surface conncetion from Intrazone to Interzone.");
                        }
                        for (j = link; j <= state.afn->IntraZoneNumOfLinks - 1; ++j) {
                            state.afn->IntraZoneLinkageData(j) = state.afn->IntraZoneLinkageData(j + 1);
                        }
                    }
                    if (state.afn->IntraZoneLinkageData(link).LinkNum == 0) link = link + 1;
                }
                if (state.afn->IntraZoneLinkageData(link).LinkNum > 0) {
                    if (state.dataGlobal->DisplayExtraWarnings) {
                        ShowWarningError(state,
                                         format(RoutineName) + CurrentModuleObject + "='" +
                                             state.afn->IntraZoneLinkageData(link).Name +
                                             " is removed from the list due to the surface connection from Intrazone to Interzone.");
                    }
                }
            }
        }

        // Read AirflowNetwork Distribution system node
        CurrentModuleObject = "AirflowNetwork:Distribution:Node";
        state.afn->DisSysNumOfNodes =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        if (state.afn->DisSysNumOfNodes > 0) {
            state.afn->DisSysNodeData.allocate(state.afn->DisSysNumOfNodes);
            for (int i = 1; i <= state.afn->DisSysNumOfNodes; ++i) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         i,
                                                                         Alphas,
                                                                         NumAlphas,
                                                                         Numbers,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericBlanks,
                                                                         lAlphaBlanks,
                                                                         cAlphaFields,
                                                                         cNumericFields);
                UtilityRoutines::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);
                state.afn->DisSysNodeData(i).Name = Alphas(1);      // Name of node
                state.afn->DisSysNodeData(i).EPlusName = Alphas(2); // Name of associated EnergyPlus node
                state.afn->DisSysNodeData(i).EPlusType = Alphas(3); // Name of associated EnergyPlus type
                state.afn->DisSysNodeData(i).Height = Numbers(1);   // Nodal height
                state.afn->DisSysNodeData(i).EPlusNodeNum = 0;      // EPlus node number
                // verify EnergyPlus object type
                if (UtilityRoutines::SameString(Alphas(3), "AirLoopHVAC:ZoneMixer") ||
                    UtilityRoutines::SameString(Alphas(3), "AirLoopHVAC:ZoneSplitter") ||
                    UtilityRoutines::SameString(Alphas(3), "AirLoopHVAC:OutdoorAirSystem") ||
                    UtilityRoutines::SameString(Alphas(3), "OAMixerOutdoorAirStreamNode") ||
                    UtilityRoutines::SameString(Alphas(3), "OutdoorAir:NodeList") || UtilityRoutines::SameString(Alphas(3), "OutdoorAir:Node") ||
                    UtilityRoutines::SameString(Alphas(3), "Other") || lAlphaBlanks(3)) {
                } else {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + "=\"" + Alphas(1) + "\" invalid " + cAlphaFields(3) + "=\"" +
                                        Alphas(3) + "\" illegal key.");
                    ShowContinueError(state,
                                      "Valid keys are: AirLoopHVAC:ZoneMixer, AirLoopHVAC:ZoneSplitter, AirLoopHVAC:OutdoorAirSystem, "
                                      "OAMixerOutdoorAirStreamNode, OutdoorAir:NodeList, OutdoorAir:Node or Other.");
                    ErrorsFound = true;
                }
                // Avoid duplication of EPlusName
                for (j = 1; j < i; ++j) {
                    if (!UtilityRoutines::SameString(Alphas(2), "")) {
                        if (UtilityRoutines::SameString(state.afn->DisSysNodeData(j).EPlusName, Alphas(2))) {
                            ShowSevereError(state,
                                            format(RoutineName) + CurrentModuleObject + "=\"" + Alphas(1) + "\" Duplicated " + cAlphaFields(2) +
                                                "=\"" + Alphas(2) + "\". Please make a correction.");
                            ErrorsFound = true;
                        }
                    }
                }
            }
        } else {
            if (state.afn->SimulateAirflowNetwork > AirflowNetworkControlMultizone + 1) {
                ShowSevereError(state, format(RoutineName) + "An " + CurrentModuleObject + " object is required but not found.");
                ErrorsFound = true;
            }
        }

        CurrentModuleObject = "AirflowNetwork:Distribution:Component:Duct";
        if (state.afn->DisSysNumOfDucts == 0) {
            if (state.afn->SimulateAirflowNetwork > AirflowNetworkControlMultizone + 1) {
                ShowSevereError(state, format(RoutineName) + "An " + CurrentModuleObject + " object is required but not found.");
                ErrorsFound = true;
            }
        }

        // Read AirflowNetwork distribution system component: DuctViewFactors
        CurrentModuleObject = "AirflowNetwork:Distribution:DuctViewFactors";
        state.afn->DisSysNumOfDuctViewFactors =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        if (state.afn->DisSysNumOfDuctViewFactors > 0) {
            state.afn->AirflowNetworkLinkageViewFactorData.allocate(
                state.afn->DisSysNumOfDuctViewFactors);
            for (int i = 1; i <= state.afn->DisSysNumOfDuctViewFactors; ++i) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         i,
                                                                         Alphas,
                                                                         NumAlphas,
                                                                         Numbers,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericBlanks,
                                                                         lAlphaBlanks,
                                                                         cAlphaFields,
                                                                         cNumericFields);
                UtilityRoutines::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);

                auto &this_VF_object(state.afn->AirflowNetworkLinkageViewFactorData(i));

                this_VF_object.LinkageName = Alphas(1); // Name of linkage

                // Surface exposure fraction
                if (Numbers(2) > 1) {
                    ShowWarningError(state,
                                     "Duct surface exposure fraction greater than 1. Check input in: " + CurrentModuleObject + " " +
                                         this_VF_object.LinkageName);
                    ShowContinueError(state, "Using value of 1 for surface exposure fraction");
                    this_VF_object.DuctExposureFraction = 1;
                } else if (Numbers(2) < 0) {
                    ShowWarningError(
                        state, "Surface exposure fraction less than 0. Check input in: " + CurrentModuleObject + " " + this_VF_object.LinkageName);
                    ShowContinueError(state, "Using value of 0 for surface exposure fraction");
                    this_VF_object.DuctExposureFraction = 0;
                } else {
                    this_VF_object.DuctExposureFraction = Numbers(1);
                }

                // Duct surface emittance
                if (Numbers(2) > 1) {
                    ShowWarningError(
                        state, "Duct surface emittance greater than 1. Check input in: " + CurrentModuleObject + " " + this_VF_object.LinkageName);
                    ShowContinueError(state, "Using value of 1 for surface emittance");
                    this_VF_object.DuctEmittance = 1;
                } else if (Numbers(2) < 0) {
                    ShowWarningError(
                        state, "Surface exposure fraction less than 0. Check input in: " + CurrentModuleObject + " " + this_VF_object.LinkageName);
                    ShowContinueError(state, "Using value of 0 for surface exposure fraction");
                    this_VF_object.DuctEmittance = 0;
                } else {
                    this_VF_object.DuctEmittance = Numbers(2);
                }

                this_VF_object.ObjectNum = i;

                int numSurfaces = NumAlphas - 1;

                this_VF_object.LinkageSurfaceData.allocate(numSurfaces);

                for (int surfNum = 1; surfNum < NumAlphas; ++surfNum) {
                    this_VF_object.LinkageSurfaceData(surfNum).SurfaceName = Alphas(surfNum + 1); // Surface name
                    this_VF_object.LinkageSurfaceData(surfNum).SurfaceNum =
                        UtilityRoutines::FindItemInList(Alphas(surfNum + 1), state.dataSurface->Surface);

                    if (this_VF_object.LinkageSurfaceData(surfNum).SurfaceNum == 0) {
                        ShowFatalError(
                            state, "Surface " + Alphas(surfNum + 1) + " not found. See: " + CurrentModuleObject + " " + this_VF_object.LinkageName);
                    }

                    // Surface view factor
                    if (!state.dataSurface->Surface(this_VF_object.LinkageSurfaceData(surfNum).SurfaceNum).HeatTransSurf) {
                        ShowWarningError(state,
                                         "Surface=" + Alphas(surfNum + 1) + " is not a heat transfer surface. Check input in: " +
                                             CurrentModuleObject + " " + this_VF_object.LinkageName);
                        ShowContinueError(state, "Using value of 0 for view factor");
                        this_VF_object.LinkageSurfaceData(surfNum).ViewFactor = 0;
                    } else if (Numbers(surfNum + 2) > 1) {
                        ShowWarningError(state,
                                         "View factor for surface " + Alphas(surfNum + 1) +
                                             " greater than 1. Check input in: " + CurrentModuleObject + " " + this_VF_object.LinkageName);
                        ShowContinueError(state, "Using value of 1 for view factor");
                        this_VF_object.LinkageSurfaceData(surfNum).ViewFactor = 1;
                    } else if (Numbers(surfNum + 2) < 0) {
                        ShowWarningError(state,
                                         "View factor for surface " + Alphas(surfNum + 1) + " less than 0. Check input in: " + CurrentModuleObject +
                                             " " + this_VF_object.LinkageName);
                        ShowContinueError(state, "Using value of 0 for view factor");
                        this_VF_object.LinkageSurfaceData(surfNum).ViewFactor = 0;
                    } else {
                        this_VF_object.LinkageSurfaceData(surfNum).ViewFactor = Numbers(surfNum + 2);
                    }
                }
            }
        }

        CurrentModuleObject = "AirflowNetwork:Distribution:Component:Fan";
        if (state.afn->DisSysNumOfCVFs == 0) {
            if (state.afn->SimulateAirflowNetwork > AirflowNetworkControlMultizone + 1) {
                ShowSevereError(state, format(RoutineName) + "An " + CurrentModuleObject + " object is required but not found.");
                ErrorsFound = true;
            }
        }

        // Read PressureController
        CurrentModuleObject = "AirflowNetwork:ZoneControl:PressureController";
        state.afn->NumOfPressureControllers =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        if (state.afn->NumOfPressureControllers > 1) {
            ShowSevereError(state,
                            format(RoutineName) + "More " + CurrentModuleObject + " are found. Currently only one( \"1\") " + CurrentModuleObject +
                                " object per simulation is allowed when using AirflowNetwork Distribution Systems.");
            ShowFatalError(
                state, format(RoutineName) + "Errors found getting " + CurrentModuleObject + " object. Previous error(s) cause program termination.");
        }

        if (state.afn->NumOfPressureControllers > 0) {
            state.afn->PressureControllerData.allocate(state.afn->NumOfPressureControllers);
            for (int i = 1; i <= state.afn->NumOfPressureControllers; ++i) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         i,
                                                                         Alphas,
                                                                         NumAlphas,
                                                                         Numbers,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericBlanks,
                                                                         lAlphaBlanks,
                                                                         cAlphaFields,
                                                                         cNumericFields);
                UtilityRoutines::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);
                state.afn->PressureControllerData(i).Name = Alphas(1);     // Object Name
                state.afn->PressureControllerData(i).ZoneName = Alphas(2); // Zone name
                state.afn->PressureControllerData(i).ZoneNum = UtilityRoutines::FindItemInList(Alphas(2), Zone);
                state.afn->PressureControllerData(i).AFNNodeNum =
                    UtilityRoutines::FindItemInList(Alphas(2),
                                                    state.afn->MultizoneZoneData,
                                                    &MultizoneZoneProp::ZoneName,
                                                    state.afn->AirflowNetworkNumOfZones);
                if (state.afn->PressureControllerData(i).ZoneNum == 0) {
                    ShowSevereError(state, format(RoutineName) + CurrentModuleObject + " object, invalid " + cAlphaFields(2) + " given.");
                    ShowContinueError(state,
                                      "..invalid " + cAlphaFields(2) + " = \"" + state.afn->PressureControllerData(i).ZoneName + "\"");
                    ErrorsFound = true;
                }

                state.afn->PressureControllerData(i).ControlObjectType = Alphas(3); // Control Object Type
                state.afn->PressureControllerData(i).ControlObjectName = Alphas(4); // Control Object Name

                {
                    auto const SELECT_CASE_var(UtilityRoutines::MakeUPPERCase(Alphas(3)));
                    if (SELECT_CASE_var == "AIRFLOWNETWORK:MULTIZONE:COMPONENT:ZONEEXHAUSTFAN") {
                        state.afn->PressureControllerData(i).ControlTypeSet = PressureCtrlExhaust;
                    } else if (SELECT_CASE_var == "AIRFLOWNETWORK:DISTRIBUTION:COMPONENT:RELIEFAIRFLOW") {
                        state.afn->PressureControllerData(i).ControlTypeSet = PressureCtrlRelief;
                    } else { // Error
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject + " object, The entered choice for " + cAlphaFields(3) +
                                            " is not valid = \"" + state.afn->PressureControllerData(i).Name + "\"");
                        ShowContinueError(state,
                                          "Valid choices are "
                                          "\"AirflowNetwork:MultiZone:Component:ZoneExhaustFan\",\"AirflowNetwork:Distribution:Component:"
                                          "ReliefAirFlow\"");
                        ShowContinueError(state, "The input choice is " + Alphas(3));
                        ErrorsFound = true;
                    }
                }

                if (state.afn->PressureControllerData(i).ControlTypeSet == PressureCtrlExhaust) {
                    // This is not great
                    bool is_EXF{false};
                    auto afe = solver->elements.find(Alphas(4));
                    if (afe != solver->elements.end()) {
                        is_EXF = afe->second->type() == ComponentType::EXF;
                    }
                    if (!is_EXF) {
                        ShowSevereError(state, format(RoutineName) + CurrentModuleObject + " object, an invalid name is given:");
                        ShowContinueError(state, ".. invalid " + cAlphaFields(4) + " = \"" + Alphas(4) + "\".");
                        ErrorsFound = true;
                    }
                }
                if (state.afn->PressureControllerData(i).ControlTypeSet == PressureCtrlRelief) {
                    // This is not great
                    bool is_REL{false};
                    auto afe = solver->elements.find(Alphas(4));
                    if (afe != solver->elements.end()) {
                        is_REL = afe->second->type() == ComponentType::REL;
                    }
                    if (!is_REL) {
                        ShowSevereError(state, format(RoutineName) + CurrentModuleObject + " object, an invalid name is given:");
                        ShowContinueError(state, ".. invalid " + cAlphaFields(4) + " = \"" + Alphas(4) + "\".");
                        ErrorsFound = true;
                    }
                }

                if (lAlphaBlanks(5)) {
                    state.afn->PressureControllerData(i).AvailSchedPtr = DataGlobalConstants::ScheduleAlwaysOn;
                } else {
                    state.afn->PressureControllerData(i).AvailSchedPtr = GetScheduleIndex(state, Alphas(5));
                    if (state.afn->PressureControllerData(i).AvailSchedPtr == 0) {
                        ShowSevereError(state,
                                        CurrentModuleObject + ", \"" + state.afn->PressureControllerData(i).Name + "\" " +
                                            cAlphaFields(5) + " not found: " + Alphas(5));
                        ErrorsFound = true;
                    }
                }
                state.afn->PressureControllerData(i).PresSetpointSchedPtr = GetScheduleIndex(state, Alphas(6));
                if (state.afn->PressureControllerData(i).PresSetpointSchedPtr == 0) {
                    ShowSevereError(state,
                                    CurrentModuleObject + ", \"" + state.afn->PressureControllerData(i).Name + "\" " +
                                        cAlphaFields(6) + " not found: " + Alphas(6));
                    ErrorsFound = true;
                }
            }
        }

        // Assign numbers of nodes and linkages
        if (state.afn->SimulateAirflowNetwork > AirflowNetworkControlSimple) {
            if (state.afn->AirflowNetworkSimu.iWPCCnt == iWPCCntr::Input) {
                state.afn->NumOfNodesMultiZone =
                    state.afn->AirflowNetworkNumOfZones + state.afn->AirflowNetworkNumOfExtNode;
            } else {
                state.afn->NumOfNodesMultiZone =
                    state.afn->AirflowNetworkNumOfZones + state.afn->NumOfExtNodes;
            }
            state.afn->NumOfLinksMultiZone = state.afn->AirflowNetworkNumOfSurfaces;
            state.afn->AirflowNetworkNumOfNodes = state.afn->NumOfNodesMultiZone;
            if (state.afn->NumOfNodesIntraZone > 0)
                state.afn->AirflowNetworkNumOfNodes =
                    state.afn->AirflowNetworkNumOfNodes + state.afn->NumOfNodesIntraZone;
            state.afn->AirflowNetworkNumOfLinks = state.afn->NumOfLinksMultiZone;
            if (state.afn->NumOfLinksIntraZone > 0)
                state.afn->AirflowNetworkNumOfLinks =
                    state.afn->AirflowNetworkNumOfLinks + state.afn->NumOfLinksIntraZone;
        }
        if (state.afn->SimulateAirflowNetwork > AirflowNetworkControlMultizone + 1) {
            state.afn->AirflowNetworkNumOfNodes = state.afn->NumOfNodesMultiZone +
                                                                 state.afn->DisSysNumOfNodes +
                                                                 state.afn->NumOfNodesIntraZone;
        }

        // Assign node data
        state.afn->AirflowNetworkNodeData.allocate(state.afn->AirflowNetworkNumOfNodes);
        // Zone node
        for (int i = 1; i <= state.afn->AirflowNetworkNumOfZones; ++i) {
            state.afn->AirflowNetworkNodeData(i).Name = state.afn->MultizoneZoneData(i).ZoneName;
            state.afn->AirflowNetworkNodeData(i).NodeTypeNum = 0;
            state.afn->AirflowNetworkNodeData(i).EPlusZoneNum = state.afn->MultizoneZoneData(i).ZoneNum;
            state.afn->AirflowNetworkNodeData(i).NodeHeight = state.afn->MultizoneZoneData(i).Height;
        }
        // External node
        if (state.afn->AirflowNetworkSimu.iWPCCnt == iWPCCntr::Input) {
            for (int i = state.afn->AirflowNetworkNumOfZones + 1; i <= state.afn->NumOfNodesMultiZone; ++i) {
                state.afn->AirflowNetworkNodeData(i).Name =
                    state.afn->MultizoneExternalNodeData(i - state.afn->AirflowNetworkNumOfZones).Name;
                state.afn->AirflowNetworkNodeData(i).NodeTypeNum = 1;
                state.afn->AirflowNetworkNodeData(i).EPlusZoneNum = 0;
                state.afn->AirflowNetworkNodeData(i).NodeHeight =
                    state.afn->MultizoneExternalNodeData(i - state.afn->AirflowNetworkNumOfZones).height;
                state.afn->AirflowNetworkNodeData(i).ExtNodeNum = i - state.afn->AirflowNetworkNumOfZones;
                state.afn->AirflowNetworkNodeData(i).OutAirNodeNum =
                    state.afn->MultizoneExternalNodeData(i - state.afn->AirflowNetworkNumOfZones).OutAirNodeNum;
            }
        } else { // Surface-Average input
            for (int i = state.afn->AirflowNetworkNumOfZones + 1; i <= state.afn->NumOfNodesMultiZone; ++i) {
                n = i - state.afn->AirflowNetworkNumOfZones;
                state.afn->AirflowNetworkNodeData(i).Name = state.afn->MultizoneExternalNodeData(n).Name;
                state.afn->AirflowNetworkNodeData(i).NodeTypeNum = 1;
                state.afn->AirflowNetworkNodeData(i).EPlusZoneNum = 0;
                state.afn->AirflowNetworkNodeData(i).ExtNodeNum = n;
            }
        }

        // Intrazone node
        if (state.afn->NumOfNodesIntraZone > 0) {
            for (int i = state.afn->NumOfNodesMultiZone + 1;
                 i <= state.afn->NumOfNodesMultiZone + state.afn->NumOfNodesIntraZone;
                 ++i) {
                n = i - state.afn->NumOfNodesMultiZone;
                state.afn->AirflowNetworkNodeData(i).Name = state.afn->IntraZoneNodeData(n).Name;
                state.afn->AirflowNetworkNodeData(i).NodeTypeNum = 0;
                state.afn->AirflowNetworkNodeData(i).EPlusZoneNum = state.afn->IntraZoneNodeData(n).ZoneNum;
                state.afn->AirflowNetworkNodeData(i).NodeHeight = state.afn->IntraZoneNodeData(n).Height;
                state.afn->AirflowNetworkNodeData(i).RAFNNodeNum = state.afn->IntraZoneNodeData(n).RAFNNodeNum;
            }
            for (int i = 1; i <= state.afn->AirflowNetworkNumOfZones; ++i) {
                if (state.afn->MultizoneZoneData(i).RAFNNodeNum > 0) {
                    state.afn->AirflowNetworkNodeData(i).RAFNNodeNum = state.afn->MultizoneZoneData(i).RAFNNodeNum;
                }
            }
        }
        state.afn->NumOfNodesMultiZone = state.afn->NumOfNodesMultiZone + state.afn->NumOfNodesIntraZone;

        // Check whether Distribution system is simulated
        if (state.afn->AirflowNetworkNumOfNodes > state.afn->NumOfNodesMultiZone) {
            // Search node types: OAMixerOutdoorAirStreamNode, OutdoorAir:NodeList, and OutdoorAir:Node
            j = 0;
            for (int i = state.afn->NumOfNodesMultiZone + 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
                if (UtilityRoutines::SameString(state.afn->DisSysNodeData(i - state.afn->NumOfNodesMultiZone).EPlusType,
                                                "OAMixerOutdoorAirStreamNode")) {
                    ++j;
                }
                if (UtilityRoutines::SameString(state.afn->DisSysNodeData(i - state.afn->NumOfNodesMultiZone).EPlusType,
                                                "OutdoorAir:NodeList")) {
                    ++j;
                }
                if (UtilityRoutines::SameString(state.afn->DisSysNodeData(i - state.afn->NumOfNodesMultiZone).EPlusType,
                                                "OutdoorAir:Node")) {
                    ++j;
                }
            }

            for (int i = state.afn->NumOfNodesMultiZone + 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
                state.afn->AirflowNetworkNodeData(i).Name =
                    state.afn->DisSysNodeData(i - state.afn->NumOfNodesMultiZone).Name;
                state.afn->AirflowNetworkNodeData(i).NodeTypeNum = 0;
                state.afn->AirflowNetworkNodeData(i).EPlusZoneNum = 0;
                state.afn->AirflowNetworkNodeData(i).NodeHeight =
                    state.afn->DisSysNodeData(i - state.afn->NumOfNodesMultiZone).Height;
                state.afn->AirflowNetworkNodeData(i).EPlusNodeNum =
                    state.afn->DisSysNodeData(i - state.afn->NumOfNodesMultiZone).EPlusNodeNum;
                // Get mixer information
                if (UtilityRoutines::SameString(state.afn->DisSysNodeData(i - state.afn->NumOfNodesMultiZone).EPlusType,
                                                "AirLoopHVAC:ZoneMixer")) {
                    state.afn->AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::MIX;
                }
                // Get splitter information
                if (UtilityRoutines::SameString(state.afn->DisSysNodeData(i - state.afn->NumOfNodesMultiZone).EPlusType,
                                                "AirLoopHVAC:ZoneSplitter")) {
                    state.afn->AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::SPL;
                }
                // Get outside air system information
                if (UtilityRoutines::SameString(state.afn->DisSysNodeData(i - state.afn->NumOfNodesMultiZone).EPlusType,
                                                "AirLoopHVAC:OutdoorAirSystem")) {
                    state.afn->AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::OAN;
                }
                // Get OA system inlet information 'OAMixerOutdoorAirStreamNode' was specified as an outdoor air node implicitly
                if (UtilityRoutines::SameString(state.afn->DisSysNodeData(i - state.afn->NumOfNodesMultiZone).EPlusType,
                                                "OAMixerOutdoorAirStreamNode")) {
                    state.afn->AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::EXT;
                    state.afn->AirflowNetworkNodeData(i).ExtNodeNum =
                        state.afn->AirflowNetworkNumOfExtNode + 1;
                    state.afn->AirflowNetworkNodeData(i).NodeTypeNum = 1;
                }
                if (UtilityRoutines::SameString(state.afn->DisSysNodeData(i - state.afn->NumOfNodesMultiZone).EPlusType,
                                                "OutdoorAir:NodeList") ||
                    UtilityRoutines::SameString(state.afn->DisSysNodeData(i - state.afn->NumOfNodesMultiZone).EPlusType,
                                                "OutdoorAir:Node")) {
                    if (j > 1) {
                        state.afn->AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::EXT;
                        state.afn->AirflowNetworkNodeData(i).ExtNodeNum =
                            state.afn->AirflowNetworkNumOfExtNode + 1;
                        state.afn->AirflowNetworkNodeData(i).NodeTypeNum = 1;
                    } else {
                        ShowSevereError(state,
                                        format(RoutineName) + "AirflowNetwork:Distribution:Node: The outdoor air node is found at " +
                                            state.afn->AirflowNetworkNodeData(i).Name);
                        ShowContinueError(state,
                                          "The node with Component Object Type = OAMixerOutdoorAirStreamNode is not found. Please check inputs.");
                        ErrorsFound = true;
                    }
                }
            }
        }

        // Start to assembly AirflowNetwork Components
        state.afn->AirflowNetworkNumOfComps =
            state.afn->AirflowNetworkNumOfDetOpenings +
            state.afn->AirflowNetworkNumOfSimOpenings +
            state.afn->AirflowNetworkNumOfSurCracks + state.afn->AirflowNetworkNumOfSurELA +
            state.afn->DisSysNumOfLeaks + state.afn->DisSysNumOfELRs +
            state.afn->DisSysNumOfDucts + state.afn->DisSysNumOfDampers +
            state.afn->DisSysNumOfCVFs + state.afn->DisSysNumOfDetFans +
            state.afn->DisSysNumOfCPDs + state.afn->DisSysNumOfCoils +
            state.afn->DisSysNumOfTermUnits + state.afn->AirflowNetworkNumOfExhFan +
            state.afn->DisSysNumOfHXs + state.afn->AirflowNetworkNumOfHorOpenings +
            state.afn->NumOfOAFans + state.afn->NumOfReliefFans +
            state.afn->AirflowNetworkNumOfSFR;
        state.afn->AirflowNetworkCompData.allocate(state.afn->AirflowNetworkNumOfComps);

        for (int i = 1; i <= state.afn->AirflowNetworkNumOfDetOpenings; ++i) { // Detailed opening component
            state.afn->AirflowNetworkCompData(i).Name = state.afn->MultizoneCompDetOpeningData(i).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::DOP;
            state.afn->AirflowNetworkCompData(i).TypeNum = i;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
        }

        j = state.afn->AirflowNetworkNumOfDetOpenings;
        for (int i = 1 + j; i <= state.afn->AirflowNetworkNumOfSimOpenings + j; ++i) { // Simple opening component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->MultizoneCompSimpleOpeningData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::SOP;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
        }

        j += state.afn->AirflowNetworkNumOfSimOpenings;
        for (int i = 1 + j; i <= state.afn->AirflowNetworkNumOfSurCracks + j; ++i) { // Surface crack component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->MultizoneSurfaceCrackData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::SCR;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
        }

        j += state.afn->AirflowNetworkNumOfSurCracks;
        for (int i = 1 + j; i <= state.afn->AirflowNetworkNumOfSurELA + j; ++i) { // Surface crack component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->MultizoneSurfaceELAData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::SEL;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
        }

        j += state.afn->AirflowNetworkNumOfSurELA;
        for (int i = 1 + j; i <= state.afn->AirflowNetworkNumOfExhFan + j; ++i) { // Zone exhaust fan component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->MultizoneCompExhaustFanData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::EXF;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
        }

        j += state.afn->AirflowNetworkNumOfExhFan;
        for (int i = 1 + j; i <= state.afn->AirflowNetworkNumOfHorOpenings + j;
             ++i) { // Distribution system crack component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->MultizoneCompHorOpeningData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::HOP;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
        }

        j += state.afn->AirflowNetworkNumOfHorOpenings;
        for (int i = 1 + j; i <= state.afn->DisSysNumOfLeaks + j; ++i) { // Distribution system crack component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->DisSysCompLeakData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::PLR;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
        }

        j += state.afn->DisSysNumOfLeaks;
        for (int i = 1 + j; i <= state.afn->DisSysNumOfELRs + j;
             ++i) { // Distribution system effective leakage ratio component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->DisSysCompELRData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::ELR;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
        }

        j += state.afn->DisSysNumOfELRs;
        for (int i = 1 + j; i <= state.afn->DisSysNumOfDucts + j;
             ++i) { // Distribution system effective leakage ratio component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->DisSysCompDuctData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::DWC;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
        }

        j += state.afn->DisSysNumOfDucts;
        for (int i = 1 + j; i <= state.afn->DisSysNumOfDampers + j;
             ++i) { // Distribution system effective leakage ratio component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->DisSysCompDamperData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::DMP;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
        }

        j += state.afn->DisSysNumOfDampers;
        for (int i = 1 + j; i <= state.afn->DisSysNumOfCVFs + j;
             ++i) { // Distribution system constant volume fan component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->DisSysCompCVFData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::CVF;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
            state.afn->AirflowNetworkCompData(i).EPlusTypeNum = iEPlusComponentType::FAN;
        }

        j += state.afn->DisSysNumOfCVFs;
        for (int i = 1 + j; i <= state.afn->DisSysNumOfDetFans + j; ++i) { // Distribution system fan component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->DisSysCompDetFanData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::FAN;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
            state.afn->AirflowNetworkCompData(i).EPlusTypeNum = iEPlusComponentType::FAN;
        }

        j += state.afn->DisSysNumOfDetFans;
        for (int i = 1 + j; i <= state.afn->DisSysNumOfCPDs + j;
             ++i) { // Distribution system constant pressure drop component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->DisSysCompCPDData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::CPD;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
        }

        j += state.afn->DisSysNumOfCPDs;
        for (int i = 1 + j; i <= state.afn->DisSysNumOfCoils + j; ++i) { // Distribution system coil component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->DisSysCompCoilData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::COI;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
            state.afn->AirflowNetworkCompData(i).EPlusTypeNum = iEPlusComponentType::COI;
        }

        j += state.afn->DisSysNumOfCoils;
        for (int i = 1 + j; i <= state.afn->DisSysNumOfTermUnits + j; ++i) { // Terminal unit component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->DisSysCompTermUnitData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::TMU;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
            state.afn->AirflowNetworkCompData(i).EPlusTypeNum = iEPlusComponentType::RHT;
        }

        j += state.afn->DisSysNumOfTermUnits;
        for (int i = 1 + j; i <= state.afn->DisSysNumOfHXs + j; ++i) { // Distribution system heat exchanger component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->DisSysCompHXData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::HEX;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
            state.afn->AirflowNetworkCompData(i).EPlusTypeNum = iEPlusComponentType::HEX;
        }

        j += state.afn->DisSysNumOfHXs;
        for (int i = 1 + j; i <= state.afn->NumOfOAFans + j; ++i) { // OA fan component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->DisSysCompOutdoorAirData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::OAF;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
        }

        j += state.afn->NumOfOAFans;
        for (int i = 1 + j; i <= state.afn->NumOfReliefFans + j; ++i) { // OA fan component
            n = i - j;
            state.afn->AirflowNetworkCompData(i).Name = state.afn->DisSysCompReliefAirData(n).name;
            solver->compnum[state.afn->AirflowNetworkCompData(i).Name] = i;
            state.afn->AirflowNetworkCompData(i).CompTypeNum = iComponentTypeNum::REL;
            state.afn->AirflowNetworkCompData(i).TypeNum = n;
            state.afn->AirflowNetworkCompData(i).EPlusName = "";
            state.afn->AirflowNetworkCompData(i).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(i).EPlusType = "";
            state.afn->AirflowNetworkCompData(i).CompNum = i;
        }

        // This is also a bit of a hack to keep things working, this needs to be removed ASAP
        j += state.afn->NumOfReliefFans;
        int ii = 1 + j;
        int type_i = 1;
        for (auto &el : state.afn->SpecifiedMassFlowData) {
            state.afn->AirflowNetworkCompData(ii).Name = el.name;
            solver->compnum[el.name] = ii;
            state.afn->AirflowNetworkCompData(ii).CompTypeNum = iComponentTypeNum::SMF;
            state.afn->AirflowNetworkCompData(ii).TypeNum = type_i;
            state.afn->AirflowNetworkCompData(ii).EPlusName = "";
            state.afn->AirflowNetworkCompData(ii).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(ii).EPlusType = "";
            state.afn->AirflowNetworkCompData(ii).CompNum = ii;
            ++ii;
            ++type_i;
        }

        type_i = 1;
        for (auto &el : state.afn->SpecifiedVolumeFlowData) {
            state.afn->AirflowNetworkCompData(ii).Name = el.name;
            solver->compnum[el.name] = ii;
            state.afn->AirflowNetworkCompData(ii).CompTypeNum = iComponentTypeNum::SVF;
            state.afn->AirflowNetworkCompData(ii).TypeNum = type_i;
            state.afn->AirflowNetworkCompData(ii).EPlusName = "";
            state.afn->AirflowNetworkCompData(ii).EPlusCompName = "";
            state.afn->AirflowNetworkCompData(ii).EPlusType = "";
            state.afn->AirflowNetworkCompData(ii).CompNum = ii;
            ++ii;
            ++type_i;
        }

        // Assign linkage data

        // Read AirflowNetwork linkage data
        CurrentModuleObject = "AirflowNetwork:Distribution:Linkage";
        state.afn->DisSysNumOfLinks =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        if (state.afn->DisSysNumOfLinks > 0 &&
            state.afn->SimulateAirflowNetwork > AirflowNetworkControlMultizone) { // Multizone + Distribution
            state.afn->AirflowNetworkNumOfLinks =
                state.afn->NumOfLinksMultiZone + state.afn->DisSysNumOfLinks;
            state.afn->AirflowNetworkLinkageData.allocate(state.afn->DisSysNumOfLinks +
                                                                         state.afn->AirflowNetworkNumOfSurfaces);
        } else { // Multizone + IntraZone only
            //    state.afn->AirflowNetworkLinkageData.allocate( state.afn->AirflowNetworkNumOfSurfaces );
            state.afn->AirflowNetworkLinkageData.allocate(state.afn->AirflowNetworkNumOfLinks);
        }

        // Assign Multizone linkage based on surfaces, by assuming every surface has a crack or opening
        j = 0;
        for (count = 1; count <= state.afn->AirflowNetworkNumOfSurfaces; ++count) {
            if (state.afn->MultizoneSurfaceData(count).SurfNum == 0) continue;
            state.afn->AirflowNetworkLinkageData(count).Name = state.afn->MultizoneSurfaceData(count).SurfName;
            state.afn->AirflowNetworkLinkageData(count).NodeNums[0] =
                state.afn->MultizoneSurfaceData(count).NodeNums[0];
            state.afn->AirflowNetworkLinkageData(count).NodeNums[1] =
                state.afn->MultizoneSurfaceData(count).NodeNums[1];
            state.afn->AirflowNetworkLinkageData(count).CompName = state.afn->MultizoneSurfaceData(count).OpeningName;
            state.afn->AirflowNetworkLinkageData(count).ZoneNum = 0;
            state.afn->AirflowNetworkLinkageData(count).LinkNum = count;
            state.afn->AirflowNetworkLinkageData(count).NodeHeights[0] = state.afn->MultizoneSurfaceData(count).CHeight;
            state.afn->AirflowNetworkLinkageData(count).NodeHeights[1] = state.afn->MultizoneSurfaceData(count).CHeight;
            if (!state.dataSurface->WorldCoordSystem) {
                if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(count).NodeNums[0])
                        .EPlusZoneNum > 0) {
                    state.afn->AirflowNetworkLinkageData(count).NodeHeights[0] -=
                        Zone(state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(count).NodeNums[0])
                                 .EPlusZoneNum)
                            .OriginZ;
                }
                if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(count).NodeNums[1])
                        .EPlusZoneNum > 0) {
                    state.afn->AirflowNetworkLinkageData(count).NodeHeights[1] -=
                        Zone(state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(count).NodeNums[1])
                                 .EPlusZoneNum)
                            .OriginZ;
                }
            }
            // Find component number
            auto afe = solver->elements.find(state.afn->AirflowNetworkLinkageData(count).CompName);
            if (afe != solver->elements.end()) {
                // found = false;
                // for (i = 1; i <= state.afn->AirflowNetworkNumOfComps; ++i) {
                state.afn->AirflowNetworkLinkageData(count).element = afe->second;
                // Get CompTypeNum here, this is a hack to hold us over until the introspection is dealt with
                auto compnum_iter = solver->compnum.find(state.afn->AirflowNetworkLinkageData(count).CompName);
                assert(compnum_iter != solver->compnum.end());
                int compnum = compnum_iter->second;
                state.afn->AirflowNetworkLinkageData(count).CompNum = compnum;

                switch (state.afn->AirflowNetworkLinkageData(count).element->type()) {
                case ComponentType::DOP: {
                    // if (state.afn->AirflowNetworkLinkageData(count).CompName ==
                    // state.afn->AirflowNetworkCompData(i).Name) {
                    //    state.afn->AirflowNetworkLinkageData(count).CompNum = i;
                    //    found = true;
                    //    if (state.afn->AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::DOP) {
                    ++j;
                    state.afn->AirflowNetworkLinkageData(count).DetOpenNum = j;
                    state.afn->MultizoneSurfaceData(count).Multiplier =
                        state.dataSurface->Surface(state.afn->MultizoneSurfaceData(count).SurfNum).Multiplier;
                    if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(count).SurfNum).Tilt < 10.0 ||
                        state.dataSurface->Surface(state.afn->MultizoneSurfaceData(count).SurfNum).Tilt > 170.0) {
                        ShowWarningError(state, "An AirflowNetwork:Multizone:Surface object has an air-flow opening corresponding to");
                        ShowContinueError(
                            state, "window or door = " + state.afn->MultizoneSurfaceData(count).SurfName + ", which is within ");
                        ShowContinueError(state, "10 deg of being horizontal. Airflows through large horizontal openings are poorly");
                        ShowContinueError(state, "modeled in the AirflowNetwork model resulting in only one-way airflow.");
                    }
                    if (!(state.dataSurface->SurfWinOriginalClass(state.afn->MultizoneSurfaceData(count).SurfNum) ==
                              SurfaceClass::Window ||
                          state.dataSurface->SurfWinOriginalClass(state.afn->MultizoneSurfaceData(count).SurfNum) ==
                              SurfaceClass::GlassDoor ||
                          state.dataSurface->SurfWinOriginalClass(state.afn->MultizoneSurfaceData(count).SurfNum) ==
                              SurfaceClass::Door ||
                          state.dataSurface->Surface(state.afn->MultizoneSurfaceData(count).SurfNum).IsAirBoundarySurf)) {
                        ShowSevereError(state,
                                        format(RoutineName) +
                                            "AirflowNetworkComponent: The opening must be assigned to a window, door, glassdoor or air boundary at " +
                                            state.afn->AirflowNetworkLinkageData(count).Name);
                        ErrorsFound = true;
                    }
                    if (state.dataSurface->SurfWinOriginalClass(state.afn->MultizoneSurfaceData(count).SurfNum) ==
                            SurfaceClass::Door ||
                        state.dataSurface->SurfWinOriginalClass(state.afn->MultizoneSurfaceData(count).SurfNum) ==
                            SurfaceClass::GlassDoor) {
                        if (state.afn->MultizoneCompDetOpeningData(state.afn->AirflowNetworkCompData(compnum).TypeNum)
                                .LVOType == 2) {
                            ShowSevereError(state,
                                            format(RoutineName) +
                                                "AirflowNetworkComponent: The opening with horizontally pivoted type must be assigned to a "
                                                "window surface at " +
                                                state.afn->AirflowNetworkLinkageData(count).Name);
                            ErrorsFound = true;
                        }
                    }
                } break;
                case ComponentType::SOP: {
                    // if (state.afn->AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::SOP) {
                    state.afn->MultizoneSurfaceData(count).Multiplier =
                        state.dataSurface->Surface(state.afn->MultizoneSurfaceData(count).SurfNum).Multiplier;
                    if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(count).SurfNum).Tilt < 10.0 ||
                        state.dataSurface->Surface(state.afn->MultizoneSurfaceData(count).SurfNum).Tilt > 170.0) {
                        ShowSevereError(state, "An AirflowNetwork:Multizone:Surface object has an air-flow opening corresponding to");
                        ShowContinueError(state,
                                          "window or door = " + state.afn->MultizoneSurfaceData(count).SurfName + ", which is within");
                        ShowContinueError(state, "10 deg of being horizontal. Airflows through horizontal openings are not allowed.");
                        ShowContinueError(state,
                                          "AirflowNetwork:Multizone:Component:SimpleOpening = " +
                                              state.afn->AirflowNetworkCompData(compnum).Name);
                        ErrorsFound = true;
                    }

                    if (!(state.dataSurface->SurfWinOriginalClass(state.afn->MultizoneSurfaceData(count).SurfNum) ==
                              SurfaceClass::Window ||
                          state.dataSurface->SurfWinOriginalClass(state.afn->MultizoneSurfaceData(count).SurfNum) ==
                              SurfaceClass::GlassDoor ||
                          state.dataSurface->SurfWinOriginalClass(state.afn->MultizoneSurfaceData(count).SurfNum) ==
                              SurfaceClass::Door ||
                          state.dataSurface->Surface(state.afn->MultizoneSurfaceData(count).SurfNum).IsAirBoundarySurf)) {
                        ShowSevereError(state,
                                        format(RoutineName) +
                                            "AirflowNetworkComponent: The opening must be assigned to a window, door, glassdoor or air boundary at " +
                                            state.afn->AirflowNetworkLinkageData(count).Name);
                        ErrorsFound = true;
                    }
                } break;
                case ComponentType::HOP: {
                    // if (state.afn->AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::HOP) {
                    state.afn->MultizoneSurfaceData(count).Multiplier =
                        state.dataSurface->Surface(state.afn->MultizoneSurfaceData(count).SurfNum).Multiplier;
                    // Get linkage height from upper and lower zones
                    if (state.afn->MultizoneZoneData(state.afn->AirflowNetworkLinkageData(count).NodeNums[0]).ZoneNum >
                        0) {
                        state.afn->AirflowNetworkLinkageData(count).NodeHeights[0] =
                            Zone(state.afn->MultizoneZoneData(state.afn->AirflowNetworkLinkageData(count).NodeNums[0])
                                     .ZoneNum)
                                .Centroid.z;
                    }
                    if (state.afn->AirflowNetworkLinkageData(count).NodeNums[1] <=
                        state.afn->AirflowNetworkNumOfZones) {
                        if (state.afn->MultizoneZoneData(state.afn->AirflowNetworkLinkageData(count).NodeNums[1])
                                .ZoneNum > 0) {
                            state.afn->AirflowNetworkLinkageData(count).NodeHeights[1] =
                                Zone(state.afn
                                         ->MultizoneZoneData(state.afn->AirflowNetworkLinkageData(count).NodeNums[1])
                                         .ZoneNum)
                                    .Centroid.z;
                        }
                    }
                    if (state.afn->AirflowNetworkLinkageData(count).NodeNums[1] > state.afn->AirflowNetworkNumOfZones) {
                        ShowSevereError(state,
                                        format(RoutineName) +
                                            "AirflowNetworkComponent: The horizontal opening must be located between two thermal zones at " +
                                            state.afn->AirflowNetworkLinkageData(count).Name);
                        ShowContinueError(state, "This component is exposed to outdoors.");
                        ErrorsFound = true;
                    } else {
                        if (!(state.afn->MultizoneZoneData(state.afn->AirflowNetworkLinkageData(count).NodeNums[0])
                                      .ZoneNum > 0 &&
                              state.afn->MultizoneZoneData(state.afn->AirflowNetworkLinkageData(count).NodeNums[1])
                                      .ZoneNum > 0)) {
                            ShowSevereError(state,
                                            format(RoutineName) +
                                                "AirflowNetworkComponent: The horizontal opening must be located between two thermal zones at " +
                                                state.afn->AirflowNetworkLinkageData(count).Name);
                            ErrorsFound = true;
                        }
                    }
                    if (!(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(count).SurfNum).Tilt > 170.0 &&
                          state.dataSurface->Surface(state.afn->MultizoneSurfaceData(count).SurfNum).Tilt < 190.0) &&
                        !(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(count).SurfNum).Tilt > -10.0 &&
                          state.dataSurface->Surface(state.afn->MultizoneSurfaceData(count).SurfNum).Tilt < 10.0)) {
                        ShowWarningError(state, "An AirflowNetwork:Multizone:Surface object has an air-flow opening corresponding to");
                        ShowContinueError(state,
                                          "window or door = " + state.afn->MultizoneSurfaceData(count).SurfName + ", which is above");
                        ShowContinueError(state, "10 deg of being horizontal. Airflows through non-horizontal openings are not modeled");
                        ShowContinueError(state,
                                          "with the object of AirflowNetwork:Multizone:Component:HorizontalOpening = " +
                                              state.afn->AirflowNetworkCompData(compnum).Name);
                    }
                    if (!(state.dataSurface->SurfWinOriginalClass(state.afn->MultizoneSurfaceData(count).SurfNum) ==
                              SurfaceClass::Window ||
                          state.dataSurface->SurfWinOriginalClass(state.afn->MultizoneSurfaceData(count).SurfNum) ==
                              SurfaceClass::GlassDoor ||
                          state.dataSurface->SurfWinOriginalClass(state.afn->MultizoneSurfaceData(count).SurfNum) ==
                              SurfaceClass::Door ||
                          state.dataSurface->Surface(state.afn->MultizoneSurfaceData(count).SurfNum).IsAirBoundarySurf)) {
                        ShowSevereError(state,
                                        format(RoutineName) +
                                            "AirflowNetworkComponent: The opening must be assigned to a window, door, glassdoor or air boundary at " +
                                            state.afn->AirflowNetworkLinkageData(count).Name);
                        ErrorsFound = true;
                    }
                } break;
                default:
                    // Nothing to do here
                    break;
                }
            } else {
                ShowSevereError(state,
                                format(RoutineName) + CurrentModuleObject + ": The component is not defined in " +
                                    state.afn->AirflowNetworkLinkageData(count).Name);
                ErrorsFound = true;
            }
        }

        // Assign intrazone links
        for (count = 1 + state.afn->AirflowNetworkNumOfSurfaces;
             count <= state.afn->NumOfLinksIntraZone + state.afn->AirflowNetworkNumOfSurfaces;
             ++count) {
            state.afn->AirflowNetworkLinkageData(count).Name =
                state.afn->IntraZoneLinkageData(count - state.afn->AirflowNetworkNumOfSurfaces).Name;
            state.afn->AirflowNetworkLinkageData(count).NodeNums[0] =
                state.afn->IntraZoneLinkageData(count - state.afn->AirflowNetworkNumOfSurfaces).NodeNums[0];
            state.afn->AirflowNetworkLinkageData(count).NodeNums[1] =
                state.afn->IntraZoneLinkageData(count - state.afn->AirflowNetworkNumOfSurfaces).NodeNums[1];
            state.afn->AirflowNetworkLinkageData(count).CompName =
                state.afn->IntraZoneLinkageData(count - state.afn->AirflowNetworkNumOfSurfaces).CompName;
            state.afn->AirflowNetworkLinkageData(count).ZoneNum = 0;
            state.afn->AirflowNetworkLinkageData(count).LinkNum = count;
            state.afn->AirflowNetworkLinkageData(count).NodeHeights[0] =
                state.afn->IntraZoneLinkageData(count - state.afn->AirflowNetworkNumOfSurfaces).NodeHeights[0];
            state.afn->AirflowNetworkLinkageData(count).NodeHeights[1] =
                state.afn->IntraZoneLinkageData(count - state.afn->AirflowNetworkNumOfSurfaces).NodeHeights[1];
            // Find component number
            auto afe = solver->elements.find(state.afn->AirflowNetworkLinkageData(count).CompName);
            if (afe != solver->elements.end()) {
                state.afn->AirflowNetworkLinkageData(count).element = afe->second;
                // Get CompTypeNum here, this is a hack to hold us over until the introspection is dealt with
                auto compnum_iter = solver->compnum.find(state.afn->AirflowNetworkLinkageData(count).CompName);
                assert(compnum_iter != solver->compnum.end());
                int compnum = compnum_iter->second;
                state.afn->AirflowNetworkLinkageData(count).CompNum = compnum;
                if (state.afn->AirflowNetworkLinkageData(count).element->type() != ComponentType::SCR &&
                    state.afn->AirflowNetworkLinkageData(count).element->type() != ComponentType::SEL) {

                    ShowSevereError(state,
                                    format(RoutineName) + state.afn->AirflowNetworkLinkageData(count).CompName +
                                        ": The component is not allowed in " + state.afn->AirflowNetworkLinkageData(count).Name);
                    ShowContinueError(state,
                                      "The allowed component type is either AirflowNetwork:MultiZone:Surface:Crack or "
                                      "AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea.");
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError(state,
                                format(RoutineName) + state.afn->AirflowNetworkLinkageData(count).CompName +
                                    ": The component is not defined in " + state.afn->AirflowNetworkLinkageData(count).Name);
                ErrorsFound = true;
            }
        }

        // Reset state.afn->AirflowNetworkNumOfSurfaces by including state.afn->NumOfLinksIntraZone
        state.afn->AirflowNetworkNumOfSurfaces =
            state.afn->AirflowNetworkNumOfSurfaces + state.afn->NumOfLinksIntraZone;
        if (state.afn->NumOfLinksIntraZone > 0)
            state.afn->NumOfLinksMultiZone = state.afn->AirflowNetworkNumOfSurfaces;

        // Assign AirflowNetwork info in RoomAirflowNetworkZoneInfo
        if (state.afn->NumOfNodesIntraZone > 0) {
            for (int i = 1; i <= state.afn->NumOfNodesMultiZone; ++i) {
                n = state.afn->AirflowNetworkNodeData(i).EPlusZoneNum;
                state.afn->AirflowNetworkNodeData(i).NumOfLinks = 0;
                if (n > 0 && state.afn->AirflowNetworkNodeData(i).RAFNNodeNum > 0) {
                    state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(n)
                        .Node(state.afn->AirflowNetworkNodeData(i).RAFNNodeNum)
                        .AirflowNetworkNodeID = i;
                    for (j = 1; j <= state.afn->AirflowNetworkNumOfSurfaces; ++j) {
                        if (state.afn->AirflowNetworkLinkageData(j).NodeNums[0] == i) {
                            state.afn->AirflowNetworkNodeData(i).NumOfLinks =
                                state.afn->AirflowNetworkNodeData(i).NumOfLinks + 1;
                        } else if (state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                            state.afn->AirflowNetworkNodeData(i).NumOfLinks =
                                state.afn->AirflowNetworkNodeData(i).NumOfLinks + 1;
                        } else {
                        }
                    }
                }
                if (state.afn->AirflowNetworkNodeData(i).RAFNNodeNum > 0) {
                    for (j = 1; j <= state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(n).NumOfAirNodes; ++j) {
                        if (state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(n).Node(j).AirflowNetworkNodeID == i) {
                            state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(n).Node(j).NumOfAirflowLinks =
                                state.afn->AirflowNetworkNodeData(i).NumOfLinks;
                            state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(n).Node(j).Link.allocate(
                                state.afn->AirflowNetworkNodeData(i).NumOfLinks);
                            k = 1;
                            for (m = 1; m <= state.afn->AirflowNetworkNumOfSurfaces; ++m) {
                                if (state.afn->AirflowNetworkLinkageData(m).NodeNums[0] == i) {
                                    state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(n).Node(j).Link(k).AirflowNetworkLinkSimuID = m;
                                    state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(n).Node(j).Link(k).AirflowNetworkLinkageDataID = m;
                                    k = k + 1;
                                    if (k > state.afn->AirflowNetworkNodeData(i).NumOfLinks) break;
                                }
                                if (state.afn->AirflowNetworkLinkageData(m).NodeNums[1] == i) {
                                    state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(n).Node(j).Link(k).AirflowNetworkLinkSimuID = m;
                                    state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(n).Node(j).Link(k).AirflowNetworkLinkageDataID = m;
                                    k = k + 1;
                                    if (k > state.afn->AirflowNetworkNodeData(i).NumOfLinks) break;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (state.afn->DisSysNumOfLinks > 0 &&
            state.afn->SimulateAirflowNetwork > AirflowNetworkControlMultizone) { // Distribution

            for (auto &e : state.afn->AirflowNetworkLinkageData)
                e.ZoneNum = 0;

            for (count = state.afn->AirflowNetworkNumOfSurfaces + 1; count <= state.afn->AirflowNetworkNumOfLinks;
                 ++count) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         CurrentModuleObject,
                                                                         count - state.afn->AirflowNetworkNumOfSurfaces,
                                                                         Alphas,
                                                                         NumAlphas,
                                                                         Numbers,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         lNumericBlanks,
                                                                         lAlphaBlanks,
                                                                         cAlphaFields,
                                                                         cNumericFields);
                UtilityRoutines::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);
                state.afn->AirflowNetworkLinkageData(count).Name = Alphas(1);
                state.afn->AirflowNetworkLinkageData(count).NodeNames[0] = Alphas(2);
                state.afn->AirflowNetworkLinkageData(count).NodeHeights[0] = 0.0;
                state.afn->AirflowNetworkLinkageData(count).NodeNames[1] = Alphas(3);
                state.afn->AirflowNetworkLinkageData(count).NodeHeights[1] = 0.0;
                state.afn->AirflowNetworkLinkageData(count).CompName = Alphas(4);
                state.afn->AirflowNetworkLinkageData(count).ZoneName = Alphas(5);
                state.afn->AirflowNetworkLinkageData(count).LinkNum = count;

                for (int i = 1; i <= state.afn->DisSysNumOfDuctViewFactors; ++i) {
                    if (state.afn->AirflowNetworkLinkageData(count).Name ==
                        state.afn->AirflowNetworkLinkageViewFactorData(i).LinkageName) {
                        state.afn->AirflowNetworkLinkageData(count).LinkageViewFactorObjectNum =
                            state.afn->AirflowNetworkLinkageViewFactorData(i).ObjectNum;
                        break;
                    }
                }

                if (!lAlphaBlanks(5)) {
                    state.afn->AirflowNetworkLinkageData(count).ZoneNum =
                        UtilityRoutines::FindItemInList(state.afn->AirflowNetworkLinkageData(count).ZoneName, Zone);
                    if (state.afn->AirflowNetworkLinkageData(count).ZoneNum == 0) {
                        ShowSevereError(state,
                                        format(RoutineName) + CurrentModuleObject + ": Invalid " + cAlphaFields(5) +
                                            " given = " + state.afn->AirflowNetworkLinkageData(count).ZoneName);
                        ErrorsFound = true;
                    }
                }
                if (Alphas(2) == Alphas(3)) {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + ", " + cAlphaFields(2) + " = " + cAlphaFields(3) + " in " +
                                        state.afn->AirflowNetworkLinkageData(count).Name);
                    ErrorsFound = true;
                }
                // Find component number
                auto afe = solver->elements.find(state.afn->AirflowNetworkLinkageData(count).CompName);
                if (afe != solver->elements.end()) {
                    state.afn->AirflowNetworkLinkageData(count).element = afe->second;

                    // Get CompTypeNum here, this is a hack to hold us over until the introspection is dealt with
                    auto compnum_iter = solver->compnum.find(state.afn->AirflowNetworkLinkageData(count).CompName);
                    assert(compnum_iter != solver->compnum.end());
                    int compnum = compnum_iter->second;
                    state.afn->AirflowNetworkLinkageData(count).CompNum = compnum;
                } else {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + ": The " + cAlphaFields(4) + " is not defined in " +
                                        state.afn->AirflowNetworkLinkageData(count).Name);
                    ErrorsFound = true;
                }
                // Find Node number
                found = false;
                for (int i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
                    if (state.afn->AirflowNetworkLinkageData(count).NodeNames[0] ==
                        state.afn->AirflowNetworkNodeData(i).Name) {
                        state.afn->AirflowNetworkLinkageData(count).NodeNums[0] = i;
                        state.afn->AirflowNetworkLinkageData(count).NodeHeights[0] +=
                            state.afn->AirflowNetworkNodeData(i).NodeHeight;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + ": The " + cAlphaFields(2) + " is not found in the node data " +
                                        state.afn->AirflowNetworkLinkageData(count).Name);
                    ErrorsFound = true;
                }
                for (int i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
                    if (state.afn->AirflowNetworkLinkageData(count).NodeNames[1] ==
                        state.afn->AirflowNetworkNodeData(i).Name) {
                        state.afn->AirflowNetworkLinkageData(count).NodeNums[1] = i;
                        state.afn->AirflowNetworkLinkageData(count).NodeHeights[1] +=
                            state.afn->AirflowNetworkNodeData(i).NodeHeight;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject + ": The " + cAlphaFields(3) + " is not found in the node data " +
                                        state.afn->AirflowNetworkLinkageData(count).Name);
                    ErrorsFound = true;
                }
            }
        } else {

            if (state.afn->SimulateAirflowNetwork > AirflowNetworkControlMultizone + 1) {
                ShowSevereError(state, format(RoutineName) + "An " + CurrentModuleObject + " object is required but not found.");
                ErrorsFound = true;
            }
        }

        // Ensure no duplicated names in AirflowNetwork component objects
        //        for (i = 1; i <= state.afn->AirflowNetworkNumOfComps; ++i) {
        //            for (j = i + 1; j <= state.afn->AirflowNetworkNumOfComps; ++j) {
        //                if (UtilityRoutines::SameString(state.afn->AirflowNetworkCompData(i).Name,
        //        state.afn->AirflowNetworkCompData(j).Name)) {
        //                    // SurfaceAirflowLeakageNames
        //                    if (i <= 4 && j <= 4) {
        //                        if (state.afn->AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::DOP)
        //                            CompName(1) = "AirflowNetwork:MultiZone:Component:DetailedOpening";
        //                        if (state.afn->AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::SOP)
        //                            CompName(1) = "AirflowNetwork:MultiZone:Component:SimpleOpening";
        //                        if (state.afn->AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::SCR) CompName(1) =
        //        "AirflowNetwork:MultiZone:Surface:Crack"; if (state.afn->AirflowNetworkCompData(i).CompTypeNum ==
        //        iComponentTypeNum::SEL) CompName(1) = "AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea"; if
        //        (state.afn->AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::DOP) CompName(2) =
        //        "AirflowNetwork:MultiZone:Component:DetailedOpening"; if (state.afn->AirflowNetworkCompData(j).CompTypeNum ==
        //        iComponentTypeNum::SOP) CompName(2) = "AirflowNetwork:MultiZone:Component:SimpleOpening"; if
        //        (state.afn->AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::SCR) CompName(2) =
        //        "AirflowNetwork:MultiZone:Surface:Crack"; if (state.afn->AirflowNetworkCompData(j).CompTypeNum ==
        //        iComponentTypeNum::SEL) CompName(2) = "AirflowNetwork:MultiZone:Surface:EffectiveLeakageArea"; ShowSevereError(state, RoutineName
        //        + "Duplicated component names are found = " + state.afn->AirflowNetworkCompData(i).Name); ShowContinueError(state,
        //        "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2)); ErrorsFound = true;
        //                    }
        //                    // Distribution component
        //                    if (i > 4 && j > 4) {
        //                        if (state.afn->AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::PLR) CompName(1) =
        //        "AirflowNetwork:Distribution:Component:Leak"; if (state.afn->AirflowNetworkCompData(i).CompTypeNum ==
        //        iComponentTypeNum::DWC) CompName(1) = "AirflowNetwork:Distribution:Component:Duct"; if
        //        (state.afn->AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::ELR) CompName(1) =
        //        "AirflowNetwork:Distribution:Component:LeakageRatio"; if (state.afn->AirflowNetworkCompData(i).CompTypeNum ==
        //        iComponentTypeNum::DMP) CompName(1) = "AIRFLOWNETWORK:DISTRIBUTION:COMPONENT DAMPER"; if
        //        (state.afn->AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::CVF) CompName(1) =
        //        "AirflowNetwork:Distribution:Component:Fan"; if (state.afn->AirflowNetworkCompData(i).CompTypeNum ==
        //        iComponentTypeNum::CPD) CompName(1) = "AirflowNetwork:Distribution:Component:ConstantPressureDrop"; if
        //        (state.afn->AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::COI) CompName(1) =
        //        "AirflowNetwork:Distribution:Component:Coil"; if (state.afn->AirflowNetworkCompData(i).CompTypeNum ==
        //        iComponentTypeNum::TMU) CompName(1) = "AirflowNetwork:Distribution:Component:TerminalUnit"; if
        //        (state.afn->AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::HEX) CompName(1) =
        //        "AirflowNetwork:Distribution:Component:HeatExchanger"; if (state.afn->AirflowNetworkCompData(j).CompTypeNum ==
        //        iComponentTypeNum::PLR) CompName(2) = "AirflowNetwork:Distribution:Component:Leak"; if
        //        (state.afn->AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::DWC) CompName(2) =
        //        "AirflowNetwork:Distribution:Component:Duct"; if (state.afn->AirflowNetworkCompData(j).CompTypeNum ==
        //        iComponentTypeNum::ELR) CompName(2) = "AirflowNetwork:Distribution:Component:LeakageRatio"; if
        //        (state.afn->AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::DMP) CompName(2) =
        //        "AIRFLOWNETWORK:DISTRIBUTION:COMPONENT DAMPER"; if (state.afn->AirflowNetworkCompData(j).CompTypeNum ==
        //        iComponentTypeNum::CVF) CompName(2) = "AirflowNetwork:Distribution:Component:Fan"; if
        //        (state.afn->AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::CPD) CompName(2) =
        //        "AirflowNetwork:Distribution:Component:ConstantPressureDrop"; if (state.afn->AirflowNetworkCompData(j).CompTypeNum
        //        == iComponentTypeNum::COI) CompName(2) = "AirflowNetwork:Distribution:Component:Coil"; if
        //        (state.afn->AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::TMU) CompName(2) =
        //        "AirflowNetwork:Distribution:Component:TerminalUnit"; if (state.afn->AirflowNetworkCompData(j).CompTypeNum ==
        //        iComponentTypeNum::HEX) CompName(2) = "AirflowNetwork:Distribution:Component:HeatExchanger"; ShowSevereError(state,
        //        format(RoutineName) + "Duplicated component names are found = " + state.afn->AirflowNetworkCompData(i).Name);
        //        ShowContinueError(state, "A unique component name is required in both objects " + CompName(1) + " and " + CompName(2)); ErrorsFound
        //        = true;
        //                    }
        //                }
        //            }
        //        }

        // Node and component validation
        for (count = 1; count <= state.afn->AirflowNetworkNumOfLinks; ++count) {
            NodeFound = false;
            for (int i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
                if (i == state.afn->AirflowNetworkLinkageData(count).NodeNums[0]) {
                    NodeFound = true;
                    break;
                }
            }
            if (!NodeFound) {
                if (count <= state.afn->AirflowNetworkNumOfSurfaces) {
                    ShowSevereError(state,
                                    format(RoutineName) + state.afn->AirflowNetworkLinkageData(count).NodeNames[0] +
                                        " in AIRFLOWNETWORK:MULTIZONE:SURFACE = " + state.afn->AirflowNetworkLinkageData(count).Name +
                                        " is not found");
                } else {
                    ShowSevereError(
                        state,
                        format(RoutineName) + state.afn->AirflowNetworkLinkageData(count).NodeNames[0] +
                            " in AIRFLOWNETWORK:DISTRIBUTION:LINKAGE = " + state.afn->AirflowNetworkLinkageData(count).Name +
                            " is not found in AIRFLOWNETWORK:DISTRIBUTION:NODE objects.");
                }
                ErrorsFound = true;
            }
            NodeFound = false;
            for (int i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
                if (i == state.afn->AirflowNetworkLinkageData(count).NodeNums[1]) {
                    NodeFound = true;
                    break;
                }
            }
            if (!NodeFound) {
                if (count <= state.afn->AirflowNetworkNumOfSurfaces) {
                    ShowSevereError(state,
                                    format(RoutineName) + state.afn->AirflowNetworkLinkageData(count).NodeNames[0] +
                                        " in AIRFLOWNETWORK:MULTIZONE:SURFACE = " + state.afn->AirflowNetworkLinkageData(count).Name +
                                        " is not found");
                } else {
                    ShowSevereError(
                        state,
                        format(RoutineName) + state.afn->AirflowNetworkLinkageData(count).NodeNames[1] +
                            " in AIRFLOWNETWORK:DISTRIBUTION:LINKAGE = " + state.afn->AirflowNetworkLinkageData(count).Name +
                            " is not found in AIRFLOWNETWORK:DISTRIBUTION:NODE objects.");
                }
                ErrorsFound = true;
            }
            CompFound = false;
            for (int i = 1; i <= state.afn->AirflowNetworkNumOfComps; ++i) {
                if (i == state.afn->AirflowNetworkLinkageData(count).CompNum) {
                    CompFound = true;
                }
            }
            if (!CompFound) {
                ShowSevereError(state,
                                format(RoutineName) + "Component = " + state.afn->AirflowNetworkLinkageData(count).CompName +
                                    " in AIRFLOWNETWORK:DISTRIBUTION:LINKAGE = " + state.afn->AirflowNetworkLinkageData(count).Name +
                                    " is not found in AirflowNetwork Component Data objects.");
                ErrorsFound = true;
            }
        }

        // Ensure every AirflowNetworkNode is used in AirflowNetworkLinkage
        for (count = 1; count <= state.afn->AirflowNetworkNumOfNodes; ++count) {
            NodeFound1 = false;
            NodeFound2 = false;
            for (int i = 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
                if (count == state.afn->AirflowNetworkLinkageData(i).NodeNums[0]) {
                    NodeFound1 = true;
                }
                if (count == state.afn->AirflowNetworkLinkageData(i).NodeNums[1]) {
                    NodeFound2 = true;
                }
            }
            if ((!NodeFound1) && count > state.afn->NumOfNodesMultiZone &&
                state.afn->AirflowNetworkNodeData(count).ExtNodeNum == 0) {
                ShowSevereError(state,
                                format(RoutineName) +
                                    "AIRFLOWNETWORK:DISTRIBUTION:NODE = " + state.afn->AirflowNetworkNodeData(count).Name +
                                    " is not found as Node 1 Name in AIRFLOWNETWORK:DISTRIBUTION:LINKAGE");
                ShowContinueError(state,
                                  "Each non-external AIRFLOWNETWORK:DISTRIBUTION:NODE has to be defined as Node 1 once in "
                                  "AIRFLOWNETWORK:DISTRIBUTION:LINKAGE");
                ErrorsFound = true;
            }
            if ((!NodeFound2) && count > state.afn->NumOfNodesMultiZone &&
                state.afn->AirflowNetworkNodeData(count).ExtNodeNum == 0) {
                ShowSevereError(state,
                                format(RoutineName) +
                                    "AIRFLOWNETWORK:DISTRIBUTION:NODE = " + state.afn->AirflowNetworkNodeData(count).Name +
                                    " is not found as Node 2 Name in AIRFLOWNETWORK:DISTRIBUTION:LINKAGE");
                ShowContinueError(state,
                                  "Each non-external AIRFLOWNETWORK:DISTRIBUTION:NODE has to be defined as Node 2 once in "
                                  "AIRFLOWNETWORK:DISTRIBUTION:LINKAGE");
                ErrorsFound = true;
            }
            if ((!NodeFound1) && (!NodeFound2) && count > state.afn->NumOfNodesMultiZone &&
                state.afn->AirflowNetworkNodeData(count).ExtNodeNum > 0) {
                ShowSevereError(state,
                                format(RoutineName) +
                                    "AIRFLOWNETWORK:DISTRIBUTION:NODE = " + state.afn->AirflowNetworkNodeData(count).Name +
                                    " is not found as Node 1 Name or Node 2 Name in AIRFLOWNETWORK:DISTRIBUTION:LINKAGE");
                ShowContinueError(state, "This external AIRFLOWNETWORK:DISTRIBUTION:NODE has to be defined in AIRFLOWNETWORK:DISTRIBUTION:LINKAGE");
                ErrorsFound = true;
            }
        }

        // Ensure there is at least one node defined as EXTERNAL node
        NodeFound = false;
        for (count = 1; count <= state.afn->AirflowNetworkNumOfNodes; ++count) {
            if (state.afn->AirflowNetworkNodeData(count).ExtNodeNum > 0) {
                NodeFound = true;
            }
        }
        if (!NodeFound) {
            ShowSevereError(state,
                            format(RoutineName) +
                                "No External Nodes found in AirflowNetwork:Multizone:ExternalNode. There must be at least 1 external node defined.");
            ErrorsFound = true;
        }

        if (state.afn->AirflowNetworkSimu.iWPCCnt == iWPCCntr::Input) {
            for (count = 1; count <= state.afn->AirflowNetworkNumOfSurfaces; ++count) {
                if (state.afn->AirflowNetworkLinkageData(count).NodeNums[0] == 0) {
                    ShowSevereError(state,
                                    "The surface is not found in AIRFLOWNETWORK:MULTIZONE:SURFACE = " +
                                        state.afn->AirflowNetworkLinkageData(count).Name);
                    ErrorsFound = true;
                }
                if (state.afn->AirflowNetworkLinkageData(count).NodeNums[1] == 0) {
                    ShowSevereError(state,
                                    "The external node is not found in AIRFLOWNETWORK:MULTIZONE:SURFACE = " +
                                        state.afn->AirflowNetworkLinkageData(count).Name);
                    ErrorsFound = true;
                }
            }
        }

        // Provide a warning when a door component is assigned as envelope leakage
        //        if (!ErrorsFound) {
        //            for (count = 1; count <= state.afn->AirflowNetworkNumOfSurfaces; ++count) {
        //                if
        //        (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(count).NodeNums[0]).ExtNodeNum
        //        > 0 &&
        //                    state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(count).NodeNums[1]).EPlusZoneNum
        //        > 0 && state.afn->AirflowNetworkLinkageData(count).CompNum > 0) { if
        //        (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(count).CompNum).CompTypeNum
        //        == iComponentTypeNum::SOP) {
        //                    }
        //                }
        //                if
        //        (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(count).NodeNums[1]).ExtNodeNum
        //        > 0 &&
        //                    state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(count).NodeNums[0]).EPlusZoneNum
        //        > 0 && state.afn->AirflowNetworkLinkageData(count).CompNum > 0) { if
        //        (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(count).CompNum).CompTypeNum
        //        == iComponentTypeNum::SOP) {
        //                    }
        //                }
        //            }
        //        }

        // Ensure the name of each heat exchanger is shown either once or twice in the field of
        if (state.afn->SimulateAirflowNetwork == AirflowNetworkControlSimpleADS ||
            state.afn->SimulateAirflowNetwork == AirflowNetworkControlMultiADS) {
            for (int i = 1; i <= state.afn->DisSysNumOfHXs; ++i) {
                count = 0;
                for (j = 1; j <= state.afn->AirflowNetworkNumOfLinks; ++j) {
                    if (UtilityRoutines::SameString(state.afn->AirflowNetworkLinkageData(j).CompName,
                                                    state.afn->DisSysCompHXData(i).name)) {
                        ++count;
                    }
                }

                if (state.afn->DisSysCompHXData(i).CoilParentExists && count != 2) {
                    ShowSevereError(state,
                                    format(RoutineName) + "The inputs of component name field as a heat exchanger in "
                                                          "AIRFLOWNETWORK:DISTRIBUTION:LINKAGE is not correct");
                    ShowContinueError(state,
                                      "The entered name of heat exchanger is " + state.afn->DisSysCompHXData(i).name +
                                          " in AirflowNetwork:Distribution:Component:HeatExchanger objects");
                    ShowContinueError(state, format("The correct appearance number is 2. The entered appearance number is {}", count));
                    ErrorsFound = true;
                }
                if ((!state.afn->DisSysCompHXData(i).CoilParentExists) && count != 1) {
                    ShowSevereError(state,
                                    format(RoutineName) + "The inputs of component name field as a heat exchanger in "
                                                          "AIRFLOWNETWORK:DISTRIBUTION:LINKAGE is not correct");
                    ShowContinueError(state,
                                      "The entered name of heat exchanger is " + state.afn->DisSysCompHXData(i).name +
                                          " in AirflowNetwork:Distribution:Component:HeatExchanger objects");
                    ShowContinueError(state, format("The correct appearance number is 1. The entered appearance number is {}", count));
                    ErrorsFound = true;
                }
            }
        }

        // Check node assignments using AirflowNetwork:Distribution:Component:OutdoorAirFlow or
        // AirflowNetwork:Distribution:Component:ReliefAirFlow
        for (count = state.afn->AirflowNetworkNumOfSurfaces + 1; count <= state.afn->AirflowNetworkNumOfLinks;
             ++count) {
            int i = state.afn->AirflowNetworkLinkageData(count).CompNum;
            j = state.afn->AirflowNetworkLinkageData(count).NodeNums[0];
            k = state.afn->AirflowNetworkLinkageData(count).NodeNums[1];

            if (state.afn->AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::OAF) {
                if (!UtilityRoutines::SameString(
                        state.afn->DisSysNodeData(j - state.afn->NumOfNodesMultiZone).EPlusType,
                        "OAMixerOutdoorAirStreamNode")) {
                    ShowSevereError(state,
                                    format(RoutineName) +
                                        "AirflowNetwork:Distribution:Linkage: When the component type is "
                                        "AirflowNetwork:Distribution:Component:OutdoorAirFlow at " +
                                        state.afn->AirflowNetworkNodeData(j).Name + ",");
                    ShowContinueError(state,
                                      "the component type in the first node should be OAMixerOutdoorAirStreamNode at " +
                                          state.afn->AirflowNetworkNodeData(j).Name);
                    ErrorsFound = true;
                }
                if (!UtilityRoutines::SameString(
                        state.afn->DisSysNodeData(k - state.afn->NumOfNodesMultiZone).EPlusType,
                        "AirLoopHVAC:OutdoorAirSystem")) {
                    ShowSevereError(state,
                                    format(RoutineName) +
                                        "AirflowNetwork:Distribution:Linkage: When the component type is "
                                        "AirflowNetwork:Distribution:Component:OutdoorAirFlow at " +
                                        state.afn->AirflowNetworkNodeData(k).Name + ",");
                    ShowContinueError(state,
                                      "the component object type in the second node should be AirLoopHVAC:OutdoorAirSystem at " +
                                          state.afn->AirflowNetworkNodeData(k).Name);
                    ErrorsFound = true;
                }
            }

            if (state.afn->AirflowNetworkCompData(i).CompTypeNum == iComponentTypeNum::REL) {
                if (!UtilityRoutines::SameString(
                        state.afn->DisSysNodeData(j - state.afn->NumOfNodesMultiZone).EPlusType,
                        "AirLoopHVAC:OutdoorAirSystem")) {
                    ShowSevereError(state,
                                    format(RoutineName) +
                                        "AirflowNetwork:Distribution:Linkage: When the component type is "
                                        "AirflowNetwork:Distribution:Component:OutdoorAirFlow at " +
                                        state.afn->AirflowNetworkNodeData(j).Name + ",");
                    ShowContinueError(state,
                                      "the component object type in the first node should be AirLoopHVAC:OutdoorAirSystem at " +
                                          state.afn->AirflowNetworkNodeData(j).Name);
                    ErrorsFound = true;
                }
                if (!UtilityRoutines::SameString(
                        state.afn->DisSysNodeData(k - state.afn->NumOfNodesMultiZone).EPlusType,
                        "OAMixerOutdoorAirStreamNode")) {
                    ShowSevereError(state,
                                    format(RoutineName) +
                                        "AirflowNetwork:Distribution:Linkage: When the component type is "
                                        "AirflowNetwork:Distribution:Component:OutdoorAirFlow at " +
                                        state.afn->AirflowNetworkNodeData(k).Name + ",");
                    ShowContinueError(state,
                                      "the component type in the second node should be OAMixerOutdoorAirStreamNode at " +
                                          state.afn->AirflowNetworkNodeData(k).Name);
                    ErrorsFound = true;
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, format(RoutineName) + "Errors found getting inputs. Previous error(s) cause program termination.");
        }

        Alphas.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        Numbers.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        if (!ErrorsFound) {
            AllocateAndInitData(state);
        }
    }

void AirflowNetworkSolverData::initialize(EnergyPlusData &state)
    {
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Lixing Gu
    //       DATE WRITTEN   Aug. 2003
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine initializes variables of additional zone loads caused by ADS.

    // USE STATEMENTS:
    auto &TimeStepSys = state.dataHVACGlobal->TimeStepSys;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int i;
    int j;
    int ZoneNum;
    auto &Zone(state.dataHeatBal->Zone);

    if (initializeOneTimeFlag) {
        exchangeData.allocate(state.dataGlobal->NumOfZones); // AirflowNetwork exchange data due to air-forced system
        for (i = 1; i <= DisSysNumOfCVFs; i++) {
            if (DisSysCompCVFData(i).FanTypeNum == AirflowNetwork::FanType_SimpleOnOff) {
                multiExchangeData.allocate(state.dataGlobal->NumOfZones);
                break;
            }
        }

        initializeOneTimeFlag = false;
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            for (i = 1; i <= state.dataGlobal->NumOfZones; ++i) {
                SetupOutputVariable(state,
                                    "AFN Zone Outdoor Air Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    exchangeData(i).SumMHr,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    Zone(i).Name);
                SetupOutputVariable(state,
                                    "AFN Zone Mixing Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    exchangeData(i).SumMMHr,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    Zone(i).Name);
                SetupOutputVariable(state,
                                    "AFN Zone Outdoor Air CO2 Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    exchangeData(i).SumMHrCO,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    Zone(i).Name);
                SetupOutputVariable(state,
                                    "AFN Zone Mixing CO2 Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    exchangeData(i).SumMMHrCO,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    Zone(i).Name);
                SetupOutputVariable(state,
                                    "AFN Zone Total CO2 Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    exchangeData(i).TotalCO2,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    Zone(i).Name);
            }
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            for (i = 1; i <= state.dataGlobal->NumOfZones; ++i) {
                if (!state.dataContaminantBalance->Contaminant.CO2Simulation) {
                    SetupOutputVariable(state,
                                        "AFN Zone Outdoor Air Mass Flow Rate",
                                        OutputProcessor::Unit::kg_s,
                                        exchangeData(i).SumMHr,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        Zone(i).Name);
                    SetupOutputVariable(state,
                                        "AFN Zone Mixing Mass Flow Rate",
                                        OutputProcessor::Unit::kg_s,
                                        exchangeData(i).SumMMHr,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        Zone(i).Name);
                }
                SetupOutputVariable(state,
                                    "AFN Zone Outdoor Air Generic Air Contaminant Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    exchangeData(i).SumMHrGC,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    Zone(i).Name);
                SetupOutputVariable(state,
                                    "AFN Zone Mixing Generic Air Contaminant Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    exchangeData(i).SumMMHrGC,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    Zone(i).Name);
                SetupOutputVariable(state,
                                    "AFN Zone Total Generic Air Contaminant Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    exchangeData(i).TotalGC,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    Zone(i).Name);
            }
        }
    }

    if (state.dataGlobal->BeginEnvrnFlag && initializeMyEnvrnFlag) {
        // Assign node values
        for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
            AirflowNetworkNodeSimu(i).TZ = 23.0;
            AirflowNetworkNodeSimu(i).WZ = 0.00084;
            AirflowNetworkNodeSimu(i).PZ = 0.0;
            AirflowNetworkNodeSimu(i).TZlast = AirflowNetworkNodeSimu(i).TZ;
            AirflowNetworkNodeSimu(i).WZlast = AirflowNetworkNodeSimu(i).WZ;
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                AirflowNetworkNodeSimu(i).CO2Z = state.dataContaminantBalance->OutdoorCO2;
                AirflowNetworkNodeSimu(i).CO2Zlast = AirflowNetworkNodeSimu(i).CO2Z;
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                AirflowNetworkNodeSimu(i).GCZ = state.dataContaminantBalance->OutdoorGC;
                AirflowNetworkNodeSimu(i).GCZlast = AirflowNetworkNodeSimu(i).GCZ;
            }
            if (AirflowNetworkNodeData(i).RAFNNodeNum > 0) {
                ZoneNum = AirflowNetworkNodeData(i).EPlusZoneNum;
                state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(ZoneNum)
                    .Node(AirflowNetworkNodeData(i).RAFNNodeNum)
                    .AirTemp = 23.0;
                state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(ZoneNum)
                    .Node(AirflowNetworkNodeData(i).RAFNNodeNum)
                    .HumRat = 0.0;
            }
        }

        for (i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
            AirflowNetworkLinkSimu(i).FLOW = 0.0;
            AirflowNetworkLinkSimu(i).FLOW2 = 0.0;
        }

        for (i = 1; i <= state.dataGlobal->NumOfZones; ++i) {
            ANZT(i) = state.dataHeatBalFanSys->MAT(i);
            ANZW(i) = state.dataHeatBalFanSys->ZoneAirHumRat(i);
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                ANCO(i) = state.dataContaminantBalance->ZoneAirCO2(i);
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                ANGC(i) = state.dataContaminantBalance->ZoneAirGC(i);
            }
        }
        if (AirflowNetworkNumOfOccuVentCtrls > 0) {
            for (i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
                if (MultizoneSurfaceData(i).OccupantVentilationControlNum > 0) {
                    MultizoneSurfaceData(i).PrevOpeningstatus = AirflowNetwork::OpenStatus::FreeOperation;
                    MultizoneSurfaceData(i).CloseElapsedTime = 0.0;
                    MultizoneSurfaceData(i).OpenElapsedTime = 0.0;
                    MultizoneSurfaceData(i).OpeningStatus = AirflowNetwork::OpenStatus::FreeOperation;
                    MultizoneSurfaceData(i).OpeningProbStatus = AirflowNetwork::ProbabilityCheck::NoAction;
                    MultizoneSurfaceData(i).ClosingProbStatus = 0;
                }
            }
        }

        initializeMyEnvrnFlag = false;
    }
    if (!state.dataGlobal->BeginEnvrnFlag) {
        initializeMyEnvrnFlag = true;
        if (SimulateAirflowNetwork > AirflowNetwork::AirflowNetworkControlSimple) {
            if (RollBackFlag) {
                for (i = 1; i <= state.dataGlobal->NumOfZones; ++i) {
                    ANZT(i) = state.dataHeatBalFanSys->XMAT(i);
                    ANZW(i) = state.dataHeatBalFanSys->WZoneTimeMinus1(i);
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation)
                        ANCO(i) = state.dataContaminantBalance->CO2ZoneTimeMinus1(i);
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation)
                        ANGC(i) = state.dataContaminantBalance->GCZoneTimeMinus1(i);
                }
            } else {
                for (i = 1; i <= state.dataGlobal->NumOfZones; ++i) {
                    ANZT(i) = state.dataHeatBalFanSys->MAT(i);
                    ANZW(i) = state.dataHeatBalFanSys->ZoneAirHumRat(i);
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation)
                        ANCO(i) = state.dataContaminantBalance->ZoneAirCO2(i);
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation)
                        ANGC(i) = state.dataContaminantBalance->ZoneAirGC(i);
                }
            }

            for (i = 1; i <= AirflowNetworkNumOfNodes; ++i) {
                if (AirflowNetworkNodeData(i).EPlusZoneNum > 0) {
                    AirflowNetworkNodeSimu(i).TZ =
                        ANZT(AirflowNetworkNodeData(i).EPlusZoneNum);
                    AirflowNetworkNodeSimu(i).WZ =
                        ANZW(AirflowNetworkNodeData(i).EPlusZoneNum);
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation)
                        AirflowNetworkNodeSimu(i).CO2Z =
                            ANCO(AirflowNetworkNodeData(i).EPlusZoneNum);
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation)
                        AirflowNetworkNodeSimu(i).GCZ =
                            ANGC(AirflowNetworkNodeData(i).EPlusZoneNum);
                }
                if (AirflowNetworkNodeData(i).ExtNodeNum > 0) {
                    if (AirflowNetworkNodeData(i).OutAirNodeNum > 0 &&
                        state.dataLoopNodes->Node(AirflowNetworkNodeData(i).OutAirNodeNum).IsLocalNode) {
                        AirflowNetworkNodeSimu(i).TZ =
                            state.dataLoopNodes->Node(AirflowNetworkNodeData(i).OutAirNodeNum).OutAirDryBulb;
                        AirflowNetworkNodeSimu(i).WZ =
                            state.dataLoopNodes->Node(AirflowNetworkNodeData(i).OutAirNodeNum).HumRat;
                    } else {
                        AirflowNetworkNodeSimu(i).TZ =
                            AirflowNetwork::OutDryBulbTempAt(state, AirflowNetworkNodeData(i).NodeHeight);
                        AirflowNetworkNodeSimu(i).WZ = state.dataEnvrn->OutHumRat;
                    }

                    if (state.dataContaminantBalance->Contaminant.CO2Simulation)
                        AirflowNetworkNodeSimu(i).CO2Z = state.dataContaminantBalance->OutdoorCO2;
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation)
                        AirflowNetworkNodeSimu(i).GCZ = state.dataContaminantBalance->OutdoorGC;
                }

                if (AirflowNetworkNodeData(i).RAFNNodeNum > 0) {
                    ZoneNum = AirflowNetworkNodeData(i).EPlusZoneNum;
                    if (state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(ZoneNum)
                            .Node(AirflowNetworkNodeData(i).RAFNNodeNum)
                            .AirflowNetworkNodeID == i) {
                        AirflowNetworkNodeSimu(i).TZ =
                            state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(ZoneNum)
                                .Node(AirflowNetworkNodeData(i).RAFNNodeNum)
                                .AirTemp;
                        AirflowNetworkNodeSimu(i).WZ =
                            state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(ZoneNum)
                                .Node(AirflowNetworkNodeData(i).RAFNNodeNum)
                                .HumRat;
                    }
                }
            }
        }
    }

    for (auto &e : exchangeData) {
        e.TotalSen = 0.0;
        e.TotalLat = 0.0;
        e.MultiZoneSen = 0.0;
        e.MultiZoneLat = 0.0;
        e.LeakSen = 0.0;
        e.LeakLat = 0.0;
        e.CondSen = 0.0;
        e.DiffLat = 0.0;
        e.RadGain = 0.0;
    }
    if (state.dataContaminantBalance->Contaminant.CO2Simulation)
        for (auto &e : exchangeData)
            e.TotalCO2 = 0.0;
    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation)
        for (auto &e : exchangeData)
            e.TotalGC = 0.0;

    // Occupant ventilation control
    Real64 CurrentEndTime = state.dataGlobal->CurrentTime + state.dataHVACGlobal->SysTimeElapsed;
    if (CurrentEndTime > CurrentEndTimeLast &&
        TimeStepSys >= TimeStepSysLast) {
        for (i = 1; i <= AirflowNetworkNumOfSurfaces; ++i) {
            if (i > AirflowNetworkNumOfSurfaces - NumOfLinksIntraZone) continue;
            if (MultizoneSurfaceData(i).OccupantVentilationControlNum > 0) {
                MultizoneSurfaceData(i).PrevOpeningstatus = MultizoneSurfaceData(i).OpeningStatus;
                MultizoneSurfaceData(i).OpenFactorLast = MultizoneSurfaceData(i).OpenFactor;
                if (MultizoneSurfaceData(i).OpenFactor > 0.0) {
                    MultizoneSurfaceData(i).OpenElapsedTime +=
                        (CurrentEndTime - CurrentEndTimeLast) * 60.0;
                    MultizoneSurfaceData(i).CloseElapsedTime = 0.0;
                } else {
                    MultizoneSurfaceData(i).OpenElapsedTime = 0.0;
                    MultizoneSurfaceData(i).CloseElapsedTime +=
                        (CurrentEndTime - CurrentEndTimeLast) * 60.0;
                }
                j = MultizoneSurfaceData(i).SurfNum;
                state.afn
                    ->OccupantVentilationControl(MultizoneSurfaceData(i).OccupantVentilationControlNum)
                    .calc(state,
                          state.dataSurface->Surface(j).Zone,
                          MultizoneSurfaceData(i).OpenElapsedTime,
                          MultizoneSurfaceData(i).CloseElapsedTime,
                          MultizoneSurfaceData(i).OpeningStatus,
                          MultizoneSurfaceData(i).OpeningProbStatus,
                          MultizoneSurfaceData(i).ClosingProbStatus);
                if (MultizoneSurfaceData(i).OpeningStatus == AirflowNetwork::OpenStatus::MinCheckForceOpen) {
                    MultizoneSurfaceData(i).OpenFactor = MultizoneSurfaceData(i).OpenFactorLast;
                }
                if (MultizoneSurfaceData(i).OpeningStatus == AirflowNetwork::OpenStatus::MinCheckForceClose) {
                    MultizoneSurfaceData(i).OpenFactor = 0.0;
                }
            }
        }
    }
    TimeStepSysLast = TimeStepSys;
    CurrentEndTimeLast = CurrentEndTime;
}

    void AllocateAndInitData(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Aug. 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes variables and allocates dynamic arrays.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int ZoneNum;
        int n;
        int SurfNum;
        auto &Zone(state.dataHeatBal->Zone);

        state.afn->AirflowNetworkNodeSimu.allocate(
            state.afn->AirflowNetworkNumOfNodes); // Node simulation variable in air distribution system
        state.afn->AirflowNetworkLinkSimu.allocate(
            state.afn->AirflowNetworkNumOfLinks); // Link simulation variable in air distribution system
        state.afn->linkReport.allocate(
            state.afn->AirflowNetworkNumOfLinks); // Report link simulation variable in air distribution system

        for (i = 1; i <= state.afn->DisSysNumOfCVFs; i++) {
            if (state.afn->DisSysCompCVFData(i).FanTypeNum == FanType_SimpleOnOff) {
                state.afn->nodeReport.allocate(state.afn->AirflowNetworkNumOfZones);
                state.afn->linkReport1.allocate(state.afn->AirflowNetworkNumOfSurfaces);
                break;
            }
        }

        state.afn->MA.allocate(state.afn->AirflowNetworkNumOfNodes *
                                                            state.afn->AirflowNetworkNumOfNodes);
        state.afn->MV.allocate(state.afn->AirflowNetworkNumOfNodes);
        state.afn->IVEC.allocate(state.afn->AirflowNetworkNumOfNodes + 20);

        state.afn->AirflowNetworkReportData.allocate(state.dataGlobal->NumOfZones);          // Report variables
        state.afn->AirflowNetworkZnRpt.allocate(state.dataGlobal->NumOfZones); // Report variables

        state.afn->ANZT.allocate(state.dataGlobal->NumOfZones); // Local zone air temperature for rollback use
        state.afn->ANZW.allocate(state.dataGlobal->NumOfZones); // Local zone humidity ratio for rollback use
        if (state.dataContaminantBalance->Contaminant.CO2Simulation)
            state.afn->ANCO.allocate(state.dataGlobal->NumOfZones); // Local zone CO2 for rollback use
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation)
            state.afn->ANGC.allocate(state.dataGlobal->NumOfZones); // Local zone generic contaminant for rollback use
        auto &solver = state.afn;
        solver->allocate(state);

        bool OnOffFanFlag = false;
        for (i = 1; i <= state.afn->DisSysNumOfCVFs; i++) {
            if (state.afn->DisSysCompCVFData(i).FanTypeNum == FanType_SimpleOnOff) {
                OnOffFanFlag = true;
            }
        }

        // CurrentModuleObject='AirflowNetwork Simulations'
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            SetupOutputVariable(state,
                                "AFN Node Temperature",
                                OutputProcessor::Unit::C,
                                state.afn->AirflowNetworkNodeSimu(i).TZ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.afn->AirflowNetworkNodeData(i).Name);
            SetupOutputVariable(state,
                                "AFN Node Humidity Ratio",
                                OutputProcessor::Unit::kgWater_kgDryAir,
                                state.afn->AirflowNetworkNodeSimu(i).WZ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.afn->AirflowNetworkNodeData(i).Name);
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                SetupOutputVariable(state,
                                    "AFN Node CO2 Concentration",
                                    OutputProcessor::Unit::ppm,
                                    state.afn->AirflowNetworkNodeSimu(i).CO2Z,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->AirflowNetworkNodeData(i).Name);
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                SetupOutputVariable(state,
                                    "AFN Node Generic Air Contaminant Concentration",
                                    OutputProcessor::Unit::ppm,
                                    state.afn->AirflowNetworkNodeSimu(i).GCZ,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->AirflowNetworkNodeData(i).Name);
            }
            if (!(state.afn->SupplyFanType == FanType_SimpleOnOff &&
                  i <= state.afn->AirflowNetworkNumOfZones)) {
                SetupOutputVariable(state,
                                    "AFN Node Total Pressure",
                                    OutputProcessor::Unit::Pa,
                                    state.afn->AirflowNetworkNodeSimu(i).PZ,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->AirflowNetworkNodeData(i).Name);
            }
            if (state.afn->AirflowNetworkNodeData(i).ExtNodeNum > 0) {
                SetupOutputVariable(state,
                                    "AFN Node Wind Pressure",
                                    OutputProcessor::Unit::Pa,
                                    state.afn->AirflowNetworkNodeSimu(i).PZ,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->AirflowNetworkNodeData(i).Name);
            }
        }

        for (i = 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
            if (!(state.afn->SupplyFanType == FanType_SimpleOnOff &&
                  i <= state.afn->AirflowNetworkNumOfSurfaces)) {
                SetupOutputVariable(state,
                                    "AFN Linkage Node 1 to Node 2 Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    state.afn->linkReport(i).FLOW,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->AirflowNetworkLinkageData(i).Name);
                SetupOutputVariable(state,
                                    "AFN Linkage Node 2 to Node 1 Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    state.afn->linkReport(i).FLOW2,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->AirflowNetworkLinkageData(i).Name);
                SetupOutputVariable(state,
                                    "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    state.afn->linkReport(i).VolFLOW,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->AirflowNetworkLinkageData(i).Name);
                SetupOutputVariable(state,
                                    "AFN Linkage Node 2 to Node 1 Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    state.afn->linkReport(i).VolFLOW2,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->AirflowNetworkLinkageData(i).Name);
                SetupOutputVariable(state,
                                    "AFN Linkage Node 1 to Node 2 Pressure Difference",
                                    OutputProcessor::Unit::Pa,
                                    state.afn->AirflowNetworkLinkSimu(i).DP,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->AirflowNetworkLinkageData(i).Name);
            }
        }

        for (i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) {
            if (state.afn->AirflowNetworkLinkageData(i).element == nullptr) {
                // This is not great
                continue;
            }
            n = state.afn->AirflowNetworkLinkageData(i).CompNum;
            if (state.afn->AirflowNetworkLinkageData(i).element->type() == ComponentType::DOP ||
                state.afn->AirflowNetworkLinkageData(i).element->type() == ComponentType::SOP ||
                state.afn->AirflowNetworkLinkageData(i).element->type() == ComponentType::HOP) {
                SurfNum = state.afn->MultizoneSurfaceData(i).SurfNum;
                SetupOutputVariable(state,
                                    "AFN Surface Venting Window or Door Opening Factor",
                                    OutputProcessor::Unit::None,
                                    state.afn->MultizoneSurfaceData(i).OpenFactor,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->MultizoneSurfaceData(i).SurfName);
                if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                    SetupEMSActuator(state,
                                     "AirFlow Network Window/Door Opening",
                                     state.afn->MultizoneSurfaceData(i).SurfName,
                                     "Venting Opening Factor",
                                     "[Fraction]",
                                     state.afn->MultizoneSurfaceData(i).EMSOpenFactorActuated,
                                     state.afn->MultizoneSurfaceData(i).EMSOpenFactor);
                }
                SetupOutputVariable(state,
                                    "AFN Surface Venting Window or Door Opening Modulation Multiplier",
                                    OutputProcessor::Unit::None,
                                    state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum),
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataSurface->Surface(SurfNum).Name);
                SetupOutputVariable(state,
                                    "AFN Surface Venting Inside Setpoint Temperature",
                                    OutputProcessor::Unit::C,
                                    state.dataSurface->SurfWinInsideTempForVentingRep(SurfNum),
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataSurface->Surface(SurfNum).Name);
                SetupOutputVariable(state,
                                    "AFN Surface Venting Availability Status",
                                    OutputProcessor::Unit::None,
                                    state.dataSurface->SurfWinVentingAvailabilityRep(SurfNum),
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataSurface->Surface(SurfNum).Name);
                if (state.afn->MultizoneSurfaceData(i).OccupantVentilationControlNum > 0) {
                    SetupOutputVariable(state,
                                        "AFN Surface Venting Window or Door Opening Factor at Previous Time Step",
                                        OutputProcessor::Unit::None,
                                        state.afn->MultizoneSurfaceData(i).OpenFactorLast,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.afn->MultizoneSurfaceData(i).SurfName);
                    SetupOutputVariable(state,
                                        "AFN Surface Opening Elapsed Time",
                                        OutputProcessor::Unit::min,
                                        state.afn->MultizoneSurfaceData(i).OpenElapsedTime,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.afn->MultizoneSurfaceData(i).SurfName);
                    SetupOutputVariable(state,
                                        "AFN Surface Closing Elapsed Time",
                                        OutputProcessor::Unit::min,
                                        state.afn->MultizoneSurfaceData(i).CloseElapsedTime,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.afn->MultizoneSurfaceData(i).SurfName);
                    SetupOutputVariable(state,
                                        "AFN Surface Opening Status at Previous Time Step",
                                        OutputProcessor::Unit::None,
                                        state.afn->MultizoneSurfaceData(i).PrevOpeningstatus,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.afn->MultizoneSurfaceData(i).SurfName);
                    SetupOutputVariable(state,
                                        "AFN Surface Opening Status",
                                        OutputProcessor::Unit::None,
                                        state.afn->MultizoneSurfaceData(i).OpeningStatus,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.afn->MultizoneSurfaceData(i).SurfName);
                    SetupOutputVariable(state,
                                        "AFN Surface Opening Probability Status",
                                        OutputProcessor::Unit::None,
                                        state.afn->MultizoneSurfaceData(i).OpeningProbStatus,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.afn->MultizoneSurfaceData(i).SurfName);
                    SetupOutputVariable(state,
                                        "AFN Surface Closing Probability Status",
                                        OutputProcessor::Unit::None,
                                        state.afn->MultizoneSurfaceData(i).ClosingProbStatus,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.afn->MultizoneSurfaceData(i).SurfName);
                }
            }
        }

        for (i = 1; i <= state.dataGlobal->NumOfZones; ++i) {
            // Multizone losses due to force air systems
            SetupOutputVariable(state,
                                "AFN Zone Infiltration Sensible Heat Gain Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).MultiZoneInfiSenGainW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Infiltration Sensible Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).MultiZoneInfiSenGainJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Ventilation Sensible Heat Gain Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).MultiZoneVentSenGainW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Ventilation Sensible Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).MultiZoneVentSenGainJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Mixing Sensible Heat Gain Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).MultiZoneMixSenGainW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Mixing Sensible Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).MultiZoneMixSenGainJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Infiltration Sensible Heat Loss Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).MultiZoneInfiSenLossW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Infiltration Sensible Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).MultiZoneInfiSenLossJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Ventilation Sensible Heat Loss Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).MultiZoneVentSenLossW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Ventilation Sensible Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).MultiZoneVentSenLossJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Mixing Sensible Heat Loss Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).MultiZoneMixSenLossW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Mixing Sensible Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).MultiZoneMixSenLossJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Infiltration Latent Heat Gain Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).MultiZoneInfiLatGainW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Infiltration Latent Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).MultiZoneInfiLatGainJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Infiltration Latent Heat Loss Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).MultiZoneInfiLatLossW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Infiltration Latent Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).MultiZoneInfiLatLossJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Ventilation Latent Heat Gain Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).MultiZoneVentLatGainW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Ventilation Latent Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).MultiZoneVentLatGainJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Ventilation Latent Heat Loss Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).MultiZoneVentLatLossW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Ventilation Latent Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).MultiZoneVentLatLossJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Mixing Latent Heat Gain Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).MultiZoneMixLatGainW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Mixing Latent Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).MultiZoneMixLatGainJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Mixing Latent Heat Loss Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).MultiZoneMixLatLossW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Mixing Latent Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).MultiZoneInfiLatLossJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            // Supply leak losses due to force air systems
            SetupOutputVariable(state,
                                "AFN Zone Duct Leaked Air Sensible Heat Gain Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).LeakSenGainW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Leaked Air Sensible Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).LeakSenGainJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Leaked Air Sensible Heat Loss Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).LeakSenLossW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Leaked Air Sensible Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).LeakSenLossJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Leaked Air Latent Heat Gain Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).LeakLatGainW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Leaked Air Latent Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).LeakLatGainJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Leaked Air Latent Heat Loss Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).LeakLatLossW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Leaked Air Latent Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).LeakLatLossJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            // Conduction losses due to force air systems
            SetupOutputVariable(state,
                                "AFN Zone Duct Conduction Sensible Heat Gain Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).CondSenGainW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Conduction Sensible Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).CondSenGainJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Conduction Sensible Heat Loss Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).CondSenLossW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Conduction Sensible Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).CondSenLossJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Diffusion Latent Heat Gain Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).DiffLatGainW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Diffusion Latent Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).DiffLatGainJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Diffusion Latent Heat Loss Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).DiffLatLossW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Diffusion Latent Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).DiffLatLossJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            // Radiation losses due to forced air systems
            SetupOutputVariable(state,
                                "AFN Zone Duct Radiation Heat Gain Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).RadGainW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Radiation Sensible Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).RadGainJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Radiation Heat Loss Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).RadLossW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Duct Radiation Sensible Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).RadLossJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            // Total losses due to force air systems
            SetupOutputVariable(state,
                                "AFN Distribution Sensible Heat Gain Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).TotalSenGainW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Distribution Sensible Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).TotalSenGainJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Distribution Sensible Heat Loss Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).TotalSenLossW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Distribution Sensible Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).TotalSenLossJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Distribution Latent Heat Gain Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).TotalLatGainW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Distribution Latent Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).TotalLatGainJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Distribution Latent Heat Loss Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkReportData(i).TotalLatLossW,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Distribution Latent Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.afn->AirflowNetworkReportData(i).TotalLatLossJ,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
        }

        for (i = 1; i <= state.dataGlobal->NumOfZones; ++i) {
            SetupOutputVariable(state,
                                "AFN Zone Infiltration Volume",
                                OutputProcessor::Unit::m3,
                                state.afn->AirflowNetworkZnRpt(i).InfilVolume,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Infiltration Mass",
                                OutputProcessor::Unit::kg,
                                state.afn->AirflowNetworkZnRpt(i).InfilMass,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Infiltration Air Change Rate",
                                OutputProcessor::Unit::ach,
                                state.afn->AirflowNetworkZnRpt(i).InfilAirChangeRate,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Ventilation Volume",
                                OutputProcessor::Unit::m3,
                                state.afn->AirflowNetworkZnRpt(i).VentilVolume,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Ventilation Mass",
                                OutputProcessor::Unit::kg,
                                state.afn->AirflowNetworkZnRpt(i).VentilMass,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Ventilation Air Change Rate",
                                OutputProcessor::Unit::ach,
                                state.afn->AirflowNetworkZnRpt(i).VentilAirChangeRate,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Mixing Volume",
                                OutputProcessor::Unit::m3,
                                state.afn->AirflowNetworkZnRpt(i).MixVolume,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Mixing Mass",
                                OutputProcessor::Unit::kg,
                                state.afn->AirflowNetworkZnRpt(i).MixMass,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                Zone(i).Name);

            SetupOutputVariable(state,
                                "AFN Zone Exfiltration Heat Transfer Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkZnRpt(i).ExfilTotalLoss,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Exfiltration Sensible Heat Transfer Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkZnRpt(i).ExfilSensiLoss,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
            SetupOutputVariable(state,
                                "AFN Zone Exfiltration Latent Heat Transfer Rate",
                                OutputProcessor::Unit::W,
                                state.afn->AirflowNetworkZnRpt(i).ExfilLatentLoss,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                Zone(i).Name);
        }

        if (OnOffFanFlag) {
            for (i = 1; i <= state.afn->AirflowNetworkNumOfZones; ++i) {
                SetupOutputVariable(state,
                                    "AFN Zone Average Pressure",
                                    OutputProcessor::Unit::Pa,
                                    state.afn->nodeReport(i).PZ,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    Zone(i).Name);
                SetupOutputVariable(state,
                                    "AFN Zone On Cycle Pressure",
                                    OutputProcessor::Unit::Pa,
                                    state.afn->nodeReport(i).PZON,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    Zone(i).Name);
                SetupOutputVariable(state,
                                    "AFN Zone Off Cycle Pressure",
                                    OutputProcessor::Unit::Pa,
                                    state.afn->nodeReport(i).PZOFF,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    Zone(i).Name);
            }
            for (i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) {
                SetupOutputVariable(state,
                                    "AFN Linkage Node 1 to 2 Average Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    state.afn->linkReport1(i).FLOW,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->MultizoneSurfaceData(i).SurfName);
                SetupOutputVariable(state,
                                    "AFN Linkage Node 2 to 1 Average Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    state.afn->linkReport1(i).FLOW2,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->MultizoneSurfaceData(i).SurfName);
                SetupOutputVariable(state,
                                    "AFN Linkage Node 1 to 2 Average Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    state.afn->linkReport1(i).VolFLOW,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->MultizoneSurfaceData(i).SurfName);
                SetupOutputVariable(state,
                                    "AFN Linkage Node 2 to 1 Average Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    state.afn->linkReport1(i).VolFLOW2,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->MultizoneSurfaceData(i).SurfName);
                SetupOutputVariable(state,
                                    "AFN Surface Average Pressure Difference",
                                    OutputProcessor::Unit::Pa,
                                    state.afn->linkReport1(i).DP,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->MultizoneSurfaceData(i).SurfName);
                SetupOutputVariable(state,
                                    "AFN Surface On Cycle Pressure Difference",
                                    OutputProcessor::Unit::Pa,
                                    state.afn->linkReport1(i).DPON,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->MultizoneSurfaceData(i).SurfName);
                SetupOutputVariable(state,
                                    "AFN Surface Off Cycle Pressure Difference",
                                    OutputProcessor::Unit::Pa,
                                    state.afn->linkReport1(i).DPOFF,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.afn->MultizoneSurfaceData(i).SurfName);
            }
        }

        // Assign node reference height
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            if (!state.afn->AirflowNetworkSimu.TExtHeightDep) state.afn->AirflowNetworkNodeData(i).NodeHeight = 0.0;
            ZoneNum = state.afn->AirflowNetworkNodeData(i).EPlusZoneNum;
            if (ZoneNum > 0) {
                if (state.dataSurface->WorldCoordSystem) {
                    state.afn->AirflowNetworkNodeData(i).NodeHeight = 0.0;
                } else {
                    state.afn->AirflowNetworkNodeData(i).NodeHeight = Zone(ZoneNum).OriginZ;
                }
            }
        }
    }

    void CalcAirflowNetworkAirBalance(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs simulations of nodal pressures and linkage airflows.

        // Using/Aliasing
        using DataHVACGlobals::VerySmallMassFlow;
        using General::SolveRoot;

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr CycFanCycComp(1); // fan cycles with compressor operation

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int j;
        int n;
        int NodeNum;
        Real64 GlobalOpenFactor;
        Real64 ZonePressure1;
        Real64 ZonePressure2;
        Real64 PressureSet;
        Real64 LocalAzimuth;
        Real64 LocalWindSpeed;
        Real64 LocalWindDir;
        Real64 LocalHumRat;
        Real64 LocalDryBulb;
        Array1D<Real64> Par; // Pressure setpoint
        Real64 constexpr ErrorToler(0.00001);
        int constexpr MaxIte(20);
        int SolFla;
        Real64 MinExhaustMassFlowrate;
        Real64 MaxExhaustMassFlowrate;
        Real64 MinReliefMassFlowrate;
        Real64 MaxReliefMassFlowrate;
        int AirLoopNum;

        auto &Node(state.dataLoopNodes->Node);

        // Validate supply and return connections
        if (state.afn->CalcAirflowNetworkAirBalanceOneTimeFlag) {
            state.afn->CalcAirflowNetworkAirBalanceOneTimeFlag = false;
            if (state.afn->CalcAirflowNetworkAirBalanceErrorsFound) {
                ShowFatalError(state, "GetAirflowNetworkInput: Program terminates for preceding reason(s).");
            }
        }

        for (n = 1; n <= state.afn->ActualNumOfNodes; ++n) {
            if (state.afn->AirflowNetworkNodeData(n).NodeTypeNum == 0) {
                state.afn->AirflowNetworkNodeSimu(n).PZ = 0.0;
            } else {
                // Assigning ambient conditions to external nodes
                i = state.afn->AirflowNetworkNodeData(n).ExtNodeNum;
                if (i > 0) {
                    state.afn->AirflowNetworkNodeSimu(n).TZ =
                        OutDryBulbTempAt(state, state.afn->AirflowNetworkNodeData(n).NodeHeight);
                    state.afn->AirflowNetworkNodeSimu(n).WZ = state.dataEnvrn->OutHumRat;
                    if (i <= state.afn->AirflowNetworkNumOfExtNode) {
                        if (state.afn->MultizoneExternalNodeData(i).OutAirNodeNum == 0) {
                            LocalWindSpeed = DataEnvironment::WindSpeedAt(state, state.afn->MultizoneExternalNodeData(i).height);
                            LocalDryBulb = OutDryBulbTempAt(state, state.afn->AirflowNetworkNodeData(n).NodeHeight);
                            LocalAzimuth = state.afn->MultizoneExternalNodeData(i).azimuth;
                            state.afn->AirflowNetworkNodeSimu(n).PZ =
                                CalcWindPressure(state,
                                                 state.afn->MultizoneExternalNodeData(i).curve,
                                                 state.afn->MultizoneExternalNodeData(i).symmetricCurve,
                                                 state.afn->MultizoneExternalNodeData(i).useRelativeAngle,
                                                 LocalAzimuth,
                                                 LocalWindSpeed,
                                                 state.dataEnvrn->WindDir,
                                                 LocalDryBulb,
                                                 state.dataEnvrn->OutHumRat);
                        } else {
                            // If and outdoor air node object is defined as the External Node Name in AirflowNetwork:MultiZone:Surface,
                            // the node object requires to define the Wind Pressure Coefficient Curve Name.
                            NodeNum = state.afn->MultizoneExternalNodeData(i).OutAirNodeNum;
                            LocalWindSpeed = Node((NodeNum)).OutAirWindSpeed;
                            LocalWindDir = Node((NodeNum)).OutAirWindDir;
                            LocalHumRat = Node((NodeNum)).HumRat;
                            LocalDryBulb = Node((NodeNum)).OutAirDryBulb;
                            LocalAzimuth = state.afn->MultizoneExternalNodeData(i).azimuth;
                            state.afn->AirflowNetworkNodeSimu(n).PZ =
                                CalcWindPressure(state,
                                                 state.afn->MultizoneExternalNodeData(i).curve,
                                                 state.afn->MultizoneExternalNodeData(i).symmetricCurve,
                                                 state.afn->MultizoneExternalNodeData(i).useRelativeAngle,
                                                 LocalAzimuth,
                                                 LocalWindSpeed,
                                                 LocalWindDir,
                                                 LocalDryBulb,
                                                 LocalHumRat);
                            state.afn->AirflowNetworkNodeSimu(n).TZ = LocalDryBulb;
                            state.afn->AirflowNetworkNodeSimu(n).WZ = LocalHumRat;
                        }
                    }

                } else {
                    ShowSevereError(state,
                                    "GetAirflowNetworkInput: AIRFLOWNETWORK:DISTRIBUTION:NODE: Invalid external node = " +
                                        state.afn->AirflowNetworkNodeData(n).Name);
                    state.afn->CalcAirflowNetworkAirBalanceErrorsFound = true;
                }
            }
        }

        for (i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) {
            if (i > state.afn->AirflowNetworkNumOfSurfaces - state.afn->NumOfLinksIntraZone) {
                continue;
            }
            if (state.afn->AirflowNetworkLinkageData(i).element->type() == ComponentType::SCR) {
                state.afn->AirflowNetworkLinkageData(i).control = state.afn->MultizoneSurfaceData(i).Factor;
            }
            if (state.afn->MultizoneSurfaceData(i).OccupantVentilationControlNum == 0)
                state.afn->MultizoneSurfaceData(i).OpenFactor = 0.0;
            j = state.afn->MultizoneSurfaceData(i).SurfNum;
            if (state.dataSurface->SurfWinOriginalClass(j) == SurfaceClass::Window ||
                state.dataSurface->SurfWinOriginalClass(j) == SurfaceClass::Door ||
                state.dataSurface->SurfWinOriginalClass(j) == SurfaceClass::GlassDoor || state.dataSurface->Surface(j).IsAirBoundarySurf) {
                if (state.afn->MultizoneSurfaceData(i).OccupantVentilationControlNum > 0) {
                    if (state.afn->MultizoneSurfaceData(i).OpeningStatus == OpenStatus::FreeOperation) {
                        if (state.afn->MultizoneSurfaceData(i).OpeningProbStatus == ProbabilityCheck::ForceChange) {
                            state.afn->MultizoneSurfaceData(i).OpenFactor = state.afn->MultizoneSurfaceData(i).Factor;
                        } else if (state.afn->MultizoneSurfaceData(i).ClosingProbStatus == ProbabilityCheck::ForceChange) {
                            state.afn->MultizoneSurfaceData(i).OpenFactor = 0.0;
                        } else if (state.afn->MultizoneSurfaceData(i).ClosingProbStatus == ProbabilityCheck::KeepStatus ||
                                   state.afn->MultizoneSurfaceData(i).OpeningProbStatus == ProbabilityCheck::KeepStatus) {
                            state.afn->MultizoneSurfaceData(i).OpenFactor =
                                state.afn->MultizoneSurfaceData(i).OpenFactorLast;
                        } else {
                            AirflowNetworkVentingControl(state, i, state.afn->MultizoneSurfaceData(i).OpenFactor);
                        }
                    }
                } else {
                    AirflowNetworkVentingControl(state, i, state.afn->MultizoneSurfaceData(i).OpenFactor);
                }
                state.afn->MultizoneSurfaceData(i).OpenFactor *= state.afn->MultizoneSurfaceData(i).WindModifier;
                if (state.afn->MultizoneSurfaceData(i).HybridVentClose) {
                    state.afn->MultizoneSurfaceData(i).OpenFactor = 0.0;
                    if (state.dataSurface->SurfWinVentingOpenFactorMultRep(j) > 0.0) state.dataSurface->SurfWinVentingOpenFactorMultRep(j) = 0.0;
                }
                if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).CompTypeNum ==
                        iComponentTypeNum::DOP ||
                    state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).CompTypeNum ==
                        iComponentTypeNum::SOP ||
                    state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).CompTypeNum ==
                        iComponentTypeNum::HOP) {
                    if (state.afn->AirflowNetworkFanActivated &&
                        (state.afn->SimulateAirflowNetwork > AirflowNetworkControlMultizone) &&
                        state.afn->MultizoneSurfaceData(i).OpenFactor > 0.0 &&
                        (state.dataSurface->Surface(j).ExtBoundCond == ExternalEnvironment ||
                         (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtBoundCond ==
                              OtherSideCoefNoCalcExt &&
                          state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtWind)) &&
                        !state.dataGlobal->WarmupFlag) {
                        // Exterior Large opening only
                        ++state.afn->MultizoneSurfaceData(i).ExtLargeOpeningErrCount;
                        if (state.afn->MultizoneSurfaceData(i).ExtLargeOpeningErrCount < 2) {
                            ShowWarningError(state,
                                             "AirflowNetwork: The window or door is open during HVAC system operation " +
                                                 state.afn->MultizoneSurfaceData(i).SurfName);
                            ShowContinueError(
                                state,
                                format("The window or door opening factor is {:.2R}", state.afn->MultizoneSurfaceData(i).OpenFactor));
                            ShowContinueErrorTimeStamp(state, "");
                        } else {
                            ShowRecurringWarningErrorAtEnd(state,
                                                           "AirFlowNetwork: " + state.afn->MultizoneSurfaceData(i).SurfName +
                                                               " The window or door is open during HVAC system operation error continues...",
                                                           state.afn->MultizoneSurfaceData(i).ExtLargeOpeningErrIndex,
                                                           state.afn->MultizoneSurfaceData(i).OpenFactor,
                                                           state.afn->MultizoneSurfaceData(i).OpenFactor);
                        }
                    }
                }
                if (state.afn->MultizoneSurfaceData(i).OpenFactor > 1.0) {
                    ++state.afn->MultizoneSurfaceData(i).OpenFactorErrCount;
                    if (state.afn->MultizoneSurfaceData(i).OpenFactorErrCount < 2) {
                        ShowWarningError(state,
                                         "AirflowNetwork: The window or door opening factor is greater than 1.0 " +
                                             state.afn->MultizoneSurfaceData(i).SurfName);
                        ShowContinueErrorTimeStamp(state, "");
                    } else {
                        ShowRecurringWarningErrorAtEnd(state,
                                                       "AirFlowNetwork: " + state.afn->MultizoneSurfaceData(i).SurfName +
                                                           " The window or door opening factor is greater than 1.0 error continues...",
                                                       state.afn->MultizoneSurfaceData(i).OpenFactorErrIndex,
                                                       state.afn->MultizoneSurfaceData(i).OpenFactor,
                                                       state.afn->MultizoneSurfaceData(i).OpenFactor);
                    }
                }
            }
        }

        // Check if the global ventilation control is applied or not
        GlobalOpenFactor = -1.0;
        for (i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) {
            if (i > state.afn->AirflowNetworkNumOfSurfaces - state.afn->NumOfLinksIntraZone) continue;
            if (state.afn->MultizoneSurfaceData(i).HybridCtrlMaster) {
                GlobalOpenFactor = state.afn->MultizoneSurfaceData(i).OpenFactor;
                break;
            }
        }
        if (GlobalOpenFactor >= 0.0) {
            for (i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) {
                if (i > state.afn->AirflowNetworkNumOfSurfaces - state.afn->NumOfLinksIntraZone) continue;
                j = state.afn->MultizoneSurfaceData(i).SurfNum;
                if (state.dataSurface->SurfWinOriginalClass(j) == SurfaceClass::Window ||
                    state.dataSurface->SurfWinOriginalClass(j) == SurfaceClass::Door ||
                    state.dataSurface->SurfWinOriginalClass(j) == SurfaceClass::GlassDoor) {
                    if (state.afn->MultizoneSurfaceData(i).HybridCtrlGlobal) {
                        state.afn->MultizoneSurfaceData(i).OpenFactor = GlobalOpenFactor;
                    }
                }
            }
        }

        if (!Par.allocated()) {
            Par.allocate(1);
            Par = 0.0;
        }

        state.afn->PressureSetFlag = 0;

        if (state.afn->NumOfPressureControllers == 1) {
            if (state.afn->PressureControllerData(1).AvailSchedPtr == DataGlobalConstants::ScheduleAlwaysOn) {
                state.afn->PressureSetFlag = state.afn->PressureControllerData(1).ControlTypeSet;
            } else {
                if (GetCurrentScheduleValue(state, state.afn->PressureControllerData(1).AvailSchedPtr) > 0.0) {
                    state.afn->PressureSetFlag = state.afn->PressureControllerData(1).ControlTypeSet;
                }
            }
            if (state.afn->PressureSetFlag > 0) {
                PressureSet = GetCurrentScheduleValue(state, state.afn->PressureControllerData(1).PresSetpointSchedPtr);
            }
        }
        auto &solver = state.afn;
        solver->initialize_calculation();

        if (!(state.afn->PressureSetFlag > 0 && state.afn->AirflowNetworkFanActivated)) {
            solver->airmov(state);
        } else if (state.afn->PressureSetFlag == PressureCtrlExhaust) {
            AirLoopNum = state.afn->AirflowNetworkNodeData(state.afn->PressureControllerData(1).AFNNodeNum).AirLoopNum;
            MinExhaustMassFlowrate = 2.0 * VerySmallMassFlow;
            MaxExhaustMassFlowrate = Node(state.afn->PressureControllerData(1).OANodeNum).MassFlowRate;
            if (state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode == CycFanCycComp &&
                state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio > 0.0) {
                MaxExhaustMassFlowrate = MaxExhaustMassFlowrate / state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio;
            }
            state.afn->ExhaustFanMassFlowRate = MinExhaustMassFlowrate;
            solver->airmov(state);
            ZonePressure1 = state.afn->AirflowNetworkNodeSimu(state.afn->PressureControllerData(1).AFNNodeNum).PZ;
            if (ZonePressure1 <= PressureSet) {
                // The highest pressure due to minimum flow rate could not reach Pressure set, bypass pressure set calculation
                if (!state.dataGlobal->WarmupFlag) {
                    if (state.afn->ErrCountLowPre == 0) {
                        ++state.afn->ErrCountLowPre;
                        ShowWarningError(state,
                                         "The calculated pressure with minimum exhaust fan rate is lower than the pressure setpoint. The pressure "
                                         "control is unable to perform.");
                        ShowContinueErrorTimeStamp(state,
                                                   format("Calculated pressure = {:.2R}[Pa], Pressure setpoint ={:.2R}", ZonePressure1, PressureSet));
                    } else {
                        ++state.afn->ErrCountLowPre;
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            state.afn->AirflowNetworkNodeData(state.afn->PressureControllerData(1).AFNNodeNum).Name +
                                ": The AFN model continues not to perform pressure control due to lower zone pressure...",
                            state.afn->ErrIndexLowPre,
                            ZonePressure1,
                            ZonePressure1);
                    }
                }
            } else {
                state.afn->ExhaustFanMassFlowRate = MaxExhaustMassFlowrate;
                solver->airmov(state);
                ZonePressure2 = state.afn->AirflowNetworkNodeSimu(state.afn->PressureControllerData(1).AFNNodeNum).PZ;
                if (ZonePressure2 >= PressureSet) {
                    // The lowest pressure due to maximum flow rate is still higher than Pressure set, bypass pressure set calculation
                    if (!state.dataGlobal->WarmupFlag) {
                        if (state.afn->ErrCountHighPre == 0) {
                            ++state.afn->ErrCountHighPre;
                            ShowWarningError(state,
                                             "The calculated pressure with maximum exhaust fan rate is higher than the pressure setpoint. The "
                                             "pressure control is unable to perform.");
                            ShowContinueErrorTimeStamp(
                                state, format("Calculated pressure = {:.2R}[Pa], Pressure setpoint = {:.2R}", ZonePressure2, PressureSet));
                        } else {
                            ++state.afn->ErrCountHighPre;
                            ShowRecurringWarningErrorAtEnd(
                                state,
                                state.afn->AirflowNetworkNodeData(state.afn->PressureControllerData(1).AFNNodeNum)
                                        .Name +
                                    ": The AFN model continues not to perform pressure control due to higher zone pressure...",
                                state.afn->ErrIndexHighPre,
                                ZonePressure2,
                                ZonePressure2);
                        }
                    }
                } else {
                    //    if ( ZonePressure1 > PressureSet && ZonePressure2 < PressureSet ) {
                    Par(1) = PressureSet;
                    General::SolveRoot(state,
                                       ErrorToler,
                                       MaxIte,
                                       SolFla,
                                       state.afn->ExhaustFanMassFlowRate,
                                       AFNPressureResidual,
                                       MinExhaustMassFlowrate,
                                       MaxExhaustMassFlowrate,
                                       Par);
                    if (SolFla == -1) {
                        if (!state.dataGlobal->WarmupFlag) {
                            if (state.afn->ErrCountVar == 0) {
                                ++state.afn->ErrCountVar;
                                ShowWarningError(state, "Iteration limit exceeded pressure setpoint using an exhaust fan. Simulation continues.");
                                ShowContinueErrorTimeStamp(
                                    state, format("Exhaust fan flow rate = {:.4R}", state.afn->ExhaustFanMassFlowRate));
                            } else {
                                ++state.afn->ErrCountVar;
                                ShowRecurringWarningErrorAtEnd(state,
                                                               state.afn->PressureControllerData(1).Name +
                                                                   "\": Iteration limit warning exceeding pressure setpoint continues...",
                                                               state.afn->ErrIndexVar,
                                                               state.afn->ExhaustFanMassFlowRate,
                                                               state.afn->ExhaustFanMassFlowRate);
                            }
                        }
                    } else if (SolFla == -2) {
                        ShowFatalError(state,
                                       "Zone pressure control failed using an exhaust fan: no solution is reached, for " +
                                           state.afn->PressureControllerData(1).Name);
                    }
                }
            }
        } else { // PressureCtrlRelief - Pressure control type is Relief Flow
            AirLoopNum = state.afn->AirflowNetworkNodeData(state.afn->PressureControllerData(1).AFNNodeNum).AirLoopNum;
            MinReliefMassFlowrate = 2.0 * VerySmallMassFlow;
            MaxReliefMassFlowrate = Node(state.afn->PressureControllerData(1).OANodeNum).MassFlowRate;
            if (state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode == CycFanCycComp &&
                state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio > 0.0) {
                MaxReliefMassFlowrate = MaxReliefMassFlowrate / state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio;
            }
            state.afn->ReliefMassFlowRate = MinReliefMassFlowrate;
            solver->initialize_calculation();
            solver->airmov(state);
            ZonePressure1 = state.afn->AirflowNetworkNodeSimu(state.afn->PressureControllerData(1).AFNNodeNum).PZ;

            if (ZonePressure1 <= PressureSet) {
                // The highest pressure due to minimum flow rate could not reach Pressure set, bypass pressure set calculation
                if (!state.dataGlobal->WarmupFlag) {
                    if (state.afn->ErrCountLowPre == 0) {
                        ++state.afn->ErrCountLowPre;
                        ShowWarningError(state,
                                         "The calculated pressure with minimum relief air rate is lower than the pressure setpoint. The pressure "
                                         "control is unable to perform.");
                        ShowContinueErrorTimeStamp(state,
                                                   format("Calculated pressure = {:.2R}[Pa], Pressure setpoint ={:.2R}", ZonePressure1, PressureSet));
                    } else {
                        ++state.afn->ErrCountLowPre;
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            state.afn->AirflowNetworkNodeData(state.afn->PressureControllerData(1).AFNNodeNum).Name +
                                ": The AFN model continues not to perform pressure control due to lower zone pressure...",
                            state.afn->ErrIndexLowPre,
                            ZonePressure1,
                            ZonePressure1);
                    }
                }
            } else {
                state.afn->ReliefMassFlowRate = MaxReliefMassFlowrate;
                solver->initialize_calculation();
                solver->airmov(state);
                ZonePressure2 = state.afn->AirflowNetworkNodeSimu(state.afn->PressureControllerData(1).AFNNodeNum).PZ;
                if (ZonePressure2 >= PressureSet) {
                    // The lowest pressure due to maximum flow rate is still higher than Pressure set, bypass pressure set calculation
                    if (!state.dataGlobal->WarmupFlag) {
                        if (state.afn->ErrCountHighPre == 0) {
                            ++state.afn->ErrCountHighPre;
                            ShowWarningError(state,
                                             "The calculated pressure with maximum relief air rate is higher than the pressure setpoint. The "
                                             "pressure control is unable to perform.");
                            ShowContinueErrorTimeStamp(
                                state, format("Calculated pressure = {:.2R}[Pa], Pressure setpoint = {:.2R}", ZonePressure2, PressureSet));
                        } else {
                            ++state.afn->ErrCountHighPre;
                            ShowRecurringWarningErrorAtEnd(
                                state,
                                state.afn->AirflowNetworkNodeData(state.afn->PressureControllerData(1).AFNNodeNum)
                                        .Name +
                                    ": The AFN model continues not to perform pressure control due to higher zone pressure...",
                                state.afn->ErrIndexHighPre,
                                ZonePressure2,
                                ZonePressure2);
                        }
                    }
                } else {
                    //    if ( ZonePressure1 > PressureSet && ZonePressure2 < PressureSet ) {
                    Par(1) = PressureSet;
                    General::SolveRoot(state,
                                       ErrorToler,
                                       MaxIte,
                                       SolFla,
                                       state.afn->ReliefMassFlowRate,
                                       AFNPressureResidual,
                                       MinReliefMassFlowrate,
                                       MaxReliefMassFlowrate,
                                       Par);
                    if (SolFla == -1) {
                        if (!state.dataGlobal->WarmupFlag) {
                            if (state.afn->ErrCountVar == 0) {
                                ++state.afn->ErrCountVar;
                                ShowWarningError(state, "Iteration limit exceeded pressure setpoint using relief air. Simulation continues.");
                                ShowContinueErrorTimeStamp(state,
                                                           format("Relief air flow rate = {:.4R}", state.afn->ReliefMassFlowRate));
                            } else {
                                ++state.afn->ErrCountVar;
                                ShowRecurringWarningErrorAtEnd(state,
                                                               state.afn->PressureControllerData(1).Name +
                                                                   "\": Iteration limit warning exceeding pressure setpoint continues...",
                                                               state.afn->ErrIndexVar,
                                                               state.afn->ReliefMassFlowRate,
                                                               state.afn->ReliefMassFlowRate);
                            }
                        }
                    } else if (SolFla == -2) {
                        ShowFatalError(state,
                                       "Zone pressure control failed using relief air: no solution is reached, for " +
                                           state.afn->PressureControllerData(1).Name);
                    }
                }
            }
        }
    }

    Real64 AFNPressureResidual(EnergyPlusData &state,
                               Real64 const ControllerMassFlowRate, // Pressure setpoint
                               Array1D<Real64> const &Par           // par(1) = PressureSet
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   April 2016
        //       MODIFIED       NA
        //       RE-ENGINEERED  NA

        // PURPOSE OF THIS FUNCTION:
        //  Calculates residual function ((ZonePressure - PressureSet)/PressureSet)

        // METHODOLOGY EMPLOYED:
        //  Calls AIRMOV to get the pressure in the controlled zone and calculates the residual as defined above

        // Return value
        Real64 AFNPressureResidual;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 PressureSet;
        Real64 ZonePressure;

        PressureSet = Par(1);

        if (state.afn->PressureSetFlag == PressureCtrlExhaust) {
            state.afn->ExhaustFanMassFlowRate = ControllerMassFlowRate;
        }

        if (state.afn->PressureSetFlag == PressureCtrlRelief) {
            state.afn->ReliefMassFlowRate = ControllerMassFlowRate;
        }
        auto &solver = state.afn;
        solver->initialize_calculation();
        solver->airmov(state);

        ZonePressure = state.afn->AirflowNetworkNodeSimu(state.afn->PressureControllerData(1).AFNNodeNum).PZ;

        if (PressureSet != 0.0) {
            AFNPressureResidual = (ZonePressure - PressureSet) / PressureSet;
        } else {
            AFNPressureResidual = (ZonePressure - PressureSet);
        }
        return AFNPressureResidual;
    }

    static int makeTable(EnergyPlusData &state, const std::string &name, const int gridIndex, const std::vector<Real64> &y)
    {
        // Add a new table and performance curve
        std::string contextString = "CalcWindPressureCoeffs: Creating table \"" + name + "\"";
        std::pair<EnergyPlusData *, std::string> callbackPair{&state, contextString};
        Btwxt::setMessageCallback(CurveManager::BtwxtMessageCallback, &callbackPair);

        int CurveNum = state.dataCurveManager->PerfCurve.size() + 1;
        state.dataCurveManager->PerfCurve.push_back(CurveManager::PerformanceCurveData());

        state.dataCurveManager->PerfCurve(CurveNum).Name = name;
        state.dataCurveManager->PerfCurve(CurveNum).ObjectType = "Table:Lookup";
        state.dataCurveManager->PerfCurve(CurveNum).NumDims = 1;

        state.dataCurveManager->PerfCurve(CurveNum).InterpolationType = CurveManager::InterpType::BtwxtMethod;

        state.dataCurveManager->PerfCurve(CurveNum).Var1Min = 0.0;
        state.dataCurveManager->PerfCurve(CurveNum).Var1MinPresent = true;
        state.dataCurveManager->PerfCurve(CurveNum).Var1Max = 360.0;
        state.dataCurveManager->PerfCurve(CurveNum).Var1MaxPresent = true;

        state.dataCurveManager->PerfCurve(CurveNum).TableIndex = gridIndex;
        state.dataCurveManager->PerfCurve(CurveNum).GridValueIndex = state.dataCurveManager->btwxtManager.addOutputValues(gridIndex, y);

        state.dataCurveManager->NumCurves += 1;
        return CurveNum;
    }

void AirflowNetworkSolverData::calculateWindPressureCoeffs(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   May 2003
    //       MODIFIED       Revised by L. Gu, Nov. 2005, to meet requirements of AirflowNetwork
    //       MODIFIED       Revised by L. Gu, Dec. 2008, to set the number of external nodes based on
    //                      the number of external surfaces
    //       MODIFIED       Revised by J. DeGraw, Feb. 2017, to use tables
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates surface-average wind pressure coefficients for
    // the walls and roof of a rectangular building.

    // METHODOLOGY EMPLOYED:
    // Interpolates correlations between surface-average wind pressure coefficient and wind direction based on
    // measurements (see REFERENCES). Applicable only to rectangular buildings.

    // REFERENCES:
    // For low-rise buildings: M.V. Swami and S. Chandra, Correlations for Pressure Distribution
    // on Buildings and Calculation of Natural-Ventilation Airflow. ASHRAE Transactions 94 (1): 243-266.
    // For high-rise buildings: 2001 ASHRAE Fundamentals Handbook, p. 16.5, Fig. 7, "Surface Averaged
    // Wall Pressure Coefficients for Tall Buildings" and p.16.6, Fig. 9, "Surface Averaged Roof Pressure
    // Coefficients for Tall Buildings; from R.E. Akins, J.A. Peterka, and J.E. Cermak. 1979.
    // Averaged Pressure Coefficients for Rectangular Buildings. Wind Engineering. Proc. Fifth
    // International Conference 7:369-80, Fort Collins, CO. Pergamon Press, NY.

    // Using/Aliasing
    using namespace DataSurfaces;

    //  index 1 is wind incidence angle (0,30,60,...,300,330 deg)
    //  index 2 is side ratio (0.25,1.0,4.0)
    // Surface-averaged wind-pressure coefficient array for walls
    static constexpr std::array<std::array<Real64, 12>, 3> CPHighRiseWall = {{
        {0.60, 0.54, 0.23, -0.25, -0.61, -0.55, -0.51, -0.55, -0.61, -0.25, 0.23, 0.54},
        {0.60, 0.48, 0.04, -0.56, -0.56, -0.42, -0.37, -0.42, -0.56, -0.56, 0.04, 0.48},
        {0.60, 0.44, -0.26, -0.70, -0.53, -0.32, -0.22, -0.32, -0.53, -0.70, -0.26, 0.44},
    }};

    //  index 1 is wind incidence angle (0,30,60,...,300,330 deg)
    //  index 2 is side ratio (0.25,0.5,1.0)
    // Surface-averaged wind-pressure coefficient array for roof
    static constexpr std::array<std::array<Real64, 12>, 3> CPHighRiseRoof = {{
        {-0.28, -0.69, -0.72, -0.76, -0.72, -0.69, -0.28, -0.69, -0.72, -0.76, -0.72, -0.69},
        {-0.47, -0.52, -0.70, -0.76, -0.70, -0.52, -0.47, -0.52, -0.70, -0.76, -0.70, -0.52},
        {-0.70, -0.55, -0.55, -0.70, -0.55, -0.55, -0.70, -0.55, -0.55, -0.70, -0.55, -0.55},
    }};

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int FacadeNum;         // Facade number
    int ExtNum;            // External number
    int AFNZnNum;          // Zone number
    Real64 SideRatio;      // For vertical facades, width of facade / width of adjacent facade
    Real64 SR;             // SideRatio restricted to 0.25 to 4.0 range
    Real64 SideRatioFac;   // LOG(SideRatio)
    Real64 IncRad;         // IncAng in radians
    int IAng;              // Incidence angle index; used in interpolation
    Real64 DelAng;         // Incidence angle difference; used in interpolation
    Real64 WtAng;          // Incidence angle weighting factor; used in interpolation
    int ISR;               // Side ratio index, for interpolation
    Real64 WtSR;           // Side ratio weighting factor; used in interpolation
    int SurfNum;           // Surface number
    int SurfDatNum;        // Surface data number
    Real64 SurfAng;        // Azimuth angle of surface normal (degrees clockwise from North)
    int FacadeNumThisSurf; // Facade number for a particular surface
    Real64 AngDiff;        // Angle difference between wind and surface direction (deg)
    Real64 AngDiffMin;     // Minimum angle difference between wind and surface direction (deg)
    std::string Name;      // External node name
    std::vector<int> curveIndex = {0, 0, 0, 0, 0};

    // Facade azimuth angle
    for (FacadeNum = 1; FacadeNum <= 4; ++FacadeNum) {
        FacadeAng(FacadeNum) = state.afn->AirflowNetworkSimu.Azimuth + (FacadeNum - 1) * 90.0;
        if (FacadeAng(FacadeNum) >= 360.0) {
            FacadeAng(FacadeNum) -= 360.0;
        }
    }

    FacadeAng(5) = state.afn->AirflowNetworkSimu.Azimuth + 90.0;

    // Create AirflowNetwork external node objects -- one for each of the external surfaces

    state.afn->MultizoneExternalNodeData.allocate(AirflowNetworkNumOfExtSurfaces);
    AirflowNetworkNumOfExtNode = AirflowNetworkNumOfExtSurfaces;
    NumOfExtNodes = AirflowNetworkNumOfExtSurfaces;
    for (ExtNum = 1; ExtNum <= NumOfExtNodes; ++ExtNum) {
        state.afn->MultizoneExternalNodeData(ExtNum).ExtNum = state.afn->AirflowNetworkNumOfZones + ExtNum;
        state.afn->MultizoneExternalNodeData(ExtNum).Name = format("ExtNode{:4}", ExtNum);
    }

    // Associate each external node with SurfaceData

    ExtNum = 0;
    for (SurfDatNum = 1; SurfDatNum <= state.afn->AirflowNetworkNumOfSurfaces; ++SurfDatNum) {
        if (SurfDatNum > state.afn->AirflowNetworkNumOfSurfaces - state.afn->NumOfLinksIntraZone) {
            continue;
        }
        SurfNum = state.afn->MultizoneSurfaceData(SurfDatNum).SurfNum;
        if (SurfNum == 0) {
            continue; // Error caught earlier
        }
        if (state.dataSurface->Surface(SurfNum).ExtBoundCond == ExternalEnvironment ||
            (state.dataSurface->Surface(SurfNum).ExtBoundCond == OtherSideCoefNoCalcExt && state.dataSurface->Surface(SurfNum).ExtWind)) {
            ++ExtNum;
            if (state.dataSurface->Surface(SurfNum).Tilt >= 45.0) { // "Vertical" surface
                SurfAng = state.dataSurface->Surface(SurfNum).Azimuth;
                FacadeNumThisSurf = 1;
                AngDiffMin = std::abs(SurfAng - FacadeAng(1));
                if (AngDiffMin > 359.0) {
                    AngDiffMin = std::abs(AngDiffMin - 360.0);
                }
                for (FacadeNum = 2; FacadeNum <= 4; ++FacadeNum) {
                    AngDiff = std::abs(SurfAng - FacadeAng(FacadeNum));
                    if (AngDiff > 359.0) {
                        AngDiff = std::abs(AngDiff - 360.0);
                    }
                    if (AngDiff < AngDiffMin) {
                        AngDiffMin = AngDiff;
                        FacadeNumThisSurf = FacadeNum;
                    }
                }
                state.afn->MultizoneExternalNodeData(ExtNum).facadeNum = FacadeNumThisSurf;
            } else { // "Roof" surface
                state.afn->MultizoneExternalNodeData(ExtNum).facadeNum = 5;
            }
            state.afn->MultizoneSurfaceData(SurfDatNum).NodeNums[1] =
                state.afn->MultizoneExternalNodeData(ExtNum).ExtNum;
            state.afn->MultizoneSurfaceData(SurfDatNum).ExternalNodeName =
                state.afn->MultizoneExternalNodeData(ExtNum).Name;
        }
    }

    // Check if using the advanced single sided model
    for (AFNZnNum = 1; AFNZnNum <= state.afn->AirflowNetworkNumOfZones; ++AFNZnNum) {
        if (state.afn->MultizoneZoneData(AFNZnNum).SingleSidedCpType == "ADVANCED") {
            ++AirflowNetworkNumOfSingleSideZones;
        }
    }

    std::vector<Real64> dirs30 = {0, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330, 360};
    std::vector<Btwxt::GridAxis> dirs30Axes;
    dirs30Axes.emplace_back(dirs30, Btwxt::Method::LINEAR, Btwxt::Method::LINEAR, std::pair<double, double>{0.0, 360.0});

    auto dirs30GridIndex = state.dataCurveManager->btwxtManager.addGrid("30 Degree Increments", Btwxt::GriddedData(dirs30Axes));

    if (AirflowNetworkNumOfSingleSideZones == 0) { // do the standard surface average coefficient calculation
        // Create the array of wind directions

        // Create a curve for each facade
        for (FacadeNum = 1; FacadeNum <= 5; ++FacadeNum) {
            if (FacadeNum == 1 || FacadeNum == 3 || FacadeNum == 5) {
                SideRatio = state.afn->AirflowNetworkSimu.AspectRatio;
            } else { // FacadeNum = 2 or 4
                SideRatio = 1.0 / state.afn->AirflowNetworkSimu.AspectRatio;
            }
            if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.BldgType, "HighRise") && FacadeNum != 5) {
                SideRatio = 1.0 / SideRatio;
            }
            SideRatioFac = std::log(SideRatio);
            std::vector<Real64> vals(13);
            for (int windDirNum = 1; windDirNum <= 12; ++windDirNum) {
                Real64 WindAng = (windDirNum - 1) * 30.0;
                IncAng = std::abs(WindAng - FacadeAng(FacadeNum));
                if (IncAng > 180.0) IncAng = 360.0 - IncAng;
                IAng = int(IncAng / 30.0) + 1;
                DelAng = mod(IncAng, 30.0);
                WtAng = 1.0 - DelAng / 30.0;

                // Wind-pressure coefficients for vertical facades, low-rise building

                if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.BldgType, "LowRise") && FacadeNum <= 4) {
                    IncRad = IncAng * DataGlobalConstants::DegToRadians;
                    Real64 const cos_IncRad_over_2(std::cos(IncRad / 2.0));
                    vals[windDirNum - 1] = 0.6 * std::log(1.248 - 0.703 * std::sin(IncRad / 2.0) - 1.175 * pow_2(std::sin(IncRad)) +
                                                          0.131 * pow_3(std::sin(2.0 * IncRad * SideRatioFac)) + 0.769 * cos_IncRad_over_2 +
                                                          0.07 * pow_2(SideRatioFac * std::sin(IncRad / 2.0)) + 0.717 * pow_2(cos_IncRad_over_2));
                }

                // Wind-pressure coefficients for vertical facades, high-rise building

                else if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.BldgType, "HighRise") && FacadeNum <= 4) {
                    SR = min(max(SideRatio, 0.25), 4.0);
                    if (SR >= 0.25 && SR < 1.0) {
                        ISR = 1;
                        WtSR = (1.0 - SR) / 0.75;
                    } else { // 1.0 <= SR <= 4.0
                        ISR = 2;
                        WtSR = (4.0 - SR) / 3.0;
                    }
                    vals[windDirNum - 1] = WtSR * (WtAng * CPHighRiseWall[ISR - 1][IAng - 1] + (1.0 - WtAng) * CPHighRiseWall[ISR - 1][IAng]) +
                                           (1.0 - WtSR) * (WtAng * CPHighRiseWall[ISR][IAng - 1] + (1.0 - WtAng) * CPHighRiseWall[ISR][IAng]);
                }

                // Wind-pressure coefficients for roof (assumed same for low-rise and high-rise buildings)

                else if ((UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.BldgType, "HighRise") ||
                          UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.BldgType, "LowRise")) &&
                         FacadeNum == 5) {
                    SR = min(max(SideRatio, 0.25), 1.0);
                    if (SR >= 0.25 && SR < 0.5) {
                        ISR = 1;
                        WtSR = (0.5 - SR) / 0.25;
                    } else { // 0.5 <= SR <= 1.0
                        ISR = 2;
                        WtSR = (1.0 - SR) / 0.5;
                    }
                    vals[windDirNum - 1] = WtSR * (WtAng * CPHighRiseRoof[ISR - 1][IAng - 1] + (1.0 - WtAng) * CPHighRiseRoof[ISR - 1][IAng]) +
                                           (1.0 - WtSR) * (WtAng * CPHighRiseRoof[ISR][IAng - 1] + (1.0 - WtAng) * CPHighRiseRoof[ISR][IAng]);
                }

            } // End of wind direction loop
            // Add new table
            vals[12] = vals[0]; // Enforce periodicity
            curveIndex[FacadeNum - 1] = AirflowNetwork::makeTable(state, format("!WPCTABLE{}", FacadeNum), dirs30GridIndex, vals);
        } // End of facade number loop

    } else { //-calculate the advanced single sided wind pressure coefficients

        // Calculate the wind pressure coefficients vs. wind direction for each external node
        // The wind pressure coeffients are stored temporarily in the "valsByFacade" vector and then
        // converted into a table near the end of this else. There will be at least seven profiles
        // (four sides plus one roof plus two for each pair of windows). The name is thus a little
        // misleading, as it isn't really the values by facade once you get beyond the first five.
        std::vector<std::vector<Real64>> valsByFacade(5);
        for (FacadeNum = 0; FacadeNum < 4; ++FacadeNum) {
            valsByFacade[FacadeNum] = std::vector<Real64>(36);
        }
        FacadeNum = 4;
        valsByFacade[FacadeNum] = std::vector<Real64>(12);
        for (FacadeNum = 1; FacadeNum <= 4; ++FacadeNum) {
            if (FacadeNum == 1 || FacadeNum == 3) {
                SideRatio = state.afn->AirflowNetworkSimu.AspectRatio;
            } else { // FacadeNum = 2 or 4
                SideRatio = 1.0 / state.afn->AirflowNetworkSimu.AspectRatio;
            }
            if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.BldgType, "HighRise") && FacadeNum != 5) {
                SideRatio = 1.0 / SideRatio;
            }
            SideRatioFac = std::log(SideRatio);
            for (int windDirNum = 1; windDirNum <= 36; ++windDirNum) {
                Real64 WindAng = (windDirNum - 1) * 10.0;
                IncAng = std::abs(WindAng - FacadeAng(FacadeNum));
                if (IncAng > 180.0) IncAng = 360.0 - IncAng;
                IAng = int(IncAng / 10.0) + 1;
                DelAng = mod(IncAng, 10.0);
                WtAng = 1.0 - DelAng / 10.0;
                // Wind-pressure coefficients for vertical facades, low-rise building
                IncRad = IncAng * DataGlobalConstants::DegToRadians;
                valsByFacade[FacadeNum - 1][windDirNum - 1] =
                    0.6 * std::log(1.248 - 0.703 * std::sin(IncRad / 2.0) - 1.175 * pow_2(std::sin(IncRad)) +
                                   0.131 * pow_3(std::sin(2.0 * IncRad * SideRatioFac)) + 0.769 * std::cos(IncRad / 2.0) +
                                   0.07 * pow_2(SideRatioFac * std::sin(IncRad / 2.0)) + 0.717 * pow_2(std::cos(IncRad / 2.0)));
            } // End of wind direction loop
        }     // End of facade number loop
        // Add a roof
        FacadeNum = 5;
        SR = min(max(SideRatio, 0.25), 1.0);
        if (SR >= 0.25 && SR < 0.5) {
            ISR = 1;
            WtSR = (0.5 - SR) / 0.25;
        } else { // 0.5 <= SR <= 1.0
            ISR = 2;
            WtSR = (1.0 - SR) / 0.5;
        }
        for (int windDirNum = 1; windDirNum <= 12; ++windDirNum) {
            Real64 WindAng = (windDirNum - 1) * 30.0;
            IncAng = std::abs(WindAng - FacadeAng(FacadeNum));
            if (IncAng > 180.0) IncAng = 360.0 - IncAng;
            IAng = int(IncAng / 30.0) + 1;
            DelAng = mod(IncAng, 30.0);
            WtAng = 1.0 - DelAng / 30.0;
            // Wind-pressure coefficients for roof (assumed same for low-rise and high-rise buildings)
            valsByFacade[FacadeNum - 1][windDirNum - 1] =
                WtSR * (WtAng * CPHighRiseRoof[ISR - 1][IAng - 1] + (1.0 - WtAng) * CPHighRiseRoof[ISR - 1][IAng]) +
                (1.0 - WtSR) * (WtAng * CPHighRiseRoof[ISR][IAng - 1] + (1.0 - WtAng) * CPHighRiseRoof[ISR][IAng]);
        }
        AirflowNetwork::CalcSingleSidedCps(state,
                                                         valsByFacade); // run the advanced single sided subroutine if at least one zone calls for it
        // Resize the curve index array
        curveIndex.resize(valsByFacade.size());
        // Create the curves

        std::vector<Real64> dirs10 = {0,   10,  20,  30,  40,  50,  60,  70,  80,  90,  100, 110, 120, 130, 140, 150, 160, 170, 180,
                                      190, 200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 300, 310, 320, 330, 340, 350, 360};

        std::vector<Btwxt::GridAxis> dirs10Axes;
        dirs10Axes.emplace_back(dirs10, Btwxt::Method::LINEAR, Btwxt::Method::LINEAR, std::pair<double, double>{0.0, 360.0});

        auto dirs10GridIndex = state.dataCurveManager->btwxtManager.addGrid("10 Degree Increments", Btwxt::GriddedData(dirs10Axes));

        for (FacadeNum = 1; FacadeNum <= 4; ++FacadeNum) {
            valsByFacade[FacadeNum - 1].push_back(valsByFacade[FacadeNum - 1][0]); // Enforce periodicity
            curveIndex[FacadeNum - 1] = AirflowNetwork::makeTable(
                state, format("!SSWPCTABLEFACADE{}", FacadeNum), dirs10GridIndex, valsByFacade[FacadeNum - 1]);
        }
        FacadeNum = 5;
        valsByFacade[FacadeNum - 1].push_back(valsByFacade[FacadeNum - 1][0]); // Enforce periodicity
        curveIndex[FacadeNum - 1] =
            AirflowNetwork::makeTable(state, format("!SSWPCTABLEFACADE{}", FacadeNum), dirs30GridIndex, valsByFacade[FacadeNum - 1]);
        for (unsigned facadeNum = 6; facadeNum <= valsByFacade.size(); ++facadeNum) {
            valsByFacade[facadeNum - 1].push_back(valsByFacade[facadeNum - 1][0]); // Enforce periodicity
            curveIndex[facadeNum - 1] =
                AirflowNetwork::makeTable(state, format("!SSWPCTABLE{}", facadeNum), dirs10GridIndex, valsByFacade[facadeNum - 1]);
        }
    }
    // Connect the external nodes to the new curves
    for (ExtNum = 1; ExtNum <= NumOfExtNodes; ++ExtNum) {
        state.afn->MultizoneExternalNodeData(ExtNum).curve =
            curveIndex[state.afn->MultizoneExternalNodeData(ExtNum).facadeNum - 1];
    }
}

    Real64 CalcWindPressure(EnergyPlusData &state,
                            int const curve,           // Curve index, change this to pointer after curve refactor
                            bool const symmetricCurve, // True if the curve is symmetric (0 to 180)
                            bool const relativeAngle,  // True if the Cp curve angle is measured relative to the surface
                            Real64 const azimuth,      // Azimuthal angle of surface
                            Real64 const windSpeed,    // Wind velocity
                            Real64 const windDir,      // Wind direction
                            Real64 const dryBulbTemp,  // Air node dry bulb temperature
                            Real64 const humRat        // Air node humidity ratio
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       Jason DeGraw, Feb. 2017, modify to use curves
        //       MODIFIED       Xuan Luo, Aug. 2017, modify to use local air condition
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates surface wind pressure based on given CP values

        // REFERENCES:
        // COMIS Fundamentals

        // Return value is wind pressure[Pa]

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 angle(windDir);
        Real64 rho; // Outdoor air density
        Real64 Cp;  // Cp value at given wind direction

        // Calculate outdoor density
        rho = PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, dryBulbTemp, humRat);

        // Calculate pressure coefficient
        if (relativeAngle) {
            angle = angle - azimuth;
            if (angle < 0.0) {
                angle += 360.0;
            }
        }
        if (symmetricCurve) {
            if (angle > 180.0) {
                angle = 360.0 - angle;
            }
        }
        Cp = CurveManager::CurveValue(state, curve, angle);

        return Cp * 0.5 * rho * windSpeed * windSpeed;
    }

    Real64 CalcDuctInsideConvResist(Real64 const Tair, // Average air temperature
                                    Real64 const mdot, // Mass flow rate
                                    Real64 const Dh,   // Hydraulic diameter
                                    Real64 const hIn   // User defined convection coefficient
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell, Tony Fontanini
        //       DATE WRITTEN   Feb. 2017
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates duct inside convection coefficients

        // REFERENCES:
        // ASTM C1340
        // Jakob, F.E.,  Fischer, R.D., Flanigan, L.J. 1987. "Experimental Validation of the Duct Submodel for the SP43 Simulation Model."
        // ASHRAE Trans. pp 1499-1514.

        Real64 hIn_final = 0;

        if (hIn == 0) {

            Real64 Tair_IP = Tair * 1.8 + 32.0;     // Convert C to F
            Real64 mdot_IP = mdot * 2.20462 * 3600; // Convert kg/s to lb/hr
            Real64 Dh_IP = Dh * 3.28084;            // Convert m to ft
            Real64 Ai_IP = pow_2(Dh_IP) * DataGlobalConstants::Pi / 4;

            Real64 CorrelationCoeff = 0.00368 + 1.5e-6 * (Tair_IP - 80);
            Real64 MassFlux = mdot_IP / Ai_IP; // lb/hr-ft2

            Real64 DuctInsideConvCoeff_IP = CorrelationCoeff * pow(MassFlux, 0.8) / pow(Dh_IP, 0.2); // BTU/hr-ft2-F

            hIn_final = DuctInsideConvCoeff_IP * pow_2(3.28084) * 1.8 * 1055.06 / 3600; // Convert BTU/hr-ft2-F to W/m2-K

        } else {
            hIn_final = hIn;
        }

        if (hIn_final == 0) {
            return 0;
        } else {
            return 1 / hIn_final;
        }
    }

    Real64 CalcDuctOutsideConvResist(EnergyPlusData &state,
                                     Real64 const Ts,      // Surface temperature
                                     Real64 const Tamb,    // Free air temperature
                                     Real64 const Wamb,    // Free air humidity ratio
                                     Real64 const Pamb,    // Free air barometric pressure
                                     Real64 const Dh,      // Hydraulic diameter
                                     Real64 const ZoneNum, // Zone number
                                     Real64 const hOut     // User defined convection coefficient
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Matt Mitchell, Tony Fontanini
        //       DATE WRITTEN   Feb. 2017
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates duct outside convection coefficients

        // REFERENCES:
        // ASTM C1340

        Real64 k = airThermConductivity(state, Ts);
        auto &Zone(state.dataHeatBal->Zone);

        Real64 hOut_final = 0;

        if (hOut == 0) {

            // Free convection
            Real64 Pr = airPrandtl(state, (Ts + Tamb) / 2, Wamb, Pamb);
            Real64 KinVisc = airKinematicVisc(state, (Ts + Tamb) / 2, Wamb, Pamb);
            Real64 Beta = 2.0 / ((Tamb + DataGlobalConstants::KelvinConv) + (Ts + DataGlobalConstants::KelvinConv));
            Real64 Gr = DataGlobalConstants::GravityConstant * Beta * std::abs(Ts - Tamb) * pow_3(Dh) / pow_2(KinVisc);
            Real64 Ra = Gr * Pr;
            Real64 Nu_free(0);

            if (Ra < 10e9) {
                Nu_free = 0.53 * pow(Ra, 0.25);
            } else {
                Nu_free = 0.13 * pow(Ra, 0.333);
            }

            Real64 V = 0;
            // Forced convection
            if (ZoneNum > 0) {
                Real64 ACH = GetZoneOutdoorAirChangeRate(state, ZoneNum); // Zone air change rate [1/hr]
                Real64 Vol = Zone(ZoneNum).Volume;                        // Zone volume [m3]
                V = pow(Vol, 0.333) * ACH / 3600;                         // Average air speed in zone [m/s]
            } else {
                V = state.dataEnvrn->WindSpeed;
            }

            Real64 Re = V * Dh / KinVisc; // Reynolds number
            Real64 c = 0;
            Real64 n = 0;

            if (Re <= 4) {
                c = 0.989;
                n = 0.33;
            } else if (4 < Re && Re <= 40) {
                c = 0.911;
                n = 0.385;
            } else if (40 < Re && Re <= 4000) {
                c = 0.683;
                n = 0.466;
            } else if (4000 < Re && Re <= 40000) {
                c = 0.193;
                n = 0.618;
            } else if (40000 < Re) {
                c = 0.0266;
                n = 0.805;
            }

            Real64 Nu_forced = c * pow(Re, n) * pow(Pr, 0.333);

            Real64 Nu_combined = pow(pow_3(Nu_free) + pow_3(Nu_forced), 0.333);
            hOut_final = Nu_combined * k / Dh;

        } else {
            hOut_final = hOut;
        }

        if (hOut_final == 0) {
            return 0;
        } else {
            return 1 / hOut_final;
        }
    }

    void CalcAirflowNetworkHeatBalance(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  Revised based on Subroutine CalcADSHeatBalance

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs AirflowNetwork thermal simulations.

        // USE STATEMENTS:
        auto &TimeStepSys = state.dataHVACGlobal->TimeStepSys;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int j;
        int LF;
        int LT;
        int CompNum;
        int NF;
        int NT;
        iComponentTypeNum CompTypeNum;
        int TypeNum;
        int ExtNodeNum;
        std::string CompName;
        Real64 Ei;
        Real64 DirSign;
        Real64 Tamb;
        Real64 Wamb;
        Real64 Pamb;
        Real64 CpAir;
        Real64 TZON;
        Real64 load;
        int ZoneNum;
        bool found;
        bool OANode;

        auto &Node(state.dataLoopNodes->Node);

        state.afn->MA = 0.0;
        state.afn->MV = 0.0;

        for (i = 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
            CompNum = state.afn->AirflowNetworkLinkageData(i).CompNum;
            CompTypeNum = state.afn->AirflowNetworkCompData(CompNum).CompTypeNum;
            CompName = state.afn->AirflowNetworkCompData(CompNum).EPlusName;
            CpAir = PsyCpAirFnW(
                (state.afn->AirflowNetworkNodeSimu(state.afn->AirflowNetworkLinkageData(i).NodeNums[0]).WZ +
                 state.afn->AirflowNetworkNodeSimu(state.afn->AirflowNetworkLinkageData(i).NodeNums[1]).WZ) /
                2.0);
            // Calculate duct conduction loss
            if (CompTypeNum == iComponentTypeNum::DWC && CompName == std::string()) { // Duct element only
                TypeNum = state.afn->AirflowNetworkCompData(CompNum).TypeNum;
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                // Fatal error when return flow is opposite to the desired direction
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW == 0.0 &&
                    state.afn->AirflowNetworkLinkSimu(i).FLOW2 > 0.0) {
                    if (state.afn->LoopOnOffFlag(state.afn->AirflowNetworkLinkageData(i).AirLoopNum)) {
                        ShowSevereError(state,
                                        "AirflowNetwork: The airflow direction is opposite to the intended direction (from node 1 to node 2) in "
                                        "AirflowNetwork:Distribution:Linkage = " +
                                            state.afn->AirflowNetworkLinkageData(i).Name);
                        ShowContinueErrorTimeStamp(state, "");
                        ShowContinueError(state,
                                          "The sum of the airflows entering the zone is greater than the airflows leaving the zone (e.g., wind "
                                          "and stack effect).");
                        ShowContinueError(state,
                                          "Please check wind speed or reduce values of \"Window/Door Opening Factor, or Crack Factor\" defined in "
                                          "AirflowNetwork:MultiZone:Surface objects.");
                        //                    ShowFatalError(state,  "AirflowNetwork: The previous error causes termination." );
                    }
                }

                if (state.afn->AirflowNetworkLinkageData(i).ZoneNum < 0) {
                    ExtNodeNum = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    if (state.afn->AirflowNetworkNodeData(ExtNodeNum).OutAirNodeNum > 0 &&
                        Node(state.afn->AirflowNetworkNodeData(ExtNodeNum).OutAirNodeNum).IsLocalNode) {
                        Tamb = Node(state.afn->AirflowNetworkNodeData(ExtNodeNum).OutAirNodeNum).OutAirDryBulb;
                        Wamb = Node(state.afn->AirflowNetworkNodeData(ExtNodeNum).OutAirNodeNum).HumRat;
                    } else {
                        Tamb = OutDryBulbTempAt(state, state.afn->AirflowNetworkNodeData(ExtNodeNum).NodeHeight);
                        Wamb = state.dataEnvrn->OutHumRat;
                    }
                } else if (state.afn->AirflowNetworkLinkageData(i).ZoneNum == 0) {
                    Tamb = state.afn->AirflowNetworkNodeSimu(LT).TZ;
                    Wamb = state.afn->AirflowNetworkNodeSimu(LT).WZ;
                } else {
                    Tamb = state.afn->ANZT(state.afn->AirflowNetworkLinkageData(i).ZoneNum);
                    Wamb = state.afn->ANZW(state.afn->AirflowNetworkLinkageData(i).ZoneNum);
                }

                Pamb = state.dataEnvrn->OutBaroPress;

                Real64 constexpr tolerance = 0.001;
                Real64 UThermal(10); // Initialize. This will get updated.
                Real64 UThermal_iter = 0;
                Real64 Tsurr = Tamb;
                Real64 Tsurr_K = Tsurr + DataGlobalConstants::KelvinConv;
                Real64 Tin = state.afn->AirflowNetworkNodeSimu(LF).TZ;
                Real64 TDuctSurf = (Tamb + Tin) / 2.0;
                Real64 TDuctSurf_K = TDuctSurf + DataGlobalConstants::KelvinConv;
                Real64 DuctSurfArea = state.afn->DisSysCompDuctData(TypeNum).L *
                                      state.afn->DisSysCompDuctData(TypeNum).hydraulicDiameter * DataGlobalConstants::Pi;

                // If user defined view factors not present, calculate air-to-air heat transfer
                if (state.afn->AirflowNetworkLinkageData(i).LinkageViewFactorObjectNum == 0) {

                    // Calculate convection coefficient if one or both not present
                    if (state.afn->DisSysCompDuctData(TypeNum).InsideConvCoeff == 0 &&
                        state.afn->DisSysCompDuctData(TypeNum).OutsideConvCoeff == 0) {
                        while (std::abs(UThermal - UThermal_iter) > tolerance) {
                            UThermal_iter = UThermal;

                            Real64 RThermConvIn = CalcDuctInsideConvResist(Tin,
                                                                           state.afn->AirflowNetworkLinkSimu(i).FLOW,
                                                                           state.afn->DisSysCompDuctData(TypeNum).hydraulicDiameter,
                                                                           state.afn->DisSysCompDuctData(TypeNum).InsideConvCoeff);
                            Real64 RThermConvOut = CalcDuctOutsideConvResist(state,
                                                                             TDuctSurf,
                                                                             Tamb,
                                                                             Wamb,
                                                                             Pamb,
                                                                             state.afn->DisSysCompDuctData(TypeNum).hydraulicDiameter,
                                                                             state.afn->AirflowNetworkLinkageData(i).ZoneNum,
                                                                             state.afn->DisSysCompDuctData(TypeNum).OutsideConvCoeff);
                            Real64 RThermConduct = 1.0 / state.afn->DisSysCompDuctData(TypeNum).UThermConduct;
                            Real64 RThermTotal = RThermConvIn + RThermConvOut + RThermConduct;
                            UThermal = pow(RThermTotal, -1);

                            // Duct conduction, assuming effectiveness = 1 - exp(-NTU)
                            Ei = General::epexp(-UThermal * DuctSurfArea,
                                                (DirSign * state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir));
                            Real64 QCondDuct = std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * CpAir * (Tamb - Tin) * (1 - Ei);

                            TDuctSurf = Tamb - QCondDuct * RThermConvOut / DuctSurfArea;
                        }
                    } else { // Air-to-air only. U and h values are all known
                        Real64 RThermConvIn = CalcDuctInsideConvResist(Tin,
                                                                       state.afn->AirflowNetworkLinkSimu(i).FLOW,
                                                                       state.afn->DisSysCompDuctData(TypeNum).hydraulicDiameter,
                                                                       state.afn->DisSysCompDuctData(TypeNum).InsideConvCoeff);
                        Real64 RThermConvOut = CalcDuctOutsideConvResist(state,
                                                                         TDuctSurf,
                                                                         Tamb,
                                                                         Wamb,
                                                                         Pamb,
                                                                         state.afn->DisSysCompDuctData(TypeNum).hydraulicDiameter,
                                                                         state.afn->AirflowNetworkLinkageData(i).ZoneNum,
                                                                         state.afn->DisSysCompDuctData(TypeNum).OutsideConvCoeff);
                        Real64 RThermConduct = 1.0 / state.afn->DisSysCompDuctData(TypeNum).UThermConduct;
                        Real64 RThermTotal = RThermConvIn + RThermConvOut + RThermConduct;
                        UThermal = pow(RThermTotal, -1);
                    }

                    Tsurr = Tamb;

                } else { // Air-to-air + radiation heat transfer

                    auto &VFObj(state.afn->AirflowNetworkLinkageViewFactorData(
                        state.afn->AirflowNetworkLinkageData(i).LinkageViewFactorObjectNum));
                    VFObj.QRad = 0;
                    VFObj.QConv = 0;

                    Real64 Tin_ave = Tin;
                    Real64 hOut = 0;

                    while (std::abs(UThermal - UThermal_iter) > tolerance) {
                        UThermal_iter = UThermal;

                        Real64 RThermConvIn = CalcDuctInsideConvResist(Tin_ave,
                                                                       state.afn->AirflowNetworkLinkSimu(i).FLOW,
                                                                       state.afn->DisSysCompDuctData(TypeNum).hydraulicDiameter,
                                                                       state.afn->DisSysCompDuctData(TypeNum).InsideConvCoeff);
                        Real64 RThermConvOut = CalcDuctOutsideConvResist(state,
                                                                         TDuctSurf,
                                                                         Tamb,
                                                                         Wamb,
                                                                         Pamb,
                                                                         state.afn->DisSysCompDuctData(TypeNum).hydraulicDiameter,
                                                                         state.afn->AirflowNetworkLinkageData(i).ZoneNum,
                                                                         state.afn->DisSysCompDuctData(TypeNum).OutsideConvCoeff);

                        if (RThermConvOut > 0.0) {
                            hOut = 1 / RThermConvOut;
                        }

                        Real64 RThermConduct = 1.0 / state.afn->DisSysCompDuctData(TypeNum).UThermConduct;

                        Real64 hrjTj_sum = 0;
                        Real64 hrj_sum = 0;

                        for (int j = 1; j <= VFObj.LinkageSurfaceData.u(); ++j) {

                            int ZoneSurfNum = VFObj.LinkageSurfaceData(j).SurfaceNum;

                            Real64 TSurfj = state.dataHeatBalSurf->SurfOutsideTempHist(1)(ZoneSurfNum);
                            Real64 TSurfj_K = TSurfj + DataGlobalConstants::KelvinConv;

                            Real64 ZoneSurfEmissivity =
                                state.dataConstruction->Construct(state.dataSurface->Surface(ZoneSurfNum).Construction).InsideAbsorpThermal;
                            Real64 ZoneSurfArea = state.dataSurface->Surface(ZoneSurfNum).Area;

                            Real64 DuctEmissivity = VFObj.DuctEmittance;
                            Real64 DuctExposureFrac = VFObj.DuctExposureFraction;
                            Real64 DuctToZoneSurfViewFactor = VFObj.LinkageSurfaceData(j).ViewFactor;

                            Real64 DuctSurfResistance = (1 - DuctEmissivity) / (DuctExposureFrac * DuctSurfArea * DuctEmissivity);
                            Real64 SpaceResistance = 1 / (DuctExposureFrac * DuctSurfArea * DuctToZoneSurfViewFactor);
                            Real64 ZoneSurfResistance = (1 - ZoneSurfEmissivity) / (ZoneSurfArea * ZoneSurfEmissivity);

                            VFObj.LinkageSurfaceData(j).SurfaceResistanceFactor =
                                DataGlobalConstants::StefanBoltzmann / (DuctSurfResistance + SpaceResistance + ZoneSurfResistance);

                            Real64 hrj = VFObj.LinkageSurfaceData(j).SurfaceResistanceFactor * (TDuctSurf_K + TSurfj_K) *
                                         (pow_2(TDuctSurf_K) + pow_2(TSurfj_K)) / DuctSurfArea;

                            hrjTj_sum += hrj * TSurfj;
                            hrj_sum += hrj;
                        }

                        Tsurr = (hOut * Tamb + hrjTj_sum) / (hOut + hrj_sum); // Surroundings temperature [C]
                        Tsurr_K = Tsurr + DataGlobalConstants::KelvinConv;

                        Real64 RThermTotal = RThermConvIn + RThermConduct + 1 / (hOut + hrj_sum);
                        UThermal = pow(RThermTotal, -1);

                        Real64 NTU = UThermal * DuctSurfArea / (DirSign * state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir);
                        Tin_ave = Tsurr + (Tin - Tsurr) * (1 / NTU) * (1 - exp(-NTU));

                        TDuctSurf = Tin_ave - UThermal * (RThermConvIn + RThermConduct) * (Tin_ave - Tsurr);
                        TDuctSurf_K = TDuctSurf + DataGlobalConstants::KelvinConv;
                    }

                    for (int j = 1; j <= VFObj.LinkageSurfaceData.u(); ++j) {
                        int ZoneSurfNum = VFObj.LinkageSurfaceData(j).SurfaceNum;
                        Real64 TSurfj = state.dataHeatBalSurf->SurfOutsideTempHist(1)(ZoneSurfNum);
                        Real64 TSurfj_K = TSurfj + DataGlobalConstants::KelvinConv;
                        VFObj.LinkageSurfaceData(j).SurfaceRadLoad = VFObj.LinkageSurfaceData(j).SurfaceResistanceFactor *
                                                                     (pow_4(TDuctSurf_K) - pow_4(TSurfj_K)); // Radiant load for this surface [W]
                        int SurfNum = VFObj.LinkageSurfaceData(j).SurfaceNum;
                        Real64 ZoneSurfaceArea = state.dataSurface->Surface(SurfNum).Area;
                        state.dataHeatBalFanSys->QRadSurfAFNDuct(SurfNum) += VFObj.LinkageSurfaceData(j).SurfaceRadLoad * TimeStepSys *
                                                                             DataGlobalConstants::SecInHour /
                                                                             ZoneSurfaceArea; // Energy to each surface per unit area [J/m2]
                        VFObj.QRad += VFObj.LinkageSurfaceData(j).SurfaceRadLoad; // Total radiant load from all surfaces for this system timestep [W]
                    }

                    VFObj.QConv = hOut * DuctSurfArea * (TDuctSurf - Tamb);
                    UThermal = (VFObj.QRad + VFObj.QConv) / (DuctSurfArea * std::abs(Tsurr - Tin));
                }

                if (!state.afn->LoopOnOffFlag(state.afn->AirflowNetworkLinkageData(i).AirLoopNum) &&
                    state.afn->AirflowNetworkLinkSimu(i).FLOW <= 0.0) {
                    Ei = General::epexp(-UThermal * DuctSurfArea, (state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir));
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * CpAir * Ei;
                    state.afn->MV(LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * Tsurr * (1.0 - Ei) * CpAir;
                } else {
                    Ei = General::epexp(-UThermal * DuctSurfArea, (DirSign * state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir));
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * CpAir * Ei;
                    state.afn->MV(LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * Tsurr * (1.0 - Ei) * CpAir;
                }
            }
            if (CompTypeNum == iComponentTypeNum::TMU) { // Reheat unit: SINGLE DUCT:CONST VOLUME:REHEAT
                TypeNum = state.afn->AirflowNetworkCompData(CompNum).TypeNum;
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                Ei = General::epexp(-0.001 * state.afn->DisSysCompTermUnitData(TypeNum).L *
                                        state.afn->DisSysCompTermUnitData(TypeNum).hydraulicDiameter * DataGlobalConstants::Pi,
                                    (DirSign * state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir));
                Tamb = state.afn->AirflowNetworkNodeSimu(LT).TZ;
                if (!state.afn->LoopOnOffFlag(state.afn->AirflowNetworkLinkageData(i).AirLoopNum) &&
                    state.afn->AirflowNetworkLinkSimu(i).FLOW <= 0.0) {
                    Ei = General::epexp(-0.001 * state.afn->DisSysCompTermUnitData(TypeNum).L *
                                            state.afn->DisSysCompTermUnitData(TypeNum).hydraulicDiameter * DataGlobalConstants::Pi,
                                        (state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir));
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * CpAir * Ei;
                    state.afn->MV(LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * Tamb * (1.0 - Ei) * CpAir;
                } else {
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * CpAir * Ei;
                    state.afn->MV(LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * Tamb * (1.0 - Ei) * CpAir;
                }
            }
            if (CompTypeNum == iComponentTypeNum::COI) { // heating or cooling coil
                TypeNum = state.afn->AirflowNetworkCompData(CompNum).TypeNum;
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
            }
            // Calculate temp in a constant pressure drop element
            if (CompTypeNum == iComponentTypeNum::CPD && CompName == std::string()) { // constant pressure element only
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                }
                if (!state.afn->LoopOnOffFlag(state.afn->AirflowNetworkLinkageData(i).AirLoopNum) &&
                    state.afn->AirflowNetworkLinkSimu(i).FLOW <= 0.0) {
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                } else {
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                }
                state.afn->MV(LT) = 0.0;
            }
            // Calculate return leak
            if ((CompTypeNum == iComponentTypeNum::PLR || CompTypeNum == iComponentTypeNum::ELR) && CompName == std::string()) {
                // Return leak element only
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                         .EPlusZoneNum > 0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                }
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0]).ExtNodeNum >
                     0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * CpAir;
                }
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                         .EPlusZoneNum > 0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                }
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1]).ExtNodeNum >
                     0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * CpAir;
                }
            }
            // Check reheat unit or coil
            if (state.afn->AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::RHT &&
                (!state.afn->AirflowNetworkLinkageData(i).VAVTermDamper)) {
                NF = 0;
                NT = 0;
                if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                        .EPlusNodeNum > 0) {
                    NF = state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                             .EPlusNodeNum;
                }
                if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                        .EPlusNodeNum > 0) {
                    NT = state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                             .EPlusNodeNum;
                }
                if ((NF == 0) || (NT == 0)) {
                    ShowFatalError(state,
                                   "Node number in the primary air loop is not found in AIRFLOWNETWORK:DISTRIBUTION:NODE = " +
                                       state.afn->AirflowNetworkLinkageData(i).Name);
                }
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    load = Node(NT).Temp - Node(NF).Temp;
                } else {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    load = Node(NF).Temp - Node(NT).Temp;
                }
                CpAir = PsyCpAirFnW(Node(NT).HumRat);
                state.afn->MV(LT) += state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir * load;
            }
        }

        // Prescribe temperature for EPlus nodes
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            found = false;
            OANode = false;
            for (j = 1; j <= state.afn->AirflowNetworkNumOfLinks; ++j) {
                if (state.afn->AirflowNetworkLinkageData(j).NodeNums[0] == i ||
                    state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                    CompNum = state.afn->AirflowNetworkLinkageData(j).CompNum;
                    if (state.afn->AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::RHT &&
                        (!state.afn->AirflowNetworkLinkageData(j).VAVTermDamper)) {
                        found = true;
                        break;
                    }
                    // Overwrite fan outlet node
                    if (state.afn->AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::FAN &&
                        state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                        found = false;
                        break;
                    }
                    // Overwrite return connection outlet
                    if (state.afn->AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::RCN) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                    if (state.afn->AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::SCN &&
                        state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                }
                if (state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i &&
                    state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(j).NodeNums[0])
                            .EPlusTypeNum == iEPlusNodeType::OAN) {
                    OANode = true;
                    break;
                }
            }
            if (found) continue;
            if (state.afn->AirflowNetworkNodeData(i).EPlusZoneNum == 0 &&
                state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::ZIN)
                continue;
            j = state.afn->AirflowNetworkNodeData(i).EPlusNodeNum;

            if (j > 0 && (state.afn->AirflowNetworkNodeData(i).EPlusZoneNum > 0 ||
                          state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::FOU ||
                          state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::COU ||
                          state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::HXO)) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = Node(j).Temp * 1.0e10;
            }
            if (j > 0 && OANode) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = Node(j).Temp * 1.0e10;
            }
            if (state.afn->AirflowNetworkNodeData(i).EPlusZoneNum > 0 &&
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) < 0.9e10) {
                ZoneNum = state.afn->AirflowNetworkNodeData(i).EPlusZoneNum;
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.afn->ANZT(ZoneNum) * 1.0e10;
            }
            if (state.afn->AirflowNetworkNodeData(i).ExtNodeNum > 0 &&
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) < 0.9e10) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                if (state.afn->AirflowNetworkNodeData(i).OutAirNodeNum > 0) {
                    state.afn->MV(i) =
                        Node(state.afn->AirflowNetworkNodeData(i).OutAirNodeNum).OutAirDryBulb * 1.0e10;
                } else {
                    state.afn->MV(i) =
                        OutDryBulbTempAt(state, state.afn->AirflowNetworkNodeData(i).NodeHeight) * 1.0e10;
                }
            }
            if (state.afn->AirflowNetworkNodeData(i).RAFNNodeNum > 0 &&
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) < 0.9e10) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                ZoneNum = state.afn->AirflowNetworkNodeData(i).EPlusZoneNum;
                if (state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(ZoneNum)
                        .Node(state.afn->AirflowNetworkNodeData(i).RAFNNodeNum)
                        .AirflowNetworkNodeID == i) {
                    state.afn->MV(i) = state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(ZoneNum)
                                                                        .Node(state.afn->AirflowNetworkNodeData(i).RAFNNodeNum)
                                                                        .AirTemp *
                                                                    1.0e10;
                }
            }
        }

        // Assign node value to distribution nodes with fan off
        for (i = 1 + state.afn->NumOfNodesMultiZone; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            j = state.afn->AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0 && !state.afn->LoopOnOffFlag(state.afn->AirflowNetworkNodeData(i).AirLoopNum) &&
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) < 1.0e9) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = Node(j).Temp * 1.0e10;
            }
            if (j == 0 && i > state.afn->NumOfNodesMultiZone &&
                !state.afn->LoopOnOffFlag(state.afn->AirflowNetworkNodeData(i).AirLoopNum)) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.afn->AirflowNetworkNodeSimu(i).TZlast * 1.0e10;
            }
        }

        // Check singularity
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            if (state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) < 1.0e-6) {
                if (i > state.afn->NumOfNodesMultiZone &&
                    !state.afn->LoopOnOffFlag(state.afn->AirflowNetworkNodeData(i).AirLoopNum)) {
                    state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                    state.afn->MV(i) = state.afn->AirflowNetworkNodeSimu(i).TZlast * 1.0e10;
                } else {
                    ShowFatalError(state,
                                   "CalcAirflowNetworkHeatBalance: A diagonal entity is zero in AirflowNetwork matrix at node " +
                                       state.afn->AirflowNetworkNodeData(i).Name);
                }
            }
        }

        // Get an inverse matrix
        MRXINV(state, state.afn->AirflowNetworkNumOfNodes);

        // Calculate node temperatures
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            TZON = 0.0;
            for (j = 1; j <= state.afn->AirflowNetworkNumOfNodes; ++j) {
                TZON += state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + j) *
                        state.afn->MV(j);
            }
            state.afn->AirflowNetworkNodeSimu(i).TZ = TZON;
        }
    }

    void CalcAirflowNetworkMoisBalance(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  Revised based on Subroutine CalcADSMoistureBalance

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs AirflowNetwork moisture simulations.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int j;
        int LF;
        int LT;
        int CompNum;
        int NF;
        int NT;
        iComponentTypeNum CompTypeNum;
        int TypeNum;
        std::string CompName;
        Real64 Ei;
        Real64 DirSign;
        Real64 Wamb;
        Real64 WZON;
        Real64 load;
        int ZoneNum;
        bool found;
        bool OANode;

        auto &Node(state.dataLoopNodes->Node);

        state.afn->MA = 0.0;
        state.afn->MV = 0.0;
        for (i = 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
            CompNum = state.afn->AirflowNetworkLinkageData(i).CompNum;
            CompTypeNum = state.afn->AirflowNetworkCompData(CompNum).CompTypeNum;
            CompName = state.afn->AirflowNetworkCompData(CompNum).EPlusName;
            // Calculate duct moisture diffusion loss
            if (CompTypeNum == iComponentTypeNum::DWC && CompName == std::string()) { // Duct component only
                TypeNum = state.afn->AirflowNetworkCompData(CompNum).TypeNum;
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                Ei = General::epexp(-state.afn->DisSysCompDuctData(TypeNum).UMoisture *
                                        state.afn->DisSysCompDuctData(TypeNum).L *
                                        state.afn->DisSysCompDuctData(TypeNum).hydraulicDiameter * DataGlobalConstants::Pi,
                                    (DirSign * state.afn->AirflowNetworkLinkSimu(i).FLOW));
                if (state.afn->AirflowNetworkLinkageData(i).ZoneNum < 0) {
                    Wamb = state.dataEnvrn->OutHumRat;
                } else if (state.afn->AirflowNetworkLinkageData(i).ZoneNum == 0) {
                    Wamb = state.afn->AirflowNetworkNodeSimu(LT).WZ;
                } else {
                    Wamb = state.afn->ANZW(state.afn->AirflowNetworkLinkageData(i).ZoneNum);
                }
                if (!state.afn->LoopOnOffFlag(state.afn->AirflowNetworkLinkageData(i).AirLoopNum) &&
                    state.afn->AirflowNetworkLinkSimu(i).FLOW <= 0.0) {
                    Ei = General::epexp(-state.afn->DisSysCompDuctData(TypeNum).UMoisture *
                                            state.afn->DisSysCompDuctData(TypeNum).L *
                                            state.afn->DisSysCompDuctData(TypeNum).hydraulicDiameter * DataGlobalConstants::Pi,
                                        (state.afn->AirflowNetworkLinkSimu(i).FLOW2));
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * Ei;
                    state.afn->MV(LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * Wamb * (1.0 - Ei);
                } else {
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * Ei;
                    state.afn->MV(LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * Wamb * (1.0 - Ei);
                }
            }
            if (CompTypeNum == iComponentTypeNum::TMU) { // Reheat unit: SINGLE DUCT:CONST VOLUME:REHEAT
                TypeNum = state.afn->AirflowNetworkCompData(CompNum).TypeNum;
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                Ei = General::epexp(-0.0001 * state.afn->DisSysCompTermUnitData(TypeNum).L *
                                        state.afn->DisSysCompTermUnitData(TypeNum).hydraulicDiameter * DataGlobalConstants::Pi,
                                    (DirSign * state.afn->AirflowNetworkLinkSimu(i).FLOW));
                Wamb = state.afn->AirflowNetworkNodeSimu(LT).WZ;
                if (!state.afn->LoopOnOffFlag(state.afn->AirflowNetworkLinkageData(i).AirLoopNum) &&
                    state.afn->AirflowNetworkLinkSimu(i).FLOW <= 0.0) {

                    Ei = General::epexp(-0.0001 * state.afn->DisSysCompTermUnitData(TypeNum).L *
                                            state.afn->DisSysCompTermUnitData(TypeNum).hydraulicDiameter * DataGlobalConstants::Pi,
                                        (state.afn->AirflowNetworkLinkSimu(i).FLOW2));
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * Ei;
                    state.afn->MV(LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * Wamb * (1.0 - Ei);
                } else {
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * Ei;
                    state.afn->MV(LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW) * Wamb * (1.0 - Ei);
                }
            }
            if (CompTypeNum == iComponentTypeNum::COI) { // heating or cooling coil
                TypeNum = state.afn->AirflowNetworkCompData(CompNum).TypeNum;
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
            }
            // Calculate temp in a constant pressure drop component
            if (CompTypeNum == iComponentTypeNum::CPD && CompName == std::string()) { // constant pressure element only
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                }
                if (!state.afn->LoopOnOffFlag(state.afn->AirflowNetworkLinkageData(i).AirLoopNum) &&
                    state.afn->AirflowNetworkLinkSimu(i).FLOW <= 0.0) {
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                } else {
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                }
                state.afn->MV(LT) = 0.0;
            }
            // Calculate return leak
            if ((CompTypeNum == iComponentTypeNum::PLR || CompTypeNum == iComponentTypeNum::ELR) && CompName == std::string()) {
                // Return leak component only
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                         .EPlusZoneNum > 0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                }
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0]).ExtNodeNum >
                     0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                }
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                         .EPlusZoneNum > 0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                }
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1]).ExtNodeNum >
                     0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                }
            }
            // Check reheat unit
            if (state.afn->AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::RHT &&
                (!state.afn->AirflowNetworkLinkageData(i).VAVTermDamper)) {
                NF = 0;
                NT = 0;
                if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                        .EPlusNodeNum > 0) {
                    NF = state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                             .EPlusNodeNum;
                }
                if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                        .EPlusNodeNum > 0) {
                    NT = state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                             .EPlusNodeNum;
                }
                if ((NF == 0) || (NT == 0)) {
                    ShowFatalError(state,
                                   "Node number in the primary air loop is not found in AIRFLOWNETWORK:DISTRIBUTION:NODE = " +
                                       state.afn->AirflowNetworkLinkageData(i).Name);
                }
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    load = Node(NT).HumRat - Node(NF).HumRat;
                } else {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    load = Node(NF).HumRat - Node(NT).HumRat;
                }
                state.afn->MV(LT) += state.afn->AirflowNetworkLinkSimu(i).FLOW * load;
            }
        }

        // Prescribe temperature for EPlus nodes
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            found = false;
            OANode = false;
            for (j = 1; j <= state.afn->AirflowNetworkNumOfLinks; ++j) {
                if (state.afn->AirflowNetworkLinkageData(j).NodeNums[0] == i ||
                    state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                    CompNum = state.afn->AirflowNetworkLinkageData(j).CompNum;
                    if (state.afn->AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::RHT &&
                        (!state.afn->AirflowNetworkLinkageData(j).VAVTermDamper)) {
                        found = true;
                        break;
                    }
                    // Overwrite fan outlet node
                    if (state.afn->AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::FAN &&
                        state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                        found = false;
                        break;
                    }
                    // Overwrite return connection outlet
                    if (state.afn->AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::RCN) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                    if (state.afn->AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::SCN &&
                        state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                }
                if (state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i &&
                    state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(j).NodeNums[0])
                            .EPlusTypeNum == iEPlusNodeType::OAN) {
                    OANode = true;
                    break;
                }
            }
            if (found) continue;
            if (state.afn->AirflowNetworkNodeData(i).EPlusZoneNum == 0 &&
                state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::ZIN)
                continue;
            j = state.afn->AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0 && (state.afn->AirflowNetworkNodeData(i).EPlusZoneNum > 0 ||
                          state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::FOU ||
                          state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::COU ||
                          state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::HXO)) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = Node(j).HumRat * 1.0e10;
            }
            if (j > 0 && OANode) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = Node(j).HumRat * 1.0e10;
            }
            if (state.afn->AirflowNetworkNodeData(i).EPlusZoneNum > 0 &&
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) < 0.9e10) {
                ZoneNum = state.afn->AirflowNetworkNodeData(i).EPlusZoneNum;
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.afn->ANZW(ZoneNum) * 1.0e10;
            }
            if (state.afn->AirflowNetworkNodeData(i).ExtNodeNum > 0) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.dataEnvrn->OutHumRat * 1.0e10;
            }
            if (state.afn->AirflowNetworkNodeData(i).RAFNNodeNum > 0 &&
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) < 0.9e10) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                ZoneNum = state.afn->AirflowNetworkNodeData(i).EPlusZoneNum;
                if (state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(ZoneNum)
                        .Node(state.afn->AirflowNetworkNodeData(i).RAFNNodeNum)
                        .AirflowNetworkNodeID == i) {
                    state.afn->MV(i) = state.dataRoomAirMod->RoomAirflowNetworkZoneInfo(ZoneNum)
                                                                        .Node(state.afn->AirflowNetworkNodeData(i).RAFNNodeNum)
                                                                        .HumRat *
                                                                    1.0e10;
                }
            }
        }

        // Assign node value to distribution nodes with fan off
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            j = state.afn->AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0 && !state.afn->LoopOnOffFlag(state.afn->AirflowNetworkNodeData(i).AirLoopNum) &&
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) < 1.0e9) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = Node(j).HumRat * 1.0e10;
            }
            if (j == 0 && i > state.afn->NumOfNodesMultiZone &&
                !state.afn->LoopOnOffFlag(state.afn->AirflowNetworkNodeData(i).AirLoopNum)) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.afn->AirflowNetworkNodeSimu(i).WZlast * 1.0e10;
            }
        }

        // Check singularity
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            if (state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) < 1.0e-8) {
                ShowFatalError(state,
                               "CalcAirflowNetworkMoisBalance: A diagonal entity is zero in AirflowNetwork matrix at node " +
                                   state.afn->AirflowNetworkNodeData(i).Name);
            }
        }

        // Get an inverse matrix
        MRXINV(state, state.afn->AirflowNetworkNumOfNodes);

        // Calculate node temperatures
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            WZON = 0.0;
            for (j = 1; j <= state.afn->AirflowNetworkNumOfNodes; ++j) {
                WZON += state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + j) *
                        state.afn->MV(j);
            }
            state.afn->AirflowNetworkNodeSimu(i).WZ = WZON;
        }
    }

    void CalcAirflowNetworkCO2Balance(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   June. 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  Revised based on Subroutine CalcAirflowNetworkMoisBalance

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs AirflowNetwork CO2 simulations.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int j;
        int LF;
        int LT;
        int CompNum;
        iComponentTypeNum CompTypeNum;
        int TypeNum;
        std::string CompName;
        Real64 DirSign;
        Real64 COZN;
        int ZoneNum;
        bool found;
        bool OANode;

        state.afn->MA = 0.0;
        state.afn->MV = 0.0;
        for (i = 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
            CompNum = state.afn->AirflowNetworkLinkageData(i).CompNum;
            CompTypeNum = state.afn->AirflowNetworkCompData(CompNum).CompTypeNum;
            CompName = state.afn->AirflowNetworkCompData(CompNum).EPlusName;
            // Calculate duct moisture diffusion loss
            if (CompTypeNum == iComponentTypeNum::DWC && CompName == std::string()) { // Duct component only
                TypeNum = state.afn->AirflowNetworkCompData(CompNum).TypeNum;
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                    std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                    -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
            }
            if (CompTypeNum == iComponentTypeNum::TMU) { // Reheat unit: SINGLE DUCT:CONST VOLUME:REHEAT
                TypeNum = state.afn->AirflowNetworkCompData(CompNum).TypeNum;
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                    std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                    -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
            }
            if (CompTypeNum == iComponentTypeNum::COI) { // heating or cooling coil
                TypeNum = state.afn->AirflowNetworkCompData(CompNum).TypeNum;
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
            }
            // Calculate temp in a constant pressure drop component
            if (CompTypeNum == iComponentTypeNum::CPD && CompName == std::string()) { // constant pressure element only
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                }
                state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                    std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                    -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                state.afn->MV(LT) = 0.0;
            }
            // Calculate return leak
            if ((CompTypeNum == iComponentTypeNum::PLR || CompTypeNum == iComponentTypeNum::ELR) && CompName == std::string()) {
                // Return leak component only
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                         .EPlusZoneNum > 0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                }
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0]).ExtNodeNum >
                     0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                }
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                         .EPlusZoneNum > 0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                }
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1]).ExtNodeNum >
                     0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                }
            }
        }

        // Prescribe temperature for EPlus nodes
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            found = false;
            OANode = false;
            for (j = 1; j <= state.afn->AirflowNetworkNumOfLinks; ++j) {
                if (state.afn->AirflowNetworkLinkageData(j).NodeNums[0] == i ||
                    state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                    CompNum = state.afn->AirflowNetworkLinkageData(j).CompNum;
                    if (state.afn->AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::RHT &&
                        (!state.afn->AirflowNetworkLinkageData(j).VAVTermDamper)) {
                        found = true;
                        break;
                    }
                    // Overwrite fan outlet node
                    if (state.afn->AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::FAN &&
                        state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                        found = false;
                        break;
                    }
                    // Overwrite return connection outlet
                    if (state.afn->AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::RCN) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                    if (state.afn->AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::SCN &&
                        state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                }
                if (state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i &&
                    state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(j).NodeNums[0])
                            .EPlusTypeNum == iEPlusNodeType::OAN) {
                    OANode = true;
                    break;
                }
            }
            if (found) continue;
            if (state.afn->AirflowNetworkNodeData(i).EPlusZoneNum == 0 &&
                state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::ZIN)
                continue;
            j = state.afn->AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0 && (state.afn->AirflowNetworkNodeData(i).EPlusZoneNum > 0 ||
                          state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::FOU ||
                          state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::COU ||
                          state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::HXO)) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.dataLoopNodes->Node(j).CO2 * 1.0e10;
            }
            if (j > 0 && OANode) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.dataLoopNodes->Node(j).CO2 * 1.0e10;
            }
            if (state.afn->AirflowNetworkNodeData(i).EPlusZoneNum > 0 &&
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) < 0.9e10) {
                ZoneNum = state.afn->AirflowNetworkNodeData(i).EPlusZoneNum;
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.afn->ANCO(ZoneNum) * 1.0e10;
            }
            if (state.afn->AirflowNetworkNodeData(i).ExtNodeNum > 0) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.dataContaminantBalance->OutdoorCO2 * 1.0e10;
            }
        }

        // Assign node value to distribution nodes with fan off
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            j = state.afn->AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0 && !state.afn->LoopOnOffFlag(state.afn->AirflowNetworkNodeData(i).AirLoopNum) &&
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) < 1.0e9) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.dataLoopNodes->Node(j).CO2 * 1.0e10;
            }
            if (j == 0 && i > state.afn->NumOfNodesMultiZone &&
                !state.afn->LoopOnOffFlag(state.afn->AirflowNetworkNodeData(i).AirLoopNum)) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.afn->AirflowNetworkNodeSimu(i).CO2Zlast * 1.0e10;
            }
        }

        // Check singularity
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            if (state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) < 1.0e-6) {
                ShowFatalError(state,
                               "CalcAirflowNetworkCO2Balance: A diagonal entity is zero in AirflowNetwork matrix at node " +
                                   state.afn->AirflowNetworkNodeData(i).Name);
            }
        }

        // Get an inverse matrix
        MRXINV(state, state.afn->AirflowNetworkNumOfNodes);

        // Calculate node temperatures
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            COZN = 0.0;
            for (j = 1; j <= state.afn->AirflowNetworkNumOfNodes; ++j) {
                COZN += state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + j) *
                        state.afn->MV(j);
            }
            state.afn->AirflowNetworkNodeSimu(i).CO2Z = COZN;
        }
    }

    void CalcAirflowNetworkGCBalance(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Jan. 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  Revised based on Subroutine CalcAirflowNetworkCO2Balance

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs AirflowNetwork generic contaminant simulations.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int j;
        int LF;
        int LT;
        int CompNum;
        iComponentTypeNum CompTypeNum;
        int TypeNum;
        std::string CompName;
        Real64 DirSign;
        Real64 COZN;
        int ZoneNum;
        bool found;
        bool OANode;

        state.afn->MA = 0.0;
        state.afn->MV = 0.0;
        for (i = 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
            CompNum = state.afn->AirflowNetworkLinkageData(i).CompNum;
            CompTypeNum = state.afn->AirflowNetworkCompData(CompNum).CompTypeNum;
            CompName = state.afn->AirflowNetworkCompData(CompNum).EPlusName;
            // Calculate duct moisture diffusion loss
            if (CompTypeNum == iComponentTypeNum::DWC && CompName == std::string()) { // Duct component only
                TypeNum = state.afn->AirflowNetworkCompData(CompNum).TypeNum;
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                    std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                    -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
            }
            if (CompTypeNum == iComponentTypeNum::TMU) { // Reheat unit: SINGLE DUCT:CONST VOLUME:REHEAT
                TypeNum = state.afn->AirflowNetworkCompData(CompNum).TypeNum;
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
                state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                    std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                    -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
            }
            if (CompTypeNum == iComponentTypeNum::COI) { // heating or cooling coil
                TypeNum = state.afn->AirflowNetworkCompData(CompNum).TypeNum;
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    DirSign = 1.0;
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    DirSign = -1.0;
                }
            }
            // Calculate temp in a constant pressure drop component
            if (CompTypeNum == iComponentTypeNum::CPD && CompName == std::string()) { // constant pressure element only
                if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0) { // flow direction is the same as input from node 1 to node 2
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                } else { // flow direction is the opposite as input from node 2 to node 1
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                }
                state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                    std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                    -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                state.afn->MV(LT) = 0.0;
            }
            // Calculate return leak
            if ((CompTypeNum == iComponentTypeNum::PLR || CompTypeNum == iComponentTypeNum::ELR) && CompName == std::string()) {
                // Return leak component only
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                         .EPlusZoneNum > 0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                }
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0]).ExtNodeNum >
                     0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW);
                }
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                         .EPlusZoneNum > 0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                }
                if ((state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1]).ExtNodeNum >
                     0) &&
                    (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                         .EPlusZoneNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    LF = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    LT = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LT) +=
                        std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                    state.afn->MA((LT - 1) * state.afn->AirflowNetworkNumOfNodes + LF) =
                        -std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2);
                }
            }
        }

        // Prescribe temperature for EPlus nodes
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            found = false;
            OANode = false;
            for (j = 1; j <= state.afn->AirflowNetworkNumOfLinks; ++j) {
                if (state.afn->AirflowNetworkLinkageData(j).NodeNums[0] == i ||
                    state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                    CompNum = state.afn->AirflowNetworkLinkageData(j).CompNum;
                    if (state.afn->AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::RHT &&
                        (!state.afn->AirflowNetworkLinkageData(j).VAVTermDamper)) {
                        found = true;
                        break;
                    }
                    // Overwrite fan outlet node
                    if (state.afn->AirflowNetworkCompData(CompNum).EPlusTypeNum == iEPlusComponentType::FAN &&
                        state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i) {
                        found = false;
                        break;
                    }
                    // Overwrite return connection outlet
                    if (state.afn->AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::RCN) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                    if (state.afn->AirflowNetworkLinkageData(j).ConnectionFlag == iEPlusComponentType::SCN &&
                        state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i) { // Modified on 9/2/09
                        found = true;
                        break;
                    }
                }
                if (state.afn->AirflowNetworkLinkageData(j).NodeNums[1] == i &&
                    state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(j).NodeNums[0])
                            .EPlusTypeNum == iEPlusNodeType::OAN) {
                    OANode = true;
                    break;
                }
            }
            if (found) continue;
            if (state.afn->AirflowNetworkNodeData(i).EPlusZoneNum == 0 &&
                state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::ZIN)
                continue;
            j = state.afn->AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0 && (state.afn->AirflowNetworkNodeData(i).EPlusZoneNum > 0 ||
                          state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::FOU ||
                          state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::COU ||
                          state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::HXO)) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.dataLoopNodes->Node(j).GenContam * 1.0e10;
            }
            if (j > 0 && OANode) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.dataLoopNodes->Node(j).GenContam * 1.0e10;
            }
            if (state.afn->AirflowNetworkNodeData(i).EPlusZoneNum > 0 &&
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) < 0.9e10) {
                ZoneNum = state.afn->AirflowNetworkNodeData(i).EPlusZoneNum;
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.afn->ANGC(ZoneNum) * 1.0e10;
            }
            if (state.afn->AirflowNetworkNodeData(i).ExtNodeNum > 0) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.dataContaminantBalance->OutdoorGC * 1.0e10;
            }
        }

        // Assign node value to distribution nodes with fan off
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            j = state.afn->AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0 && !state.afn->LoopOnOffFlag(state.afn->AirflowNetworkNodeData(i).AirLoopNum) &&
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) < 1.0e9) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.dataLoopNodes->Node(j).GenContam * 1.0e10;
            }
            if (j == 0 && i > state.afn->NumOfNodesMultiZone &&
                !state.afn->LoopOnOffFlag(state.afn->AirflowNetworkNodeData(i).AirLoopNum)) {
                state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) = 1.0e10;
                state.afn->MV(i) = state.afn->AirflowNetworkNodeSimu(i).GCZlast * 1.0e10;
            }
        }

        // Check singularity
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            if (state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + i) < 1.0e-6) {
                ShowFatalError(state,
                               "CalcAirflowNetworkGCBalance: A diagonal entity is zero in AirflowNetwork matrix at node " +
                                   state.afn->AirflowNetworkNodeData(i).Name);
            }
        }

        // Get an inverse matrix
        MRXINV(state, state.afn->AirflowNetworkNumOfNodes);

        // Calculate node temperatures
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            COZN = 0.0;
            for (j = 1; j <= state.afn->AirflowNetworkNumOfNodes; ++j) {
                COZN += state.afn->MA((i - 1) * state.afn->AirflowNetworkNumOfNodes + j) *
                        state.afn->MV(j);
            }
            state.afn->AirflowNetworkNodeSimu(i).GCZ = COZN;
        }
    }

    void MRXINV(EnergyPlusData &state, int const NORDER)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  Revised based on Subroutine ADSINV

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine inverses a matrix

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int j;
        int K;
        int M;
        Real64 R1;
        Real64 S;

        state.afn->IVEC = 0;
        for (i = 1; i <= NORDER; ++i) {
            state.afn->IVEC(i + 20) = i;
        }
        for (i = 1; i <= NORDER; ++i) {
            R1 = 0.0;
            M = i;
            for (j = i; j <= NORDER; ++j) {
                if (std::abs(R1) < std::abs(state.afn->MA((i - 1) * NORDER + j))) {
                    M = j;
                    R1 = state.afn->MA((i - 1) * NORDER + j);
                }
            }
            if (i != M) {
                K = state.afn->IVEC(M + 20);
                state.afn->IVEC(M + 20) = state.afn->IVEC(i + 20);
                state.afn->IVEC(i + 20) = K;
                for (j = 1; j <= NORDER; ++j) {
                    S = state.afn->MA((j - 1) * NORDER + i);
                    state.afn->MA((j - 1) * NORDER + i) =
                        state.afn->MA((j - 1) * NORDER + M);
                    state.afn->MA((j - 1) * NORDER + M) = S;
                }
            }
            state.afn->MA((i - 1) * NORDER + i) = 1.0;
            for (j = 1; j <= NORDER; ++j) {
                state.afn->MA((i - 1) * NORDER + j) /= R1;
            }
            for (j = 1; j <= NORDER; ++j) {
                if (i == j) continue;
                R1 = state.afn->MA((j - 1) * NORDER + i);
                if (std::abs(R1) <= 1.0E-20) continue;
                state.afn->MA((j - 1) * NORDER + i) = 0.0;
                for (K = 1; K <= NORDER; ++K) {
                    state.afn->MA((j - 1) * NORDER + K) -=
                        R1 * state.afn->MA((i - 1) * NORDER + K);
                }
            }
        }
        for (i = 1; i <= NORDER; ++i) {
            if (state.afn->IVEC(i + 20) == i) continue;
            M = i;
            while (NORDER > M) {
                ++M;
                if (state.afn->IVEC(M + 20) == i) break;
            }
            state.afn->IVEC(M + 20) = state.afn->IVEC(i + 20);
            for (j = 1; j <= NORDER; ++j) {
                R1 = state.afn->MA((i - 1) * NORDER + j);
                state.afn->MA((i - 1) * NORDER + j) = state.afn->MA((M - 1) * NORDER + j);
                state.afn->MA((M - 1) * NORDER + j) = R1;
            }
            state.afn->IVEC(i + 20) = i;
        }
    }

    void ReportAirflowNetwork(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   2/1/04
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine reports outputs of air distribution systems

        // Using/Aliasing
        auto &NumPrimaryAirSys = state.dataHVACGlobal->NumPrimaryAirSys;
        auto &TimeStepSys = state.dataHVACGlobal->TimeStepSys;

        auto &Zone(state.dataHeatBal->Zone);

        // SUBROUTINE PARAMETER DEFINITIONS:
        constexpr Real64 Lam(2.5e6); // Heat of vaporization (J/kg)

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int n;
        int M;
        int ZN1;
        int ZN2;
        Real64 AirDensity;
        Real64 CpAir;
        Real64 Tamb;
        Real64 hg; // latent heat of vaporization
        Real64 ReportingConstant;
        Real64 ReportingFraction;
        int AirLoopNum;
        int FanNum;
        Real64 RepOnOffFanRunTimeFraction;

        if (state.afn->SimulateAirflowNetwork < AirflowNetworkControlMultizone) return;

        if (!state.afn->onetime) {
            state.afn->onceZoneFlag.dimension(state.dataGlobal->NumOfZones, false);
            state.afn->onceSurfFlag.dimension(state.afn->AirflowNetworkNumOfLinks, false);
            state.afn->onetime = true;
        }
        ReportingConstant = TimeStepSys * DataGlobalConstants::SecInHour;

        state.dataHeatBal->ZoneTotalExfiltrationHeatLoss = 0.0;

        for (auto &e : state.afn->AirflowNetworkReportData) {
            e.MultiZoneInfiSenGainW = 0.0;
            e.MultiZoneInfiSenGainJ = 0.0;
            e.MultiZoneInfiSenLossW = 0.0;
            e.MultiZoneInfiSenLossJ = 0.0;
            e.MultiZoneInfiLatGainW = 0.0;
            e.MultiZoneInfiLatGainJ = 0.0;
            e.MultiZoneInfiLatLossW = 0.0;
            e.MultiZoneInfiLatLossJ = 0.0;
            e.MultiZoneVentSenGainW = 0.0;
            e.MultiZoneVentSenGainJ = 0.0;
            e.MultiZoneVentSenLossW = 0.0;
            e.MultiZoneVentSenLossJ = 0.0;
            e.MultiZoneVentLatGainW = 0.0;
            e.MultiZoneVentLatGainJ = 0.0;
            e.MultiZoneVentLatLossW = 0.0;
            e.MultiZoneVentLatLossJ = 0.0;
            e.MultiZoneMixSenGainW = 0.0;
            e.MultiZoneMixSenGainJ = 0.0;
            e.MultiZoneMixSenLossW = 0.0;
            e.MultiZoneMixSenLossJ = 0.0;
            e.MultiZoneMixLatGainW = 0.0;
            e.MultiZoneMixLatGainJ = 0.0;
            e.MultiZoneMixLatLossW = 0.0;
            e.MultiZoneMixLatLossJ = 0.0;
            e.LeakSenGainW = 0.0;
            e.LeakSenGainJ = 0.0;
            e.LeakSenLossW = 0.0;
            e.LeakSenLossJ = 0.0;
            e.LeakLatGainW = 0.0;
            e.LeakLatGainJ = 0.0;
            e.LeakLatLossW = 0.0;
            e.LeakLatLossJ = 0.0;
            e.CondSenGainW = 0.0;
            e.CondSenGainJ = 0.0;
            e.CondSenLossW = 0.0;
            e.CondSenLossJ = 0.0;
            e.DiffLatGainW = 0.0;
            e.DiffLatGainJ = 0.0;
            e.DiffLatLossW = 0.0;
            e.DiffLatLossJ = 0.0;
            e.RadGainW = 0.0;
            e.RadGainJ = 0.0;
            e.RadLossW = 0.0;
            e.RadLossJ = 0.0;
            e.TotalSenGainW = 0.0;
            e.TotalSenGainJ = 0.0;
            e.TotalSenLossW = 0.0;
            e.TotalSenLossJ = 0.0;
            e.TotalLatGainW = 0.0;
            e.TotalLatGainJ = 0.0;
            e.TotalLatLossW = 0.0;
            e.TotalLatLossJ = 0.0;
        }

        // Calculate sensible and latent loads in each zone from multizone airflows
        if (state.afn->SimulateAirflowNetwork == AirflowNetworkControlMultizone ||
            state.afn->SimulateAirflowNetwork == AirflowNetworkControlMultiADS ||
            (state.afn->SimulateAirflowNetwork == AirflowNetworkControlSimpleADS &&
             state.afn->AirflowNetworkFanActivated)) {
            for (i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) { // Multizone airflow energy
                n = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                M = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                ZN1 = state.afn->AirflowNetworkNodeData(n).EPlusZoneNum;
                ZN2 = state.afn->AirflowNetworkNodeData(M).EPlusZoneNum;
                // Find a linkage from a zone to outdoors
                if (ZN1 > 0 && ZN2 == 0) {
                    if (state.dataSurface->SurfHasLinkedOutAirNode(state.afn->MultizoneSurfaceData(i).SurfNum)) {
                        Tamb = state.dataSurface->SurfOutDryBulbTemp(state.afn->MultizoneSurfaceData(i).SurfNum);
                        CpAir = PsyCpAirFnW(Psychrometrics::PsyWFnTdbTwbPb(
                            state,
                            Tamb,
                            state.dataSurface->SurfOutWetBulbTemp(state.afn->MultizoneSurfaceData(i).SurfNum),
                            state.dataEnvrn->OutBaroPress));
                    } else {
                        Tamb = Zone(ZN1).OutDryBulbTemp;
                        CpAir = PsyCpAirFnW(state.dataEnvrn->OutHumRat);
                    }
                    hg = Psychrometrics::PsyHgAirFnWTdb(state.dataHeatBalFanSys->ZoneAirHumRat(ZN1), state.dataHeatBalFanSys->MAT(ZN1));

                    if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum)
                                .CompTypeNum == iComponentTypeNum::SCR ||
                        state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum)
                                .CompTypeNum == iComponentTypeNum::SEL) {
                        if (Tamb > state.dataHeatBalFanSys->MAT(ZN1)) {
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiSenGainW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (Tamb - state.dataHeatBalFanSys->MAT(ZN1)));
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiSenGainJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (Tamb - state.dataHeatBalFanSys->MAT(ZN1))) *
                                ReportingConstant;
                        } else {
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiSenLossW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (state.dataHeatBalFanSys->MAT(ZN1) - Tamb));
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiSenLossJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (state.dataHeatBalFanSys->MAT(ZN1) - Tamb)) *
                                ReportingConstant;
                        }
                        if (state.dataEnvrn->OutHumRat > state.dataHeatBalFanSys->ZoneAirHumRat(ZN1)) {
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiLatGainW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 *
                                 (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                                hg;
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiLatGainJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 *
                                 (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                                hg * ReportingConstant;
                        } else {
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiLatLossW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 *
                                 (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataEnvrn->OutHumRat)) *
                                hg;
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiLatLossJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 *
                                 (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataEnvrn->OutHumRat)) *
                                hg * ReportingConstant;
                        }
                    } else {
                        if (Tamb > state.dataHeatBalFanSys->MAT(ZN1)) {
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentSenGainW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (Tamb - state.dataHeatBalFanSys->MAT(ZN1)));
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentSenGainJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (Tamb - state.dataHeatBalFanSys->MAT(ZN1))) *
                                ReportingConstant;
                        } else {
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentSenLossW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (state.dataHeatBalFanSys->MAT(ZN1) - Tamb));
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentSenLossJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (state.dataHeatBalFanSys->MAT(ZN1) - Tamb)) *
                                ReportingConstant;
                        }
                        if (state.dataEnvrn->OutHumRat > state.dataHeatBalFanSys->ZoneAirHumRat(ZN1)) {
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentLatGainW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 *
                                 (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                                hg;
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentLatGainJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 *
                                 (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                                hg * ReportingConstant;
                        } else {
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentLatLossW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 *
                                 (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataEnvrn->OutHumRat)) *
                                hg;
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentLatLossJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW2 *
                                 (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataEnvrn->OutHumRat)) *
                                hg * ReportingConstant;
                        }
                    }
                }
                if (ZN1 == 0 && ZN2 > 0) {
                    if (state.dataSurface->SurfHasLinkedOutAirNode(state.afn->MultizoneSurfaceData(i).SurfNum)) {
                        Tamb = state.dataSurface->SurfOutDryBulbTemp(state.afn->MultizoneSurfaceData(i).SurfNum);
                        CpAir = PsyCpAirFnW(Psychrometrics::PsyWFnTdbTwbPb(
                            state,
                            Tamb,
                            state.dataSurface->SurfOutWetBulbTemp(state.afn->MultizoneSurfaceData(i).SurfNum),
                            state.dataEnvrn->OutBaroPress));
                    } else {
                        Tamb = Zone(ZN2).OutDryBulbTemp;
                        CpAir = PsyCpAirFnW(state.dataEnvrn->OutHumRat);
                    }
                    hg = Psychrometrics::PsyHgAirFnWTdb(state.dataHeatBalFanSys->ZoneAirHumRat(ZN2), state.dataHeatBalFanSys->MAT(ZN2));

                    if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum)
                                .CompTypeNum == iComponentTypeNum::SCR ||
                        state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum)
                                .CompTypeNum == iComponentTypeNum::SEL) {
                        if (Tamb > state.dataHeatBalFanSys->MAT(ZN2)) {
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiSenGainW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir * (Tamb - state.dataHeatBalFanSys->MAT(ZN2)));
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiSenGainJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir * (Tamb - state.dataHeatBalFanSys->MAT(ZN2))) *
                                ReportingConstant;
                        } else {
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiSenLossW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir * (state.dataHeatBalFanSys->MAT(ZN2) - Tamb));
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiSenLossJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir * (state.dataHeatBalFanSys->MAT(ZN2) - Tamb)) *
                                ReportingConstant;
                        }
                        if (state.dataEnvrn->OutHumRat > state.dataHeatBalFanSys->ZoneAirHumRat(ZN2)) {
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiLatGainW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW *
                                 (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                                hg;
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiLatGainJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW *
                                 (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                                hg * ReportingConstant;
                        } else {
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiLatLossW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW *
                                 (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataEnvrn->OutHumRat)) *
                                hg;
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiLatLossJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW *
                                 (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataEnvrn->OutHumRat)) *
                                hg * ReportingConstant;
                        }
                    } else {
                        if (Tamb > state.dataHeatBalFanSys->MAT(ZN2)) {
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentSenGainW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir * (Tamb - state.dataHeatBalFanSys->MAT(ZN2)));
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentSenGainJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir * (Tamb - state.dataHeatBalFanSys->MAT(ZN2))) *
                                ReportingConstant;
                        } else {
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentSenLossW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir * (state.dataHeatBalFanSys->MAT(ZN2) - Tamb));
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentSenLossJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir * (state.dataHeatBalFanSys->MAT(ZN2) - Tamb)) *
                                ReportingConstant;
                        }
                        if (state.dataEnvrn->OutHumRat > state.dataHeatBalFanSys->ZoneAirHumRat(ZN2)) {
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentLatGainW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW *
                                 (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                                hg;
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentLatGainJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW *
                                 (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                                hg * ReportingConstant;
                        } else {
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentLatLossW +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW *
                                 (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataEnvrn->OutHumRat)) *
                                hg;
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentLatLossJ +=
                                (state.afn->AirflowNetworkLinkSimu(i).FLOW *
                                 (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataEnvrn->OutHumRat)) *
                                hg * ReportingConstant;
                        }
                    }
                }

                if (ZN1 > 0 && ZN2 > 0) {
                    CpAir = PsyCpAirFnW((state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) + state.dataHeatBalFanSys->ZoneAirHumRat(ZN2)) / 2.0);
                    hg = Psychrometrics::PsyHgAirFnWTdb((state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) + state.dataHeatBalFanSys->ZoneAirHumRat(ZN2)) /
                                                            2.0,
                                                        (state.dataHeatBalFanSys->MAT(ZN1) + state.dataHeatBalFanSys->MAT(ZN2)) / 2.0);
                    if (state.dataHeatBalFanSys->MAT(ZN1) > state.dataHeatBalFanSys->MAT(ZN2)) {
                        state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixSenGainW +=
                            (state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir *
                             (state.dataHeatBalFanSys->MAT(ZN1) - state.dataHeatBalFanSys->MAT(ZN2)));
                        state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixSenGainJ +=
                            (state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir *
                             (state.dataHeatBalFanSys->MAT(ZN1) - state.dataHeatBalFanSys->MAT(ZN2))) *
                            ReportingConstant;
                    } else {
                        state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixSenLossW +=
                            (state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir *
                             (state.dataHeatBalFanSys->MAT(ZN2) - state.dataHeatBalFanSys->MAT(ZN1)));
                        state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixSenLossJ +=
                            (state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir *
                             (state.dataHeatBalFanSys->MAT(ZN2) - state.dataHeatBalFanSys->MAT(ZN1))) *
                            ReportingConstant;
                    }
                    if (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) > state.dataHeatBalFanSys->ZoneAirHumRat(ZN2)) {
                        state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixLatGainW +=
                            (state.afn->AirflowNetworkLinkSimu(i).FLOW *
                             (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                            hg;
                        state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixLatGainJ +=
                            (state.afn->AirflowNetworkLinkSimu(i).FLOW *
                             (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                            hg * ReportingConstant;
                    } else {
                        state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixLatLossW +=
                            (state.afn->AirflowNetworkLinkSimu(i).FLOW *
                             (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                            hg;
                        state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixLatLossJ +=
                            (state.afn->AirflowNetworkLinkSimu(i).FLOW *
                             (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                            hg * ReportingConstant;
                    }
                    if (state.dataHeatBalFanSys->MAT(ZN2) > state.dataHeatBalFanSys->MAT(ZN1)) {
                        state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixSenGainW +=
                            (state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir *
                             (state.dataHeatBalFanSys->MAT(ZN2) - state.dataHeatBalFanSys->MAT(ZN1)));
                        state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixSenGainJ +=
                            (state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir *
                             (state.dataHeatBalFanSys->MAT(ZN2) - state.dataHeatBalFanSys->MAT(ZN1))) *
                            ReportingConstant;
                    } else {
                        state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixSenLossW +=
                            (state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir *
                             (state.dataHeatBalFanSys->MAT(ZN1) - state.dataHeatBalFanSys->MAT(ZN2)));
                        state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixSenLossJ +=
                            (state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir *
                             (state.dataHeatBalFanSys->MAT(ZN1) - state.dataHeatBalFanSys->MAT(ZN2))) *
                            ReportingConstant;
                    }
                    if (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) > state.dataHeatBalFanSys->ZoneAirHumRat(ZN1)) {
                        state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixLatGainW +=
                            (state.afn->AirflowNetworkLinkSimu(i).FLOW2 *
                             (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                            hg;
                        state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixLatGainJ +=
                            (state.afn->AirflowNetworkLinkSimu(i).FLOW2 *
                             (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                            hg * ReportingConstant;
                    } else {
                        state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixLatLossW +=
                            std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2 *
                                     (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                            hg;
                        state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixLatLossJ +=
                            (state.afn->AirflowNetworkLinkSimu(i).FLOW2 *
                             (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                            hg * ReportingConstant;
                    }
                }
            }
        }

        // Assign data for report
        if (state.afn->SimulateAirflowNetwork > AirflowNetworkControlMultizone) {
            for (i = 1; i <= state.dataGlobal->NumOfZones; ++i) {
                if (state.afn->exchangeData(i).LeakSen > 0.0) {
                    state.afn->AirflowNetworkReportData(i).LeakSenGainW =
                        state.afn->exchangeData(i).LeakSen;
                    state.afn->AirflowNetworkReportData(i).LeakSenGainJ =
                        state.afn->exchangeData(i).LeakSen * ReportingConstant;
                } else {
                    state.afn->AirflowNetworkReportData(i).LeakSenLossW =
                        -state.afn->exchangeData(i).LeakSen;
                    state.afn->AirflowNetworkReportData(i).LeakSenLossJ =
                        -state.afn->exchangeData(i).LeakSen * ReportingConstant;
                }
                if (state.afn->exchangeData(i).LeakLat > 0.0) {
                    state.afn->AirflowNetworkReportData(i).LeakLatGainW =
                        state.afn->exchangeData(i).LeakLat * Lam;
                    state.afn->AirflowNetworkReportData(i).LeakLatGainJ =
                        state.afn->exchangeData(i).LeakLat * Lam * ReportingConstant;
                } else {
                    state.afn->AirflowNetworkReportData(i).LeakLatLossW =
                        -state.afn->exchangeData(i).LeakLat * Lam;
                    state.afn->AirflowNetworkReportData(i).LeakLatLossJ =
                        -state.afn->exchangeData(i).LeakLat * Lam * ReportingConstant;
                }
                if (state.afn->exchangeData(i).CondSen > 0.0) {
                    state.afn->AirflowNetworkReportData(i).CondSenGainW =
                        state.afn->exchangeData(i).CondSen;
                    state.afn->AirflowNetworkReportData(i).CondSenGainJ =
                        state.afn->exchangeData(i).CondSen * ReportingConstant;
                } else {
                    state.afn->AirflowNetworkReportData(i).CondSenLossW =
                        -state.afn->exchangeData(i).CondSen;
                    state.afn->AirflowNetworkReportData(i).CondSenLossJ =
                        -state.afn->exchangeData(i).CondSen * ReportingConstant;
                }
                if (state.afn->exchangeData(i).DiffLat > 0.0) {
                    state.afn->AirflowNetworkReportData(i).DiffLatGainW =
                        state.afn->exchangeData(i).DiffLat * Lam;
                    state.afn->AirflowNetworkReportData(i).DiffLatGainJ =
                        state.afn->exchangeData(i).DiffLat * Lam * ReportingConstant;
                } else {
                    state.afn->AirflowNetworkReportData(i).DiffLatLossW =
                        -state.afn->exchangeData(i).DiffLat * Lam;
                    state.afn->AirflowNetworkReportData(i).DiffLatLossJ =
                        -state.afn->exchangeData(i).DiffLat * Lam * ReportingConstant;
                }
                if (state.afn->exchangeData(i).RadGain < 0.0) {
                    state.afn->AirflowNetworkReportData(i).RadGainW = -state.afn->exchangeData(i).RadGain;
                    state.afn->AirflowNetworkReportData(i).RadGainJ =
                        -state.afn->exchangeData(i).RadGain * ReportingConstant;
                } else {
                    state.afn->AirflowNetworkReportData(i).RadLossW = state.afn->exchangeData(i).RadGain;
                    state.afn->AirflowNetworkReportData(i).RadLossJ =
                        state.afn->exchangeData(i).RadGain * ReportingConstant;
                }
                if (state.afn->exchangeData(i).TotalSen > 0.0) {
                    state.afn->AirflowNetworkReportData(i).TotalSenGainW =
                        state.afn->exchangeData(i).TotalSen;
                    state.afn->AirflowNetworkReportData(i).TotalSenGainJ =
                        state.afn->exchangeData(i).TotalSen * ReportingConstant;
                } else {
                    state.afn->AirflowNetworkReportData(i).TotalSenLossW =
                        -state.afn->exchangeData(i).TotalSen;
                    state.afn->AirflowNetworkReportData(i).TotalSenLossJ =
                        -state.afn->exchangeData(i).TotalSen * ReportingConstant;
                }
                if (state.afn->exchangeData(i).TotalLat > 0.0) {
                    state.afn->AirflowNetworkReportData(i).TotalLatGainW =
                        state.afn->exchangeData(i).TotalLat * Lam;
                    state.afn->AirflowNetworkReportData(i).TotalLatGainJ =
                        state.afn->exchangeData(i).TotalLat * Lam * ReportingConstant;
                } else {
                    state.afn->AirflowNetworkReportData(i).TotalLatLossW =
                        -state.afn->exchangeData(i).TotalLat * Lam;
                    state.afn->AirflowNetworkReportData(i).TotalLatLossJ =
                        -state.afn->exchangeData(i).TotalLat * Lam * ReportingConstant;
                }
            }
        }

        // Zone report

        for (auto &e : state.afn->AirflowNetworkZnRpt) {
            e.InfilVolume = 0.0;
            e.InfilMass = 0.0;
            e.InfilAirChangeRate = 0.0;
            e.VentilVolume = 0.0;
            e.VentilMass = 0.0;
            e.VentilAirChangeRate = 0.0;
            e.MixVolume = 0.0;
            e.MixMass = 0.0;
        }

        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            if (state.afn->DisSysNumOfCVFs == 0) continue;
            for (FanNum = 1; FanNum <= state.afn->DisSysNumOfCVFs; ++FanNum) {
                if (state.afn->DisSysCompCVFData(FanNum).AirLoopNum == AirLoopNum) break;
            }
            if (state.afn->DisSysCompCVFData(FanNum).FanTypeNum == FanType_SimpleOnOff &&
                state.afn->LoopOnOffFanRunTimeFraction(AirLoopNum) < 1.0 &&
                state.afn->LoopOnOffFanRunTimeFraction(AirLoopNum) > 0.0) {
                // ON Cycle calculation
                state.afn->onceZoneFlag = false;
                for (i = 1; i <= state.dataGlobal->NumOfZones; ++i) {
                    if (state.afn->AirflowNetworkNodeData(i).AirLoopNum > 0 &&
                        state.afn->AirflowNetworkNodeData(i).AirLoopNum != AirLoopNum)
                        continue;
                    if (state.afn->AirflowNetworkNodeData(i).AirLoopNum == AirLoopNum) {
                        RepOnOffFanRunTimeFraction = state.afn->LoopOnOffFanRunTimeFraction(AirLoopNum);
                    }
                    if (state.afn->AirflowNetworkNodeData(i).AirLoopNum == 0) {
                        RepOnOffFanRunTimeFraction = state.afn->MaxOnOffFanRunTimeFraction;
                    }
                    if (state.afn->AirflowNetworkNodeData(i).AirLoopNum == 0 &&
                        state.afn->onceZoneFlag(i))
                        continue;
                    state.afn->AirflowNetworkReportData(i).MultiZoneInfiSenGainW *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneInfiSenGainJ *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneInfiSenLossW *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneInfiSenLossJ *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneInfiLatGainW *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneInfiLatGainJ *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneInfiLatLossW *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneInfiLatLossJ *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneVentSenGainW *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneVentSenGainJ *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneVentSenLossW *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneVentSenLossJ *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneVentLatGainW *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneVentLatGainJ *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneVentLatLossW *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneVentLatLossJ *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneMixSenGainW *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneMixSenGainJ *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneMixSenLossW *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneMixSenLossJ *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneMixLatGainW *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneMixLatGainJ *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneMixLatLossW *= RepOnOffFanRunTimeFraction;
                    state.afn->AirflowNetworkReportData(i).MultiZoneMixLatLossJ *= RepOnOffFanRunTimeFraction;
                    if (state.afn->AirflowNetworkNodeData(i).AirLoopNum == 0) {
                        state.afn->onceZoneFlag(i) = true;
                    }
                }
                // Off Cycle addon
                state.afn->onceSurfFlag = false;
                for (i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) { // Multizone airflow energy
                    n = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    M = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    ZN1 = state.afn->AirflowNetworkNodeData(n).EPlusZoneNum;
                    ZN2 = state.afn->AirflowNetworkNodeData(M).EPlusZoneNum;
                    // Find a linkage from a zone to outdoors
                    if (ZN1 > 0 && ZN2 == 0) {
                        if (state.afn->AirflowNetworkNodeData(n).AirLoopNum > 0 &&
                            state.afn->AirflowNetworkNodeData(n).AirLoopNum != AirLoopNum)
                            continue;
                        if (state.afn->AirflowNetworkNodeData(n).AirLoopNum == AirLoopNum) {
                            RepOnOffFanRunTimeFraction = state.afn->LoopOnOffFanRunTimeFraction(AirLoopNum);
                        }
                        if (state.afn->AirflowNetworkNodeData(n).AirLoopNum == 0) {
                            RepOnOffFanRunTimeFraction = state.afn->MaxOnOffFanRunTimeFraction;
                        }
                        if (state.afn->AirflowNetworkNodeData(n).AirLoopNum == 0 &&
                            state.afn->onceSurfFlag(i))
                            continue;
                        ReportingFraction = (1.0 - RepOnOffFanRunTimeFraction);
                        Tamb = Zone(ZN1).OutDryBulbTemp;
                        CpAir = PsyCpAirFnW(state.dataEnvrn->OutHumRat);
                        if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum)
                                    .CompTypeNum == iComponentTypeNum::SCR ||
                            state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum)
                                    .CompTypeNum == iComponentTypeNum::SEL) {
                            if (Tamb > state.dataHeatBalFanSys->MAT(ZN1)) {
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiSenGainW +=
                                    (state.afn->linkReport1(i).FLOW2OFF * CpAir *
                                     (Tamb - state.dataHeatBalFanSys->MAT(ZN1))) *
                                    (1.0 - RepOnOffFanRunTimeFraction);
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiSenGainJ +=
                                    (state.afn->linkReport1(i).FLOW2OFF * CpAir *
                                     (Tamb - state.dataHeatBalFanSys->MAT(ZN1))) *
                                    ReportingConstant * ReportingFraction;
                            } else {
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiSenLossW +=
                                    (state.afn->linkReport1(i).FLOW2OFF * CpAir *
                                     (state.dataHeatBalFanSys->MAT(ZN1) - Tamb)) *
                                    (1.0 - RepOnOffFanRunTimeFraction);
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiSenLossJ +=
                                    (state.afn->linkReport1(i).FLOW2OFF * CpAir *
                                     (state.dataHeatBalFanSys->MAT(ZN1) - Tamb)) *
                                    ReportingConstant * ReportingFraction;
                            }
                            if (state.dataEnvrn->OutHumRat > state.dataHeatBalFanSys->ZoneAirHumRat(ZN1)) {
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiLatGainW +=
                                    (state.afn->linkReport1(i).FLOW2OFF *
                                     (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                                    ReportingFraction;
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiLatGainJ +=
                                    (state.afn->linkReport1(i).FLOW2OFF *
                                     (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                                    ReportingConstant * ReportingFraction;
                            } else {
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiLatLossW +=
                                    (state.afn->linkReport1(i).FLOW2OFF *
                                     (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataEnvrn->OutHumRat)) *
                                    ReportingFraction;
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneInfiLatLossJ +=
                                    (state.afn->linkReport1(i).FLOW2OFF *
                                     (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataEnvrn->OutHumRat)) *
                                    ReportingConstant * ReportingFraction;
                            }
                        } else {
                            if (Tamb > state.dataHeatBalFanSys->MAT(ZN1)) {
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentSenGainW +=
                                    (state.afn->linkReport1(i).FLOW2OFF * CpAir *
                                     (Tamb - state.dataHeatBalFanSys->MAT(ZN1))) *
                                    (1.0 - RepOnOffFanRunTimeFraction);
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentSenGainJ +=
                                    (state.afn->linkReport1(i).FLOW2OFF * CpAir *
                                     (Tamb - state.dataHeatBalFanSys->MAT(ZN1))) *
                                    ReportingConstant * ReportingFraction;
                            } else {
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentSenLossW +=
                                    (state.afn->linkReport1(i).FLOW2OFF * CpAir *
                                     (state.dataHeatBalFanSys->MAT(ZN1) - Tamb)) *
                                    (1.0 - RepOnOffFanRunTimeFraction);
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentSenLossJ +=
                                    (state.afn->linkReport1(i).FLOW2OFF * CpAir *
                                     (state.dataHeatBalFanSys->MAT(ZN1) - Tamb)) *
                                    ReportingConstant * ReportingFraction;
                            }
                            if (state.dataEnvrn->OutHumRat > state.dataHeatBalFanSys->ZoneAirHumRat(ZN1)) {
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentLatGainW +=
                                    (state.afn->linkReport1(i).FLOW2OFF *
                                     (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                                    ReportingFraction;
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentLatGainJ +=
                                    (state.afn->linkReport1(i).FLOW2OFF *
                                     (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                                    ReportingConstant * ReportingFraction;
                            } else {
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentLatLossW +=
                                    (state.afn->linkReport1(i).FLOW2OFF *
                                     (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataEnvrn->OutHumRat)) *
                                    ReportingFraction;
                                state.afn->AirflowNetworkReportData(ZN1).MultiZoneVentLatLossJ +=
                                    (state.afn->linkReport1(i).FLOW2OFF *
                                     (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataEnvrn->OutHumRat)) *
                                    ReportingConstant * ReportingFraction;
                            }
                        }
                        if (state.afn->AirflowNetworkNodeData(n).AirLoopNum == 0) {
                            state.afn->onceSurfFlag(i) = true;
                        }
                    }
                    if (ZN1 == 0 && ZN2 > 0) {
                        if (state.afn->AirflowNetworkNodeData(M).AirLoopNum > 0 &&
                            state.afn->AirflowNetworkNodeData(M).AirLoopNum != AirLoopNum)
                            continue;
                        if (state.afn->AirflowNetworkNodeData(M).AirLoopNum == AirLoopNum) {
                            RepOnOffFanRunTimeFraction = state.afn->LoopOnOffFanRunTimeFraction(AirLoopNum);
                        }
                        if (state.afn->AirflowNetworkNodeData(M).AirLoopNum == 0) {
                            RepOnOffFanRunTimeFraction = state.afn->MaxOnOffFanRunTimeFraction;
                        }
                        if (state.afn->AirflowNetworkNodeData(M).AirLoopNum == 0 &&
                            state.afn->onceSurfFlag(i))
                            continue;
                        ReportingFraction = (1.0 - RepOnOffFanRunTimeFraction);
                        Tamb = Zone(ZN2).OutDryBulbTemp;
                        CpAir = PsyCpAirFnW(state.dataEnvrn->OutHumRat);
                        if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum)
                                    .CompTypeNum == iComponentTypeNum::SCR ||
                            state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum)
                                    .CompTypeNum == iComponentTypeNum::SEL) {
                            if (Tamb > state.dataHeatBalFanSys->MAT(ZN2)) {
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiSenGainW +=
                                    (state.afn->linkReport1(i).FLOWOFF * CpAir *
                                     (Tamb - state.dataHeatBalFanSys->MAT(ZN2))) *
                                    ReportingFraction;
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiSenGainJ +=
                                    (state.afn->linkReport1(i).FLOWOFF * CpAir *
                                     (Tamb - state.dataHeatBalFanSys->MAT(ZN2))) *
                                    ReportingConstant * ReportingFraction;
                            } else {
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiSenLossW +=
                                    (state.afn->linkReport1(i).FLOWOFF * CpAir *
                                     (state.dataHeatBalFanSys->MAT(ZN2) - Tamb)) *
                                    ReportingFraction;
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiSenLossJ +=
                                    (state.afn->linkReport1(i).FLOWOFF * CpAir *
                                     (state.dataHeatBalFanSys->MAT(ZN2) - Tamb)) *
                                    ReportingConstant * ReportingFraction;
                            }
                            if (state.dataEnvrn->OutHumRat > state.dataHeatBalFanSys->ZoneAirHumRat(ZN2)) {
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiLatGainW +=
                                    (state.afn->linkReport1(i).FLOWOFF *
                                     (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                                    ReportingFraction;
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiLatGainJ +=
                                    (state.afn->linkReport1(i).FLOWOFF *
                                     (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                                    ReportingConstant * ReportingFraction;
                            } else {
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiLatLossW +=
                                    (state.afn->linkReport1(i).FLOWOFF *
                                     (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataEnvrn->OutHumRat)) *
                                    ReportingFraction;
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneInfiLatLossJ +=
                                    (state.afn->linkReport1(i).FLOWOFF *
                                     (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataEnvrn->OutHumRat)) *
                                    ReportingConstant * ReportingFraction;
                            }
                        } else {
                            if (Tamb > state.dataHeatBalFanSys->MAT(ZN2)) {
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentSenGainW +=
                                    (state.afn->linkReport1(i).FLOWOFF * CpAir *
                                     (Tamb - state.dataHeatBalFanSys->MAT(ZN2))) *
                                    ReportingFraction;
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentSenGainJ +=
                                    (state.afn->linkReport1(i).FLOWOFF * CpAir *
                                     (Tamb - state.dataHeatBalFanSys->MAT(ZN2))) *
                                    ReportingConstant * ReportingFraction;
                            } else {
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentSenLossW +=
                                    (state.afn->linkReport1(i).FLOWOFF * CpAir *
                                     (state.dataHeatBalFanSys->MAT(ZN2) - Tamb)) *
                                    ReportingFraction;
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentSenLossJ +=
                                    (state.afn->linkReport1(i).FLOWOFF * CpAir *
                                     (state.dataHeatBalFanSys->MAT(ZN2) - Tamb)) *
                                    ReportingConstant * ReportingFraction;
                            }
                            if (state.dataEnvrn->OutHumRat > state.dataHeatBalFanSys->ZoneAirHumRat(ZN2)) {
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentLatGainW +=
                                    (state.afn->linkReport1(i).FLOWOFF *
                                     (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                                    ReportingFraction;
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentLatGainJ +=
                                    (state.afn->linkReport1(i).FLOWOFF *
                                     (state.dataEnvrn->OutHumRat - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                                    ReportingConstant * ReportingFraction;
                            } else {
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentLatLossW +=
                                    (state.afn->linkReport1(i).FLOWOFF *
                                     (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataEnvrn->OutHumRat)) *
                                    ReportingFraction;
                                state.afn->AirflowNetworkReportData(ZN2).MultiZoneVentLatLossJ +=
                                    (state.afn->linkReport1(i).FLOWOFF *
                                     (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataEnvrn->OutHumRat)) *
                                    ReportingConstant * ReportingFraction;
                            }
                        }
                        if (state.afn->AirflowNetworkNodeData(M).AirLoopNum == 0) {
                            state.afn->onceSurfFlag(i) = true;
                        }
                    }

                    if (ZN1 > 0 && ZN2 > 0) {
                        ReportingFraction = (1.0 - state.afn->MaxOnOffFanRunTimeFraction);
                        CpAir = PsyCpAirFnW(state.dataHeatBalFanSys->ZoneAirHumRat(ZN1));
                        if (state.dataHeatBalFanSys->MAT(ZN1) > state.dataHeatBalFanSys->MAT(ZN2)) {
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixSenGainW +=
                                (state.afn->linkReport1(i).FLOWOFF * CpAir *
                                 (state.dataHeatBalFanSys->MAT(ZN1) - state.dataHeatBalFanSys->MAT(ZN2))) *
                                ReportingFraction;
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixSenGainJ +=
                                (state.afn->linkReport1(i).FLOWOFF * CpAir *
                                 (state.dataHeatBalFanSys->MAT(ZN1) - state.dataHeatBalFanSys->MAT(ZN2))) *
                                ReportingConstant * ReportingFraction;
                        } else {
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixSenLossW +=
                                (state.afn->linkReport1(i).FLOWOFF * CpAir *
                                 (state.dataHeatBalFanSys->MAT(ZN2) - state.dataHeatBalFanSys->MAT(ZN1))) *
                                ReportingFraction;
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixSenLossJ +=
                                (state.afn->linkReport1(i).FLOWOFF * CpAir *
                                 (state.dataHeatBalFanSys->MAT(ZN2) - state.dataHeatBalFanSys->MAT(ZN1))) *
                                ReportingConstant * ReportingFraction;
                        }
                        if (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) > state.dataHeatBalFanSys->ZoneAirHumRat(ZN2)) {
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixLatGainW +=
                                (state.afn->linkReport1(i).FLOWOFF *
                                 (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                                ReportingFraction;
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixLatGainJ +=
                                (state.afn->linkReport1(i).FLOWOFF *
                                 (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                                ReportingConstant * ReportingFraction;
                        } else {
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixLatLossW +=
                                (state.afn->linkReport1(i).FLOWOFF *
                                 (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                                ReportingFraction;
                            state.afn->AirflowNetworkReportData(ZN2).MultiZoneMixLatLossJ +=
                                (state.afn->linkReport1(i).FLOWOFF *
                                 (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                                ReportingConstant * ReportingFraction;
                        }
                        CpAir = PsyCpAirFnW(state.dataHeatBalFanSys->ZoneAirHumRat(ZN2));
                        if (state.dataHeatBalFanSys->MAT(ZN2) > state.dataHeatBalFanSys->MAT(ZN1)) {
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixSenGainW +=
                                (state.afn->linkReport1(i).FLOW2OFF * CpAir *
                                 (state.dataHeatBalFanSys->MAT(ZN2) - state.dataHeatBalFanSys->MAT(ZN1))) *
                                ReportingFraction;
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixSenGainJ +=
                                (state.afn->linkReport1(i).FLOW2OFF * CpAir *
                                 (state.dataHeatBalFanSys->MAT(ZN2) - state.dataHeatBalFanSys->MAT(ZN1))) *
                                ReportingConstant * ReportingFraction;
                        } else {
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixSenLossW +=
                                (state.afn->linkReport1(i).FLOW2OFF * CpAir *
                                 (state.dataHeatBalFanSys->MAT(ZN1) - state.dataHeatBalFanSys->MAT(ZN2))) *
                                ReportingFraction;
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixSenLossJ +=
                                (state.afn->linkReport1(i).FLOW2OFF * CpAir *
                                 (state.dataHeatBalFanSys->MAT(ZN1) - state.dataHeatBalFanSys->MAT(ZN2))) *
                                ReportingConstant * ReportingFraction;
                        }

                        if (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) > state.dataHeatBalFanSys->ZoneAirHumRat(ZN1)) {
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixLatGainW +=
                                (state.afn->linkReport1(i).FLOW2OFF *
                                 (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                                ReportingFraction;
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixLatGainJ +=
                                (state.afn->linkReport1(i).FLOW2OFF *
                                 (state.dataHeatBalFanSys->ZoneAirHumRat(ZN2) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN1))) *
                                ReportingConstant * ReportingFraction;
                        } else {
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixLatLossW +=
                                std::abs(state.afn->linkReport1(i).FLOW2OFF *
                                         (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                                ReportingFraction;
                            state.afn->AirflowNetworkReportData(ZN1).MultiZoneMixLatLossJ +=
                                (state.afn->linkReport1(i).FLOW2OFF *
                                 (state.dataHeatBalFanSys->ZoneAirHumRat(ZN1) - state.dataHeatBalFanSys->ZoneAirHumRat(ZN2))) *
                                ReportingConstant * ReportingFraction;
                        }
                    }
                }
            }
        }

        if (!(state.afn->SimulateAirflowNetwork == AirflowNetworkControlMultizone ||
              state.afn->SimulateAirflowNetwork == AirflowNetworkControlMultiADS))
            return;

        for (i = 1; i <= state.dataGlobal->NumOfZones; ++i) { // Start of zone loads report variable update loop ...
            Tamb = Zone(i).OutDryBulbTemp;
            CpAir = PsyCpAirFnW(state.dataHeatBalFanSys->ZoneAirHumRatAvg(i));
            AirDensity = PsyRhoAirFnPbTdbW(
                state, state.dataEnvrn->OutBaroPress, state.dataHeatBalFanSys->MAT(i), state.dataHeatBalFanSys->ZoneAirHumRatAvg(i));

            state.afn->AirflowNetworkZnRpt(i).InfilMass =
                (state.afn->exchangeData(i).SumMCp / CpAir) * ReportingConstant;
            state.afn->AirflowNetworkZnRpt(i).InfilVolume =
                state.afn->AirflowNetworkZnRpt(i).InfilMass / AirDensity;
            state.afn->AirflowNetworkZnRpt(i).InfilAirChangeRate =
                state.afn->AirflowNetworkZnRpt(i).InfilVolume / (TimeStepSys * Zone(i).Volume);
            state.afn->AirflowNetworkZnRpt(i).VentilMass =
                (state.afn->exchangeData(i).SumMVCp / CpAir) * ReportingConstant;
            state.afn->AirflowNetworkZnRpt(i).VentilVolume =
                state.afn->AirflowNetworkZnRpt(i).VentilMass / AirDensity;
            state.afn->AirflowNetworkZnRpt(i).VentilAirChangeRate =
                state.afn->AirflowNetworkZnRpt(i).VentilVolume / (TimeStepSys * Zone(i).Volume);
            state.afn->AirflowNetworkZnRpt(i).MixMass =
                (state.afn->exchangeData(i).SumMMCp / CpAir) * ReportingConstant;
            state.afn->AirflowNetworkZnRpt(i).MixVolume =
                state.afn->AirflowNetworkZnRpt(i).MixMass / AirDensity;
            // save values for predefined report
            Real64 stdDensAFNInfilVolume = state.afn->AirflowNetworkZnRpt(i).InfilMass / state.dataEnvrn->StdRhoAir;
            Real64 stdDensAFNNatVentVolume = state.afn->AirflowNetworkZnRpt(i).VentilMass / state.dataEnvrn->StdRhoAir;
            state.dataHeatBal->ZonePreDefRep(i).AFNVentVolStdDen = stdDensAFNNatVentVolume;
            state.dataHeatBal->ZonePreDefRep(i).AFNVentVolTotalStdDen += stdDensAFNNatVentVolume;
            state.dataHeatBal->ZonePreDefRep(i).AFNInfilVolTotalStdDen += stdDensAFNInfilVolume;
            if (state.dataHeatBal->ZonePreDefRep(i).isOccupied) {
                state.dataHeatBal->ZonePreDefRep(i).AFNVentVolTotalOccStdDen += stdDensAFNNatVentVolume;
                state.dataHeatBal->ZonePreDefRep(i).AFNInfilVolTotalOccStdDen += stdDensAFNInfilVolume;
                state.dataHeatBal->ZonePreDefRep(i).AFNInfilVolTotalOcc +=
                    (state.afn->AirflowNetworkZnRpt(i).InfilVolume +
                     state.afn->AirflowNetworkZnRpt(i).VentilVolume) *
                    Zone(i).Multiplier * Zone(i).ListMultiplier;
                if ((state.afn->AirflowNetworkZnRpt(i).InfilVolume +
                     state.afn->AirflowNetworkZnRpt(i).VentilVolume) <
                    state.dataHeatBal->ZonePreDefRep(i).AFNInfilVolMin) {
                    state.dataHeatBal->ZonePreDefRep(i).AFNInfilVolMin =
                        (state.afn->AirflowNetworkZnRpt(i).InfilVolume +
                         state.afn->AirflowNetworkZnRpt(i).VentilVolume) *
                        Zone(i).Multiplier * Zone(i).ListMultiplier;
                }
            }

            Real64 H2OHtOfVap = Psychrometrics::PsyHgAirFnWTdb(state.dataEnvrn->OutHumRat, Zone(i).OutDryBulbTemp);
            state.afn->AirflowNetworkZnRpt(i).InletMass = 0;
            state.afn->AirflowNetworkZnRpt(i).OutletMass = 0;
            if (state.dataZoneEquip->ZoneEquipConfig(i).IsControlled) {
                for (int j = 1; j <= state.dataZoneEquip->ZoneEquipConfig(i).NumInletNodes; ++j) {
                    state.afn->AirflowNetworkZnRpt(i).InletMass +=
                        state.dataLoopNodes->Node(state.dataZoneEquip->ZoneEquipConfig(i).InletNode(j)).MassFlowRate * ReportingConstant;
                }
                for (int j = 1; j <= state.dataZoneEquip->ZoneEquipConfig(i).NumExhaustNodes; ++j) {
                    state.afn->AirflowNetworkZnRpt(i).OutletMass +=
                        state.dataLoopNodes->Node(state.dataZoneEquip->ZoneEquipConfig(i).ExhaustNode(j)).MassFlowRate * ReportingConstant;
                }
                for (int j = 1; j <= state.dataZoneEquip->ZoneEquipConfig(i).NumReturnNodes; ++j) {
                    state.afn->AirflowNetworkZnRpt(i).OutletMass +=
                        state.dataLoopNodes->Node(state.dataZoneEquip->ZoneEquipConfig(i).ReturnNode(j)).MassFlowRate * ReportingConstant;
                }
            }
            state.afn->AirflowNetworkZnRpt(i).ExfilMass =
                state.afn->AirflowNetworkZnRpt(i).InfilMass +
                state.afn->AirflowNetworkZnRpt(i).VentilMass +
                state.afn->AirflowNetworkZnRpt(i).MixMass +
                state.afn->AirflowNetworkZnRpt(i).InletMass -
                state.afn->AirflowNetworkZnRpt(i).OutletMass;
            state.afn->AirflowNetworkZnRpt(i).ExfilSensiLoss =
                state.afn->AirflowNetworkZnRpt(i).ExfilMass / ReportingConstant *
                (state.dataHeatBalFanSys->MAT(i) - Tamb) * CpAir;
            state.afn->AirflowNetworkZnRpt(i).ExfilLatentLoss =
                state.afn->AirflowNetworkZnRpt(i).ExfilMass / ReportingConstant *
                (state.dataHeatBalFanSys->ZoneAirHumRat(i) - state.dataEnvrn->OutHumRat) * H2OHtOfVap;
            state.afn->AirflowNetworkZnRpt(i).ExfilTotalLoss =
                state.afn->AirflowNetworkZnRpt(i).ExfilSensiLoss +
                state.afn->AirflowNetworkZnRpt(i).ExfilLatentLoss;

            state.dataHeatBal->ZoneTotalExfiltrationHeatLoss +=
                state.afn->AirflowNetworkZnRpt(i).ExfilTotalLoss * ReportingConstant;
        } // ... end of zone loads report variable update loop.

        // Rewrite AirflowNetwork airflow rate
        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            if (state.afn->DisSysNumOfCVFs == 0) continue;
            for (FanNum = 1; FanNum <= state.afn->DisSysNumOfCVFs; ++FanNum) {
                if (state.afn->DisSysCompCVFData(FanNum).AirLoopNum == AirLoopNum) break;
            }
            state.afn->onceSurfFlag = false;

            for (i = 1; i <= state.afn->NumOfLinksMultiZone; ++i) {
                if (state.afn->onceSurfFlag(i)) continue;
                if (state.afn->DisSysCompCVFData(FanNum).AirLoopNum == AirLoopNum) {
                    Tamb = OutDryBulbTempAt(state, state.afn->AirflowNetworkLinkageData(i).NodeHeights[0]);
                    AirDensity = PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, Tamb, state.dataEnvrn->OutHumRat);
                    if (state.afn->DisSysCompCVFData(FanNum).FanTypeNum == FanType_SimpleOnOff &&
                        state.afn->LoopOnOffFanRunTimeFraction(AirLoopNum) < 1.0 &&
                        state.afn->LoopOnOffFanRunTimeFraction(AirLoopNum) > 0.0) {
                        state.afn->linkReport(i).VolFLOW =
                            state.afn->linkReport1(i).FLOW / AirDensity;
                        state.afn->linkReport(i).VolFLOW2 =
                            state.afn->linkReport1(i).FLOW2 / AirDensity;
                    } else {
                        state.afn->linkReport(i).VolFLOW =
                            state.afn->linkReport(i).FLOW / AirDensity;
                        state.afn->linkReport(i).VolFLOW2 =
                            state.afn->linkReport(i).FLOW2 / AirDensity;
                    }
                    state.afn->onceSurfFlag(i) = true;
                }
            }

            if (state.afn->AirflowNetworkNumOfLinks > state.afn->NumOfLinksMultiZone) {
                for (i = state.afn->NumOfLinksMultiZone + 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
                    if (state.afn->onceSurfFlag(i)) continue;
                    if (state.afn->DisSysCompCVFData(FanNum).AirLoopNum == AirLoopNum) {
                        n = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                        M = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                        AirDensity = PsyRhoAirFnPbTdbW(
                            state,
                            (state.afn->AirflowNetworkNodeSimu(n).PZ + state.afn->AirflowNetworkNodeSimu(M).PZ) / 2.0 +
                                state.dataEnvrn->OutBaroPress,
                            (state.afn->AirflowNetworkNodeSimu(n).TZ + state.afn->AirflowNetworkNodeSimu(M).TZ) / 2.0,
                            (state.afn->AirflowNetworkNodeSimu(n).WZ + state.afn->AirflowNetworkNodeSimu(M).WZ) / 2.0);
                        if (state.afn->DisSysCompCVFData(FanNum).FanTypeNum == FanType_SimpleOnOff &&
                            state.afn->LoopOnOffFanRunTimeFraction(AirLoopNum) < 1.0 &&
                            state.afn->LoopOnOffFanRunTimeFraction(AirLoopNum) > 0.0) {
                            state.afn->linkReport(i).VolFLOW =
                                state.afn->linkReport(i).FLOW / AirDensity *
                                (1.0 - state.afn->LoopOnOffFanRunTimeFraction(AirLoopNum));
                            state.afn->linkReport(i).VolFLOW2 =
                                state.afn->linkReport(i).FLOW2 / AirDensity *
                                (1.0 - state.afn->LoopOnOffFanRunTimeFraction(AirLoopNum));
                            state.afn->onceSurfFlag(i) = true;
                        } else {
                            state.afn->linkReport(i).VolFLOW =
                                state.afn->linkReport(i).FLOW / AirDensity;
                            state.afn->linkReport(i).VolFLOW2 =
                                state.afn->linkReport(i).FLOW2 / AirDensity;
                        }
                    }
                }
            }
        }
    }

    void UpdateAirflowNetwork(EnergyPlusData &state,
                              Optional_bool_const FirstHVACIteration) // True when solution technique on first iteration
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   12/10/05
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine update variables used in the AirflowNetwork model.

        // Using/Aliasing
        auto &NumPrimaryAirSys = state.dataHVACGlobal->NumPrimaryAirSys;
        using DataHVACGlobals::VerySmallMassFlow;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int j;
        int n;
        int M;
        int ZN1;
        int ZN2;
        int Node1;
        int Node2;
        int Node3;
        Real64 CpAir;
        Real64 Qsen;
        Real64 Qlat;
        Real64 AirDensity;
        Real64 Tamb;
        Real64 PartLoadRatio;
        Real64 OnOffRatio;
        Real64 NodeMass;
        Real64 AFNMass;
        bool WriteFlag;

        auto &Zone(state.dataHeatBal->Zone);
        auto &Node(state.dataLoopNodes->Node);

        for (auto &e : state.afn->exchangeData) {
            e.SumMCp = 0.0;
            e.SumMCpT = 0.0;
            e.SumMVCp = 0.0;
            e.SumMVCpT = 0.0;
            e.SumMHr = 0.0;
            e.SumMHrW = 0.0;
            e.SumMMCp = 0.0;
            e.SumMMCpT = 0.0;
            e.SumMMHr = 0.0;
            e.SumMMHrW = 0.0;
        }
        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            for (auto &e : state.afn->exchangeData) {
                e.SumMHrCO = 0.0;
                e.SumMMHrCO = 0.0;
            }
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            for (auto &e : state.afn->exchangeData) {
                e.SumMHrGC = 0.0;
                e.SumMMHrGC = 0.0;
            }
        }

        // Calculate sensible and latent loads in each zone from multizone airflows
        if (state.afn->SimulateAirflowNetwork == AirflowNetworkControlMultizone ||
            state.afn->SimulateAirflowNetwork == AirflowNetworkControlMultiADS ||
            (state.afn->SimulateAirflowNetwork == AirflowNetworkControlSimpleADS &&
             state.afn->AirflowNetworkFanActivated)) {
            for (i = 1; i <= state.afn->NumOfLinksMultiZone; ++i) { // Multizone airflow energy
                n = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                M = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                ZN1 = state.afn->AirflowNetworkNodeData(n).EPlusZoneNum;
                ZN2 = state.afn->AirflowNetworkNodeData(M).EPlusZoneNum;
                if (ZN1 > 0 && ZN2 == 0) {
                    // Find a linkage from outdoors to this zone
                    Tamb = Zone(ZN1).OutDryBulbTemp;
                    CpAir = PsyCpAirFnW(state.dataEnvrn->OutHumRat);
                    if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum)
                                .CompTypeNum == iComponentTypeNum::SCR ||
                        state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum)
                                .CompTypeNum == iComponentTypeNum::SEL) {
                        state.afn->exchangeData(ZN1).SumMCp +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir;
                        state.afn->exchangeData(ZN1).SumMCpT +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir * Tamb;
                    } else {
                        state.afn->exchangeData(ZN1).SumMVCp +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir;
                        state.afn->exchangeData(ZN1).SumMVCpT +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir * Tamb;
                    }
                    state.afn->exchangeData(ZN1).SumMHr += state.afn->AirflowNetworkLinkSimu(i).FLOW2;
                    state.afn->exchangeData(ZN1).SumMHrW +=
                        state.afn->AirflowNetworkLinkSimu(i).FLOW2 * state.dataEnvrn->OutHumRat;
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        state.afn->exchangeData(ZN1).SumMHrCO +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW2 * state.dataContaminantBalance->OutdoorCO2;
                    }
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        state.afn->exchangeData(ZN1).SumMHrGC +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW2 * state.dataContaminantBalance->OutdoorGC;
                    }
                }
                if (ZN1 == 0 && ZN2 > 0) {
                    // Find a linkage from outdoors to this zone
                    Tamb = Zone(ZN2).OutDryBulbTemp;
                    CpAir = PsyCpAirFnW(state.dataEnvrn->OutHumRat);
                    if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum)
                                .CompTypeNum == iComponentTypeNum::SCR ||
                        state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum)
                                .CompTypeNum == iComponentTypeNum::SEL) {
                        state.afn->exchangeData(ZN2).SumMCp +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir;
                        state.afn->exchangeData(ZN2).SumMCpT +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir * Tamb;
                    } else {
                        state.afn->exchangeData(ZN2).SumMVCp +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir;
                        state.afn->exchangeData(ZN2).SumMVCpT +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir * Tamb;
                    }
                    state.afn->exchangeData(ZN2).SumMHr += state.afn->AirflowNetworkLinkSimu(i).FLOW;
                    state.afn->exchangeData(ZN2).SumMHrW +=
                        state.afn->AirflowNetworkLinkSimu(i).FLOW * state.dataEnvrn->OutHumRat;
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        state.afn->exchangeData(ZN2).SumMHrCO +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW * state.dataContaminantBalance->OutdoorCO2;
                    }
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        state.afn->exchangeData(ZN2).SumMHrGC +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW * state.dataContaminantBalance->OutdoorGC;
                    }
                }
                if (ZN1 > 0 && ZN2 > 0) {
                    // Find a linkage from outdoors to this zone
                    CpAir = PsyCpAirFnW(state.afn->ANZW(ZN1));
                    state.afn->exchangeData(ZN2).SumMMCp +=
                        state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir;
                    state.afn->exchangeData(ZN2).SumMMCpT +=
                        state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir * state.afn->ANZT(ZN1);
                    state.afn->exchangeData(ZN2).SumMMHr += state.afn->AirflowNetworkLinkSimu(i).FLOW;
                    state.afn->exchangeData(ZN2).SumMMHrW +=
                        state.afn->AirflowNetworkLinkSimu(i).FLOW * state.afn->ANZW(ZN1);
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        state.afn->exchangeData(ZN2).SumMMHrCO +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW * state.afn->ANCO(ZN1);
                    }
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        state.afn->exchangeData(ZN2).SumMMHrGC +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW * state.afn->ANGC(ZN1);
                    }
                    CpAir = PsyCpAirFnW(state.afn->ANZW(ZN2));
                    state.afn->exchangeData(ZN1).SumMMCp +=
                        state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir;
                    state.afn->exchangeData(ZN1).SumMMCpT +=
                        state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir * state.afn->ANZT(ZN2);
                    state.afn->exchangeData(ZN1).SumMMHr += state.afn->AirflowNetworkLinkSimu(i).FLOW2;
                    state.afn->exchangeData(ZN1).SumMMHrW +=
                        state.afn->AirflowNetworkLinkSimu(i).FLOW2 * state.afn->ANZW(ZN2);
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        state.afn->exchangeData(ZN1).SumMMHrCO +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW2 * state.afn->ANCO(ZN2);
                    }
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        state.afn->exchangeData(ZN1).SumMMHrGC +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW2 * state.afn->ANGC(ZN2);
                    }
                }
            }
        }
        // End of update of multizone airflow calculations

        // Initialize these values
        for (auto &e : state.afn->exchangeData) {
            e.LeakSen = 0.0;
            e.CondSen = 0.0;
            e.LeakLat = 0.0;
            e.DiffLat = 0.0;
            e.MultiZoneSen = 0.0;
            e.MultiZoneLat = 0.0;
            e.RadGain = 0.0;
        }

        // Rewrite AirflowNetwork airflow rate
        for (i = 1; i <= state.afn->NumOfLinksMultiZone; ++i) {
            Tamb = OutDryBulbTempAt(state, state.afn->AirflowNetworkLinkageData(i).NodeHeights[0]);
            AirDensity = PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, Tamb, state.dataEnvrn->OutHumRat);
            state.afn->AirflowNetworkLinkSimu(i).VolFLOW = state.afn->AirflowNetworkLinkSimu(i).FLOW / AirDensity;
            state.afn->AirflowNetworkLinkSimu(i).VolFLOW2 = state.afn->AirflowNetworkLinkSimu(i).FLOW2 / AirDensity;
        }

        for (std::size_t i = 0; i < state.afn->linkReport.size(); ++i) {
            auto &r(state.afn->linkReport[i]);
            auto &s(state.afn->AirflowNetworkLinkSimu[i]);
            r.FLOW = s.FLOW;
            r.FLOW2 = s.FLOW2;
            r.VolFLOW = s.VolFLOW;
            r.VolFLOW2 = s.VolFLOW2;
        }

        // Save zone loads from multizone calculation for later summation
        bool OnOffFanFlag = false;
        for (i = 1; i <= state.afn->DisSysNumOfCVFs; i++) {
            if (state.afn->DisSysCompCVFData(i).FanTypeNum == FanType_SimpleOnOff) {
                OnOffFanFlag = true;
                break;
            }
        }
        if (present(FirstHVACIteration)) {
            if (FirstHVACIteration && OnOffFanFlag) {
                state.afn->multiExchangeData = state.afn->exchangeData;
                for (i = 1; i <= state.afn->AirflowNetworkNumOfZones; ++i) {
                    state.afn->nodeReport(i).PZ = state.afn->AirflowNetworkNodeSimu(i).PZ;
                    state.afn->nodeReport(i).PZOFF = state.afn->AirflowNetworkNodeSimu(i).PZ;
                    state.afn->nodeReport(i).PZON = 0.0;
                }
                for (i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) {
                    state.afn->linkReport1(i).FLOW = state.afn->AirflowNetworkLinkSimu(i).FLOW;
                    state.afn->linkReport1(i).FLOW2 = state.afn->AirflowNetworkLinkSimu(i).FLOW2;
                    state.afn->linkReport1(i).VolFLOW = state.afn->AirflowNetworkLinkSimu(i).VolFLOW;
                    state.afn->linkReport1(i).VolFLOW2 = state.afn->AirflowNetworkLinkSimu(i).VolFLOW2;
                    state.afn->linkReport1(i).FLOWOFF = state.afn->AirflowNetworkLinkSimu(i).FLOW;
                    state.afn->linkReport1(i).FLOW2OFF = state.afn->AirflowNetworkLinkSimu(i).FLOW2;
                    state.afn->linkReport1(i).VolFLOWOFF = state.afn->AirflowNetworkLinkSimu(i).VolFLOW;
                    state.afn->linkReport1(i).VolFLOW2OFF = state.afn->AirflowNetworkLinkSimu(i).VolFLOW2;
                    state.afn->linkReport1(i).DP = state.afn->AirflowNetworkLinkSimu(i).DP;
                    state.afn->linkReport1(i).DPOFF = state.afn->AirflowNetworkLinkSimu(i).DP;
                    state.afn->linkReport1(i).DPON = 0.0;
                }
            }
        }

        if (!state.afn->AirflowNetworkFanActivated &&
            (state.afn->SimulateAirflowNetwork > AirflowNetworkControlMultizone)) {
            for (i = state.afn->NumOfNodesMultiZone + state.afn->NumOfNodesIntraZone + 1;
                 i <= state.afn->AirflowNetworkNumOfNodes;
                 ++i) {
                state.afn->AirflowNetworkNodeSimu(i).PZ = 0.0;
            }
            for (i = state.afn->AirflowNetworkNumOfSurfaces + 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
                state.afn->AirflowNetworkLinkSimu(i).DP = 0.0;
                state.afn->linkReport(i).FLOW = 0.0;
                state.afn->linkReport(i).FLOW2 = 0.0;
                state.afn->linkReport(i).VolFLOW = 0.0;
                state.afn->linkReport(i).VolFLOW2 = 0.0;
            }
        }

        if (!(state.afn->AirflowNetworkFanActivated &&
              state.afn->SimulateAirflowNetwork > AirflowNetworkControlMultizone))
            return;

        if (state.afn->SimulateAirflowNetwork > AirflowNetworkControlMultizone + 1) {
            for (i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) { // Multizone airflow energy
                n = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                M = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                ZN1 = state.afn->AirflowNetworkNodeData(n).EPlusZoneNum;
                ZN2 = state.afn->AirflowNetworkNodeData(M).EPlusZoneNum;
                // Find a linkage from a zone to outdoors
                if (ZN1 > 0 && ZN2 == 0) {
                    Tamb = Zone(ZN1).OutDryBulbTemp;
                    CpAir = PsyCpAirFnW(state.dataEnvrn->OutHumRat);
                    state.afn->exchangeData(ZN1).MultiZoneSen +=
                        state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir * (Tamb - state.afn->ANZT(ZN1));
                    state.afn->exchangeData(ZN1).MultiZoneLat +=
                        state.afn->AirflowNetworkLinkSimu(i).FLOW2 *
                        (state.dataEnvrn->OutHumRat - state.afn->ANZW(ZN1));
                }
                if (ZN1 == 0 && ZN2 > 0) {
                    Tamb = Zone(ZN2).OutDryBulbTemp;
                    CpAir = PsyCpAirFnW(state.dataEnvrn->OutHumRat);
                    state.afn->exchangeData(ZN2).MultiZoneSen +=
                        state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir * (Tamb - state.afn->ANZT(ZN2));
                    state.afn->exchangeData(ZN2).MultiZoneLat +=
                        state.afn->AirflowNetworkLinkSimu(i).FLOW * (state.dataEnvrn->OutHumRat - state.afn->ANZW(ZN2));
                }

                if (ZN1 > 0 && ZN2 > 0) {
                    if (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0) { // Flow from ZN1 to ZN2
                        CpAir = PsyCpAirFnW(state.afn->ANZW(ZN1));
                        state.afn->exchangeData(ZN2).MultiZoneSen +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir *
                            (state.afn->ANZT(ZN1) - state.afn->ANZT(ZN2));
                        state.afn->exchangeData(ZN2).MultiZoneLat +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW *
                            (state.afn->ANZW(ZN1) - state.afn->ANZW(ZN2));
                        CpAir = PsyCpAirFnW(state.afn->ANZW(ZN2));
                        state.afn->exchangeData(ZN1).MultiZoneSen +=
                            std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * CpAir *
                            (state.afn->ANZT(ZN2) - state.afn->ANZT(ZN1));
                        state.afn->exchangeData(ZN1).MultiZoneLat +=
                            std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) *
                            (state.afn->ANZW(ZN2) - state.afn->ANZW(ZN1));
                    } else {
                        CpAir = PsyCpAirFnW(state.afn->ANZW(ZN2));
                        state.afn->exchangeData(ZN1).MultiZoneSen +=
                            std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) * CpAir *
                            (state.afn->ANZT(ZN2) - state.afn->ANZT(ZN1));
                        state.afn->exchangeData(ZN1).MultiZoneLat +=
                            std::abs(state.afn->AirflowNetworkLinkSimu(i).FLOW2) *
                            (state.afn->ANZW(ZN2) - state.afn->ANZW(ZN1));
                    }
                }
            }
        }

        int AirLoopNum;
        int FanNum;
        Real64 MaxPartLoadRatio = 0.0;
        Real64 OnOffFanRunTimeFraction = 0.0;
        state.afn->MaxOnOffFanRunTimeFraction = 0.0;
        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            MaxPartLoadRatio = max(MaxPartLoadRatio, state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio);
            state.afn->MaxOnOffFanRunTimeFraction =
                max(state.afn->MaxOnOffFanRunTimeFraction,
                    state.afn->LoopOnOffFanRunTimeFraction(AirLoopNum));
        }
        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            for (FanNum = 1; FanNum <= state.afn->DisSysNumOfCVFs; ++FanNum) {
                if (state.afn->DisSysCompCVFData(FanNum).AirLoopNum == AirLoopNum) break;
            }
            PartLoadRatio = 1.0;
            state.afn->LoopPartLoadRatio(AirLoopNum) = 1.0;
            OnOffFanRunTimeFraction = 1.0;
            state.afn->LoopOnOffFanRunTimeFraction(AirLoopNum) = 1.0;
            // Calculate the part load ratio, can't be greater than 1 for a simple ONOFF fan
            if (state.afn->DisSysCompCVFData(FanNum).FanTypeNum == FanType_SimpleOnOff &&
                Node(state.afn->DisSysCompCVFData(FanNum).InletNode).MassFlowRate > VerySmallMassFlow &&
                state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode == CycFanCycCoil) {
                // Hard code here
                PartLoadRatio = state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio;
                state.afn->LoopPartLoadRatio(AirLoopNum) =
                    state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio;
                OnOffFanRunTimeFraction = max(state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopHeatingCoilMaxRTF,
                                              state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopOnOffFanRTF,
                                              state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopDXCoilRTF);
                state.afn->LoopOnOffFanRunTimeFraction(AirLoopNum) =
                    max(state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopHeatingCoilMaxRTF,
                        state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopOnOffFanRTF,
                        state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopDXCoilRTF);
            }
            state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopHeatingCoilMaxRTF = 0.0;

            if (state.afn->DisSysCompCVFData(FanNum).FanTypeNum == FanType_SimpleOnOff &&
                state.afn->LoopPartLoadRatio(AirLoopNum) < 1.0) {
                for (std::size_t i = 0; i < state.afn->linkReport.size(); ++i) {
                    auto &r(state.afn->linkReport[i]);
                    auto &s(state.afn->AirflowNetworkLinkSimu[i]);
                    auto &t(state.afn->AirflowNetworkLinkageData[i]);
                    if (t.AirLoopNum == AirLoopNum) {
                        r.FLOW = s.FLOW * state.afn->LoopPartLoadRatio(AirLoopNum);
                        r.FLOW2 = s.FLOW2 * state.afn->LoopPartLoadRatio(AirLoopNum);
                        r.VolFLOW = s.VolFLOW * state.afn->LoopPartLoadRatio(AirLoopNum);
                        r.VolFLOW2 = s.VolFLOW2 * state.afn->LoopPartLoadRatio(AirLoopNum);
                    }
                    if (t.AirLoopNum == 0) {
                        r.FLOW = s.FLOW * MaxPartLoadRatio;
                        r.FLOW2 = s.FLOW2 * MaxPartLoadRatio;
                        r.VolFLOW = s.VolFLOW * MaxPartLoadRatio;
                        r.VolFLOW2 = s.VolFLOW2 * MaxPartLoadRatio;
                    }
                }
            }
        }

        // One time warning
        if (state.afn->UpdateAirflowNetworkMyOneTimeFlag) {
            for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
                for (FanNum = 1; FanNum <= state.afn->DisSysNumOfCVFs; ++FanNum) {
                    if (state.afn->DisSysCompCVFData(FanNum).AirLoopNum == AirLoopNum) break;
                }
                if (state.afn->DisSysCompCVFData(FanNum).FanTypeNum == FanType_SimpleOnOff &&
                    state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode == ContFanCycCoil) {
                    OnOffRatio = std::abs((state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOnMassFlowrate -
                                           state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOffMassFlowrate) /
                                          state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOnMassFlowrate);
                    if (OnOffRatio > 0.1) {
                        ShowWarningError(state,
                                         "The absolute percent difference of supply air mass flow rate between HVAC operation and No HVAC operation "
                                         "is above 10% with fan operation mode = ContFanCycCoil.");
                        ShowContinueError(state,
                                          "The added zone loads using the AirflowNetwork model may not be accurate because the zone loads are "
                                          "calculated based on the mass flow rate during HVAC operation.");
                        ShowContinueError(
                            state,
                            format("The mass flow rate during HVAC operation = {:.2R} The mass flow rate during no HVAC operation = {:.2R}",
                                   state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOnMassFlowrate,
                                   state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOffMassFlowrate));
                        state.afn->UpdateAirflowNetworkMyOneTimeFlag = false;
                    }
                }
            }
        }

        // Check mass flow differences in the zone inlet zones and splitter nodes between node and AFN links
        if (state.afn->UpdateAirflowNetworkMyOneTimeFlag1) {
            if ((!state.afn->VAVSystem) && state.dataGlobal->DisplayExtraWarnings) {
                WriteFlag = false;
                for (i = 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
                    Node1 = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                    Node2 = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
                    if (state.afn->AirflowNetworkNodeData(Node1).EPlusTypeNum == iEPlusNodeType::SPI ||
                        state.afn->AirflowNetworkNodeData(Node2).EPlusTypeNum == iEPlusNodeType::SPO ||
                        state.afn->AirflowNetworkNodeData(Node2).EPlusTypeNum == iEPlusNodeType::ZIN) {
                        if (state.afn->AirflowNetworkNodeData(Node1).EPlusTypeNum == iEPlusNodeType::SPI) {
                            Node3 = Node1;
                        } else {
                            Node3 = Node2;
                        }
                        if (state.afn->AirflowNetworkNodeData(Node2).EPlusTypeNum == iEPlusNodeType::ZIN) {
                            if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum)
                                    .EPlusTypeNum == iEPlusComponentType::Invalid)
                                continue;
                        }
                        NodeMass = Node(state.afn->AirflowNetworkNodeData(Node3).EPlusNodeNum).MassFlowRate;
                        AFNMass = state.afn->AirflowNetworkLinkSimu(i).FLOW;
                        if (NodeMass > 0.0 && AFNMass > NodeMass + 0.01) {
                            ShowWarningError(state,
                                             "The mass flow rate difference is found between System Node = '" +
                                                 state.dataLoopNodes->NodeID(state.afn->AirflowNetworkNodeData(Node3).EPlusNodeNum) +
                                                 "' and AFN Link = '" + state.afn->AirflowNetworkLinkageData(i).Name + "'.");
                            ShowContinueError(state,
                                              format("The system node max mass flow rate = {:.3R} kg/s. The AFN node mass flow rate = {:.3R} kg.s.",
                                                     NodeMass,
                                                     AFNMass));
                            WriteFlag = true;
                        }
                    }
                }
                state.afn->UpdateAirflowNetworkMyOneTimeFlag1 = false;
                if (WriteFlag) {
                    ShowWarningError(state,
                                     "Please adjust the rate of Maximum Air Flow Rate field in the terminal objects or duct pressure resistance.");
                }
            } else {
                state.afn->UpdateAirflowNetworkMyOneTimeFlag1 = false;
            }
        }

        // Assign airflows to EPLus nodes
        for (i = 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
            if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).CompTypeNum ==
                    iComponentTypeNum::DWC ||
                state.afn->AirflowNetworkLinkageData(i).VAVTermDamper) {
                // Exclude envelope leakage Crack element
                Node1 = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
                Node2 = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];

                j = state.afn->AirflowNetworkNodeData(Node1).EPlusNodeNum;
                if (j > 0 && state.afn->AirflowNetworkNodeData(Node1).EPlusZoneNum == 0) {
                    Node(j).MassFlowRate =
                        state.afn->AirflowNetworkLinkSimu(i).FLOW *
                        state.afn->LoopPartLoadRatio(state.afn->AirflowNetworkNodeData(Node1).AirLoopNum);
                    if (!state.afn->LoopOnOffFlag(state.afn->AirflowNetworkLinkageData(i).AirLoopNum))
                        Node(j).MassFlowRate = 0.0;
                    if (!(state.afn->AirflowNetworkNodeData(Node1).EPlusTypeNum == iEPlusNodeType::DIN ||
                          state.afn->AirflowNetworkNodeData(Node1).EPlusTypeNum == iEPlusNodeType::DOU)) {
                        Node(j).MassFlowRateMaxAvail = state.afn->AirflowNetworkLinkSimu(i).FLOW *
                                                       state.afn->LoopPartLoadRatio(
                                                           state.afn->AirflowNetworkNodeData(Node1).AirLoopNum);
                        Node(j).MassFlowRateMax = state.afn->AirflowNetworkLinkSimu(i).FLOW;
                    }
                }

                j = state.afn->AirflowNetworkNodeData(Node2).EPlusNodeNum;
                if (j > 0) {
                    Node(j).MassFlowRate =
                        state.afn->AirflowNetworkLinkSimu(i).FLOW *
                        state.afn->LoopPartLoadRatio(state.afn->AirflowNetworkNodeData(Node2).AirLoopNum);
                    if (!state.afn->LoopOnOffFlag(state.afn->AirflowNetworkLinkageData(i).AirLoopNum))
                        Node(j).MassFlowRate = 0.0;
                    if (!(state.afn->AirflowNetworkNodeData(Node2).EPlusTypeNum == iEPlusNodeType::DIN ||
                          state.afn->AirflowNetworkNodeData(Node2).EPlusTypeNum == iEPlusNodeType::DOU)) {
                        Node(j).MassFlowRateMaxAvail = state.afn->AirflowNetworkLinkSimu(i).FLOW *
                                                       state.afn->LoopPartLoadRatio(
                                                           state.afn->AirflowNetworkNodeData(Node2).AirLoopNum);
                        Node(j).MassFlowRateMax = state.afn->AirflowNetworkLinkSimu(i).FLOW;
                    }
                }
            }
        }

        // Assign AirflowNetwork nodal values to Node array
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            j = state.afn->AirflowNetworkNodeData(i).EPlusNodeNum;
            if (j > 0) {
                Node(j).Enthalpy =
                    PsyHFnTdbW(state.afn->AirflowNetworkNodeSimu(i).TZ, state.afn->AirflowNetworkNodeSimu(i).WZ);
                Node(j).Temp = state.afn->AirflowNetworkNodeSimu(i).TZ;
                Node(j).HumRat = state.afn->AirflowNetworkNodeSimu(i).WZ;
                if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                    Node(j).CO2 = state.afn->AirflowNetworkNodeSimu(i).CO2Z;
                }
                if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                    Node(j).GenContam = state.afn->AirflowNetworkNodeSimu(i).GCZ;
                }
            }
        }

        // Calculate sensible loads from forced air flow
        for (i = 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
            Node1 = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
            Node2 = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
            CpAir = PsyCpAirFnW(
                (state.afn->AirflowNetworkNodeSimu(Node1).WZ + state.afn->AirflowNetworkNodeSimu(Node2).WZ) / 2.0);
            // Calculate sensible loads from duct conduction losses and loads from duct radiation
            if (state.afn->AirflowNetworkLinkageData(i).ZoneNum > 0 &&
                state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).CompTypeNum ==
                    iComponentTypeNum::DWC) {
                Qsen = state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir *
                       (state.afn->AirflowNetworkNodeSimu(Node2).TZ - state.afn->AirflowNetworkNodeSimu(Node1).TZ);
                if (state.afn->AirflowNetworkLinkageData(i).LinkageViewFactorObjectNum != 0) {
                    auto &DuctRadObj(state.afn->AirflowNetworkLinkageViewFactorData(
                        state.afn->AirflowNetworkLinkageData(i).LinkageViewFactorObjectNum));
                    Qsen -= DuctRadObj.QRad;
                    state.afn->exchangeData(state.afn->AirflowNetworkLinkageData(i).ZoneNum).RadGain -=
                        DuctRadObj.QRad;
                }
                // When the Airloop is shut off, no duct sensible losses
                if (!state.afn->LoopOnOffFlag(state.afn->AirflowNetworkLinkageData(i).AirLoopNum))
                    Qsen = 0.0;
                state.afn->exchangeData(state.afn->AirflowNetworkLinkageData(i).ZoneNum).CondSen -= Qsen;
            }
            // Calculate sensible leakage losses
            if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).CompTypeNum ==
                    iComponentTypeNum::PLR ||
                state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).CompTypeNum ==
                    iComponentTypeNum::ELR) {
                // Calculate supply leak sensible losses
                if ((state.afn->AirflowNetworkNodeData(Node2).EPlusZoneNum > 0) &&
                    (state.afn->AirflowNetworkNodeData(Node1).EPlusNodeNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    ZN2 = state.afn->AirflowNetworkNodeData(Node2).EPlusZoneNum;
                    Qsen = state.afn->AirflowNetworkLinkSimu(i).FLOW * CpAir *
                           (state.afn->AirflowNetworkNodeSimu(Node1).TZ - state.afn->AirflowNetworkNodeSimu(Node2).TZ);
                    if (!state.afn->LoopOnOffFlag(state.afn->AirflowNetworkLinkageData(i).AirLoopNum))
                        Qsen = 0.0;
                    state.afn->exchangeData(ZN2).LeakSen += Qsen;
                }
                if ((state.afn->AirflowNetworkNodeData(Node1).EPlusZoneNum > 0) &&
                    (state.afn->AirflowNetworkNodeData(Node2).EPlusNodeNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    ZN1 = state.afn->AirflowNetworkNodeData(Node1).EPlusZoneNum;
                    Qsen = state.afn->AirflowNetworkLinkSimu(i).FLOW2 * CpAir *
                           (state.afn->AirflowNetworkNodeSimu(Node2).TZ - state.afn->AirflowNetworkNodeSimu(Node1).TZ);
                    if (!state.afn->LoopOnOffFlag(state.afn->AirflowNetworkLinkageData(i).AirLoopNum))
                        Qsen = 0.0;
                    state.afn->exchangeData(ZN1).LeakSen += Qsen;
                }
            }
        }

        // Calculate latent loads from forced air flow
        for (i = 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
            Node1 = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
            Node2 = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
            // Calculate latent loads from duct conduction losses
            if (state.afn->AirflowNetworkLinkageData(i).ZoneNum > 0 &&
                state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).CompTypeNum ==
                    iComponentTypeNum::DWC) {
                Qlat = state.afn->AirflowNetworkLinkSimu(i).FLOW *
                       (state.afn->AirflowNetworkNodeSimu(Node2).WZ - state.afn->AirflowNetworkNodeSimu(Node1).WZ);
                if (!state.afn->LoopOnOffFlag(state.afn->AirflowNetworkLinkageData(i).AirLoopNum))
                    Qlat = 0.0;
                state.afn->exchangeData(state.afn->AirflowNetworkLinkageData(i).ZoneNum).DiffLat -= Qlat;
            }
            // Calculate latent leakage losses
            if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).CompTypeNum ==
                    iComponentTypeNum::PLR ||
                state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).CompTypeNum ==
                    iComponentTypeNum::ELR) {
                // Calculate supply leak latent losses
                if ((state.afn->AirflowNetworkNodeData(Node2).EPlusZoneNum > 0) &&
                    (state.afn->AirflowNetworkNodeData(Node1).EPlusNodeNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW > 0.0)) {
                    ZN2 = state.afn->AirflowNetworkNodeData(Node2).EPlusZoneNum;
                    Qlat = state.afn->AirflowNetworkLinkSimu(i).FLOW *
                           (state.afn->AirflowNetworkNodeSimu(Node1).WZ - state.afn->AirflowNetworkNodeSimu(Node2).WZ);
                    if (!state.afn->LoopOnOffFlag(state.afn->AirflowNetworkLinkageData(i).AirLoopNum))
                        Qlat = 0.0;
                    state.afn->exchangeData(ZN2).LeakLat += Qlat;
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        state.afn->exchangeData(ZN2).TotalCO2 +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW * (state.afn->AirflowNetworkNodeSimu(Node1).CO2Z -
                                                                                        state.afn->AirflowNetworkNodeSimu(Node2).CO2Z);
                    }
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        state.afn->exchangeData(ZN2).TotalGC +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW * (state.afn->AirflowNetworkNodeSimu(Node1).GCZ -
                                                                                        state.afn->AirflowNetworkNodeSimu(Node2).GCZ);
                    }
                }
                if ((state.afn->AirflowNetworkNodeData(Node1).EPlusZoneNum > 0) &&
                    (state.afn->AirflowNetworkNodeData(Node2).EPlusNodeNum == 0) &&
                    (state.afn->AirflowNetworkLinkSimu(i).FLOW2 > 0.0)) {
                    ZN1 = state.afn->AirflowNetworkNodeData(Node1).EPlusZoneNum;
                    Qlat = state.afn->AirflowNetworkLinkSimu(i).FLOW2 *
                           (state.afn->AirflowNetworkNodeSimu(Node2).WZ - state.afn->AirflowNetworkNodeSimu(Node1).WZ);
                    if (!state.afn->LoopOnOffFlag(state.afn->AirflowNetworkLinkageData(i).AirLoopNum))
                        Qlat = 0.0;
                    state.afn->exchangeData(ZN1).LeakLat += Qlat;
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        state.afn->exchangeData(ZN1).TotalCO2 +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW2 *
                            (state.afn->AirflowNetworkNodeSimu(Node2).CO2Z -
                             state.afn->AirflowNetworkNodeSimu(Node1).CO2Z);
                    }
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        state.afn->exchangeData(ZN1).TotalGC +=
                            state.afn->AirflowNetworkLinkSimu(i).FLOW2 * (state.afn->AirflowNetworkNodeSimu(Node2).GCZ -
                                                                                         state.afn->AirflowNetworkNodeSimu(Node1).GCZ);
                    }
                }
            }
        }

        // Sum all the loads
        for (i = 1; i <= state.dataGlobal->NumOfZones; ++i) {
            state.afn->exchangeData(i).TotalSen = state.afn->exchangeData(i).LeakSen +
                                                                               state.afn->exchangeData(i).CondSen +
                                                                               state.afn->exchangeData(i).RadGain;
            state.afn->exchangeData(i).TotalLat =
                state.afn->exchangeData(i).LeakLat + state.afn->exchangeData(i).DiffLat;
        }

        // Simple ONOFF fan
        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            for (FanNum = 1; FanNum <= state.afn->DisSysNumOfCVFs; ++FanNum) {
                if (state.afn->DisSysCompCVFData(FanNum).AirLoopNum == AirLoopNum) break;
            }
            if (state.afn->DisSysCompCVFData(FanNum).FanTypeNum == FanType_SimpleOnOff && OnOffFanRunTimeFraction < 1.0) {
                for (i = 1; i <= state.dataGlobal->NumOfZones; ++i) {
                    state.afn->exchangeData(i).MultiZoneSen *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).MultiZoneLat *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).LeakSen *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).LeakLat *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).CondSen *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).DiffLat *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).RadGain *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).TotalSen *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).TotalLat *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).SumMCp *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).SumMCpT *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).SumMVCp *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).SumMVCpT *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).SumMHr *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).SumMHrW *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).SumMMCp *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).SumMMCpT *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).SumMMHr *= OnOffFanRunTimeFraction;
                    state.afn->exchangeData(i).SumMMHrW *= OnOffFanRunTimeFraction;
                    if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                        state.afn->exchangeData(i).SumMHrCO *= OnOffFanRunTimeFraction;
                        state.afn->exchangeData(i).SumMMHrCO *= OnOffFanRunTimeFraction;
                    }
                    if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                        state.afn->exchangeData(i).SumMHrGC *= OnOffFanRunTimeFraction;
                        state.afn->exchangeData(i).SumMMHrGC *= OnOffFanRunTimeFraction;
                    }
                }
                if (state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode == CycFanCycCoil) {
                    for (i = 1; i <= state.dataGlobal->NumOfZones; ++i) {
                        state.afn->exchangeData(i).SumMCp +=
                            state.afn->multiExchangeData(i).SumMCp * (1.0 - OnOffFanRunTimeFraction);
                        state.afn->exchangeData(i).SumMCpT +=
                            state.afn->multiExchangeData(i).SumMCpT * (1.0 - OnOffFanRunTimeFraction);
                        state.afn->exchangeData(i).SumMVCp +=
                            state.afn->multiExchangeData(i).SumMVCp * (1.0 - OnOffFanRunTimeFraction);
                        state.afn->exchangeData(i).SumMVCpT +=
                            state.afn->multiExchangeData(i).SumMVCpT * (1.0 - OnOffFanRunTimeFraction);
                        state.afn->exchangeData(i).SumMHr +=
                            state.afn->multiExchangeData(i).SumMHr * (1.0 - OnOffFanRunTimeFraction);
                        state.afn->exchangeData(i).SumMHrW +=
                            state.afn->multiExchangeData(i).SumMHrW * (1.0 - OnOffFanRunTimeFraction);
                        state.afn->exchangeData(i).SumMMCp +=
                            state.afn->multiExchangeData(i).SumMMCp * (1.0 - OnOffFanRunTimeFraction);
                        state.afn->exchangeData(i).SumMMCpT +=
                            state.afn->multiExchangeData(i).SumMMCpT * (1.0 - OnOffFanRunTimeFraction);
                        state.afn->exchangeData(i).SumMMHr +=
                            state.afn->multiExchangeData(i).SumMMHr * (1.0 - OnOffFanRunTimeFraction);
                        state.afn->exchangeData(i).SumMMHrW +=
                            state.afn->multiExchangeData(i).SumMMHrW * (1.0 - OnOffFanRunTimeFraction);
                        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                            state.afn->exchangeData(i).SumMHrCO +=
                                state.afn->multiExchangeData(i).SumMHrCO * (1.0 - OnOffFanRunTimeFraction);
                            state.afn->exchangeData(i).SumMMHrCO +=
                                state.afn->multiExchangeData(i).SumMMHrCO * (1.0 - OnOffFanRunTimeFraction);
                        }
                        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                            state.afn->exchangeData(i).SumMHrGC +=
                                state.afn->multiExchangeData(i).SumMHrGC * (1.0 - OnOffFanRunTimeFraction);
                            state.afn->exchangeData(i).SumMMHrGC +=
                                state.afn->multiExchangeData(i).SumMMHrGC * (1.0 - OnOffFanRunTimeFraction);
                        }
                    }
                }
            }

            if (state.afn->DisSysCompCVFData(FanNum).FanTypeNum == FanType_SimpleOnOff) {
                for (i = 1; i <= state.afn->AirflowNetworkNumOfZones; ++i) {
                    if (state.afn->AirflowNetworkNodeData(i).AirLoopNum == AirLoopNum) {
                        state.afn->nodeReport(i).PZ =
                            state.afn->AirflowNetworkNodeSimu(i).PZ *
                                state.afn->LoopPartLoadRatio(AirLoopNum) +
                            state.afn->nodeReport(i).PZOFF *
                                (1.0 - state.afn->LoopPartLoadRatio(AirLoopNum));
                        state.afn->nodeReport(i).PZON = state.afn->AirflowNetworkNodeSimu(i).PZ;
                    }
                }
                for (i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) {
                    PartLoadRatio = MaxPartLoadRatio;
                    for (j = 1; j <= state.afn->AirflowNetworkNumOfZones; ++j) {
                        if (state.afn->MultizoneZoneData(j).ZoneNum == state.afn->MultizoneSurfaceData(i).ZonePtr) {
                            if (state.afn->AirflowNetworkNodeData(j).AirLoopNum == AirLoopNum) {
                                PartLoadRatio = state.afn->LoopPartLoadRatio(AirLoopNum);
                                break;
                            }
                        }
                    }
                    state.afn->linkReport1(i).FLOW =
                        state.afn->AirflowNetworkLinkSimu(i).FLOW * PartLoadRatio +
                        state.afn->linkReport1(i).FLOWOFF * (1.0 - PartLoadRatio);
                    state.afn->linkReport1(i).FLOW2 =
                        state.afn->AirflowNetworkLinkSimu(i).FLOW2 * PartLoadRatio +
                        state.afn->linkReport1(i).FLOW2OFF * (1.0 - PartLoadRatio);
                    state.afn->linkReport1(i).VolFLOW =
                        state.afn->AirflowNetworkLinkSimu(i).VolFLOW * PartLoadRatio +
                        state.afn->linkReport1(i).VolFLOWOFF * (1.0 - PartLoadRatio);
                    state.afn->linkReport1(i).VolFLOW2 =
                        state.afn->AirflowNetworkLinkSimu(i).VolFLOW2 * PartLoadRatio +
                        state.afn->linkReport1(i).VolFLOW2OFF * (1.0 - PartLoadRatio);
                    state.afn->linkReport1(i).DP =
                        state.afn->AirflowNetworkLinkSimu(i).DP * PartLoadRatio +
                        state.afn->linkReport1(i).DPOFF * (1.0 - PartLoadRatio);
                    state.afn->linkReport1(i).DPON = state.afn->AirflowNetworkLinkSimu(i).DP;
                }
            }
        }

        // Save values
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            state.afn->AirflowNetworkNodeSimu(i).TZlast = state.afn->AirflowNetworkNodeSimu(i).TZ;
            state.afn->AirflowNetworkNodeSimu(i).WZlast = state.afn->AirflowNetworkNodeSimu(i).WZ;
            if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
                state.afn->AirflowNetworkNodeSimu(i).CO2Zlast = state.afn->AirflowNetworkNodeSimu(i).CO2Z;
            }
            if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
                state.afn->AirflowNetworkNodeSimu(i).GCZlast = state.afn->AirflowNetworkNodeSimu(i).GCZ;
            }
        }
    }

    void AirflowNetworkVentingControl(EnergyPlusData &state,
                                      int const i,       // AirflowNetwork surface number
                                      Real64 &OpenFactor // Window or door opening factor (used to calculate airflow)
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Winkelmann
        //       DATE WRITTEN   April 2003
        //       MODIFIED       Feb 2004, FCW: allow venting control of interior window/door
        //       MODIFIED       Nov. 2005, LG: to fit the requirement for AirflowNetwork Model
        //       RE-ENGINEERED

        // PURPOSE OF THIS SUBROUTINE:
        // Determines the venting opening factor for an exterior or interior window or door
        // as determined by the venting control method.

        // Using/Aliasing
        using ScheduleManager::GetCurrentScheduleValue;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 VentTemp;                // Venting temperature (C)
        Real64 ZoneAirEnthalpy;         // Enthalpy of zone air (J/kg)
        Real64 OpenFactorMult;          // Window/door opening modulation multiplier on venting open factor
        Real64 DelTemp;                 // Inside-outside air temperature difference (K)
        Real64 DelEnthal;               // Inside-outside air enthalpy difference (J/kg)
        int IZ;                         // AirflowNetwork zone number
        int ZoneNum;                    // EnergyPlus zone number
        int SurfNum;                    // Heat transfer surface number
        Real64 LimValVentOpenFacMult;   // Limiting value of venting opening factor multiplier
        Real64 LowerValInOutTempDiff;   // Lower value of inside/outside temperature difference for opening factor modulation
        Real64 UpperValInOutTempDiff;   // Upper value of inside/outside temperature difference for opening factor modulation
        Real64 LowerValInOutEnthalDiff; // Lower value of inside/outside enthalpy difference for opening factor modulation
        Real64 UpperValInOutEnthalDiff; // Upper value of inside/outside enthalpy difference for opening factor modulation
        bool VentingAllowed;            // True if venting schedule allows venting
        int VentCtrlNum;                // Venting control strategy 1: Temperature control; 2: Enthalpy control
        Real64 VentingSchVal;           // Current time step value of venting schedule
        Real64 Tamb;                    // Outdoor dry bulb temperature at surface centroid height
        int PeopleInd;

        if (state.afn->MultizoneSurfaceData(i).EMSOpenFactorActuated) { // EMS sets value to use
            OpenFactor = state.afn->MultizoneSurfaceData(i).EMSOpenFactor;
            SurfNum = state.afn->MultizoneSurfaceData(i).SurfNum;
            if (state.afn->MultizoneSurfaceData(i).Factor > 0.0) {
                state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = OpenFactor / state.afn->MultizoneSurfaceData(i).Factor;
            } else {
                state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = OpenFactor;
            }
            return;
        }

        SurfNum = state.afn->MultizoneSurfaceData(i).SurfNum;

        state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = -1.0;

        // Get venting temperature and venting strategy for exterior window or door
        // and determine whether venting is allowed

        state.dataSurface->SurfWinVentingAvailabilityRep(SurfNum) = 1.0;
        VentingAllowed = true;
        IZ = state.afn->MultizoneSurfaceData(i).NodeNums[0];
        // Revise for RoomAirflowNetwork model
        if (state.afn->MultizoneSurfaceData(i).RAFNflag) IZ = state.afn->MultizoneSurfaceData(i).ZonePtr;
        ZoneNum = state.afn->MultizoneZoneData(IZ).ZoneNum;

        // Note in the following that individual venting control for a window/door takes
        // precedence over zone-level control
        if (state.afn->MultizoneSurfaceData(i).IndVentControl) {
            VentTemp = GetCurrentScheduleValue(state, state.afn->MultizoneSurfaceData(i).VentSchNum);
            VentCtrlNum = state.afn->MultizoneSurfaceData(i).VentSurfCtrNum;
            if (state.afn->MultizoneSurfaceData(i).VentingSchNum > 0) {
                VentingSchVal = GetCurrentScheduleValue(state, state.afn->MultizoneSurfaceData(i).VentingSchNum);
                if (VentingSchVal <= 0.0) {
                    VentingAllowed = false;
                    state.dataSurface->SurfWinVentingAvailabilityRep(SurfNum) = 0.0;
                }
            }
        } else {
            // Zone level only by Gu on Nov. 8, 2005
            VentTemp = GetCurrentScheduleValue(state, state.afn->MultizoneZoneData(IZ).VentSchNum);
            VentCtrlNum = state.afn->MultizoneZoneData(IZ).VentCtrNum;
            if (state.afn->MultizoneZoneData(IZ).VentingSchNum > 0) {
                VentingSchVal = GetCurrentScheduleValue(state, state.afn->MultizoneZoneData(IZ).VentingSchNum);
                if (VentingSchVal <= 0.0) {
                    VentingAllowed = false;
                    state.dataSurface->SurfWinVentingAvailabilityRep(SurfNum) = 0.0;
                }
            }
        }

        state.dataSurface->SurfWinInsideTempForVentingRep(SurfNum) = VentTemp;
        OpenFactor = 0.0;

        // Venting based on inside-outside air temperature difference

        if ((VentCtrlNum == VentControlType::Temp || VentCtrlNum == VentControlType::AdjTemp) && VentingAllowed) {
            Tamb = state.dataSurface->SurfOutDryBulbTemp(SurfNum);
            // Check whether this surface is an interior wall or not. If Yes, use adjacent zone conditions
            if (VentCtrlNum == VentControlType::AdjTemp && state.afn->MultizoneSurfaceData(i).IndVentControl) {
                Tamb = state.afn->ANZT(
                    state.afn->MultizoneZoneData(state.afn->MultizoneSurfaceData(i).NodeNums[1]).ZoneNum);
            }
            if (state.afn->ANZT(ZoneNum) > Tamb && state.afn->ANZT(ZoneNum) > VentTemp) {
                OpenFactor = state.afn->MultizoneSurfaceData(i).Factor;
                state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = 1.0;
                // Modulation of OpenFactor
                if (state.afn->MultizoneSurfaceData(i).IndVentControl) {
                    LimValVentOpenFacMult = state.afn->MultizoneSurfaceData(i).ModulateFactor;
                    LowerValInOutTempDiff = state.afn->MultizoneSurfaceData(i).LowValueTemp;
                    UpperValInOutTempDiff = state.afn->MultizoneSurfaceData(i).UpValueTemp;
                } else {
                    LimValVentOpenFacMult = state.afn->MultizoneZoneData(IZ).OpenFactor;
                    LowerValInOutTempDiff = state.afn->MultizoneZoneData(IZ).LowValueTemp;
                    UpperValInOutTempDiff = state.afn->MultizoneZoneData(IZ).UpValueTemp;
                }
                if (LimValVentOpenFacMult != 1.0) {
                    DelTemp = state.afn->ANZT(ZoneNum) - Tamb;
                    if (DelTemp <= LowerValInOutTempDiff) {
                        OpenFactorMult = 1.0;
                    } else if (DelTemp >= UpperValInOutTempDiff) {
                        OpenFactorMult = LimValVentOpenFacMult;
                    } else {
                        OpenFactorMult =
                            LimValVentOpenFacMult +
                            ((UpperValInOutTempDiff - DelTemp) / (UpperValInOutTempDiff - LowerValInOutTempDiff)) * (1 - LimValVentOpenFacMult);
                    }
                    OpenFactor *= OpenFactorMult;
                    state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = OpenFactorMult;
                }
            } else {
                OpenFactor = 0.0;
                state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = -1.0;
            }
        }

        // Venting based on inside-outside air enthalpy difference

        if ((VentCtrlNum == VentControlType::Enth || VentCtrlNum == VentControlType::AdjEnth) && VentingAllowed) {
            ZoneAirEnthalpy = PsyHFnTdbW(state.afn->ANZT(ZoneNum), state.afn->ANZW(ZoneNum));
            // Check whether this surface is an interior wall or not. If Yes, use adjacent zone conditions
            if (VentCtrlNum == VentControlType::AdjEnth && state.afn->MultizoneSurfaceData(i).IndVentControl) {
                state.dataEnvrn->OutEnthalpy = PsyHFnTdbW(
                    state.afn->ANZT(
                        state.afn->MultizoneZoneData(state.afn->MultizoneSurfaceData(i).NodeNums[1]).ZoneNum),
                    state.afn->ANZW(
                        state.afn->MultizoneZoneData(state.afn->MultizoneSurfaceData(i).NodeNums[1]).ZoneNum));
            }
            if (ZoneAirEnthalpy > state.dataEnvrn->OutEnthalpy && state.afn->ANZT(ZoneNum) > VentTemp) {
                OpenFactor = state.afn->MultizoneSurfaceData(i).Factor;
                // Modulation of OpenFactor
                if (state.afn->MultizoneSurfaceData(i).IndVentControl) {
                    LimValVentOpenFacMult = state.afn->MultizoneSurfaceData(i).ModulateFactor;
                    LowerValInOutEnthalDiff = state.afn->MultizoneSurfaceData(i).LowValueEnth;
                    UpperValInOutEnthalDiff = state.afn->MultizoneSurfaceData(i).UpValueEnth;
                } else {
                    LimValVentOpenFacMult = state.afn->MultizoneZoneData(IZ).OpenFactor;
                    LowerValInOutEnthalDiff = state.afn->MultizoneZoneData(IZ).LowValueEnth;
                    UpperValInOutEnthalDiff = state.afn->MultizoneZoneData(IZ).UpValueEnth;
                }
                state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = 1.0;

                if (LimValVentOpenFacMult != 1.0) {
                    DelEnthal = ZoneAirEnthalpy - state.dataEnvrn->OutEnthalpy;
                    if (DelEnthal <= LowerValInOutEnthalDiff) {
                        OpenFactorMult = 1.0;
                    } else if (DelEnthal >= UpperValInOutEnthalDiff) {
                        OpenFactorMult = LimValVentOpenFacMult;
                    } else {
                        OpenFactorMult =
                            LimValVentOpenFacMult + ((UpperValInOutEnthalDiff - DelEnthal) / (UpperValInOutEnthalDiff - LowerValInOutEnthalDiff)) *
                                                        (1 - LimValVentOpenFacMult);
                    }
                    OpenFactor *= OpenFactorMult;
                    state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = OpenFactorMult;
                }
            } else {
                OpenFactor = 0.0;
                state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = -1.0;
            }
        }

        // Constant venting (opening factor as specified in IDF) - C-PH - added by Philip Haves 3/8/01
        // subject to venting availability

        if (VentCtrlNum == VentControlType::Const && VentingAllowed) { // Constant
            OpenFactor = state.afn->MultizoneSurfaceData(i).Factor;
            state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = 1.0;
        }

        if (VentCtrlNum == VentControlType::ASH55) {
            if (VentingAllowed && (!state.dataGlobal->BeginEnvrnFlag) && (!state.dataGlobal->WarmupFlag)) {
                PeopleInd = state.afn->MultizoneZoneData(IZ).ASH55PeopleInd;
                if (PeopleInd > 0 && state.dataThermalComforts->ThermalComfortData(PeopleInd).ThermalComfortAdaptiveASH5590 != -1) {
                    if (state.dataThermalComforts->ThermalComfortData(PeopleInd).ThermalComfortOpTemp >
                        state.dataThermalComforts->ThermalComfortData(PeopleInd).TComfASH55) {
                        OpenFactor = state.afn->MultizoneSurfaceData(i).Factor;
                        state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = 1.0;
                    } else {
                        OpenFactor = 0.0;
                    }
                } else {
                    OpenFactor = 0.0;
                }
            } else {
                OpenFactor = 0.0;
            }
        }

        if (VentCtrlNum == VentControlType::CEN15251) {
            if (VentingAllowed && (!state.dataGlobal->BeginEnvrnFlag) && (!state.dataGlobal->WarmupFlag)) {
                PeopleInd = state.afn->MultizoneZoneData(IZ).CEN15251PeopleInd;
                if (PeopleInd > 0 && state.dataThermalComforts->ThermalComfortData(PeopleInd).ThermalComfortAdaptiveCEN15251CatI != -1) {
                    if (state.dataThermalComforts->ThermalComfortData(PeopleInd).ThermalComfortOpTemp >
                        state.dataThermalComforts->ThermalComfortData(PeopleInd).TComfCEN15251) {
                        OpenFactor = state.afn->MultizoneSurfaceData(i).Factor;
                        state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = 1.0;
                    } else {
                        OpenFactor = 0.0;
                    }
                } else {
                    OpenFactor = 0.0;
                }
            } else {
                OpenFactor = 0.0;
            }
        }

        // No venting, i.e, window/door always closed - added YJH 8 Aug 02

        if (VentCtrlNum == VentControlType::NoVent) { // Novent
            OpenFactor = 0.0;
            state.dataSurface->SurfWinVentingOpenFactorMultRep(SurfNum) = -1.0;
        }
    }

    void AssignFanAirLoopNum(EnergyPlusData &state)
    {
        // Assign the system Fan AirLoop Number based on the zone inlet node

        for (int i = 1; i <= state.afn->AirflowNetworkNumOfZones; i++) {
            for (int j = 1; j <= state.dataGlobal->NumOfZones; j++) {
                if (!state.dataZoneEquip->ZoneEquipConfig(j).IsControlled) continue;
                if ((state.afn->MultizoneZoneData(i).ZoneNum == j) && (state.dataZoneEquip->ZoneEquipConfig(j).NumInletNodes > 0)) {
                    for (int k = 1; k <= state.afn->DisSysNumOfCVFs; k++) {
                        if (state.afn->DisSysCompCVFData(k).AirLoopNum == 0) {
                            state.afn->DisSysCompCVFData(k).AirLoopNum =
                                state.dataZoneEquip->ZoneEquipConfig(j).InletNodeAirLoopNum(1);
                        }
                    }
                }
            }
        }
    }

    void ValidateDistributionSystem(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Oct. 2005
        //       MODIFIED       L. Gu, Jan. 2009: allow a desuperheater coil and three heat exchangers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine validates the inputs of distribution system, since node data from a primary airloop
        // are not available in the first call during reading input data of airflownetwork objects.
        // Note: this routine shouldn't be called more than once

        // Using/Aliasing
        using BranchNodeConnections::GetNodeConnectionType;
        using MixedAir::GetNumOAMixers;
        using MixedAir::GetOAMixerInletNodeNumber;
        using MixedAir::GetOAMixerReliefNodeNumber;
        using SingleDuct::GetHVACSingleDuctSysIndex;
        using namespace DataLoopNode;
        auto &NumPrimaryAirSys = state.dataHVACGlobal->NumPrimaryAirSys;
        using DXCoils::SetDXCoilAirLoopNumber;
        using Fans::SetFanAirLoopNumber;
        using HeatingCoils::SetHeatingCoilAirLoopNumber;
        using HVACStandAloneERV::GetStandAloneERVNodeNumber;
        using SplitterComponent::GetSplitterNodeNumbers;
        using SplitterComponent::GetSplitterOutletNumber;
        using WaterThermalTanks::GetHeatPumpWaterHeaterNodeNumber;
        using ZoneDehumidifier::GetZoneDehumidifierNodeNumber;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("ValidateDistributionSystem: "); // include trailing blank space

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int j;
        int k;
        int n;
        int S1;
        int S2;
        int R1;
        int R2;
        bool LocalError;
        Array1D_bool NodeFound;

        bool ErrorsFound(false);
        bool IsNotOK(false);
        bool errFlag(false);
        Array1D<DataLoopNode::ConnectionType> NodeConnectionType; // Specifies the type of node connection
        std::string CurrentModuleObject;

        bool HPWHFound(false);          // Flag for HPWH identification
        bool StandaloneERVFound(false); // Flag for Standalone ERV (ZoneHVAC:EnergyRecoveryVentilator) identification

        // Validate supply and return connections
        NodeFound.dimension(state.dataLoopNodes->NumOfNodes, false);
        // Validate inlet and outlet nodes for zone exhaust fans
        for (i = 1; i <= state.afn->AirflowNetworkNumOfExhFan; ++i) {
            NodeFound(state.afn->MultizoneCompExhaustFanData(i).InletNode) = true;
            NodeFound(state.afn->MultizoneCompExhaustFanData(i).OutletNode) = true;
        }
        // Validate EPlus Node names and types
        for (i = 1; i <= state.afn->DisSysNumOfNodes; ++i) {
            if (UtilityRoutines::SameString(state.afn->DisSysNodeData(i).EPlusName, "") ||
                UtilityRoutines::SameString(state.afn->DisSysNodeData(i).EPlusName, "Other"))
                continue;
            LocalError = false;
            for (j = 1; j <= state.dataLoopNodes->NumOfNodes; ++j) { // NodeID
                if (state.afn->DisSysNodeData(i).EPlusName == state.dataLoopNodes->NodeID(j)) {
                    state.afn->DisSysNodeData(i).AirLoopNum = GetAirLoopNumber(state, j);
                    if (state.afn->DisSysNodeData(i).AirLoopNum == 0) {
                        ShowSevereError(state,
                                        format(RoutineName) + "The Node or Component Name defined in " +
                                            state.afn->DisSysNodeData(i).Name + " is not found in the AirLoopHVAC.");
                        ShowContinueError(state,
                                          "The entered name is " + state.afn->DisSysNodeData(i).EPlusName +
                                              " in an AirflowNetwork:Distribution:Node object.");
                        ErrorsFound = true;
                    }
                    state.afn->DisSysNodeData(i).EPlusNodeNum = j;
                    state.afn->AirflowNetworkNodeData(state.afn->NumOfNodesMultiZone + i).EPlusNodeNum = j;
                    state.afn->AirflowNetworkNodeData(state.afn->NumOfNodesMultiZone + i).AirLoopNum =
                        state.afn->DisSysNodeData(i).AirLoopNum;
                    NodeFound(j) = true;
                    LocalError = true;
                    break;
                }
            }
            // Check outdoor air node
            if (UtilityRoutines::SameString(state.afn->DisSysNodeData(i).EPlusType, "OutdoorAir:NodeList") ||
                UtilityRoutines::SameString(state.afn->DisSysNodeData(i).EPlusType, "OutdoorAir:Node")) {
                if (!LocalError) {
                    ShowSevereError(state,
                                    format(RoutineName) + "The Node or Component Name defined in " +
                                        state.afn->DisSysNodeData(i).Name + " is not found in the " +
                                        state.afn->DisSysNodeData(i).EPlusType);
                    ShowContinueError(state,
                                      "The entered name is " + state.afn->DisSysNodeData(i).EPlusName +
                                          " in an AirflowNetwork:Distribution:Node object.");
                    ErrorsFound = true;
                }
            }
            if (state.afn->DisSysNodeData(i).EPlusNodeNum == 0) {
                ShowSevereError(state,
                                format(RoutineName) + "Primary Air Loop Node is not found in AIRFLOWNETWORK:DISTRIBUTION:NODE = " +
                                    state.afn->DisSysNodeData(i).Name);
                ErrorsFound = true;
            }
        }

        // Determine node numbers for zone inlets
        for (i = 1; i <= state.dataGlobal->NumOfZones; ++i) {
            if (!state.dataZoneEquip->ZoneEquipConfig(i).IsControlled) continue;
            for (j = 1; j <= state.dataZoneEquip->ZoneEquipConfig(i).NumInletNodes; ++j) {
                for (k = 1; k <= state.afn->AirflowNetworkNumOfNodes; ++k) {
                    if (state.dataZoneEquip->ZoneEquipConfig(i).InletNode(j) == state.afn->AirflowNetworkNodeData(k).EPlusNodeNum) {
                        state.afn->AirflowNetworkNodeData(k).EPlusTypeNum = iEPlusNodeType::ZIN;
                        break;
                    }
                }
            }
        }

        // Eliminate node not related to AirLoopHVAC
        for (k = 1; k <= state.dataBranchNodeConnections->NumOfNodeConnections; ++k) {
            if (NodeFound(state.dataBranchNodeConnections->NodeConnections(k).NodeNumber)) continue;
            if (state.dataBranchNodeConnections->NodeConnections(k).FluidStream == NodeInputManager::CompFluidStream::Secondary) {
                NodeFound(state.dataBranchNodeConnections->NodeConnections(k).NodeNumber) = true;
            }
        }

        // Eliminate nodes with fluidtype = water
        for (k = 1; k <= state.dataLoopNodes->NumOfNodes; ++k) {
            if (NodeFound(k)) continue;
            if (state.dataLoopNodes->Node(k).FluidType == DataLoopNode::NodeFluidType::Water) {
                NodeFound(k) = true;
            }
        }

        // Eliminate local external air node for network
        for (k = 1; k <= state.dataLoopNodes->NumOfNodes; ++k) {
            if (NodeFound(k)) continue;
            if (state.dataLoopNodes->Node(k).IsLocalNode) NodeFound(k) = true;
        }

        // Ensure all the nodes used in Eplus are a subset of AirflowNetwork Nodes
        for (i = 1; i <= state.dataLoopNodes->NumOfNodes; ++i) {
            if (NodeFound(i)) continue;
            // Skip the inlet and outlet nodes of zone dehumidifiers
            if (GetZoneDehumidifierNodeNumber(state, i)) NodeFound(i) = true;

            if (state.afn->AirflowNetworkSimu.AllowSupportZoneEqp) {
                // Skip HPWH nodes that don't have to be included in the AFN
                if (GetHeatPumpWaterHeaterNodeNumber(state, i)) {
                    NodeFound(i) = true;
                    HPWHFound = true;
                }

                // Skip Standalone ERV nodes that don't have to be included in the AFN
                if (GetStandAloneERVNodeNumber(state, i)) {
                    NodeFound(i) = true;
                    StandaloneERVFound = true;
                }
            }

            for (j = 1; j <= state.dataGlobal->NumOfZones; ++j) {
                if (!state.dataZoneEquip->ZoneEquipConfig(j).IsControlled) continue;
                if (state.dataZoneEquip->ZoneEquipConfig(j).ZoneNode == i) {
                    if (state.dataZoneEquip->ZoneEquipConfig(j).ActualZoneNum > state.afn->AirflowNetworkNumOfNodes) {
                        ShowSevereError(state,
                                        format(RoutineName) + "'" + state.dataLoopNodes->NodeID(i) +
                                            "' is not defined as an AirflowNetwork:Distribution:Node object.");
                        ShowContinueError(state,
                                          "This Node is the zone air node for Zone '" + state.dataZoneEquip->ZoneEquipConfig(j).ZoneName + "'.");
                        ErrorsFound = true;
                    } else {
                        NodeFound(i) = true;
                        state.afn->AirflowNetworkNodeData(state.dataZoneEquip->ZoneEquipConfig(j).ActualZoneNum).EPlusNodeNum = i;
                    }
                    break;
                }
            }

            //   skip nodes that are not part of an airflow network

            //     DX COIL CONDENSER NODE TEST:
            //     Outside air nodes are used for DX coil condenser inlet nodes, these are specified in an outside air node or
            //     OutdoorAir:NodeList object (and classified with NodeConnectionType as OutsideAir). In addition,
            //     this same node is specified in a Coil:DX:CoolingBypassFactorEmpirical object (and classified with
            //     NodeConnectionType as OutsideAirReference). In the NodeConnectionType structure, both of these nodes have a
            //     unique index but have the same node number. The Outside Air Node will usually be listed first. Search for all
            //     indexes with the same node number and check if it is classified as NodeConnectionType = OutsideAirReference.
            //     Mark this node as found since it is not used in an airflownetwork simulation.
            //     Example (using AirflowNetwork_MultiZone_SmallOffice.idf with a single OA Mixer):
            //             (the example shown below is identical to AirflowNetwork_SimpleHouse.idf with no OA Mixer except
            //              that the NodeConnections indexes are (7) and (31), respectively and the NodeNumber = 6)
            //   The GetNodeConnectionType CALL below returns DataLoopNode::NodeConnectionType::OutsideAir = 7 and
            //   DataLoopNode::NodeConnectionType::OutsideAirReference = 14.
            //     NodeConnections info from OUTSIDE AIR NODE object read:
            //     NodeConnections(9)NodeNumber      = 10
            //     NodeConnections(9)NodeName        = ACDXCOIL 1 CONDENSER NODE
            //     NodeConnections(9)ObjectType      = OUTSIDE AIR NODE
            //     NodeConnections(9)ObjectName      = OUTSIDE AIR NODE
            //     NodeConnections(9)ConnectionType  = OutsideAir
            //     NodeConnections info from Coil:DX:CoolingBypassFactorEmpirical object read:
            //     NodeConnections(64)NodeNumber     = 10
            //     NodeConnections(64)NodeName       = ACDXCOIL 1 CONDENSER NODE
            //     NodeConnections(64)ObjectType     = COIL:DX:COOLINGBYPASSFACTOREMPIRICAL
            //     NodeConnections(64)ObjectName     = ACDXCOIL 1
            //     NodeConnections(64)ConnectionType = OutsideAirReference

            errFlag = false;
            GetNodeConnectionType(state, i, NodeConnectionType, errFlag); // Gets all connection types for a given node number
            if (errFlag) {
                ShowContinueError(state, "...occurs in Airflow Network simulation.");
            } else {
                //   skip nodes for air cooled condensers
                for (j = 1; j <= isize(NodeConnectionType); ++j) {
                    if (NodeConnectionType(j) == DataLoopNode::ConnectionType::OutsideAirReference) {
                        NodeFound(i) = true;
                    }
                }
            }

            if (!NodeFound(i)) {
                // Check if this node is the OA relief node. For the time being, OA relief node is not used
                if (GetNumOAMixers(state) > 1) {
                    //                        ShowSevereError(state,  format(RoutineName) + "Only one OutdoorAir:Mixer is allowed in the
                    // AirflowNetwork model." );                         ErrorsFound = true;
                    int OAFanNum;
                    int OARelNum;
                    int OAMixerNum;

                    for (OAFanNum = 1; OAFanNum <= state.afn->NumOfOAFans; ++OAFanNum) {
                        state.afn->DisSysCompOutdoorAirData(OAFanNum).InletNode =
                            GetOAMixerInletNodeNumber(state, state.afn->DisSysCompOutdoorAirData(OAFanNum).OAMixerNum);
                        //                            NodeFound( state.afn->DisSysCompOutdoorAirData( OAFanNum ).InletNode
                        //                            ) = true;
                    }
                    for (OARelNum = 1; OARelNum <= state.afn->NumOfReliefFans; ++OARelNum) {
                        state.afn->DisSysCompReliefAirData(OARelNum).OutletNode =
                            GetOAMixerInletNodeNumber(state, state.afn->DisSysCompReliefAirData(OARelNum).OAMixerNum);
                        //                            NodeFound( state.afn->DisSysCompOutdoorAirData( OAFanNum ).InletNode
                        //                            ) = true;
                    }
                    // Check NodeFound status
                    for (OAMixerNum = 1; OAMixerNum <= GetNumOAMixers(state); ++OAMixerNum) {
                        if (i == GetOAMixerReliefNodeNumber(state, OAMixerNum)) {
                            NodeFound(i) = true;
                            break;
                        } else if (i == GetOAMixerInletNodeNumber(state, OAMixerNum)) {
                            NodeFound(i) = true;
                            break;
                        } else {
                            if (OAMixerNum == GetNumOAMixers(state)) {
                                ShowSevereError(state,
                                                format(RoutineName) + "'" + state.dataLoopNodes->NodeID(i) +
                                                    "' is not defined as an AirflowNetwork:Distribution:Node object.");
                                ErrorsFound = true;
                            }
                        }
                    }
                } else if (GetNumOAMixers(state) == 0) {
                    ShowSevereError(state,
                                    format(RoutineName) + "'" + state.dataLoopNodes->NodeID(i) +
                                        "' is not defined as an AirflowNetwork:Distribution:Node object.");
                    ErrorsFound = true;
                } else {
                    // TODO: I fail to see how you could enter this block given than NumOAMixers (returned by GetNumOAMixers())
                    // is initialized to zero, and we check above if '> 0' or '== 0'
                    if (state.afn->NumOfOAFans == 1 &&
                        state.afn->DisSysCompOutdoorAirData(1).InletNode == 0) {
                        state.afn->DisSysCompOutdoorAirData(1).InletNode = GetOAMixerInletNodeNumber(state, 1);
                    }
                    if (state.afn->NumOfReliefFans == 1 &&
                        state.afn->DisSysCompReliefAirData(1).OutletNode == 0) {
                        state.afn->DisSysCompReliefAirData(1).OutletNode = GetOAMixerInletNodeNumber(state, 1);
                    }
                    if (i == GetOAMixerReliefNodeNumber(state, 1)) {
                        NodeFound(i) = true;
                    } else if (i == GetOAMixerInletNodeNumber(state, 1)) {
                        NodeFound(i) = true;
                    } else {
                        ShowSevereError(state,
                                        format(RoutineName) + "'" + state.dataLoopNodes->NodeID(i) +
                                            "' is not defined as an AirflowNetwork:Distribution:Node object.");
                        ErrorsFound = true;
                    }
                }
            }
        }
        if (HPWHFound) {
            ShowWarningError(state,
                             "ValidateDistributionSystem: Heat pump water heater is simulated along with an AirflowNetwork but is not included in "
                             "the AirflowNetwork.");
        }
        if (StandaloneERVFound) {
            ShowWarningError(state,
                             "ValidateDistributionSystem: A ZoneHVAC:EnergyRecoveryVentilator is simulated along with an AirflowNetwork but is not "
                             "included in the AirflowNetwork.");
        }
        NodeFound.deallocate();

        // Assign AirLoop Number to every node and linkage
        // Zone first
        for (i = 1; i <= state.afn->AirflowNetworkNumOfZones; i++) {
            for (j = 1; j <= state.dataGlobal->NumOfZones; j++) {
                if (!state.dataZoneEquip->ZoneEquipConfig(j).IsControlled) continue;
                if ((state.afn->MultizoneZoneData(i).ZoneNum == j) && (state.dataZoneEquip->ZoneEquipConfig(j).NumInletNodes > 0)) {
                    // No multiple Airloop
                    state.afn->AirflowNetworkNodeData(i).AirLoopNum = state.dataZoneEquip->ZoneEquipConfig(j).InletNodeAirLoopNum(1);
                }
            }
        }
        // Air Distribution system
        for (i = state.afn->AirflowNetworkNumOfSurfaces + 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
            j = state.afn->AirflowNetworkLinkageData(i).NodeNums[0];
            k = state.afn->AirflowNetworkLinkageData(i).NodeNums[1];
            if (state.afn->AirflowNetworkNodeData(j).AirLoopNum == 0 &&
                state.afn->AirflowNetworkNodeData(k).AirLoopNum == 0) {
                // Error messaage
                ShowSevereError(state,
                                "ValidateDistributionSystem: AIRFLOWNETWORK:DISTRIBUTION:LINKAGE = " +
                                    state.afn->AirflowNetworkLinkageData(i).Name + " is not valid for AirLoopNum assignment");
                ShowContinueError(
                    state,
                    "AirLoopNum is not found in both nodes for the linkage: " + state.afn->AirflowNetworkLinkageData(i).NodeNames[0] +
                        " and " + state.afn->AirflowNetworkLinkageData(i).NodeNames[1]);
                ShowContinueError(state,
                                  "Please ensure one of two AIRFLOWNETWORK:DISTRIBUTION:NODEs in the first AIRFLOWNETWORK:DISTRIBUTION:LINKAGE "
                                  "object should be defined as EnergyPlus NodeID.");
                ErrorsFound = true;
            }
            if (state.afn->AirflowNetworkNodeData(j).AirLoopNum > 0 &&
                state.afn->AirflowNetworkNodeData(k).AirLoopNum == 0) {
                state.afn->AirflowNetworkNodeData(k).AirLoopNum = state.afn->AirflowNetworkNodeData(j).AirLoopNum;
            }
            if (state.afn->AirflowNetworkNodeData(j).AirLoopNum == 0 &&
                state.afn->AirflowNetworkNodeData(k).AirLoopNum > 0) {
                state.afn->AirflowNetworkNodeData(j).AirLoopNum = state.afn->AirflowNetworkNodeData(k).AirLoopNum;
            }
            if (state.afn->AirflowNetworkNodeData(j).AirLoopNum == state.afn->AirflowNetworkNodeData(k).AirLoopNum) {
                state.afn->AirflowNetworkLinkageData(i).AirLoopNum = state.afn->AirflowNetworkNodeData(j).AirLoopNum;
            }
            if (state.afn->AirflowNetworkNodeData(j).AirLoopNum != state.afn->AirflowNetworkNodeData(k).AirLoopNum &&
                state.afn->AirflowNetworkNodeData(j).AirLoopNum > 0 &&
                state.afn->AirflowNetworkNodeData(k).AirLoopNum > 0) {
                state.afn->AirflowNetworkLinkageData(i).AirLoopNum = state.afn->AirflowNetworkNodeData(j).AirLoopNum;
                ShowSevereError(state,
                                "The AirLoopNum defined in both AIRFLOWNETWORK:DISTRIBUTION:NODE objects in " +
                                    state.afn->AirflowNetworkLinkageData(i).Name +
                                    " are not the same. Please make sure both nodes should be listed in the same AirLoop as a valid linkage.");
                ShowContinueError(
                    state,
                    "AirLoop defined in " + state.afn->AirflowNetworkNodeData(j).Name + " is " +
                        state.dataAirSystemsData->PrimaryAirSystems(state.afn->AirflowNetworkNodeData(j).AirLoopNum).Name +
                        ", and AirLoop defined in " + state.afn->AirflowNetworkNodeData(k).Name + " is " +
                        state.dataAirSystemsData->PrimaryAirSystems(state.afn->AirflowNetworkNodeData(k).AirLoopNum).Name);
                ErrorsFound = true;
            }
            // Set AirLoopNum to fans and coils
            if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).EPlusTypeNum ==
                iEPlusComponentType::FAN) {
                n = state.afn
                        ->DisSysCompCVFData(
                            state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).TypeNum)
                        .FanIndex;
                state.afn
                    ->DisSysCompCVFData(
                        state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).TypeNum)
                    .AirLoopNum = state.afn->AirflowNetworkLinkageData(i).AirLoopNum;
                if (state.afn
                        ->DisSysCompCVFData(
                            state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).TypeNum)
                        .FanModelFlag) {
                    state.dataHVACFan
                        ->fanObjs[state.afn
                                      ->DisSysCompCVFData(state.afn
                                                              ->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum)
                                                              .TypeNum)
                                      .FanIndex]
                        ->AirLoopNum = state.afn->AirflowNetworkLinkageData(i).AirLoopNum;
                } else {
                    SetFanAirLoopNumber(state, n, state.afn->AirflowNetworkLinkageData(i).AirLoopNum);
                }
            }
            if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).EPlusTypeNum ==
                iEPlusComponentType::COI) {
                state.afn
                    ->DisSysCompCoilData(
                        state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).TypeNum)
                    .AirLoopNum = state.afn->AirflowNetworkLinkageData(i).AirLoopNum;
            }
        }

        // Validate coil name and type
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:Coil";
        state.afn->MultiSpeedHPIndicator = 0;
        for (i = 1; i <= state.afn->DisSysNumOfCoils; ++i) {
            {
                auto const SELECT_CASE_var(UtilityRoutines::MakeUPPERCase(state.afn->DisSysCompCoilData(i).EPlusType));

                if (SELECT_CASE_var == "COIL:COOLING:DX") {
                    ValidateComponent(state,
                                      "Coil:Cooling:DX",
                                      state.afn->DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        // Replace the convenience function with in-place code
                        std::string mycoil = state.afn->DisSysCompCoilData(i).name;
                        auto it = std::find_if(state.dataCoilCooingDX->coilCoolingDXs.begin(),
                                               state.dataCoilCooingDX->coilCoolingDXs.end(),
                                               [&mycoil](const CoilCoolingDX &coil) { return coil.name == mycoil; });
                        if (it != state.dataCoilCooingDX->coilCoolingDXs.end()) {
                            // Set the airloop number on the CoilCoolingDX object, which is used to collect the runtime fraction
                            it->airLoopNum = state.afn->DisSysCompCoilData(i).AirLoopNum;
                        } else {
                            ShowSevereError(state,
                                            "SetDXCoilAirLoopNumber: Could not find Coil \"Name=\"" +
                                                state.afn->DisSysCompCoilData(i).name + "\"");
                        }
                        // SetDXCoilAirLoopNumber(state.afn->DisSysCompCoilData(i).name,
                        // state.afn->DisSysCompCoilData(i).AirLoopNum);
                    }
                } else if (SELECT_CASE_var == "COIL:COOLING:DX:SINGLESPEED") {
                    ValidateComponent(state,
                                      "Coil:Cooling:DX:SingleSpeed",
                                      state.afn->DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetDXCoilAirLoopNumber(
                            state, state.afn->DisSysCompCoilData(i).name, state.afn->DisSysCompCoilData(i).AirLoopNum);
                    }

                } else if (SELECT_CASE_var == "COIL:HEATING:DX:SINGLESPEED") {
                    ValidateComponent(state,
                                      "Coil:Heating:DX:SingleSpeed",
                                      state.afn->DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetDXCoilAirLoopNumber(
                            state, state.afn->DisSysCompCoilData(i).name, state.afn->DisSysCompCoilData(i).AirLoopNum);
                    }

                } else if (SELECT_CASE_var == "COIL:HEATING:FUEL") {
                    ValidateComponent(state,
                                      "Coil:Heating:Fuel",
                                      state.afn->DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetHeatingCoilAirLoopNumber(state,
                                                    state.afn->DisSysCompCoilData(i).name,
                                                    state.afn->DisSysCompCoilData(i).AirLoopNum,
                                                    ErrorsFound);
                    }

                } else if (SELECT_CASE_var == "COIL:HEATING:ELECTRIC") {
                    ValidateComponent(state,
                                      "Coil:Heating:Electric",
                                      state.afn->DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetHeatingCoilAirLoopNumber(state,
                                                    state.afn->DisSysCompCoilData(i).name,
                                                    state.afn->DisSysCompCoilData(i).AirLoopNum,
                                                    ErrorsFound);
                    }

                } else if (SELECT_CASE_var == "COIL:COOLING:WATER") {
                    ValidateComponent(state,
                                      "Coil:Cooling:Water",
                                      state.afn->DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else if (SELECT_CASE_var == "COIL:HEATING:WATER") {
                    ValidateComponent(state,
                                      "Coil:Heating:Water",
                                      state.afn->DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else if (SELECT_CASE_var == "COIL:COOLING:WATER:DETAILEDGEOMETRY") {
                    ValidateComponent(state,
                                      "Coil:Cooling:Water:DetailedGeometry",
                                      state.afn->DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else if (SELECT_CASE_var == "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE") {
                    ValidateComponent(state,
                                      "Coil:Cooling:DX:TwoStageWithHumidityControlMode",
                                      state.afn->DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetDXCoilAirLoopNumber(
                            state, state.afn->DisSysCompCoilData(i).name, state.afn->DisSysCompCoilData(i).AirLoopNum);
                    }

                } else if (SELECT_CASE_var == "COIL:COOLING:DX:MULTISPEED") {
                    ValidateComponent(state,
                                      "Coil:Cooling:DX:MultiSpeed",
                                      state.afn->DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    ++state.afn->MultiSpeedHPIndicator;
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetDXCoilAirLoopNumber(
                            state, state.afn->DisSysCompCoilData(i).name, state.afn->DisSysCompCoilData(i).AirLoopNum);
                    }

                } else if (SELECT_CASE_var == "COIL:HEATING:DX:MULTISPEED") {
                    ValidateComponent(state,
                                      "Coil:Heating:DX:MultiSpeed",
                                      state.afn->DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    ++state.afn->MultiSpeedHPIndicator;
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetDXCoilAirLoopNumber(
                            state, state.afn->DisSysCompCoilData(i).name, state.afn->DisSysCompCoilData(i).AirLoopNum);
                    }

                } else if (SELECT_CASE_var == "COIL:HEATING:DESUPERHEATER") {
                    ValidateComponent(state,
                                      "Coil:Heating:Desuperheater",
                                      state.afn->DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else if (SELECT_CASE_var == "COIL:COOLING:DX:TWOSPEED") {
                    ValidateComponent(state,
                                      "Coil:Cooling:DX:TwoSpeed",
                                      state.afn->DisSysCompCoilData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    } else {
                        SetDXCoilAirLoopNumber(
                            state, state.afn->DisSysCompCoilData(i).name, state.afn->DisSysCompCoilData(i).AirLoopNum);
                    }

                } else {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject +
                                        " Invalid coil type = " + state.afn->DisSysCompCoilData(i).name);
                    ErrorsFound = true;
                }
            }
        }

        // Validate terminal unit name and type
        for (i = 1; i <= state.afn->DisSysNumOfTermUnits; ++i) {
            if (UtilityRoutines::SameString(state.afn->DisSysCompTermUnitData(i).EPlusType,
                                            "AirTerminal:SingleDuct:ConstantVolume:Reheat") ||
                UtilityRoutines::SameString(state.afn->DisSysCompTermUnitData(i).EPlusType, "AirTerminal:SingleDuct:VAV:Reheat")) {
                LocalError = false;
                if (UtilityRoutines::SameString(state.afn->DisSysCompTermUnitData(i).EPlusType,
                                                "AirTerminal:SingleDuct:ConstantVolume:Reheat"))
                    GetHVACSingleDuctSysIndex(state,
                                              state.afn->DisSysCompTermUnitData(i).name,
                                              n,
                                              LocalError,
                                              "AirflowNetwork:Distribution:Component:TerminalUnit");
                if (UtilityRoutines::SameString(state.afn->DisSysCompTermUnitData(i).EPlusType, "AirTerminal:SingleDuct:VAV:Reheat"))
                    GetHVACSingleDuctSysIndex(state,
                                              state.afn->DisSysCompTermUnitData(i).name,
                                              n,
                                              LocalError,
                                              "AirflowNetwork:Distribution:Component:TerminalUnit",
                                              state.afn->DisSysCompTermUnitData(i).DamperInletNode,
                                              state.afn->DisSysCompTermUnitData(i).DamperOutletNode);
                if (LocalError) ErrorsFound = true;
                if (state.afn->VAVSystem) {
                    for (j = 1; j <= state.afn->DisSysNumOfCVFs; j++) {
                        if (state.afn->DisSysCompCVFData(j).FanTypeNum == FanType_SimpleVAV) {
                            if (state.afn->DisSysCompCVFData(j).AirLoopNum ==
                                    state.afn->DisSysCompTermUnitData(i).AirLoopNum &&
                                !UtilityRoutines::SameString(state.afn->DisSysCompTermUnitData(i).EPlusType,
                                                             "AirTerminal:SingleDuct:VAV:Reheat")) {
                                ShowSevereError(state,
                                                format(RoutineName) + CurrentModuleObject + " Invalid terminal type for a VAV system = " +
                                                    state.afn->DisSysCompTermUnitData(i).name);
                                ShowContinueError(state, "The input type = " + state.afn->DisSysCompTermUnitData(i).EPlusType);
                                ShowContinueError(state, "A VAV system requires all terminal units with type = AirTerminal:SingleDuct:VAV:Reheat");
                                ErrorsFound = true;
                            }
                        }
                    }
                }
            } else {
                ShowSevereError(state,
                                format(RoutineName) + "AIRFLOWNETWORK:DISTRIBUTION:COMPONENT TERMINAL UNIT: Invalid Terminal unit type = " +
                                    state.afn->DisSysCompTermUnitData(i).name);
                ErrorsFound = true;
            }
        }

        // Validate heat exchanger name and type
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:HeatExchanger";
        for (i = 1; i <= state.afn->DisSysNumOfHXs; ++i) {
            {
                auto const SELECT_CASE_var(UtilityRoutines::MakeUPPERCase(state.afn->DisSysCompHXData(i).EPlusType));

                if (SELECT_CASE_var == "HEATEXCHANGER:AIRTOAIR:FLATPLATE") {
                    ValidateComponent(state,
                                      "HeatExchanger:AirToAir:FlatPlate",
                                      state.afn->DisSysCompHXData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else if (SELECT_CASE_var == "HEATEXCHANGER:AIRTOAIR:SENSIBLEANDLATENT") {
                    ValidateComponent(state,
                                      "HeatExchanger:AirToAir:SensibleAndLatent",
                                      state.afn->DisSysCompHXData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else if (SELECT_CASE_var == "HEATEXCHANGER:DESICCANT:BALANCEDFLOW") {
                    ValidateComponent(state,
                                      "HeatExchanger:Desiccant:BalancedFlow",
                                      state.afn->DisSysCompHXData(i).name,
                                      IsNotOK,
                                      format(RoutineName) + CurrentModuleObject);
                    if (IsNotOK) {
                        ErrorsFound = true;
                    }

                } else {
                    ShowSevereError(state,
                                    format(RoutineName) + CurrentModuleObject +
                                        " Invalid heat exchanger type = " + state.afn->DisSysCompHXData(i).EPlusType);
                    ErrorsFound = true;
                }
            }
        }

        // Assign supply and return connection
        for (j = 1; j <= NumPrimaryAirSys; ++j) {
            S1 = 0;
            S2 = 0;
            R1 = 0;
            R2 = 0;
            for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
                if (state.afn->AirflowNetworkNodeData(i).EPlusNodeNum ==
                    state.dataAirLoop->AirToZoneNodeInfo(j).AirLoopSupplyNodeNum(1))
                    S1 = i;
                if (state.afn->AirflowNetworkNodeData(i).EPlusNodeNum ==
                    state.dataAirLoop->AirToZoneNodeInfo(j).ZoneEquipSupplyNodeNum(1))
                    S2 = i;
                if (state.afn->AirflowNetworkNodeData(i).EPlusNodeNum ==
                    state.dataAirLoop->AirToZoneNodeInfo(j).ZoneEquipReturnNodeNum(1))
                    R1 = i;
                if (state.afn->AirflowNetworkNodeData(i).EPlusNodeNum ==
                    state.dataAirLoop->AirToZoneNodeInfo(j).AirLoopReturnNodeNum(1))
                    R2 = i;
            }
            for (i = 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
                if (state.afn->AirflowNetworkLinkageData(i).NodeNums[0] == R1 &&
                    state.afn->AirflowNetworkLinkageData(i).NodeNums[1] == R2) {
                    state.afn->AirflowNetworkLinkageData(i).ConnectionFlag = iEPlusComponentType::RCN;
                }
                if (state.afn->AirflowNetworkLinkageData(i).NodeNums[0] == S1 &&
                    state.afn->AirflowNetworkLinkageData(i).NodeNums[1] == S2) {
                    state.afn->AirflowNetworkLinkageData(i).ConnectionFlag = iEPlusComponentType::SCN;
                }
            }
        }

        // Assign fan inlet and outlet node, and coil outlet
        for (i = 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
            j = state.afn->AirflowNetworkLinkageData(i).CompNum;
            if (state.afn->AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::CVF) {
                if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                        .EPlusTypeNum == iEPlusNodeType::Invalid)
                    state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                        .EPlusTypeNum = iEPlusNodeType::FIN;
                state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1]).EPlusTypeNum =
                    iEPlusNodeType::FOU;
            }
            if (state.afn->AirflowNetworkCompData(j).EPlusTypeNum == iEPlusComponentType::COI) {
                state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1]).EPlusTypeNum =
                    iEPlusNodeType::COU;
            }
            if (state.afn->AirflowNetworkCompData(j).EPlusTypeNum == iEPlusComponentType::HEX) {
                state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1]).EPlusTypeNum =
                    iEPlusNodeType::HXO;
            }
            if (state.afn->AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::TMU) {
                if (state.afn->DisSysCompTermUnitData(state.afn->AirflowNetworkCompData(j).TypeNum).DamperInletNode >
                    0) {
                    if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                                .EPlusNodeNum ==
                            state.afn->DisSysCompTermUnitData(state.afn->AirflowNetworkCompData(j).TypeNum)
                                .DamperInletNode &&
                        state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                                .EPlusNodeNum ==
                            state.afn->DisSysCompTermUnitData(state.afn->AirflowNetworkCompData(j).TypeNum)
                                .DamperOutletNode) {
                        state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                            .EPlusTypeNum = iEPlusNodeType::DIN;
                        state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                            .EPlusTypeNum = iEPlusNodeType::DOU;
                        state.afn->AirflowNetworkLinkageData(i).VAVTermDamper = true;
                    }
                }
            }
        }

        // Validate the position of constant pressure drop component
        CurrentModuleObject = "AirflowNetwork:Distribution:Component:ConstantPressureDrop";
        for (i = 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
            if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).CompTypeNum ==
                iComponentTypeNum::CPD) {
                for (j = 1; j <= state.afn->AirflowNetworkNumOfLinks; ++j) {
                    if (state.afn->AirflowNetworkLinkageData(i).NodeNums[0] ==
                        state.afn->AirflowNetworkLinkageData(j).NodeNums[1]) {
                        if (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(j).CompNum)
                                .CompTypeNum != iComponentTypeNum::DWC) {
                            ShowSevereError(state,
                                            format(RoutineName) + "An " + CurrentModuleObject + " object (" +
                                                state.afn->AirflowNetworkLinkageData(i).CompName + ')');
                            ShowContinueError(state,
                                              "must connect a duct component upstream and not " +
                                                  state.afn->AirflowNetworkLinkageData(j).Name);
                            ErrorsFound = true;
                        }
                    }
                }
                if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                        .EPlusTypeNum == iEPlusNodeType::SPL) {
                    ShowSevereError(state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" +
                                        state.afn->AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(
                        state,
                        "does not allow a AirLoopHVAC:ZoneSplitter node = " +
                            state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                                .Name);
                    ErrorsFound = true;
                }
                if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                        .EPlusTypeNum == iEPlusNodeType::SPL) {
                    ShowSevereError(state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" +
                                        state.afn->AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(
                        state,
                        "does not allow a AirLoopHVAC:ZoneSplitter node = " +
                            state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                                .Name);
                    ErrorsFound = true;
                }
                if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                        .EPlusTypeNum == iEPlusNodeType::MIX) {
                    ShowSevereError(state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" +
                                        state.afn->AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(
                        state,
                        "does not allow a AirLoopHVAC:ZoneMixer node = " +
                            state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                                .Name);
                    ErrorsFound = true;
                }
                if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                        .EPlusTypeNum == iEPlusNodeType::MIX) {
                    ShowSevereError(state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" +
                                        state.afn->AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(
                        state,
                        "does not allow a AirLoopHVAC:ZoneMixer node = " +
                            state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                                .Name);
                    ErrorsFound = true;
                }
                if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                        .EPlusNodeNum > 0) {
                    ShowSevereError(state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" +
                                        state.afn->AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(
                        state,
                        "does not allow to connect an EnergyPlus node = " +
                            state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                                .Name);
                    ErrorsFound = true;
                }
                if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                        .EPlusNodeNum > 0) {
                    ShowSevereError(state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" +
                                        state.afn->AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(
                        state,
                        "does not allow to connect an EnergyPlus node = " +
                            state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                                .Name);
                    ErrorsFound = true;
                }
                if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                        .EPlusZoneNum > 0) {
                    ShowSevereError(state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" +
                                        state.afn->AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(
                        state,
                        "does not allow to connect an EnergyPlus zone = " +
                            state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                                .Name);
                    ErrorsFound = true;
                }
                if (state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                        .EPlusZoneNum > 0) {
                    ShowSevereError(state,
                                    format(RoutineName) + "An " + CurrentModuleObject + " object (" +
                                        state.afn->AirflowNetworkLinkageData(i).CompName + ')');
                    ShowContinueError(
                        state,
                        "does not allow to connect an EnergyPlus zone = " +
                            state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[1])
                                .Name);
                    ErrorsFound = true;
                }
            }
        }

        for (i = state.afn->NumOfNodesMultiZone + 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            if (state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::SPL) {
                LocalError = false;
                j = GetSplitterOutletNumber(state, "", 1, LocalError);
                state.afn->SplitterNodeNumbers.allocate(j + 2);
                state.afn->SplitterNodeNumbers = GetSplitterNodeNumbers(state, "", 1, LocalError);
                if (LocalError) ErrorsFound = true;
            }
        }

        // Assigning inlet and outlet nodes for a splitter
        for (i = 1; i <= state.afn->AirflowNetworkNumOfNodes; ++i) {
            if (state.afn->AirflowNetworkNodeData(i).EPlusNodeNum == state.afn->SplitterNodeNumbers(1)) {
                if (state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::Invalid)
                    state.afn->AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::SPI;
            }
            for (j = 1; j <= state.afn->SplitterNodeNumbers(2); ++j) {
                if (state.afn->AirflowNetworkNodeData(i).EPlusNodeNum ==
                    state.afn->SplitterNodeNumbers(j + 2)) {
                    if (state.afn->AirflowNetworkNodeData(i).EPlusTypeNum == iEPlusNodeType::Invalid)
                        state.afn->AirflowNetworkNodeData(i).EPlusTypeNum = iEPlusNodeType::SPO;
                }
            }
        }

        // Add additional output variables
        if (state.afn->DisSysNumOfCVFs > 1) {
            bool OnOffFanFlag = false;
            for (i = 1; i <= state.afn->DisSysNumOfCVFs; i++) {
                if (state.afn->DisSysCompCVFData(i).FanTypeNum == FanType_SimpleOnOff &&
                    !state.afn->DisSysCompCVFData(i).FanModelFlag) {
                    OnOffFanFlag = true;
                    break;
                }
                if (state.afn->DisSysCompCVFData(i).FanModelFlag &&
                    state.afn->DisSysCompCVFData(i).FanTypeNum == FanType_SimpleOnOff) {
                    int fanIndex = HVACFan::getFanObjectVectorIndex(state, state.afn->DisSysCompCVFData(i).name);
                    if (state.dataHVACFan->fanObjs[fanIndex]->AirPathFlag) {
                        state.afn->DisSysCompCVFData(i).FanTypeNum = FanType_SimpleConstVolume;
                    } else {
                        OnOffFanFlag = true;
                        break;
                    }
                }
            }
            if (OnOffFanFlag) {
                for (j = 1; j <= state.afn->AirflowNetworkNumOfZones; ++j) {
                    if (!state.dataZoneEquip->ZoneEquipConfig(state.afn->AirflowNetworkNodeData(j).EPlusZoneNum).IsControlled)
                        continue;
                    for (i = 1; i <= state.afn->DisSysNumOfCVFs; i++) {
                        if (state.afn->DisSysCompCVFData(i).AirLoopNum ==
                                state.afn->AirflowNetworkNodeData(j).AirLoopNum &&
                            state.afn->DisSysCompCVFData(i).FanTypeNum != FanType_SimpleOnOff) {
                            SetupOutputVariable(state,
                                                "AFN Node Total Pressure",
                                                OutputProcessor::Unit::Pa,
                                                state.afn->AirflowNetworkNodeSimu(j).PZ,
                                                OutputProcessor::SOVTimeStepType::System,
                                                OutputProcessor::SOVStoreType::Average,
                                                state.afn->AirflowNetworkNodeData(j).Name);
                        }
                    }
                }
                for (i = 1; i <= state.afn->NumOfLinksMultiZone; ++i) {
                    if (!state.dataZoneEquip
                             ->ZoneEquipConfig(
                                 state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                                     .EPlusZoneNum)
                             .IsControlled)
                        continue;
                    for (j = 1; j <= state.afn->DisSysNumOfCVFs; j++) {
                        if (state.afn->DisSysCompCVFData(j).AirLoopNum ==
                                state.afn->AirflowNetworkNodeData(state.afn->AirflowNetworkLinkageData(i).NodeNums[0])
                                    .AirLoopNum &&
                            state.afn->DisSysCompCVFData(j).FanTypeNum != FanType_SimpleOnOff) {
                            SetupOutputVariable(state,
                                                "AFN Linkage Node 1 to Node 2 Mass Flow Rate",
                                                OutputProcessor::Unit::kg_s,
                                                state.afn->linkReport(i).FLOW,
                                                OutputProcessor::SOVTimeStepType::System,
                                                OutputProcessor::SOVStoreType::Average,
                                                state.afn->AirflowNetworkLinkageData(i).Name);
                            SetupOutputVariable(state,
                                                "AFN Linkage Node 2 to Node 1 Mass Flow Rate",
                                                OutputProcessor::Unit::kg_s,
                                                state.afn->linkReport(i).FLOW2,
                                                OutputProcessor::SOVTimeStepType::System,
                                                OutputProcessor::SOVStoreType::Average,
                                                state.afn->AirflowNetworkLinkageData(i).Name);
                            SetupOutputVariable(state,
                                                "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
                                                OutputProcessor::Unit::m3_s,
                                                state.afn->linkReport(i).VolFLOW,
                                                OutputProcessor::SOVTimeStepType::System,
                                                OutputProcessor::SOVStoreType::Average,
                                                state.afn->AirflowNetworkLinkageData(i).Name);
                            SetupOutputVariable(state,
                                                "AFN Linkage Node 2 to Node 1 Volume Flow Rate",
                                                OutputProcessor::Unit::m3_s,
                                                state.afn->linkReport(i).VolFLOW2,
                                                OutputProcessor::SOVTimeStepType::System,
                                                OutputProcessor::SOVStoreType::Average,
                                                state.afn->AirflowNetworkLinkageData(i).Name);
                            SetupOutputVariable(state,
                                                "AFN Linkage Node 1 to Node 2 Pressure Difference",
                                                OutputProcessor::Unit::Pa,
                                                state.afn->AirflowNetworkLinkSimu(i).DP,
                                                OutputProcessor::SOVTimeStepType::System,
                                                OutputProcessor::SOVStoreType::Average,
                                                state.afn->AirflowNetworkLinkageData(i).Name);
                        }
                    }
                }
            }
        }
        bool FanModelConstFlag = false;
        for (i = 1; i <= state.afn->DisSysNumOfCVFs; i++) {
            if (state.afn->DisSysCompCVFData(i).FanModelFlag) {
                int fanIndex = HVACFan::getFanObjectVectorIndex(state, state.afn->DisSysCompCVFData(i).name);
                if (state.afn->DisSysCompCVFData(i).FanTypeNum == FanType_SimpleOnOff &&
                    state.dataHVACFan->fanObjs[fanIndex]->AirPathFlag) {
                    state.afn->DisSysCompCVFData(i).FanTypeNum = FanType_SimpleConstVolume;
                    state.afn->SupplyFanType = FanType_SimpleConstVolume;
                    FanModelConstFlag = true;
                    break;
                }
            }
        }
        if (FanModelConstFlag) {
            for (i = 1; i <= state.afn->AirflowNetworkNumOfSurfaces; ++i) {
                if (state.afn->SupplyFanType == FanType_SimpleConstVolume) {
                    SetupOutputVariable(state,
                                        "AFN Linkage Node 1 to Node 2 Mass Flow Rate",
                                        OutputProcessor::Unit::kg_s,
                                        state.afn->linkReport(i).FLOW,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.afn->AirflowNetworkLinkageData(i).Name);
                    SetupOutputVariable(state,
                                        "AFN Linkage Node 2 to Node 1 Mass Flow Rate",
                                        OutputProcessor::Unit::kg_s,
                                        state.afn->linkReport(i).FLOW2,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.afn->AirflowNetworkLinkageData(i).Name);
                    SetupOutputVariable(state,
                                        "AFN Linkage Node 1 to Node 2 Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        state.afn->linkReport(i).VolFLOW,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.afn->AirflowNetworkLinkageData(i).Name);
                    SetupOutputVariable(state,
                                        "AFN Linkage Node 2 to Node 1 Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        state.afn->linkReport(i).VolFLOW2,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.afn->AirflowNetworkLinkageData(i).Name);
                    SetupOutputVariable(state,
                                        "AFN Linkage Node 1 to Node 2 Pressure Difference",
                                        OutputProcessor::Unit::Pa,
                                        state.afn->AirflowNetworkLinkSimu(i).DP,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.afn->AirflowNetworkLinkageData(i).Name);
                }
            }
        }

        // Add AirLoopNum to pressure control object
        for (i = 1; i <= state.afn->NumOfPressureControllers; ++i) {
            for (j = 1; j <= state.dataGlobal->NumOfZones; ++j) {
                if (state.afn->PressureControllerData(i).ZoneNum == j) {
                    for (k = 1; k <= state.dataZoneEquip->ZoneEquipConfig(j).NumInletNodes; ++k) {
                        if (state.dataZoneEquip->ZoneEquipConfig(j).InletNodeAirLoopNum(k) > 0) {
                            state.afn->PressureControllerData(i).AirLoopNum =
                                state.dataZoneEquip->ZoneEquipConfig(j).InletNodeAirLoopNum(k);
                            if (state.afn->PressureControllerData(i).ControlTypeSet == PressureCtrlRelief) {
                                state.afn->PressureControllerData(i).OANodeNum =
                                    state.dataAirSystemsData->PrimaryAirSystems(state.afn->PressureControllerData(i).AirLoopNum)
                                        .OAMixOAInNodeNum;
                                for (n = 1; n <= state.afn->NumOfReliefFans; ++n) {
                                    if (state.afn->DisSysCompReliefAirData(n).OutletNode ==
                                        state.afn->PressureControllerData(i).OANodeNum) {
                                        state.afn->DisSysCompReliefAirData(n).PressCtrlNum = i;
                                    }
                                }
                            }
                            if (state.afn->PressureControllerData(i).ControlTypeSet == PressureCtrlExhaust) {
                                state.afn->PressureControllerData(i).OANodeNum =
                                    state.dataZoneEquip->ZoneEquipConfig(state.afn->PressureControllerData(i).ZoneNum).ExhaustNode(1);
                                for (n = 1; n <= state.afn->AirflowNetworkNumOfExhFan; ++n) {
                                    if (state.afn->MultizoneCompExhaustFanData(n).EPlusZoneNum ==
                                        state.afn->PressureControllerData(i).ZoneNum) {
                                        state.afn->MultizoneCompExhaustFanData(n).PressCtrlNum = i;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Check number of fans specified in an AirLoop #6748
        int BranchNum;
        int CompNum;
        int NumOfFans;
        std::string FanNames;
        for (BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(1).NumBranches; ++BranchNum) {
            NumOfFans = 0;
            FanNames = "";
            for (CompNum = 1; CompNum <= state.dataAirSystemsData->PrimaryAirSystems(1).Branch(BranchNum).TotalComponents; ++CompNum) {
                if (UtilityRoutines::SameString(state.dataAirSystemsData->PrimaryAirSystems(1).Branch(BranchNum).Comp(CompNum).TypeOf,
                                                "Fan:ConstantVolume") ||
                    UtilityRoutines::SameString(state.dataAirSystemsData->PrimaryAirSystems(1).Branch(BranchNum).Comp(CompNum).TypeOf, "Fan:OnOff") ||
                    UtilityRoutines::SameString(state.dataAirSystemsData->PrimaryAirSystems(1).Branch(BranchNum).Comp(CompNum).TypeOf,
                                                "Fan:VariableVolume")) {
                    NumOfFans++;
                    if (NumOfFans > 1) {
                        FanNames += state.dataAirSystemsData->PrimaryAirSystems(1).Branch(BranchNum).Comp(CompNum).Name;
                        break;
                    } else {
                        FanNames += state.dataAirSystemsData->PrimaryAirSystems(1).Branch(BranchNum).Comp(CompNum).Name + ",";
                    }
                }
            }
            if (NumOfFans > 1) break;
        }
        if (NumOfFans > 1) {
            ShowSevereError(state,
                            format(RoutineName) + "An AirLoop branch, " + state.dataAirSystemsData->PrimaryAirSystems(1).Branch(BranchNum).Name +
                                ", has two or more fans: " + FanNames);
            ShowContinueError(state,
                              "The AirflowNetwork model allows a single supply fan in an AirLoop only. Please make changes in the input "
                              "file accordingly.");
            ErrorsFound = true;
        }

        if (ErrorsFound) {
            ShowFatalError(state, format(RoutineName) + "Program terminates for preceding reason(s).");
        }
    }

    void ValidateFanFlowRate(EnergyPlusData &state)
    {

        // Catch a fan flow rate from EPlus input file and add a flag for VAV terminal damper
        for (int i = 1; i <= state.afn->AirflowNetworkNumOfLinks; ++i) {
            switch (state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).CompTypeNum) {
            case iComponentTypeNum::CVF: { // 'CVF'
                int typeNum =
                    state.afn->AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).TypeNum;
                if (state.afn->DisSysCompCVFData(typeNum).FanTypeNum == FanType_SimpleVAV) {
                    if (state.afn->DisSysCompCVFData(typeNum).FanModelFlag) {
                        state.afn->DisSysCompCVFData(typeNum).MaxAirMassFlowRate =
                            state.dataHVACFan->fanObjs[state.afn->DisSysCompCVFData(typeNum).FanIndex]->designAirVolFlowRate *
                            state.dataEnvrn->StdRhoAir;
                    } else {
                        Real64 FanFlow; // Return type
                        GetFanVolFlow(state, state.afn->DisSysCompCVFData(typeNum).FanIndex, FanFlow);
                        state.afn->DisSysCompCVFData(typeNum).MaxAirMassFlowRate = FanFlow * state.dataEnvrn->StdRhoAir;
                    }
                }
            } break;
            case iComponentTypeNum::FAN:
            case iComponentTypeNum::SOP:
            case iComponentTypeNum::TMU:
                break;
            default:
                break;
            }
        }
    }

    void ValidateExhaustFanInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Dec. 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine validate zone exhaust fan and associated surface

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("ValidateExhaustFanInput: "); // include trailing blank space

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int j;
        int k;
        bool ErrorsFound(false);
        bool found;
        int EquipTypeNum; // Equipment type number
        std::string CurrentModuleObject;

        // Validate supply and return connections
        if (state.afn->ValidateExhaustFanInputOneTimeFlag) {
            CurrentModuleObject = "AirflowNetwork:MultiZone:Component:ZoneExhaustFan";
            if (std::any_of(state.dataZoneEquip->ZoneEquipConfig.begin(),
                            state.dataZoneEquip->ZoneEquipConfig.end(),
                            [](DataZoneEquipment::EquipConfiguration const &e) { return e.IsControlled; })) {
                state.afn->AirflowNetworkZoneExhaustFan.dimension(state.dataGlobal->NumOfZones, false);
            }
            // Ensure the number of exhaust fan defined in the AirflowNetwork model matches the number of Zone Exhaust Fan objects
            if (state.afn->NumOfExhaustFans != state.afn->AirflowNetworkNumOfExhFan) {
                ShowSevereError(state,
                                format(RoutineName) + "The number of " + CurrentModuleObject +
                                    " is not equal to the number of Fan:ZoneExhaust fans defined in ZoneHVAC:EquipmentConnections");
                ShowContinueError(state, format("The number of {} is {}", CurrentModuleObject, state.afn->AirflowNetworkNumOfExhFan));
                ShowContinueError(state,
                                  format("The number of Zone exhaust fans defined in ZoneHVAC:EquipmentConnections is {}",
                                         state.afn->NumOfExhaustFans));
                ErrorsFound = true;
            }

            for (i = 1; i <= state.afn->AirflowNetworkNumOfExhFan; ++i) {
                // Get zone number
                for (j = 1; j <= state.dataGlobal->NumOfZones; ++j) {
                    if (!state.dataZoneEquip->ZoneEquipConfig(j).IsControlled) continue;
                    for (k = 1; k <= state.dataZoneEquip->ZoneEquipConfig(j).NumExhaustNodes; ++k) {
                        if (state.dataZoneEquip->ZoneEquipConfig(j).ExhaustNode(k) ==
                            state.afn->MultizoneCompExhaustFanData(i).InletNode) {
                            state.afn->MultizoneCompExhaustFanData(i).EPlusZoneNum =
                                state.dataZoneEquip->ZoneEquipConfig(j).ActualZoneNum;
                            break;
                        }
                    }
                }
                if (state.afn->MultizoneCompExhaustFanData(i).EPlusZoneNum == 0) {
                    ShowSevereError(state,
                                    format(RoutineName) + "Zone name in " + CurrentModuleObject + "  = " +
                                        state.afn->MultizoneCompExhaustFanData(i).name +
                                        " does not match the zone name in ZoneHVAC:EquipmentConnections");
                    ErrorsFound = true;
                }
                // Ensure a surface using zone exhaust fan to expose to the same zone
                found = false;
                for (j = 1; j <= state.afn->AirflowNetworkNumOfSurfaces; ++j) {
                    if (UtilityRoutines::SameString(state.afn->MultizoneSurfaceData(j).OpeningName,
                                                    state.afn->MultizoneCompExhaustFanData(i).name)) {
                        found = true;
                        if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(j).SurfNum).ExtBoundCond !=
                                ExternalEnvironment &&
                            !(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtBoundCond ==
                                  OtherSideCoefNoCalcExt &&
                              state.dataSurface->Surface(state.afn->MultizoneSurfaceData(i).SurfNum).ExtWind)) {
                            ShowSevereError(state,
                                            format(RoutineName) + "The surface using " + CurrentModuleObject +
                                                " is not an exterior surface: " + state.afn->MultizoneSurfaceData(j).SurfName);
                            ErrorsFound = true;
                        }
                        break;
                    }
                }
                if (!found) {
                    ShowSevereError(state,
                                    CurrentModuleObject + "  = " + state.afn->MultizoneCompExhaustFanData(i).name +
                                        " is defined and never used.");
                    ErrorsFound = true;
                } else {
                    if (state.afn->MultizoneCompExhaustFanData(i).EPlusZoneNum !=
                        state.dataSurface->Surface(state.afn->MultizoneSurfaceData(j).SurfNum).Zone) {
                        ShowSevereError(state,
                                        format(RoutineName) + "Zone name in " + CurrentModuleObject + "  = " +
                                            state.afn->MultizoneCompExhaustFanData(i).name + " does not match the zone name");
                        ShowContinueError(state,
                                          "the surface is exposed to " +
                                              state.dataSurface->Surface(state.afn->MultizoneSurfaceData(j).SurfNum).Name);
                        ErrorsFound = true;
                    } else {
                        state.afn->AirflowNetworkZoneExhaustFan(
                            state.afn->MultizoneCompExhaustFanData(i).EPlusZoneNum) = true;
                    }
                }
            }

            // Ensure all zone exhaust fans are defined
            for (j = 1; j <= state.dataGlobal->NumOfZones; ++j) {
                if (!state.dataZoneEquip->ZoneEquipConfig(j).IsControlled) continue;
                for (EquipTypeNum = 1; EquipTypeNum <= state.dataZoneEquip->ZoneEquipList(j).NumOfEquipTypes; ++EquipTypeNum) {
                    if (state.dataZoneEquip->ZoneEquipList(j).EquipTypeEnum(EquipTypeNum) == DataZoneEquipment::ZoneEquip::ZoneExhaustFan) {
                        found = false;
                        for (k = 1; k <= state.dataZoneEquip->ZoneEquipConfig(j).NumExhaustNodes; ++k) {
                            for (i = 1; i <= state.afn->AirflowNetworkNumOfExhFan; ++i) {
                                if (state.dataZoneEquip->ZoneEquipConfig(j).ExhaustNode(k) ==
                                    state.afn->MultizoneCompExhaustFanData(i).InletNode) {
                                    state.afn->MultizoneCompExhaustFanData(i).EPlusZoneNum =
                                        state.dataZoneEquip->ZoneEquipConfig(j).ActualZoneNum;
                                    found = true;
                                }
                            }
                            if (!found) {
                                ShowSevereError(state, format(RoutineName) + "Fan:ZoneExhaust is not defined in " + CurrentModuleObject);
                                ShowContinueError(state,
                                                  "Zone Air Exhaust Node in ZoneHVAC:EquipmentConnections =" +
                                                      state.dataLoopNodes->NodeID(state.dataZoneEquip->ZoneEquipConfig(j).ExhaustNode(k)));
                                ErrorsFound = true;
                            }
                        }
                    }
                }
            }

            state.afn->ValidateExhaustFanInputOneTimeFlag = false;
            if (ErrorsFound) {
                ShowFatalError(state, format(RoutineName) + "Program terminates for preceding reason(s).");
            }
        } // End if OneTimeFlag_FindFirstLastPtr
    }

    void HybridVentilationControl(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Dec. 2006
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone hybrid ventilation managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs hybrid ventilation control

        auto &HybridVentSysAvailActualZoneNum = state.dataHVACGlobal->HybridVentSysAvailActualZoneNum;
        auto &HybridVentSysAvailAirLoopNum = state.dataHVACGlobal->HybridVentSysAvailAirLoopNum;
        auto &HybridVentSysAvailANCtrlStatus = state.dataHVACGlobal->HybridVentSysAvailANCtrlStatus;
        auto &HybridVentSysAvailMaster = state.dataHVACGlobal->HybridVentSysAvailMaster;
        auto &HybridVentSysAvailVentCtrl = state.dataHVACGlobal->HybridVentSysAvailVentCtrl;
        auto &HybridVentSysAvailWindModifier = state.dataHVACGlobal->HybridVentSysAvailWindModifier;
        auto &NumHybridVentSysAvailMgrs = state.dataHVACGlobal->NumHybridVentSysAvailMgrs;

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr HybridVentCtrl_Close(2);                                       // Open windows or doors
        int constexpr IndividualCtrlType(0);                                         // Individual window or door control
        int constexpr GlobalCtrlType(1);                                             // Global window or door control
        static constexpr std::string_view RoutineName("HybridVentilationControl: "); // include trailing blank space

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int SysAvailNum;       // Hybrid ventilation control number
        int AirLoopNum;        // Airloop number
        int ControlledZoneNum; // Controlled zone number
        int ActualZoneNum;     // Actual zone number
        int ANSurfaceNum;      // AirflowNetwork Surface Number
        int SurfNum;           // Surface number
        int ControlType;       // Hybrid ventilation control type: 0 individual; 1 global
        bool Found;            // Logical to indicate whether a master surface is found or not

        for (auto &e : state.afn->MultizoneSurfaceData) {
            e.HybridVentClose = false;
            e.HybridCtrlGlobal = false;
            e.HybridCtrlMaster = false;
            e.WindModifier = 1.0;
        }
        ControlType = IndividualCtrlType;

        for (SysAvailNum = 1; SysAvailNum <= NumHybridVentSysAvailMgrs; ++SysAvailNum) {
            AirLoopNum = HybridVentSysAvailAirLoopNum(SysAvailNum);
            state.afn->VentilationCtrl = HybridVentSysAvailVentCtrl(SysAvailNum);
            if (HybridVentSysAvailANCtrlStatus(SysAvailNum) > 0) {
                ControlType = GetCurrentScheduleValue(state, HybridVentSysAvailANCtrlStatus(SysAvailNum));
            }
            Found = false;
            ActualZoneNum = 0;
            for (ControlledZoneNum = 1; ControlledZoneNum <= state.dataGlobal->NumOfZones; ++ControlledZoneNum) {
                if (!state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).IsControlled) continue;
                // Ensure all the zones served by this AirLoopHVAC to be controlled by the hybrid ventilation
                for (int zoneInNode = 1; zoneInNode <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    if (AirLoopNum > 0) {
                        if (AirLoopNum == state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode)) {
                            ActualZoneNum = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ActualZoneNum;
                            break;
                        }
                    } else {
                        if (HybridVentSysAvailActualZoneNum(SysAvailNum) == state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ActualZoneNum) {
                            ActualZoneNum = HybridVentSysAvailActualZoneNum(SysAvailNum);
                        }
                    }
                }
                if (ActualZoneNum > 0) {
                    for (ANSurfaceNum = 1; ANSurfaceNum <= state.afn->AirflowNetworkNumOfSurfaces; ++ANSurfaceNum) {
                        SurfNum = state.afn->MultizoneSurfaceData(ANSurfaceNum).SurfNum;
                        if (state.dataSurface->Surface(SurfNum).Zone == ActualZoneNum) {
                            if (state.afn->VentilationCtrl == HybridVentCtrl_Close) {
                                state.afn->MultizoneSurfaceData(ANSurfaceNum).HybridVentClose = true;
                            } else {
                                if (HybridVentSysAvailWindModifier(SysAvailNum) >= 0) {
                                    state.afn->MultizoneSurfaceData(ANSurfaceNum).WindModifier =
                                        HybridVentSysAvailWindModifier(SysAvailNum);
                                }
                                if (ControlType == GlobalCtrlType) {
                                    state.afn->MultizoneSurfaceData(ANSurfaceNum).HybridCtrlGlobal = true;
                                    if (HybridVentSysAvailMaster(SysAvailNum) == ActualZoneNum) {
                                        if ((state.dataSurface->SurfWinOriginalClass(SurfNum) == SurfaceClass::Window ||
                                             state.dataSurface->SurfWinOriginalClass(SurfNum) == SurfaceClass::Door ||
                                             state.dataSurface->SurfWinOriginalClass(SurfNum) == SurfaceClass::GlassDoor) &&
                                            state.dataSurface->Surface(SurfNum).ExtBoundCond == ExternalEnvironment) {
                                            state.afn->MultizoneSurfaceData(ANSurfaceNum).HybridCtrlMaster = true;
                                            Found = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (ControlType == GlobalCtrlType && !Found && !state.dataGlobal->WarmupFlag &&
                state.afn->VentilationCtrl != HybridVentCtrl_Close) {
                ++state.afn->HybridGlobalErrCount;
                if (state.afn->HybridGlobalErrCount < 2) {
                    ShowWarningError(state,
                                     format(RoutineName) +
                                         "The hybrid ventilation control schedule value indicates global control in the controlled zone = " +
                                         state.dataHeatBal->Zone(HybridVentSysAvailMaster(SysAvailNum)).Name);
                    ShowContinueError(state,
                                      "The exterior surface containing an opening component in the controlled zone is not found.  No global control "
                                      "will not be modeled.");
                    ShowContinueError(state, "The individual control is assumed.");
                    ShowContinueErrorTimeStamp(state, "");
                } else {
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        format(RoutineName) + "The hybrid ventilation control requires a global control. The individual control continues...",
                        state.afn->HybridGlobalErrIndex,
                        double(ControlType),
                        double(ControlType));
                }
            }
        }
    }

    void CalcSingleSidedCps(EnergyPlusData &state, std::vector<std::vector<Real64>> &valsByFacade, int numWindDir)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Sam Brunswick
        //       DATE WRITTEN   September 2013
        //       MODIFIED       Revised by J. DeGraw, May 2017, to use tables
        //       RE-ENGINEERED  n/a

        // PURPOSE OF THIS SUBROUTINE:
        // Modify the wind pressure coefficients for single sided ventilation.

        // Using/Aliasing
        using namespace DataEnvironment;

        // Locals
        int windDirNum;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int AFNZnNum; // counters
        int SrfNum;
        int ExtOpenNum;
        int ZnNum;
        int DetOpenNum; // row index of surface in state.afn->MultizoneCompDetOpeningData
        int SimOpenNum; // row index of surface in MultizoneCompSimOpeningData
        int MZDZoneNum; // row index of surface zone in state.afn->MultizoneZoneData
        Real64 X1;
        Real64 Y1;
        Real64 X2;
        Real64 Y2;
        Real64 ZoneAng1;
        Real64 ZoneAng2;
        Real64 ZoneAngDiff;
        Array1D<Real64> ZoneAng;        // Azimuth angle of the exterior wall of the zone
        Array1D<Real64> PiFormula;      // Formula for the mean pressure difference
        Array1D<Real64> SigmaFormula;   // Formula for the fluctuating pressure difference
        Array1D<Real64> Sprime;         // The dimensionless ratio of the window separation to the building width
        Array1D<Real64> CPV1;           // Wind pressure coefficient for the first opening in the zone
        Array1D<Real64> CPV2;           // Wind pressure coefficient for the second opening in the zone
        std::string Name;               // External node name
        Array1D_int NumofExtSurfInZone; // List of the number of exterior openings in each zone

        struct AFNExtSurfacesProp // External opening information
        {
            // Members
            int SurfNum;          // row index of the external opening in the Surface array
            std::string SurfName; // Surface name
            int MSDNum;           // row index of the external opening in the state.afn->MultizoneSurfaceData array
            int ZoneNum;          // EnergyPlus zone number
            int MZDZoneNum;       // row index of the zone in the state.afn->MultizoneZoneData array
            int ExtNodeNum;       // External node number; = row index in state.afn->MultizoneExternalNodeData array +
                                  // state.afn->AirflowNetworkNumOfZones
            std::string ZoneName; // EnergyPlus zone name
            int facadeNum;
            int curve;                     // wind pressure coefficient curve index
            iComponentTypeNum CompTypeNum; // Opening type (detailed, simple, etc.)
            Real64 NodeHeight;             // Elevation of the opening node
            Real64 OpeningArea;            // Opening area (=Height*Width)
            Real64 Height;                 // Opening height = state.afn->MultizoneSurfaceData()%Height
            Real64 Width;                  // Opening width  = state.afn->MultizoneSurfaceData()%Width
            Real64 DischCoeff;             // Opening discharge coefficient

            // Default Constructor
            AFNExtSurfacesProp()
                : SurfNum(0), MSDNum(0), ZoneNum(0), MZDZoneNum(0), ExtNodeNum(0), facadeNum(0), curve(0), CompTypeNum(iComponentTypeNum::Invalid),
                  NodeHeight(0.0), OpeningArea(0.0), Height(0.0), Width(0.0), DischCoeff(0.0)
            {
            }
        };

        // Object Data
        Array1D<AFNExtSurfacesProp> AFNExtSurfaces; // Surface numbers of all exterior openings
        auto &solver = state.afn;
        // Count the total number of exterior simple and detailed openings and the number in each zone
        // Verify that each zone with "ADVANCED" single sided wind pressure coefficients has exactly two openings.
        // If it doesn't have two openings, change "ADVANCED" to "STANDARD"
        NumofExtSurfInZone.dimension(state.afn->AirflowNetworkNumOfZones, 0);
        for (AFNZnNum = 1; AFNZnNum <= state.afn->AirflowNetworkNumOfZones; ++AFNZnNum) {
            if (state.afn->MultizoneZoneData(AFNZnNum).SingleSidedCpType == "ADVANCED") {
                for (SrfNum = 1; SrfNum <= state.afn->AirflowNetworkNumOfSurfaces; ++SrfNum) {
                    if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(SrfNum).SurfNum).ExtBoundCond ==
                        ExternalEnvironment) { // check if outdoor boundary condition
                        MZDZoneNum = UtilityRoutines::FindItemInList(
                            state.dataSurface->Surface(state.afn->MultizoneSurfaceData(SrfNum).SurfNum).ZoneName,
                            state.afn->MultizoneZoneData,
                            &MultizoneZoneProp::ZoneName);
                        if (MZDZoneNum == AFNZnNum) {
                            // This is terrible, should not do it this way
                            auto afe = solver->elements.find(state.afn->MultizoneSurfaceData(SrfNum).OpeningName);
                            if (afe != solver->elements.end()) {
                                auto type = afe->second->type();
                                if (type == ComponentType::DOP) {
                                    ++state.afn->AFNNumOfExtOpenings;
                                    ++NumofExtSurfInZone(AFNZnNum);
                                } else if (type == ComponentType::SOP) {
                                    ++state.afn->AFNNumOfExtOpenings;
                                    ++NumofExtSurfInZone(AFNZnNum);
                                }
                            }
                        }
                    }
                }
                if (NumofExtSurfInZone(AFNZnNum) == 0) {
                    ShowWarningError(state,
                                     "AirflowNetwork:Multizone:Zone = " + state.afn->MultizoneZoneData(AFNZnNum).ZoneName +
                                         " has single side wind pressure coefficient type \"ADVANCED\", but has no exterior "
                                         "AirflowNetwork:MultiZone:Component:DetailedOpening and/or AirflowNetwork:MultiZone:Component:SimpleOpening "
                                         "objects.");
                    ShowContinueError(state,
                                      "Zones must have exactly two exterior openings in order for the \"ADVANCED\" single sided wind pressure "
                                      "coefficient model to be used.");
                    ShowContinueError(state,
                                      "The wind pressure coefficient model for this zone will be set to \"STANDARD\" and simulation continues.");
                    state.afn->MultizoneZoneData(AFNZnNum).SingleSidedCpType = "STANDARD";
                } else if (NumofExtSurfInZone(AFNZnNum) == 1) {
                    ShowWarningError(state, "AirflowNetwork:Multizone:Zone = " + state.afn->MultizoneZoneData(AFNZnNum).ZoneName);
                    ShowContinueError(state,
                                      "has single side wind pressure coefficient type \"ADVANCED\", but has only one exterior "
                                      "AirflowNetwork:MultiZone:Component:DetailedOpening and/or "
                                      "AirflowNetwork:MultiZone:Component:SimpleOpening objects.");
                    ShowContinueError(state,
                                      "Zones must have exactly two openings in order for the \"ADVANCED\" single side wind pressure coefficient "
                                      "model to be used.");
                    ShowContinueError(state,
                                      "The wind pressure coefficient model for this zone will be set to \"STANDARD\" and simulation continues.");
                    state.afn->MultizoneZoneData(AFNZnNum).SingleSidedCpType = "STANDARD";
                } else if (NumofExtSurfInZone(AFNZnNum) > 2) {
                    ShowWarningError(state,
                                     format("AirflowNetwork:Multizone:Zone = {} has single side wind pressure coefficient type "
                                            "\"ADVANCED\", but has {} exterior "
                                            "AirflowNetwork:MultiZone:Component:DetailedOpening and/or "
                                            "AirflowNetwork:MultiZone:Component:SimpleOpening objects.",
                                            state.afn->MultizoneZoneData(AFNZnNum).ZoneName,
                                            NumofExtSurfInZone(AFNZnNum)));
                    ShowContinueError(state,
                                      "Zones must have exactly two openings in order for the \"ADVANCED\" single side wind pressure coefficient "
                                      "model to be used.");
                    ShowContinueError(state,
                                      "The wind pressure coefficient model for this zone will be set to \"STANDARD\" and simulation continues.");
                    state.afn->MultizoneZoneData(AFNZnNum).SingleSidedCpType = "STANDARD";
                }
            }
        }
        if (state.afn->AFNNumOfExtOpenings == 0) return;
        // Recount the number of single sided zones
        state.afn->AirflowNetworkNumOfSingleSideZones = 0;
        for (AFNZnNum = 1; AFNZnNum <= state.afn->AirflowNetworkNumOfZones; ++AFNZnNum) {
            if (state.afn->MultizoneZoneData(AFNZnNum).SingleSidedCpType == "ADVANCED") {
                ++state.afn->AirflowNetworkNumOfSingleSideZones;
            }
        }
        if (state.afn->AirflowNetworkNumOfSingleSideZones == 0)
            return; // Bail if no zones call for the advanced single sided model.
        // Recount the number of detailed and simple exterior openings in zones with "ADVANCED" single sided wind pressure coefficients
        state.afn->AFNNumOfExtOpenings = 0;
        for (SrfNum = 1; SrfNum <= state.afn->AirflowNetworkNumOfSurfaces; ++SrfNum) {
            MZDZoneNum =
                UtilityRoutines::FindItemInList(state.dataSurface->Surface(state.afn->MultizoneSurfaceData(SrfNum).SurfNum).ZoneName,
                                                state.afn->MultizoneZoneData,
                                                &MultizoneZoneProp::ZoneName);
            if (state.afn->MultizoneZoneData(MZDZoneNum).SingleSidedCpType == "ADVANCED") {
                if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(SrfNum).SurfNum).ExtBoundCond ==
                    ExternalEnvironment) { // check if outdoor boundary condition
                    // This is terrible, should not do it this way
                    auto afe = solver->elements.find(state.afn->MultizoneSurfaceData(SrfNum).OpeningName);
                    if (afe != solver->elements.end()) {
                        auto type = afe->second->type();
                        if (type == ComponentType::DOP) {
                            ++state.afn->AFNNumOfExtOpenings;
                        } else if (type == ComponentType::SOP) {
                            ++state.afn->AFNNumOfExtOpenings;
                        }
                    }
                }
            }
        }
        AFNExtSurfaces.allocate(state.afn->AFNNumOfExtOpenings);
        // Create array of properties for all the exterior single sided openings
        ExtOpenNum = 1;
        for (SrfNum = 1; SrfNum <= state.afn->AirflowNetworkNumOfSurfaces; ++SrfNum) {
            if (state.dataSurface->Surface(state.afn->MultizoneSurfaceData(SrfNum).SurfNum).ExtBoundCond == ExternalEnvironment) {
                if (state.afn->AirflowNetworkNumOfDetOpenings > 0) {
                    DetOpenNum = UtilityRoutines::FindItemInList(state.afn->MultizoneSurfaceData(SrfNum).OpeningName,
                                                                 state.afn->MultizoneCompDetOpeningData,
                                                                 &AirflowNetwork::DetailedOpening::name);
                    MZDZoneNum = UtilityRoutines::FindItemInList(
                        state.dataSurface->Surface(state.afn->MultizoneSurfaceData(SrfNum).SurfNum).ZoneName,
                        state.afn->MultizoneZoneData,
                        &MultizoneZoneProp::ZoneName);
                    if (state.afn->MultizoneZoneData(MZDZoneNum).SingleSidedCpType == "ADVANCED") {
                        if (DetOpenNum > 0) {
                            AFNExtSurfaces(ExtOpenNum).MSDNum = SrfNum;
                            AFNExtSurfaces(ExtOpenNum).SurfNum = state.afn->MultizoneSurfaceData(SrfNum).SurfNum;
                            AFNExtSurfaces(ExtOpenNum).NodeHeight = state.dataSurface->Surface(AFNExtSurfaces(ExtOpenNum).SurfNum).Centroid.z;
                            AFNExtSurfaces(ExtOpenNum).SurfName =
                                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(SrfNum).SurfNum).Name;
                            AFNExtSurfaces(ExtOpenNum).ZoneNum =
                                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(SrfNum).SurfNum).Zone;
                            AFNExtSurfaces(ExtOpenNum).ZoneName =
                                state.dataSurface->Surface(state.afn->MultizoneSurfaceData(SrfNum).SurfNum).ZoneName;
                            AFNExtSurfaces(ExtOpenNum).MZDZoneNum = UtilityRoutines::FindItemInList(
                                AFNExtSurfaces(ExtOpenNum).ZoneName, state.afn->MultizoneZoneData, &MultizoneZoneProp::ZoneName);
                            AFNExtSurfaces(ExtOpenNum).CompTypeNum = iComponentTypeNum::DOP;
                            AFNExtSurfaces(ExtOpenNum).Height = state.afn->MultizoneSurfaceData(SrfNum).Height;
                            AFNExtSurfaces(ExtOpenNum).Width = state.afn->MultizoneSurfaceData(SrfNum).Width;
                            AFNExtSurfaces(ExtOpenNum).OpeningArea = state.afn->MultizoneSurfaceData(SrfNum).Width *
                                                                     state.afn->MultizoneSurfaceData(SrfNum).Height *
                                                                     state.afn->MultizoneSurfaceData(SrfNum).OpenFactor;
                            AFNExtSurfaces(ExtOpenNum).ExtNodeNum = state.afn->MultizoneSurfaceData(ExtOpenNum).NodeNums[1];
                            AFNExtSurfaces(ExtOpenNum).facadeNum = state.afn
                                                                       ->MultizoneExternalNodeData(AFNExtSurfaces(ExtOpenNum).ExtNodeNum -
                                                                                                   state.afn->AirflowNetworkNumOfZones)
                                                                       .facadeNum;
                            AFNExtSurfaces(ExtOpenNum).curve = state.afn
                                                                   ->MultizoneExternalNodeData(AFNExtSurfaces(ExtOpenNum).ExtNodeNum -
                                                                                               state.afn->AirflowNetworkNumOfZones)
                                                                   .curve;
                            AFNExtSurfaces(ExtOpenNum).DischCoeff = state.afn->MultizoneCompDetOpeningData(DetOpenNum).DischCoeff2;
                            ++ExtOpenNum;
                        }
                    }
                } else if (state.afn->AirflowNetworkNumOfSimOpenings > 0) {
                    SimOpenNum = UtilityRoutines::FindItemInList(state.afn->MultizoneSurfaceData(SrfNum).OpeningName,
                                                                 state.afn->MultizoneCompSimpleOpeningData,
                                                                 &AirflowNetwork::SimpleOpening::name);
                    if (SimOpenNum > 0) {
                        AFNExtSurfaces(ExtOpenNum).MSDNum = SrfNum;
                        AFNExtSurfaces(ExtOpenNum).SurfNum = state.afn->MultizoneSurfaceData(SrfNum).SurfNum;
                        AFNExtSurfaces(ExtOpenNum).SurfName =
                            state.dataSurface->Surface(state.afn->MultizoneSurfaceData(SrfNum).SurfNum).Name;
                        AFNExtSurfaces(ExtOpenNum).ZoneNum =
                            state.dataSurface->Surface(state.afn->MultizoneSurfaceData(SrfNum).SurfNum).Zone;
                        AFNExtSurfaces(ExtOpenNum).ZoneName =
                            state.dataSurface->Surface(state.afn->MultizoneSurfaceData(SrfNum).SurfNum).ZoneName;
                        AFNExtSurfaces(ExtOpenNum).MZDZoneNum = UtilityRoutines::FindItemInList(
                            AFNExtSurfaces(ExtOpenNum).ZoneName, state.afn->MultizoneZoneData, &MultizoneZoneProp::ZoneName);
                        AFNExtSurfaces(ExtOpenNum).CompTypeNum = iComponentTypeNum::SOP;
                        AFNExtSurfaces(ExtOpenNum).Height = state.afn->MultizoneSurfaceData(SrfNum).Height;
                        AFNExtSurfaces(ExtOpenNum).Width = state.afn->MultizoneSurfaceData(SrfNum).Width;
                        AFNExtSurfaces(ExtOpenNum).OpeningArea = state.afn->MultizoneSurfaceData(SrfNum).Width *
                                                                 state.afn->MultizoneSurfaceData(SrfNum).Height *
                                                                 state.afn->MultizoneSurfaceData(SrfNum).OpenFactor;
                        AFNExtSurfaces(ExtOpenNum).ExtNodeNum = state.afn->MultizoneSurfaceData(ExtOpenNum).NodeNums[1];
                        AFNExtSurfaces(ExtOpenNum).curve = state.afn
                                                               ->MultizoneExternalNodeData(AFNExtSurfaces(ExtOpenNum).ExtNodeNum -
                                                                                           state.afn->AirflowNetworkNumOfZones)
                                                               .curve;
                        AFNExtSurfaces(ExtOpenNum).DischCoeff = state.afn->MultizoneCompSimpleOpeningData(SimOpenNum).DischCoeff;
                        ++ExtOpenNum;
                    }
                }
            }
        }
        // Calculate the azimuth and the coordinates of the centroid of each opening.
        // Calculate Sprime and state.afn->DeltaCp for each zone.
        PiFormula.allocate(numWindDir);
        SigmaFormula.allocate(numWindDir);
        state.afn->DeltaCp.allocate(state.afn->AirflowNetworkNumOfZones);
        state.afn->EPDeltaCP.allocate(state.afn->AirflowNetworkNumOfZones);
        Sprime.allocate(state.afn->AirflowNetworkNumOfZones);
        ZoneAng.allocate(state.afn->AirflowNetworkNumOfZones);
        for (ZnNum = 1; ZnNum <= state.afn->AirflowNetworkNumOfZones; ++ZnNum) {
            state.afn->DeltaCp(ZnNum).WindDir.allocate(numWindDir);
            state.afn->EPDeltaCP(ZnNum).WindDir.allocate(numWindDir);
            for (windDirNum = 1; windDirNum <= numWindDir; ++windDirNum) {
                state.afn->DeltaCp(ZnNum).WindDir(windDirNum) = 0.0;
                state.afn->EPDeltaCP(ZnNum).WindDir(windDirNum) = 0.0;
            }
        }
        Sprime = 0.0;
        ZoneAng = 0.0;
        for (ZnNum = 1; ZnNum <= state.afn->AirflowNetworkNumOfZones; ++ZnNum) {
            if (state.afn->MultizoneZoneData(ZnNum).SingleSidedCpType == "ADVANCED") {
                state.afn->OpenNuminZone = 1;
                for (ExtOpenNum = 1; ExtOpenNum <= state.afn->AFNNumOfExtOpenings; ++ExtOpenNum) {
                    if (state.afn->OpenNuminZone > 2) break; // Tuned
                    if (AFNExtSurfaces(ExtOpenNum).MZDZoneNum == ZnNum) {
                        if (state.afn->OpenNuminZone == 1) {
                            X1 = state.dataSurface->Surface(AFNExtSurfaces(ExtOpenNum).SurfNum).Centroid.x;
                            Y1 = state.dataSurface->Surface(AFNExtSurfaces(ExtOpenNum).SurfNum).Centroid.y;
                            ZoneAng1 = state.dataSurface->Surface(AFNExtSurfaces(ExtOpenNum).SurfNum).Azimuth;
                            ++state.afn->OpenNuminZone;
                        } else if (state.afn->OpenNuminZone == 2) {
                            X2 = state.dataSurface->Surface(AFNExtSurfaces(ExtOpenNum).SurfNum).Centroid.x;
                            Y2 = state.dataSurface->Surface(AFNExtSurfaces(ExtOpenNum).SurfNum).Centroid.y;
                            ZoneAng2 = state.dataSurface->Surface(AFNExtSurfaces(ExtOpenNum).SurfNum).Azimuth;
                            ++state.afn->OpenNuminZone;
                        }
                    }
                }
                ZoneAngDiff = ZoneAng1 - ZoneAng2;
                if (ZoneAngDiff > 0.01) {
                    ShowWarningError(state,
                                     "AirflowNetwork:Multizone:Zone = " + state.afn->MultizoneZoneData(AFNZnNum).ZoneName +
                                         " has single side wind pressure coefficient type \"ADVANCED\", but has openings which are not coplanar.");
                    ShowContinueError(state, "The openings should be coplanar for the model to be valid. Simulation Continues.");
                }
                ZoneAng(ZnNum) = ZoneAng1;
                Sprime(ZnNum) = std::sqrt(pow_2(X1 - X2) + pow_2(Y1 - Y2)) / state.afn->MultizoneZoneData(ZnNum).BuildWidth;
                // Calculate state.afn->DeltaCp for each wind direction for each zone
                for (windDirNum = 1; windDirNum <= numWindDir; ++windDirNum) {
                    state.dataEnvrn->WindDir = (windDirNum - 1) * 10.0;
                    Real64 WindAng = (windDirNum - 1) * 10.0;
                    state.afn->IncAng = std::abs(WindAng - ZoneAng(ZnNum));
                    if (std::abs(state.afn->IncAng) > 180.0) state.afn->IncAng -= 360.0;
                    if (UtilityRoutines::SameString(state.afn->AirflowNetworkSimu.WPCCntr, "SurfaceAverageCalculation")) {
                        if (std::abs(state.afn->IncAng) <= 67.5) {
                            PiFormula(windDirNum) =
                                0.44 *
                                sign(std::sin(2.67 * std::abs(state.afn->IncAng) * DataGlobalConstants::Pi / 180.0),
                                     state.afn->IncAng);
                        } else if (std::abs(state.afn->IncAng) <= 180.0) {
                            PiFormula(windDirNum) = -0.69 * sign(std::sin((288 - 1.6 * std::abs(state.afn->IncAng)) *
                                                                          DataGlobalConstants::Pi / 180.0),
                                                                 state.afn->IncAng);
                        }
                        SigmaFormula(windDirNum) = 0.423 - 0.00163 * std::abs(state.afn->IncAng);
                        state.afn->DeltaCp(ZnNum).WindDir(windDirNum) =
                            (0.02 + (0.346 * std::abs(PiFormula(windDirNum)) + 0.084 * SigmaFormula(windDirNum)) * Sprime(ZnNum));
                    }
                }
            }
        }

        // Calculate the single sided Cp arrays from state.afn->DeltaCp for each single sided opening
        CPV1.allocate(numWindDir); // These two arrays should probably be removed
        CPV2.allocate(numWindDir);
        CPV1 = 0.0;
        CPV2 = 0.0;
        SrfNum = 6;
        for (ZnNum = 1; ZnNum <= state.afn->AirflowNetworkNumOfZones; ++ZnNum) {
            if (state.afn->MultizoneZoneData(ZnNum).SingleSidedCpType == "ADVANCED") {
                state.afn->OpenNuminZone = 1;
                for (ExtOpenNum = 1; ExtOpenNum <= state.afn->AFNNumOfExtOpenings; ++ExtOpenNum) {
                    if (state.afn->OpenNuminZone > 2) break; // Tuned
                    if (AFNExtSurfaces(ExtOpenNum).MZDZoneNum == ZnNum) {
                        Real64 const VelRatio_2(std::pow(10.0 / AFNExtSurfaces(ExtOpenNum).NodeHeight, 2.0 * state.dataEnvrn->SiteWindExp));
                        Real64 const AFNEExtSurface_fac(0.5 * (1.0 / pow_2(AFNExtSurfaces(ExtOpenNum).DischCoeff)));
                        if (state.afn->OpenNuminZone == 1) {
                            std::vector<Real64> cpvalues(numWindDir);
                            for (windDirNum = 1; windDirNum <= numWindDir; ++windDirNum) {
                                Real64 unmodifiedValue = valsByFacade[AFNExtSurfaces(ExtOpenNum).facadeNum - 1][windDirNum - 1] +
                                                         AFNEExtSurface_fac * state.afn->DeltaCp(ZnNum).WindDir(windDirNum);
                                cpvalues[windDirNum - 1] = CPV1(windDirNum) = VelRatio_2 * unmodifiedValue;
                            }
                            valsByFacade.push_back(cpvalues);
                            state.afn
                                ->MultizoneExternalNodeData(AFNExtSurfaces(ExtOpenNum).ExtNodeNum -
                                                            state.afn->AirflowNetworkNumOfZones)
                                .facadeNum = SrfNum;
                            ++state.afn->OpenNuminZone;
                            ++SrfNum;
                        } else if (state.afn->OpenNuminZone == 2) {
                            std::vector<Real64> cpvalues(numWindDir);
                            for (windDirNum = 1; windDirNum <= numWindDir; ++windDirNum) {
                                Real64 unmodifiedValue = valsByFacade[AFNExtSurfaces(ExtOpenNum).facadeNum - 1][windDirNum - 1] -
                                                         AFNEExtSurface_fac * state.afn->DeltaCp(ZnNum).WindDir(windDirNum);
                                cpvalues[windDirNum - 1] = CPV2(windDirNum) = VelRatio_2 * unmodifiedValue;
                                state.afn->EPDeltaCP(ZnNum).WindDir(windDirNum) = std::abs(CPV2(windDirNum) - CPV1(windDirNum));
                            }
                            valsByFacade.push_back(cpvalues);
                            state.afn
                                ->MultizoneExternalNodeData(AFNExtSurfaces(ExtOpenNum).ExtNodeNum -
                                                            state.afn->AirflowNetworkNumOfZones)
                                .facadeNum = SrfNum;
                            ++state.afn->OpenNuminZone;
                            ++SrfNum;
                        }
                    }
                }
            }
        }
        // Rewrite the CPVNum for all nodes that correspond with a simple or detailed opening
        // Does this loop really do anything?
        for (ZnNum = 1; ZnNum <= state.afn->AirflowNetworkNumOfZones; ++ZnNum) {
            state.afn->OpenNuminZone = 1;
            for (ExtOpenNum = 1; ExtOpenNum <= state.afn->AFNNumOfExtOpenings; ++ExtOpenNum) {
                if (AFNExtSurfaces(ExtOpenNum).MZDZoneNum == ZnNum) {
                    if (state.afn->OpenNuminZone == 1) {
                        ++state.afn->OpenNuminZone;
                    } else if (state.afn->OpenNuminZone == 2) {
                        ++state.afn->OpenNuminZone;
                    }
                }
            }
        }
    }

    Real64 GetZoneOutdoorAirChangeRate(EnergyPlusData &state, int const ZoneNum) // hybrid ventilation system controlled zone number
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   May. 2007
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This function outputs air change per hour in a given zone

        // Using/Aliasing
        auto &TimeStepSys = state.dataHVACGlobal->TimeStepSys;

        // Return value
        Real64 ACH; // Zone air change rate [ACH]

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 InfilVolume; // Zone infiltration volume
        Real64 RhoAir;      // Zone air density [kg/m3]
        Real64 CpAir;       // Zone air specific heat

        CpAir = PsyCpAirFnW(state.dataHeatBalFanSys->ZoneAirHumRat(ZoneNum));
        RhoAir = PsyRhoAirFnPbTdbW(
            state, state.dataEnvrn->OutBaroPress, state.dataHeatBalFanSys->MAT(ZoneNum), state.dataHeatBalFanSys->ZoneAirHumRat(ZoneNum));
        InfilVolume = ((state.afn->exchangeData(ZoneNum).SumMCp +
                        state.afn->exchangeData(ZoneNum).SumMVCp) /
                       CpAir / RhoAir) *
                      TimeStepSys * DataGlobalConstants::SecInHour;
        ACH = InfilVolume / (TimeStepSys * state.dataHeatBal->Zone(ZoneNum).Volume);

        return ACH;
    }

    int GetAirLoopNumber(EnergyPlusData &state, int const NodeNumber) // Get air loop number for each distribution node and linkage
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Feb. 2018

        // PURPOSE OF THIS SUBROUTINE:
        // This function outputs an AirLoopNum based on node number

        // Using/Aliasing
        using BranchNodeConnections::GetChildrenData;
        using BranchNodeConnections::GetNumChildren;
        using BranchNodeConnections::IsParentObject;
        auto &NumPrimaryAirSys = state.dataHVACGlobal->NumPrimaryAirSys;
        using SingleDuct::GetHVACSingleDuctSysIndex;

        // Return value
        int AirLoopNumber = 0;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int BranchNum;
        int NumOfNodes;
        int NodeNum;
        int OutNum;
        int SupAirPath;
        int SupAirPathOutNodeNum;
        int CtrlZoneNum;
        int ZoneInNum;
        int ZoneOutNum;
        int AirLoopNum;
        int TUNum = 0;
        int TermNum = 0;
        bool LocalError;
        int NumOfComp;
        int NumOfSubComp;
        bool ErrorsFound;
        std::string TypeOfComp;
        std::string NameOfComp;
        int NumOfSubSubComp;

        for (AirLoopNum = 1; AirLoopNum <= NumPrimaryAirSys; ++AirLoopNum) {
            // Check OAMixer OA inlet node
            if (NodeNumber == state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).OAMixOAInNodeNum) {
                return AirLoopNum;
            }
            // Check branch
            for (BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumBranches; ++BranchNum) {
                NumOfNodes = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).TotalNodes;
                for (NodeNum = 1; NodeNum <= NumOfNodes; ++NodeNum) {
                    if (NodeNumber == state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).NodeNum(NodeNum)) {
                        return AirLoopNum;
                    }
                }
                for (NumOfComp = 1; NumOfComp <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).TotalComponents;
                     ++NumOfComp) {
                    if (NodeNumber == state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(NumOfComp).NodeNumIn) {
                        return AirLoopNum;
                    }
                    if (NodeNumber == state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(NumOfComp).NodeNumOut) {
                        return AirLoopNum;
                    }
                    if (state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(NumOfComp).NumSubComps == 0) {
                        std::string TypeOfComp = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(NumOfComp).TypeOf;
                        std::string NameOfComp = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(NumOfComp).Name;
                        if (IsParentObject(state, TypeOfComp, NameOfComp)) {

                            int NumChildren = GetNumChildren(state, TypeOfComp, NameOfComp);
                            Array1D_string SubCompTypes;
                            Array1D_string SubCompNames;
                            Array1D_string InletNodeNames;
                            Array1D_int InletNodeNumbers;
                            Array1D_string OutletNodeNames;
                            Array1D_int OutletNodeNumbers;

                            SubCompTypes.allocate(NumChildren);
                            SubCompNames.allocate(NumChildren);
                            InletNodeNames.allocate(NumChildren);
                            InletNodeNumbers.allocate(NumChildren);
                            OutletNodeNames.allocate(NumChildren);
                            OutletNodeNumbers.allocate(NumChildren);

                            GetChildrenData(state,
                                            TypeOfComp,
                                            NameOfComp,
                                            NumChildren,
                                            SubCompTypes,
                                            SubCompNames,
                                            InletNodeNames,
                                            InletNodeNumbers,
                                            OutletNodeNames,
                                            OutletNodeNumbers,
                                            ErrorsFound);

                            for (NumOfSubComp = 1; NumOfSubComp <= NumChildren; ++NumOfSubComp) {
                                if (NodeNumber == InletNodeNumbers(NumOfSubComp)) {
                                    SubCompTypes.deallocate();
                                    SubCompNames.deallocate();
                                    InletNodeNames.deallocate();
                                    InletNodeNumbers.deallocate();
                                    OutletNodeNames.deallocate();
                                    OutletNodeNumbers.deallocate();
                                    return AirLoopNum;
                                }
                                if (NodeNumber == OutletNodeNumbers(NumOfSubComp)) {
                                    SubCompTypes.deallocate();
                                    SubCompNames.deallocate();
                                    InletNodeNames.deallocate();
                                    InletNodeNumbers.deallocate();
                                    OutletNodeNames.deallocate();
                                    OutletNodeNumbers.deallocate();
                                    return AirLoopNum;
                                }
                            }
                            for (NumOfSubComp = 1; NumOfSubComp <= NumChildren; ++NumOfSubComp) {
                                std::string TypeOfComp = SubCompTypes(NumOfSubComp);
                                std::string NameOfComp = SubCompNames(NumOfSubComp);
                                if (IsParentObject(state, TypeOfComp, NameOfComp)) {

                                    int NumGrandChildren = GetNumChildren(state, TypeOfComp, NameOfComp);
                                    Array1D_string SubSubCompTypes;
                                    Array1D_string SubSubCompNames;
                                    Array1D_string SubSubInletNodeNames;
                                    Array1D_int SubSubInletNodeNumbers;
                                    Array1D_string SubSubOutletNodeNames;
                                    Array1D_int SubSubOutletNodeNumbers;

                                    SubSubCompTypes.allocate(NumGrandChildren);
                                    SubSubCompNames.allocate(NumGrandChildren);
                                    SubSubInletNodeNames.allocate(NumGrandChildren);
                                    SubSubInletNodeNumbers.allocate(NumGrandChildren);
                                    SubSubOutletNodeNames.allocate(NumGrandChildren);
                                    SubSubOutletNodeNumbers.allocate(NumGrandChildren);

                                    GetChildrenData(state,
                                                    TypeOfComp,
                                                    NameOfComp,
                                                    NumGrandChildren,
                                                    SubSubCompTypes,
                                                    SubSubCompNames,
                                                    SubSubInletNodeNames,
                                                    SubSubInletNodeNumbers,
                                                    SubSubOutletNodeNames,
                                                    SubSubOutletNodeNumbers,
                                                    ErrorsFound);
                                    for (int SubSubCompNum = 1; SubSubCompNum <= NumGrandChildren; ++SubSubCompNum) {
                                        if (NodeNumber == SubSubInletNodeNumbers(SubSubCompNum)) {
                                            SubSubCompTypes.deallocate();
                                            SubSubCompNames.deallocate();
                                            SubSubInletNodeNames.deallocate();
                                            SubSubInletNodeNumbers.deallocate();
                                            SubSubOutletNodeNames.deallocate();
                                            SubSubOutletNodeNumbers.deallocate();
                                            SubCompTypes.deallocate();
                                            SubCompNames.deallocate();
                                            InletNodeNames.deallocate();
                                            InletNodeNumbers.deallocate();
                                            OutletNodeNames.deallocate();
                                            OutletNodeNumbers.deallocate();
                                            return AirLoopNum;
                                        }
                                        if (NodeNumber == SubSubOutletNodeNumbers(SubSubCompNum)) {
                                            SubSubCompTypes.deallocate();
                                            SubSubCompNames.deallocate();
                                            SubSubInletNodeNames.deallocate();
                                            SubSubInletNodeNumbers.deallocate();
                                            SubSubOutletNodeNames.deallocate();
                                            SubSubOutletNodeNumbers.deallocate();
                                            SubCompTypes.deallocate();
                                            SubCompNames.deallocate();
                                            InletNodeNames.deallocate();
                                            InletNodeNumbers.deallocate();
                                            OutletNodeNames.deallocate();
                                            OutletNodeNumbers.deallocate();
                                            return AirLoopNum;
                                        }
                                    }
                                    SubSubCompTypes.deallocate();
                                    SubSubCompNames.deallocate();
                                    SubSubInletNodeNames.deallocate();
                                    SubSubInletNodeNumbers.deallocate();
                                    SubSubOutletNodeNames.deallocate();
                                    SubSubOutletNodeNumbers.deallocate();
                                }
                            }

                            SubCompTypes.deallocate();
                            SubCompNames.deallocate();
                            InletNodeNames.deallocate();
                            InletNodeNumbers.deallocate();
                            OutletNodeNames.deallocate();
                            OutletNodeNumbers.deallocate();
                        }
                    } else {
                        for (NumOfSubComp = 1;
                             NumOfSubComp <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(NumOfComp).NumSubComps;
                             ++NumOfSubComp) {
                            if (NodeNumber == state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                                                  .Branch(BranchNum)
                                                  .Comp(NumOfComp)
                                                  .SubComp(NumOfSubComp)
                                                  .NodeNumIn) {
                                return AirLoopNum;
                            }
                            if (NodeNumber == state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                                                  .Branch(BranchNum)
                                                  .Comp(NumOfComp)
                                                  .SubComp(NumOfSubComp)
                                                  .NodeNumOut) {
                                return AirLoopNum;
                            }
                            for (NumOfSubSubComp = 1; NumOfSubSubComp <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                                                                             .Branch(BranchNum)
                                                                             .Comp(NumOfComp)
                                                                             .SubComp(NumOfSubComp)
                                                                             .NumSubSubComps;
                                 ++NumOfSubSubComp) {
                                if (NodeNumber == state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                                                      .Branch(BranchNum)
                                                      .Comp(NumOfComp)
                                                      .SubComp(NumOfSubComp)
                                                      .SubSubComp(NumOfSubSubComp)
                                                      .NodeNumIn) {
                                    return AirLoopNum;
                                }
                                if (NodeNumber == state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum)
                                                      .Branch(BranchNum)
                                                      .Comp(NumOfComp)
                                                      .SubComp(NumOfSubComp)
                                                      .SubSubComp(NumOfSubSubComp)
                                                      .NodeNumOut) {
                                    return AirLoopNum;
                                }
                            }
                        }
                    }
                }
            }

            // Check connection between supply and demand
            for (OutNum = 1; OutNum <= state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumSupplyNodes; ++OutNum) {
                // AirLoop supply outlet node
                if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).AirLoopSupplyNodeNum(OutNum) == NodeNumber) {
                    return AirLoopNum;
                }
                if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(OutNum) == NodeNumber) {
                    return AirLoopNum;
                }
                // supply path
                for (SupAirPath = 1; SupAirPath <= state.dataZoneEquip->NumSupplyAirPaths; ++SupAirPath) {
                    if (state.dataZoneEquip->SupplyAirPath(SupAirPath).InletNodeNum ==
                        state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(OutNum)) {
                        for (SupAirPathOutNodeNum = 1; SupAirPathOutNodeNum <= state.dataZoneEquip->SupplyAirPath(SupAirPath).NumOutletNodes;
                             ++SupAirPathOutNodeNum) {
                            if (state.dataZoneEquip->SupplyAirPath(SupAirPath).OutletNode(SupAirPathOutNodeNum) == NodeNumber) {
                                return AirLoopNum;
                            }
                            for (TUNum = 1; TUNum <= state.afn->DisSysNumOfTermUnits; ++TUNum) {
                                if (UtilityRoutines::SameString(state.afn->DisSysCompTermUnitData(TUNum).EPlusType,
                                                                "AirTerminal:SingleDuct:VAV:Reheat")) {
                                    LocalError = false;
                                    GetHVACSingleDuctSysIndex(state,
                                                              state.afn->DisSysCompTermUnitData(TUNum).name,
                                                              TermNum,
                                                              LocalError,
                                                              "AirflowNetwork:Distribution:Component:TerminalUnit",
                                                              state.afn->DisSysCompTermUnitData(TUNum).DamperInletNode,
                                                              state.afn->DisSysCompTermUnitData(TUNum).DamperOutletNode);
                                    if (state.dataZoneEquip->SupplyAirPath(SupAirPath).OutletNode(SupAirPathOutNodeNum) ==
                                        state.afn->DisSysCompTermUnitData(TUNum).DamperInletNode) {
                                        if (state.afn->DisSysCompTermUnitData(TUNum).DamperOutletNode == NodeNumber) {
                                            state.afn->DisSysCompTermUnitData(TUNum).AirLoopNum = AirLoopNum;
                                            return AirLoopNum;
                                        }
                                    }
                                    if (LocalError) {
                                    }
                                }
                            }
                        }
                    }
                }
                // return path
                if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).AirLoopReturnNodeNum(OutNum) == NodeNumber) {
                    return AirLoopNum;
                }
                if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipReturnNodeNum(OutNum) == NodeNumber) {
                    return AirLoopNum;
                }
                for (int retPathNum = 1; retPathNum <= state.dataZoneEquip->NumReturnAirPaths; ++retPathNum) {
                    if (state.dataZoneEquip->ReturnAirPath(retPathNum).OutletNodeNum ==
                        state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipReturnNodeNum(1)) {
                        if (state.dataZoneEquip->ReturnAirPath(retPathNum).OutletNodeNum == NodeNumber) {
                            return AirLoopNum;
                        }
                    }
                }
                // Supply inlet node

                // Terminal damper node
            }
        }

        for (CtrlZoneNum = 1; CtrlZoneNum <= state.dataGlobal->NumOfZones; ++CtrlZoneNum) {
            if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).IsControlled) continue;
            for (ZoneInNum = 1; ZoneInNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).NumInletNodes; ++ZoneInNum) {
                if (state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).InletNode(ZoneInNum) == NodeNumber) {
                    return state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).InletNodeAirLoopNum(ZoneInNum);
                }
            }
            for (ZoneOutNum = 1; ZoneOutNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).NumReturnNodes; ++ZoneOutNum) {
                if (state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).ReturnNode(ZoneOutNum) == NodeNumber) {
                    return state.dataZoneEquip->ZoneEquipConfig(CtrlZoneNum).ReturnNodeAirLoopNum(ZoneOutNum);
                }
            }
        }

        return AirLoopNumber;
    }

    void OccupantVentilationControlProp::calc(EnergyPlusData &state,
                                              int const ZoneNum,
                                              Real64 const TimeOpenDuration,
                                              Real64 const TimeCloseDuration,
                                              int &OpeningStatus,
                                              int &OpeningProbStatus,
                                              int &ClosingProbStatus)
    {

        Real64 Tcomfort;    // Thermal comfort temperature
        Real64 ComfortBand; // Thermal comfort band
        Real64 Toperative;  // Operative temperature
        Real64 OutDryBulb;  // Outdoor dry-bulb temperature

        auto &Zone(state.dataHeatBal->Zone);

        if (TimeOpenDuration > 0) {
            if (TimeOpenDuration >= MinOpeningTime) {
                OpeningStatus = OpenStatus::FreeOperation; // free operation
            } else {
                OpeningStatus = OpenStatus::MinCheckForceOpen; // forced to open
            }
        }
        if (TimeCloseDuration > 0) {
            if (TimeCloseDuration >= MinClosingTime) {
                OpeningStatus = OpenStatus::FreeOperation; // free operation
            } else {
                OpeningStatus = OpenStatus::MinCheckForceClose; // forced to close
            }
        }

        if (MinTimeControlOnly) return;

        if (Zone(ZoneNum).HasLinkedOutAirNode) {
            OutDryBulb = Zone(ZoneNum).OutDryBulbTemp;
        } else {
            OutDryBulb = OutDryBulbTempAt(state, Zone(ZoneNum).Centroid.z);
        }

        if (OutDryBulb < ComfortBouPoint) {
            Tcomfort = CurveValue(state, ComfortLowTempCurveNum, OutDryBulb);
        } else {
            Tcomfort = CurveValue(state, ComfortHighTempCurveNum, OutDryBulb);
        }
        ComfortBand = -0.0028 * (100 - MaxPPD) * (100 - MaxPPD) + 0.3419 * (100 - MaxPPD) - 6.6275;
        Toperative = 0.5 * (state.dataHeatBalFanSys->MAT(ZoneNum) + state.dataHeatBal->ZoneMRT(ZoneNum));

        if (Toperative > (Tcomfort + ComfortBand)) {
            if (openingProbability(state, ZoneNum, TimeCloseDuration)) {
                OpeningProbStatus = ProbabilityCheck::ForceChange;
                ; // forced to open
            } else {
                OpeningProbStatus = ProbabilityCheck::KeepStatus; // Keep previous status
            }
        } else {
            OpeningProbStatus = ProbabilityCheck::NoAction; // free operation
        }

        if (Toperative < (Tcomfort - ComfortBand)) {
            if (closingProbability(state, TimeOpenDuration)) {
                ClosingProbStatus = ProbabilityCheck::ForceChange; // forced to close
            } else {
                ClosingProbStatus = ProbabilityCheck::KeepStatus; // Keep previous status
            }
        } else {
            ClosingProbStatus = ProbabilityCheck::NoAction; // free operation
        }
    }

    bool OccupantVentilationControlProp::openingProbability(EnergyPlusData &state,
                                                            int const ZoneNum,
                                                            Real64 const TimeCloseDuration) // function to perform calculations of opening probability
    {
        using DataHVACGlobals::DualSetPointWithDeadBand;
        using DataHVACGlobals::SingleCoolingSetPoint;
        using DataHVACGlobals::SingleHeatCoolSetPoint;
        using DataHVACGlobals::SingleHeatingSetPoint;

        Real64 SchValue;
        Real64 RandomValue;

        if (TimeCloseDuration < MinClosingTime) {
            return false;
        }
        if (OccupancyCheck) {
            if (state.dataHeatBal->ZoneIntGain(ZoneNum).NOFOCC <= 0.0) {
                return false;
            }
        }

        {
            auto const SELECT_CASE_var(state.dataHeatBalFanSys->TempControlType(ZoneNum)); // Check zone setpoints
            if (SELECT_CASE_var == 0) {                                                    // Uncontrolled

            } else if (SELECT_CASE_var == SingleHeatingSetPoint) {
                if (state.dataHeatBalFanSys->MAT(ZoneNum) <= state.dataHeatBalFanSys->ZoneThermostatSetPointLo(ZoneNum)) {
                    return false;
                }
            } else if (SELECT_CASE_var == SingleCoolingSetPoint) {
                if (state.dataHeatBalFanSys->MAT(ZoneNum) >= state.dataHeatBalFanSys->ZoneThermostatSetPointHi(ZoneNum)) {
                    return false;
                }
            } else if (SELECT_CASE_var == SingleHeatCoolSetPoint) {
                return false;
            } else if (SELECT_CASE_var == DualSetPointWithDeadBand) {
                if (state.dataHeatBalFanSys->MAT(ZoneNum) < state.dataHeatBalFanSys->ZoneThermostatSetPointLo(ZoneNum) ||
                    state.dataHeatBalFanSys->MAT(ZoneNum) > state.dataHeatBalFanSys->ZoneThermostatSetPointHi(ZoneNum)) {
                    return false;
                }
            }
        }

        if (OpeningProbSchNum == 0) {
            return true;
        } else {
            SchValue = GetCurrentScheduleValue(state, OpeningProbSchNum);
            RandomValue = Real64(rand()) / RAND_MAX;
            if (SchValue > RandomValue) {
                return true;
            } else {
                return false;
            }
        }
    }

    bool OccupantVentilationControlProp::closingProbability(EnergyPlusData &state,
                                                            Real64 const TimeOpenDuration) // function to perform calculations of closing probability
    {
        Real64 SchValue;
        Real64 RandomValue;

        if (TimeOpenDuration < MinOpeningTime) {
            return false;
        }
        if (ClosingProbSchNum == 0) {
            return true;
        } else {
            SchValue = GetCurrentScheduleValue(state, ClosingProbSchNum);
            RandomValue = Real64(rand()) / RAND_MAX;
            if (SchValue > RandomValue) {
                return true;
            } else {
                return false;
            }
        }
    }

    void AirflowNetworkSolverData::allocate(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Aug. 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine allocates dynamic arrays for AirflowNetworkSolver.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        iComponentTypeNum j;
        int n;

        // Formats

        // Assume a network to simulate multizone airflow is a subset of the network to simulate air distribution system.
        // Network array size is allocated based on the network of air distribution system.
        // If multizone airflow is simulated only, the array size is allocated based on the multizone network.

        auto &NetworkNumOfLinks = ActualNumOfLinks;
        auto &NetworkNumOfNodes = ActualNumOfNodes;

        NetworkNumOfLinks = AirflowNetworkNumOfLinks;
        NetworkNumOfNodes = AirflowNetworkNumOfNodes;

        AFECTL.allocate(NetworkNumOfLinks);
        AFLOW2.allocate(NetworkNumOfLinks);
        AFLOW.allocate(NetworkNumOfLinks);
        PW.allocate(NetworkNumOfLinks);
        PS.allocate(NetworkNumOfLinks);

        // TZ.allocate(NetworkNumOfNodes);
        // WZ.allocate(NetworkNumOfNodes);
        PZ.allocate(NetworkNumOfNodes);
        // RHOZ.allocate(NetworkNumOfNodes);
        // SQRTDZ.allocate(NetworkNumOfNodes);
        // VISCZ.allocate(NetworkNumOfNodes);
        SUMAF.allocate(NetworkNumOfNodes);

        for (int it = 0; it <= NetworkNumOfNodes + 1; ++it)
            properties.emplace_back(AIRDENSITY(state, 20.0, 101325.0, 0.0));

        ID.allocate(NetworkNumOfNodes);
        IK.allocate(NetworkNumOfNodes + 1);
#ifdef SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS
        newIK.allocate(NetworkNumOfNodes + 1);
#endif
        AD.allocate(NetworkNumOfNodes);
        SUMF.allocate(NetworkNumOfNodes);

        n = 0;
        for (i = 1; i <= AirflowNetworkNumOfLinks; ++i) {
            j = AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum;
            if (j == iComponentTypeNum::DOP) {
                ++n;
            }
        }

        dos.allocate(AirflowNetworkNumOfLinks, n);

        PB = 101325.0;
        //   LIST = 5
        // LIST = 0;

        for (n = 1; n <= NetworkNumOfNodes; ++n) {
            ID(n) = n;
        }
        for (i = 1; i <= NetworkNumOfLinks; ++i) {
            AFECTL(i) = 1.0;
            AFLOW(i) = 0.0;
            AFLOW2(i) = 0.0;
        }

        for (i = 1; i <= NetworkNumOfNodes; ++i) {
            // TZ(i) = AirflowNetworkNodeSimu(i).TZ;
            // WZ(i) = AirflowNetworkNodeSimu(i).WZ;
            PZ(i) = AirflowNetworkNodeSimu(i).PZ;
            properties[i].temperature = AirflowNetworkNodeSimu(i).TZ;
            properties[i].humidity_ratio = AirflowNetworkNodeSimu(i).WZ;
            // properties[i].pressure = AirflowNetworkNodeSimu(i).PZ;
        }

        // Assign linkage values
        for (i = 1; i <= NetworkNumOfLinks; ++i) {
            PW(i) = 0.0;
        }
        // Write an ouput file used for AIRNET input
        /*
        if (LIST >= 5) {
            Unit11 = GetNewUnitNumber();
            ObjexxFCL::gio::open(Unit11, DataStringGlobals::eplusADSFileName);
            for (i = 1; i <= NetworkNumOfNodes; ++i) {
                ObjexxFCL::gio::write(Unit11, Format_901) << i << state.afn->AirflowNetworkNodeData(i).NodeTypeNum <<
        state.afn->AirflowNetworkNodeData(i).NodeHeight << TZ(i)
                                               << PZ(i);
            }
            ObjexxFCL::gio::write(Unit11, Format_900) << 0;
            for (i = 1; i <= AirflowNetworkNumOfComps; ++i) {
                j = state.afn->AirflowNetworkCompData(i).TypeNum;
                {
                    auto const SELECT_CASE_var(state.afn->AirflowNetworkCompData(i).CompTypeNum);
                    if (SELECT_CASE_var == CompTypeNum_PLR) { //'PLR'  Power law component
                        //              WRITE(Unit11,902)
        state.afn->AirflowNetworkCompData(i)%CompNum,1,DisSysCompLeakData(j)%FlowCoef, &
                        //                  DisSysCompLeakData(j)%FlowCoef,DisSysCompLeakData(j)%FlowCoef,DisSysCompLeakData(j)%FlowExpo
                    } else if (SELECT_CASE_var == CompTypeNum_SCR) { //'SCR'  Surface crack component
                        ObjexxFCL::gio::write(Unit11, Format_902) << state.afn->AirflowNetworkCompData(i).CompNum << 1 <<
        MultizoneSurfaceCrackData(j).FlowCoef
                                                       << MultizoneSurfaceCrackData(j).FlowCoef << MultizoneSurfaceCrackData(j).FlowCoef
                                                       << MultizoneSurfaceCrackData(j).FlowExpo;
                    } else if (SELECT_CASE_var == CompTypeNum_DWC) { //'DWC' Duct component
                        //              WRITE(Unit11,902)
        state.afn->AirflowNetworkCompData(i)%CompNum,2,DisSysCompDuctData(j)%L,DisSysCompDuctData(j)%D, &
                        //                               DisSysCompDuctData(j)%A,DisSysCompDuctData(j)%Rough
                        //              WRITE(Unit11,903) DisSysCompDuctData(i)%TurDynCoef,DisSysCompDuctData(j)%LamFriCoef, &
                        //                               DisSysCompDuctData(j)%LamFriCoef,DisSysCompDuctData(j)%InitLamCoef
                        //           CASE (CompTypeNum_CVF) ! 'CVF' Constant volume fan component
                        //              WRITE(Unit11,904) state.afn->AirflowNetworkCompData(i)%CompNum,4,DisSysCompCVFData(j)%FlowRate
                    } else if (SELECT_CASE_var == CompTypeNum_EXF) { // 'EXF' Zone exhaust fan
                        ObjexxFCL::gio::write(Unit11, Format_904) << state.afn->AirflowNetworkCompData(i).CompNum << 4 <<
        MultizoneCompExhaustFanData(j).FlowRate; } else {
                    }
                }
            }
            ObjexxFCL::gio::write(Unit11, Format_900) << 0;
            for (i = 1; i <= NetworkNumOfLinks; ++i) {
                gio::write(Unit11, Format_910) << i << state.afn->AirflowNetworkLinkageData(i).NodeNums[0] <<
        state.afn->AirflowNetworkLinkageData(i).NodeHeights[0]
                                               << state.afn->AirflowNetworkLinkageData(i).NodeNums[1] <<
        state.afn->AirflowNetworkLinkageData(i).NodeHeights[1]
                                               << state.afn->AirflowNetworkLinkageData(i).CompNum << 0 << 0;
            }
            ObjexxFCL::gio::write(Unit11, Format_900) << 0;
        }
        */
        setsky();

        // SETSKY figures out the IK stuff -- which is why E+ doesn't allocate AU until here
#ifdef SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS
        //   ! only printing to screen, can be commented
        //   print*, "SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS is defined"
        //   write(*,'(2(a,i8))') "AllocateAirflowNetworkData: after SETSKY, allocating AU.  NetworkNumOfNodes=", &
        //        NetworkNumOfNodes, " IK(NetworkNumOfNodes+1)= NNZE=", IK(NetworkNumOfNodes+1)
        //   print*, " NetworkNumOfLinks=", NetworkNumOfLinks
        // allocate same size as others -- this will be maximum  !noel
        newAU.allocate(IK(NetworkNumOfNodes + 1));
#endif

        // noel, GNU says the AU is indexed above its upper bound
        // ALLOCATE(AU(IK(NetworkNumOfNodes+1)-1))
        AU.allocate(IK(NetworkNumOfNodes + 1));
    }

    void AirflowNetworkSolverData::initialize_calculation()
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   Aug. 2003
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes variables for AirflowNetworkSolver.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        for (int i = 1; i <= ActualNumOfNodes; ++i) {
            ID(i) = i;
        }
        for (int i = 1; i <= ActualNumOfLinks; ++i) {
            AFECTL(i) = 1.0;
            AFLOW(i) = 0.0;
            AFLOW2(i) = 0.0;
        }

        for (int i = 1; i <= ActualNumOfNodes; ++i) {
            // TZ(i) = AirflowNetworkNodeSimu(i).TZ;
            // WZ(i) = AirflowNetworkNodeSimu(i).WZ;
            PZ(i) = AirflowNetworkNodeSimu(i).PZ;
            properties[i].temperature = AirflowNetworkNodeSimu(i).TZ;
            properties[i].humidity_ratio = AirflowNetworkNodeSimu(i).WZ;
            // properties[i].pressure = AirflowNetworkNodeSimu(i).PZ;
        }
    }

    void AirflowNetworkSolverData::setsky()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   1998
        //       MODIFIED       Feb. 2006 (L. Gu) to meet requirements of AirflowNetwork
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine sets up the "IK" array describing the sparse matrix [A] in skyline
        //     form by using the location matrix.

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // AIRNET

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // IK(K) - pointer to the top of column/row "K".
        int i;
        int j;
        int k;
        int L;
        int M;
        int N1;
        int N2;

        // Initialize "IK".
        for (i = 1; i <= ActualNumOfNodes + 1; ++i) {
            IK(i) = 0;
        }
        // Determine column heights.
        for (M = 1; M <= ActualNumOfLinks; ++M) {
            j = AirflowNetworkLinkageData(M).NodeNums[1];
            if (j == 0) continue;
            L = ID(j);
            i = AirflowNetworkLinkageData(M).NodeNums[0];
            k = ID(i);
            N1 = std::abs(L - k);
            N2 = max(k, L);
            IK(N2) = max(IK(N2), N1);
        }
        // Convert heights to column addresses.
        j = IK(1);
        IK(1) = 1;
        for (k = 1; k <= ActualNumOfNodes; ++k) {
            i = IK(k + 1);
            IK(k + 1) = IK(k) + j;
            j = i;
        }
    }

    void AirflowNetworkSolverData::airmov(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNETf
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is a driver for AIRNET to calculate nodal pressures and linkage airflows

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // na

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i;
        int m;
        int n;
        int ITER;

        // Formats

        // static ObjexxFCL::gio::Fmt Format_900("(,/,11X,'i    n    m       DP',12x,'F1',12X,'F2')");
        // static ObjexxFCL::gio::Fmt Format_901("(1X,A6,3I5,3F14.6)");
        // static ObjexxFCL::gio::Fmt Format_902("(,/,11X,'n       P',12x,'sumF')");
        // static ObjexxFCL::gio::Fmt Format_903("(1X,A6,I5,3F14.6)");
        // static ObjexxFCL::gio::Fmt Format_907("(,/,' CPU seconds for ',A,F12.3)");

        auto &NetworkNumOfLinks = ActualNumOfLinks;
        auto &NetworkNumOfNodes = ActualNumOfNodes;

        // Initialize pressure for pressure control and for Initialization Type = LinearInitializationMethod
        if ((AirflowNetworkSimu.InitFlag == 0) ||
            (PressureSetFlag > 0 && AirflowNetworkFanActivated)) {
            for (n = 1; n <= NetworkNumOfNodes; ++n) {
                if (AirflowNetworkNodeData(n).NodeTypeNum == 0) PZ(n) = 0.0;
            }
        }
        // Compute zone air properties.
        for (n = 1; n <= NetworkNumOfNodes; ++n) {
            properties[n].density = AIRDENSITY(state, state.dataEnvrn->StdBaroPress + PZ(n), properties[n].temperature, properties[n].humidity_ratio);
            // RHOZ(n) = PsyRhoAirFnPbTdbW(StdBaroPress + PZ(n), TZ(n), WZ(n));
            if (AirflowNetworkNodeData(n).ExtNodeNum > 0) {
                properties[n].density =
                    AIRDENSITY(state, state.dataEnvrn->StdBaroPress + PZ(n), state.dataEnvrn->OutDryBulbTemp, state.dataEnvrn->OutHumRat);
                properties[n].temperature = state.dataEnvrn->OutDryBulbTemp;
                properties[n].humidity_ratio = state.dataEnvrn->OutHumRat;
            }
            properties[n].sqrt_density = std::sqrt(properties[n].density);
            properties[n].viscosity = 1.71432e-5 + 4.828e-8 * properties[n].temperature;
            // if (LIST >= 2) ObjexxFCL::gio::write(outputFile, Format_903) << "D,V:" << n << properties[n].density << properties[n].viscosity;
        }
        // Compute stack pressures.
        for (i = 1; i <= NetworkNumOfLinks; ++i) {
            n = AirflowNetworkLinkageData(i).NodeNums[0];
            m = AirflowNetworkLinkageData(i).NodeNums[1];
            if (AFLOW(i) > 0.0) {
                PS(i) =
                    9.80 * (properties[n].density * (AirflowNetworkNodeData(n).NodeHeight -
                                                     AirflowNetworkNodeData(m).NodeHeight) +
                            state.afn->AirflowNetworkLinkageData(i).NodeHeights[1] * (properties[m].density - properties[n].density));
            } else if (AFLOW(i) < 0.0) {
                PS(i) =
                    9.80 * (properties[m].density * (AirflowNetworkNodeData(n).NodeHeight -
                                                     AirflowNetworkNodeData(m).NodeHeight) +
                            AirflowNetworkLinkageData(i).NodeHeights[0] * (properties[m].density - properties[n].density));
            } else {
                PS(i) = 4.90 * ((properties[n].density + properties[m].density) * (AirflowNetworkNodeData(n).NodeHeight -
                                                                                   AirflowNetworkNodeData(m).NodeHeight) +
                                (AirflowNetworkLinkageData(i).NodeHeights[0] +
                                 AirflowNetworkLinkageData(i).NodeHeights[1]) *
                                    (properties[m].density - properties[n].density));
            }
        }

        // Calculate pressure field in a large opening
        dos.pstack(state, properties, PZ);
        solvzp(state, ITER);

        // Report element flows and zone pressures.
        for (n = 1; n <= NetworkNumOfNodes; ++n) {
            SUMAF(n) = 0.0;
        }
        // if (LIST >= 1) ObjexxFCL::gio::write(outputFile, Format_900);
        for (i = 1; i <= NetworkNumOfLinks; ++i) {
            n = AirflowNetworkLinkageData(i).NodeNums[0];
            m = AirflowNetworkLinkageData(i).NodeNums[1];
            // if (LIST >= 1) {
            //    gio::write(outputFile, Format_901) << "Flow: " << i << n << m << AirflowNetworkLinkSimu(i).DP << AFLOW(i) << AFLOW2(i);
            //}
            if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum ==
                iComponentTypeNum::HOP) {
                SUMAF(n) = SUMAF(n) - AFLOW(i);
                SUMAF(m) += AFLOW(i);
            } else {
                SUMAF(n) = SUMAF(n) - AFLOW(i) - AFLOW2(i);
                SUMAF(m) += AFLOW(i) + AFLOW2(i);
            }
        }
        // for (n = 1; n <= NetworkNumOfNodes; ++n) {
        //    if (LIST >= 1) gio::write(outputFile, Format_903) << "Room: " << n << PZ(n) << SUMAF(n) << properties[n].temperature;
        //}

        for (i = 1; i <= NetworkNumOfLinks; ++i) {
            if (AFLOW2(i) != 0.0) {
            }
            if (AFLOW(i) > 0.0) {
                AirflowNetworkLinkSimu(i).FLOW = AFLOW(i);
                AirflowNetworkLinkSimu(i).FLOW2 = 0.0;
            } else {
                AirflowNetworkLinkSimu(i).FLOW = 0.0;
                AirflowNetworkLinkSimu(i).FLOW2 = -AFLOW(i);
            }
            if (AirflowNetworkCompData(AirflowNetworkLinkageData(i).CompNum).CompTypeNum ==
                iComponentTypeNum::HOP) {
                if (AFLOW(i) > 0.0) {
                    AirflowNetworkLinkSimu(i).FLOW = AFLOW(i) + AFLOW2(i);
                    AirflowNetworkLinkSimu(i).FLOW2 = AFLOW2(i);
                } else {
                    AirflowNetworkLinkSimu(i).FLOW = AFLOW2(i);
                    AirflowNetworkLinkSimu(i).FLOW2 = -AFLOW(i) + AFLOW2(i);
                }
            }
            if (AirflowNetworkLinkageData(i).DetOpenNum > 0) {
                if (AFLOW2(i) != 0.0) {
                    AirflowNetworkLinkSimu(i).FLOW = AFLOW(i) + AFLOW2(i);
                    AirflowNetworkLinkSimu(i).FLOW2 = AFLOW2(i);
                }
            }
            if (AirflowNetworkCompData(state.afn->AirflowNetworkLinkageData(i).CompNum).CompTypeNum ==
                    iComponentTypeNum::SOP &&
                AFLOW2(i) != 0.0) {
                if (AFLOW(i) >= 0.0) {
                    AirflowNetworkLinkSimu(i).FLOW = AFLOW(i);
                    AirflowNetworkLinkSimu(i).FLOW2 = std::abs(AFLOW2(i));
                } else {
                    AirflowNetworkLinkSimu(i).FLOW = std::abs(AFLOW2(i));
                    AirflowNetworkLinkSimu(i).FLOW2 = -AFLOW(i);
                }
            }
        }

        for (i = 1; i <= NetworkNumOfNodes; ++i) {
            AirflowNetworkNodeSimu(i).PZ = PZ(i);
        }
    }

    void AirflowNetworkSolverData::solvzp(EnergyPlusData &state, int &ITER) // number of iterations
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves zone pressures by modified Newton-Raphson iteration

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        auto &NetworkNumOfLinks = ActualNumOfLinks;
        auto &NetworkNumOfNodes = ActualNumOfNodes;

        // Argument array dimensioning (these used to be arguments, need to also test newAU and newIK)
        EP_SIZE_CHECK(IK, NetworkNumOfNodes + 1);
        EP_SIZE_CHECK(AD, NetworkNumOfNodes);
        EP_SIZE_CHECK(AU, IK(NetworkNumOfNodes + 1));

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // noel GNU says AU is being indexed beyound bounds
        // REAL(r64), INTENT(INOUT) :: AU(IK(NetworkNumOfNodes+1)-1) ! the upper triangle of [A] before and after factoring

        // SUBROUTINE PARAMETER DEFINITIONS:

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        //     NNZE   - number of nonzero entries in the "AU" array.
        //     LFLAG   - if = 1, use laminar relationship (initialization).
        //     I       - element number.
        //     N       - number of node/zone 1.
        //     M       - number of node/zone 2.
        //     F       - flows through the element (kg/s).
        //     DF      - partial derivatives:  DF/DP.
        //     NF      - number of flows, 1 or 2.
        //     SUMF    - sum of flows into node/zone.
        //     CCF     - current pressure correction (Pa).
        //     PCF     - previous pressure correction (Pa).
        //     CEF     - convergence enhancement factor.
        int n;
        int NNZE;
        int NSYM;
        bool LFLAG;
        int CONVG;
        int ACCEL;
        Array1D<Real64> PCF(NetworkNumOfNodes);
        Array1D<Real64> CEF(NetworkNumOfNodes);
        Real64 C;
        Real64 SSUMF;
        Real64 SSUMAF;
        Real64 ACC0;
        Real64 ACC1;
        Array1D<Real64> CCF(NetworkNumOfNodes);

        // auto &outputFile = std::cout;

        ACC1 = 0.0;
        ACCEL = 0;
        NSYM = 0;
        NNZE = IK(NetworkNumOfNodes + 1) - 1;
        // if (LIST >= 2) print(outputFile, "Initialization{:16}{:16}{:16}\n", NetworkNumOfNodes, NetworkNumOfLinks, NNZE);
        ITER = 0;

        for (n = 1; n <= NetworkNumOfNodes; ++n) {
            PCF(n) = 0.0;
            CEF(n) = 0.0;
        }

        if (AirflowNetworkSimu.InitFlag != 1) {
            // Initialize node/zone pressure values by assuming only linear relationship between
            // airflows and pressure drops.
            LFLAG = true;
            filjac(state, NNZE, LFLAG);
            for (n = 1; n <= NetworkNumOfNodes; ++n) {
                if (AirflowNetworkNodeData(n).NodeTypeNum == 0) PZ(n) = SUMF(n);
            }
            // Data dump.
//            if (LIST >= 3) {
//                DUMPVD("AD:", AD, NetworkNumOfNodes, outputFile);
//                DUMPVD("AU:", AU, NNZE, outputFile);
//                DUMPVR("AF:", SUMF, NetworkNumOfNodes, outputFile);
//            }
// Solve linear system for approximate PZ.
#ifdef SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS
            facsky(state, newAU, AD, newAU, newIK, NetworkNumOfNodes, NSYM);     // noel
            slvsky(newAU, AD, newAU, PZ, newIK, NetworkNumOfNodes, NSYM); // noel
#else
            facsky(AU, AD, AU, IK, NetworkNumOfNodes, NSYM);
            slvsky(AU, AD, AU, PZ, IK, NetworkNumOfNodes, NSYM);
#endif
            // if (LIST >= 2) DUMPVD("PZ:", PZ, NetworkNumOfNodes, outputFile);
        }
        // Solve nonlinear airflow network equations by modified Newton's method.
        while (ITER < AirflowNetworkSimu.MaxIteration) {
            LFLAG = false;
            ++ITER;
            //            if (LIST >= 2) {
            //                print(outputFile, "Begin iteration {}\n", ITER);
            //            }
            // Set up the Jacobian matrix.
            filjac(state, NNZE, LFLAG);
            // Data dump.
            //            if (LIST >= 3) {
            //                DUMPVR("SUMF:", SUMF, NetworkNumOfNodes, outputFile);
            //                DUMPVR("SUMAF:", SUMAF, NetworkNumOfNodes, outputFile);
            //            }
            // Check convergence.
            CONVG = 1;
            SSUMF = 0.0;
            SSUMAF = 0.0;
            for (n = 1; n <= NetworkNumOfNodes; ++n) {
                SSUMF += std::abs(SUMF(n));
                SSUMAF += SUMAF(n);
                if (CONVG == 1) {
                    if (std::abs(SUMF(n)) <= AirflowNetworkSimu.AbsTol) continue;
                    if (std::abs(SUMF(n) / SUMAF(n)) > AirflowNetworkSimu.RelTol) CONVG = 0;
                }
            }
            ACC0 = ACC1;
            if (SSUMAF > 0.0) ACC1 = SSUMF / SSUMAF;
            if (CONVG == 1 && ITER > 1) return;
            if (ITER >= state.afn->AirflowNetworkSimu.MaxIteration) break;
            // Data dump.
            //            if (LIST >= 3) {
            //                DUMPVD("AD:", AD, NetworkNumOfNodes, outputFile);
            //                DUMPVD("AU:", AU, NNZE, outputFile);
            //            }
            // Solve AA * CCF = SUMF.
            for (n = 1; n <= NetworkNumOfNodes; ++n) {
                CCF(n) = SUMF(n);
            }
#ifdef SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS
            facsky(state, newAU, AD, newAU, newIK, NetworkNumOfNodes, NSYM);      // noel
            slvsky(newAU, AD, newAU, CCF, newIK, NetworkNumOfNodes, NSYM); // noel
#else
            facsky(AU, AD, AU, IK, NetworkNumOfNodes, NSYM);
            slvsky(AU, AD, AU, CCF, IK, NetworkNumOfNodes, NSYM);
#endif
            // Revise PZ (Steffensen iteration on the N-R correction factors to handle oscillating corrections).
            if (ACCEL == 1) {
                ACCEL = 0;
            } else {
                if (ITER > 2 && ACC1 > 0.5 * ACC0) ACCEL = 1;
            }
            for (n = 1; n <= NetworkNumOfNodes; ++n) {
                if (AirflowNetworkNodeData(n).NodeTypeNum == 1) continue;
                CEF(n) = 1.0;
                if (ACCEL == 1) {
                    C = CCF(n) / PCF(n);
                    if (C < AirflowNetworkSimu.ConvLimit) CEF(n) = 1.0 / (1.0 - C);
                    C = CCF(n) * CEF(n);
                } else {
                    //            IF (CCF(N) .EQ. 0.0d0) CCF(N)=TINY(CCF(N))  ! 1.0E-40
                    if (CCF(n) == 0.0) CCF(n) = DataGlobalConstants::rTinyValue; // 1.0E-40 (Epsilon)
                    PCF(n) = CCF(n);
                    C = CCF(n);
                }
                if (std::abs(C) > AirflowNetworkSimu.MaxPressure) {
                    CEF(n) *= AirflowNetworkSimu.MaxPressure / std::abs(C);
                    PZ(n) -= CCF(n) * CEF(n);
                } else {
                    PZ(n) -= C;
                }
            }
            // Data revision dump.
            //            if (LIST >= 2) {
            //                for (n = 1; n <= NetworkNumOfNodes; ++n) {
            //                    if (state.afn->AirflowNetworkNodeData(n).NodeTypeNum == 0) {
            //                        print(outputFile, "Rev: {:5}{:3}{:14.6E} {:8.4F}{:24.14F}", n, SUMF(n), CCF(n), CEF(n), PZ(n));
            //                    }
            //                }
            //            }
        }

        // Error termination.
        ShowSevereError(state, "Too many iterations (SOLVZP) in Airflow Network simulation");
        ++AirflowNetworkSimu.ExtLargeOpeningErrCount;
        if (AirflowNetworkSimu.ExtLargeOpeningErrCount < 2) {
            ShowWarningError(state,
                             "AirflowNetwork: SOLVER, Changing values for initialization flag, Relative airflow convergence, Absolute airflow "
                             "convergence, Convergence acceleration limit or Maximum Iteration Number may solve the problem.");
            ShowContinueErrorTimeStamp(state, "");
            ShowContinueError(state,
                              "..Iterations=" + std::to_string(ITER) +
                                  ", Max allowed=" + std::to_string(AirflowNetworkSimu.MaxIteration));
            ShowFatalError(state, "AirflowNetwork: SOLVER, The previous error causes termination.");
        } else {
            ShowRecurringWarningErrorAtEnd(state,
                                           "AirFlowNetwork: Too many iterations (SOLVZP) in AirflowNetwork simulation continues.",
                                           state.afn->AirflowNetworkSimu.ExtLargeOpeningErrIndex);
        }
    }

    void AirflowNetworkSolverData::filjac(EnergyPlusData &state,
                        int const NNZE,  // number of nonzero entries in the "AU" array.
                        bool const LFLAG // if = 1, use laminar relationship (initialization).
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine creates matrices for solution of flows

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        // I       - component number.
        // N       - number of node/zone 1.
        // M       - number of node/zone 2.
        // F       - flows through the element (kg/s).
        // DF      - partial derivatives:  DF/DP.
        // NF      - number of flows, 1 or 2.
        int i;
        int j;
        int n;
        int FLAG;
        int NF;
#ifdef SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS
        int LHK; // noel
        int JHK;
        int JHK1;
        int newsum;
        int newh;
        int ispan;
        int thisIK;
        bool allZero; // noel
#endif
        Array1D<Real64> X(4);
        Real64 DP;
        std::array<Real64, 2> F{{0.0, 0.0}};
        std::array<Real64, 2> DF{{0.0, 0.0}};

        auto &NetworkNumOfLinks = ActualNumOfLinks;
        auto &NetworkNumOfNodes = ActualNumOfNodes;

        for (n = 1; n <= NetworkNumOfNodes; ++n) {
            SUMF(n) = 0.0;
            SUMAF(n) = 0.0;
            if (AirflowNetworkNodeData(n).NodeTypeNum == 1) {
                AD(n) = 1.0;
            } else {
                AD(n) = 0.0;
            }
        }
        for (n = 1; n <= NNZE; ++n) {
            AU(n) = 0.0;
        }
        // Loop(s) to calculate control, etc.
        // Set up the Jacobian matrix.
        for (i = 1; i <= NetworkNumOfLinks; ++i) {
            if (AirflowNetworkLinkageData(i).element == nullptr) {
                continue;
            }
            n = AirflowNetworkLinkageData(i).NodeNums[0];
            int m = AirflowNetworkLinkageData(i).NodeNums[1];
            //!!! Check array of DP. DpL is used for multizone air flow calculation only
            //!!! and is not for forced air calculation
            if (i > NumOfLinksMultiZone) {
                DP = PZ(n) - PZ(m) + PS(i) + PW(i);
            } else {
                DP = PZ(n) - PZ(m) + dos.DpL(i, 1) + PW(i);
            }
            Real64 multiplier = 1.0;
            Real64 control = AirflowNetworkLinkageData(i).control;
            // if (LIST >= 4) ObjexxFCL::gio::write(outputFile, Format_901) << "PS:" << i << n << M << PS(i) << PW(i) << AirflowNetworkLinkSimu(i).DP;
            j = AirflowNetworkLinkageData(i).CompNum;

            NF = AirflowNetworkLinkageData(i).element->calculate(
                state, LFLAG, DP, i, multiplier, control, properties[n], properties[m], F, DF);
            if (AirflowNetworkLinkageData(i).element->type() == ComponentType::CPD && DP != 0.0) {
                DP = DisSysCompCPDData(state.afn->AirflowNetworkCompData(j).TypeNum).DP;
            }

            AirflowNetworkLinkSimu(i).DP = DP;
            AFLOW(i) = F[0];
            AFLOW2(i) = 0.0;
            if (state.afn->AirflowNetworkCompData(j).CompTypeNum == iComponentTypeNum::DOP) {
                AFLOW2(i) = F[1];
            }
            // if (LIST >= 3) ObjexxFCL::gio::write(outputFile, Format_901) << " NRi:" << i << n << M << AirflowNetworkLinkSimu(i).DP << F[0] <<
            // DF[0];
            FLAG = 1;
            if (AirflowNetworkNodeData(n).NodeTypeNum == 0) {
                ++FLAG;
                X(1) = DF[0];
                X(2) = -DF[0];
                SUMF(n) += F[0];
                SUMAF(n) += std::abs(F[0]);
            }
            if (AirflowNetworkNodeData(m).NodeTypeNum == 0) {
                FLAG += 2;
                X(4) = DF[0];
                X(3) = -DF[0];
                SUMF(m) -= F[0];
                SUMAF(m) += std::abs(F[0]);
            }
            if (FLAG != 1) filsky(X, state.afn->AirflowNetworkLinkageData(i).NodeNums, IK, AU, AD, FLAG);
            if (NF == 1) continue;
            AFLOW2(i) = F[1];
            // if (LIST >= 3) ObjexxFCL::gio::write(outputFile, Format_901) << " NRj:" << i << n << m << AirflowNetworkLinkSimu(i).DP << F[1] <<
            // DF[1];
            FLAG = 1;
            if (state.afn->AirflowNetworkNodeData(n).NodeTypeNum == 0) {
                ++FLAG;
                X(1) = DF[1];
                X(2) = -DF[1];
                SUMF(n) += F[1];
                SUMAF(n) += std::abs(F[1]);
            }
            if (state.afn->AirflowNetworkNodeData(m).NodeTypeNum == 0) {
                FLAG += 2;
                X(4) = DF[1];
                X(3) = -DF[1];
                SUMF(m) -= F[1];
                SUMAF(m) += std::abs(F[1]);
            }
            if (FLAG != 1) filsky(X, state.afn->AirflowNetworkLinkageData(i).NodeNums, IK, AU, AD, FLAG);
        }

#ifdef SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS

        // After the matrix values have been set, we can look at them and see if any columns are filled with zeros.
        // If they are, let's remove them from the matrix -- but only for the purposes of doing the solve.
        // They way I do this is building a separate IK array (newIK) that simply changes the column heights.
        // So the affected SOLVEs would use this newIK and nothing else changes.
        for (n = 1; n <= NetworkNumOfNodes + 1; ++n) {
            newIK(n) = IK(n);
            // print*, " NetworkNumOfNodes  n=", n, " IK(n)=", IK(n)
        }

        newsum = IK(2) - IK(1); // always 0?

        JHK = 1;
        for (n = 2; n <= NetworkNumOfNodes; ++n) {
            JHK1 = IK(n + 1); // starts at IK(3)-IK(2)
            LHK = JHK1 - JHK;
            if (LHK <= 0) {
                newIK(n + 1) = newIK(n);
                continue;
            }
            // write(*,'(4(a,i8))') "n=", n, " ik=", ik(n), " JHK=", JHK, " LHK=", LHK

            // is the entire column zero?  noel
            allZero = true;
            for (i = 0; i <= LHK - 1; ++i) {
                if (AU(JHK + i) != 0.0) {
                    allZero = false;
                    break;
                }
            }

            newh = LHK;
            if (allZero) {
                // print*, "allzero n=", n
                newh = 0;
            } else {
                // DO i=0,LHK-1
                //   write(*, '(2(a,i8),a, f15.3)') "  n=", n, " i=", i, " AU(JHK+i)=", AU(JHK+i)
                // enddo
            }
            newIK(n + 1) = newIK(n) + newh;
            newsum += newh;

            // do i = LHK-1,0, -1
            //   write(*, '(2(a,i8),a, f15.3)') "  n=", n, " i=", i, " AU(JHK+i)=", AU(JHK+i)
            // enddo
            JHK = JHK1;
        }

        // this is just a print to screen, is not necessary
        //     if (firstTime) then
        //        write(*, '(2(a,i8))') " After SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS: newsum=", newsum, " oldsum=", IK(NetworkNumOfNodes+1)
        //        firstTime=.FALSE.
        //     endif

        // Now fill newAU from AU, using newIK
        thisIK = 1;
        for (n = 2; n <= NetworkNumOfNodes; ++n) {
            thisIK = newIK(n);
            ispan = newIK(n + 1) - thisIK;

            if (ispan <= 0) continue;
            for (i = 0; i <= ispan - 1; ++i) {
                newAU(thisIK + i) = AU(IK(n) + i);
            }
        }
#endif
    }

    void AirflowNetworkSolverData::facsky(EnergyPlusData &state,
                Array1D<Real64> &AU, // the upper triangle of [A] before and after factoring
                Array1D<Real64> &AD,   // the main diagonal of [A] before and after factoring
                Array1D<Real64> &AL,   // the lower triangle of [A] before and after factoring
                const Array1D_int &IK, // pointer to the top of column/row "K"
                int const NEQ,         // number of equations
                int const NSYM         // symmetry:  0 = symmetric matrix, 1 = non-symmetric
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  This subroutine is revised from FACSKY developed by George Walton, NIST

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine performs L-U factorization of a skyline ordered matrix, [A]
        // The algorithm has been restructured for clarity.
        // Note dependence on compiler for optimizing the inner do loops.

        // METHODOLOGY EMPLOYED:
        //     L-U factorization of a skyline ordered matrix, [A], used for
        //     solution of simultaneous linear algebraic equations [A] * X = B.
        //     No pivoting!  No scaling!  No warnings!!!
        //     Related routines:  SLVSKY, SETSKY, FILSKY.

        // REFERENCES:
        //     Algorithm is described in "The Finite Element Method Displayed",
        //     by G. Dhatt and G. Touzot, John Wiley & Sons, New York, 1984.

        // USE STATEMENTS:
        // na

        // Argument array dimensioning
        EP_SIZE_CHECK(IK, ActualNumOfNodes + 1);
        EP_SIZE_CHECK(AU, IK(ActualNumOfNodes + 1));
        EP_SIZE_CHECK(AD, ActualNumOfNodes);
        EP_SIZE_CHECK(AL, IK(ActualNumOfNodes + 1) - 1);

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // noel, GNU says the AU is indexed above its upper bound
        // REAL(r64), INTENT(INOUT) :: AU(IK(NetworkNumOfNodes+1)-1) ! the upper triangle of [A] before and after factoring

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int JHK;
        int JHK1;
        int LHK;
        int LHK1;
        int IMIN;
        int IMIN1;
        int JHJ;
        int JHJ1;
        int IC;
        int i;
        int j;
        int k;
        Real64 T1;
        Real64 T2;
        Real64 SDOT;
        Real64 SUMD;

        AD(1) = 1.0 / AD(1);
        JHK = 1;
        for (k = 2; k <= NEQ; ++k) {
            SUMD = 0.0;
            JHK1 = IK(k + 1);
            LHK = JHK1 - JHK;
            if (LHK > 0) {
                LHK1 = LHK - 1;
                IMIN = k - LHK1;
                IMIN1 = IMIN - 1;
                if (NSYM == 1) AL(JHK) *= AD(IMIN1);
                if (LHK1 != 0) {
                    JHJ = IK(IMIN);
                    if (NSYM == 0) {
                        for (j = 1; j <= LHK1; ++j) {
                            JHJ1 = IK(IMIN + j);
                            IC = min(j, JHJ1 - JHJ);
                            if (IC > 0) {
                                SDOT = 0.0;
                                for (i = 0; i <= IC - 1; ++i) {
                                    SDOT += AU(JHJ1 - IC + i) * AU(JHK + j - IC + i);
                                }
                                AU(JHK + j) -= SDOT;
                            }
                            JHJ = JHJ1;
                        }
                    } else {
                        for (j = 1; j <= LHK1; ++j) {
                            JHJ1 = IK(IMIN + j);
                            IC = min(j, JHJ1 - JHJ);
                            SDOT = 0.0;
                            if (IC > 0) {
                                for (i = 0; i <= IC - 1; ++i) {
                                    SDOT += AL(JHJ1 - IC + i) * AU(JHK + j - IC + i);
                                }
                                AU(JHK + j) -= SDOT;
                                SDOT = 0.0;
                                for (i = 0; i <= IC - 1; ++i) {
                                    SDOT += AU(JHJ1 - IC + i) * AL(JHK + j - IC + i);
                                }
                            }
                            AL(JHK + j) = (AL(JHK + j) - SDOT) * AD(IMIN1 + j);
                            JHJ = JHJ1;
                        }
                    }
                }
                if (NSYM == 0) {
                    for (i = 0; i <= LHK1; ++i) {
                        T1 = AU(JHK + i);
                        T2 = T1 * AD(IMIN1 + i);
                        AU(JHK + i) = T2;
                        SUMD += T1 * T2;
                    }
                } else {
                    for (i = 0; i <= LHK1; ++i) {
                        SUMD += AU(JHK + i) * AL(JHK + i);
                    }
                }
            }
            if (AD(k) - SUMD == 0.0) {
                ShowSevereError(state, "AirflowNetworkSolver: L-U factorization in Subroutine FACSKY.");
                ShowContinueError(state,
                                  "The denominator used in L-U factorizationis equal to 0.0 at node = " +
                                      state.afn->AirflowNetworkNodeData(k).Name + '.');
                ShowContinueError(
                    state, "One possible cause is that this node may not be connected directly, or indirectly via airflow network connections ");
                ShowContinueError(
                    state, "(e.g., AirflowNetwork:Multizone:SurfaceCrack, AirflowNetwork:Multizone:Component:SimpleOpening, etc.), to an external");
                ShowContinueError(state, "node (AirflowNetwork:MultiZone:Surface).");
                ShowContinueError(state,
                                  "Please send your input file and weather file to EnergyPlus support/development team for further investigation.");
                ShowFatalError(state, "Preceding condition causes termination.");
            }
            AD(k) = 1.0 / (AD(k) - SUMD);
            JHK = JHK1;
        }
    }

    void AirflowNetworkSolverData::slvsky(const Array1D<Real64> &AU, // the upper triangle of [A] before and after factoring
                const Array1D<Real64> &AD, // the main diagonal of [A] before and after factoring
                const Array1D<Real64> &AL, // the lower triangle of [A] before and after factoring
                Array1D<Real64> &B,        // "B" vector (input); "X" vector (output).
                const Array1D_int &IK,     // pointer to the top of column/row "K"
                int const NEQ,             // number of equations
                int const NSYM             // symmetry:  0 = symmetric matrix, 1 = non-symmetric
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  This subroutine is revised from CLVSKY developed by George Walton, NIST

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine solves simultaneous linear algebraic equations [A] * X = B
        // using L-U factored skyline form of [A] from "FACSKY"

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Argument array dimensioning
        EP_SIZE_CHECK(IK, ActualNumOfNodes + 1);
        EP_SIZE_CHECK(AU, IK(ActualNumOfNodes + 1));
        EP_SIZE_CHECK(AD, ActualNumOfNodes);
        EP_SIZE_CHECK(AL, IK(ActualNumOfNodes + 1) - 1);
        EP_SIZE_CHECK(B, ActualNumOfNodes);

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // noel, GNU says the AU is indexed above its upper bound
        // REAL(r64), INTENT(INOUT) :: AU(IK(NetworkNumOfNodes+1)-1) ! the upper triangle of [A] before and after factoring

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        int i;
        int JHK;
        int JHK1;
        int k;
        int LHK;
        Real64 SDOT;
        Real64 T1;

        JHK = 1;
        for (k = 2; k <= NEQ; ++k) {
            JHK1 = IK(k + 1);
            LHK = JHK1 - JHK;
            if (LHK <= 0) continue;
            SDOT = 0.0;
            if (NSYM == 0) {
                for (i = 0; i <= LHK - 1; ++i) {
                    SDOT += AU(JHK + i) * B(k - LHK + i);
                }
            } else {
                for (i = 0; i <= LHK - 1; ++i) {
                    SDOT += AL(JHK + i) * B(k - LHK + i);
                }
            }
            B(k) -= SDOT;
            JHK = JHK1;
        }
        if (NSYM == 0) {
            for (k = 1; k <= NEQ; ++k) {
                B(k) *= AD(k);
            }
        }
        k = NEQ + 1;
        JHK1 = IK(k);
        while (k != 1) {
            --k;
            if (NSYM == 1) B(k) *= AD(k);
            if (k == 1) break;
            //        IF(K.EQ.1) RETURN
            JHK = IK(k);
            T1 = B(k);
            for (i = 0; i <= JHK1 - JHK - 1; ++i) {
                B(k - JHK1 + JHK + i) -= AU(JHK + i) * T1;
            }
            JHK1 = JHK;
        }
    }

    void AirflowNetworkSolverData::filsky(const Array1D<Real64> &X,    // element array (row-wise sequence)
                std::array<int, 2> const LM, // location matrix
                const Array1D_int &IK,       // pointer to the top of column/row "K"
                Array1D<Real64> &AU,         // the upper triangle of [A] before and after factoring
                Array1D<Real64> &AD,         // the main diagonal of [A] before and after factoring
                int const FLAG               // mode of operation
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         George Walton
        //       DATE WRITTEN   Extracted from AIRNET
        //       MODIFIED       Lixing Gu, 2/1/04
        //                      Revised the subroutine to meet E+ needs
        //       MODIFIED       Lixing Gu, 6/8/05
        //       RE-ENGINEERED  This subroutine is revised from FILSKY developed by George Walton, NIST

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine adds element array "X" to the sparse skyline matrix [A]

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // USE STATEMENTS:
        // na

        // Argument array dimensioning
        EP_SIZE_CHECK(X, 4);
        EP_SIZE_CHECK(IK, ActualNumOfNodes + 1);
        EP_SIZE_CHECK(AU, IK(ActualNumOfNodes + 1));
        EP_SIZE_CHECK(AD, ActualNumOfNodes);

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // noel, GNU says the AU is indexed above its upper bound
        // REAL(r64), INTENT(INOUT) :: AU(IK(NetworkNumOfNodes+1)-1) ! the upper triangle of [A] before and after factoring

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int j;
        int k;
        int L;

        // K = row number, L = column number.
        if (FLAG > 1) {
            k = LM[0];
            L = LM[1];
            if (FLAG == 4) {
                AD(k) += X(1);
                if (k < L) {
                    j = IK(L + 1) - L + k;
                    AU(j) += X(2);
                } else {
                    j = IK(k + 1) - k + L;
                    AU(j) += X(3);
                }
                AD(L) += X(4);
            } else if (FLAG == 3) {
                AD(L) += X(4);
            } else if (FLAG == 2) {
                AD(k) += X(1);
            }
        }
    }

} // namespace AirflowNetwork

} // namespace EnergyPlus
