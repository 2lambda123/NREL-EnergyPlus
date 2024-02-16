// EnergyPlus, Copyright (c) 1996-2024, The Board of Trustees of the University of Illinois,
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
#include <memory>
#include <string>

// EnergyPlus Headers
#include <EnergyPlus/AirLoopHVACDOAS.hh>
#include <EnergyPlus/BoilerSteam.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/HVACFan.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ReportCoilSelection.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WeatherManager.hh>

namespace EnergyPlus {

void createCoilSelectionReportObj(EnergyPlusData &state)
{
    state.dataRptCoilSelection->coilSelectionReportObj = std::make_unique<ReportCoilSelection>();
}

CoilSelectionData::CoilSelectionData( // constructor
    std::string const &coilName)
    : isCooling(false), isHeating(false), coilNum(-999), airloopNum(-999), oaControllerNum(-999), zoneEqNum(-999), oASysNum(-999), zoneHVACTypeNum(0),
      zoneHVACIndex(0), typeof_Coil(-999), coilSizingMethodConcurrence(-999), coilSizingMethodCapacity(-999), coilSizingMethodAirFlow(-999),
      isCoilSizingForTotalLoad(false), capIsAutosized(false), volFlowIsAutosized(false), coilWaterFlowUser(-999.0), oaPretreated(false),
      isSupplementalHeater(false), coilTotCapFinal(-999.0), coilSensCapFinal(-999.0), coilRefAirVolFlowFinal(-999.0),
      coilRefWaterVolFlowFinal(-999.0), coilTotCapAtPeak(-999.0), coilSensCapAtPeak(-999.0), coilDesMassFlow(-999.0), coilDesVolFlow(-999.0),
      coilDesEntTemp(-999.0), coilDesEntWetBulb(-999.0), coilDesEntHumRat(-999.0), coilDesEntEnth(-999.0), coilDesLvgTemp(-999.0),
      coilDesLvgWetBulb(-999.0), coilDesLvgHumRat(-999.0), coilDesLvgEnth(-999.0), coilDesWaterMassFlow(-999.0), coilDesWaterEntTemp(-999.0),
      coilDesWaterLvgTemp(-999.0), coilDesWaterTempDiff(-999.0), pltSizNum(-999), waterLoopNum(-999), oaPeakTemp(-999.00), oaPeakHumRat(-999.0),
      oaPeakWetBulb(-999.0), oaPeakVolFlow(-999.0), oaPeakVolFrac(-999.0), oaDoaTemp(-999.0), oaDoaHumRat(-999.0), raPeakTemp(-999.0),
      raPeakHumRat(-999.0), rmPeakTemp(-999.0), rmPeakHumRat(-999.0), rmPeakRelHum(-999.0), rmSensibleAtPeak(-999.0), rmLatentAtPeak(0.0),
      coilIdealSizCapOverSimPeakCap(-999.0), coilIdealSizCapUnderSimPeakCap(-999.0), reheatLoadMult(-999.0), minRatio(-999.0), maxRatio(-999.0),
      cpMoistAir(-999.0), cpDryAir(-999.0), rhoStandAir(-999.0), rhoFluid(-999.0), cpFluid(-999.0), coilCapFTIdealPeak(1.0), coilRatedTotCap(-999.0),
      coilRatedSensCap(-999.0), ratedAirMassFlow(-999.0), ratedCoilInDb(-999.0), ratedCoilInWb(-999.0), ratedCoilInHumRat(-999.0),
      ratedCoilInEnth(-999.0), ratedCoilOutDb(-999.0), ratedCoilOutWb(-999.0), ratedCoilOutHumRat(-999.0), ratedCoilOutEnth(-999.0),
      ratedCoilEff(-999.0), ratedCoilBpFactor(-999.0), ratedCoilAppDewPt(-999.0), ratedCoilOadbRef(-999.0), ratedCoilOawbRef(-999.0),

      supFanModelType(DataAirSystems::Invalid), supFanNum(0), supFanVecIndex(-1), fanSizeMaxAirVolumeFlow(-999.0), fanSizeMaxAirMassFlow(-999.0),
      fanHeatGainIdealPeak(-999.0), coilAndFanNetTotalCapacityIdealPeak(-999.0), plantDesMaxMassFlowRate(-999.0), plantDesRetTemp(-999.0),
      plantDesSupTemp(-999.0), plantDesDeltaTemp(-999.0), plantDesCapacity(-999.0), coilCapPrcntPlantCap(-999.0), coilFlowPrcntPlantFlow(-999.0),
      coilUA(-999.0)
{
    coilName_ = coilName;
    coilLocation = "unknown";
    desDayNameAtSensPeak = "unknown";
    coilSensePeakHrMin = "unknown";
    desDayNameAtTotalPeak = "unknown";
    coilTotalPeakHrMin = "unknown";
    desDayNameAtAirFlowPeak = "unknown";
    airPeakHrMin = "unknown";
    typeHVACname = "unknown";
    userNameforHVACsystem = "unknown";
    coilSizingMethodConcurrenceName = "N/A";
    coilSizingMethodCapacityName = "N/A";
    coilSizingMethodAirFlowName = "N/A";
    coilPeakLoadTypeToSizeOnName = "N/A";
    coilCapAutoMsg = "unknown";
    coilVolFlowAutoMsg = "unknown";
    coilWaterFlowAutoMsg = "unknown";
    coilOAPretreatMsg = "unknown";
    plantLoopName = "unknown";
    fanAssociatedWithCoilName = "unknown";
    fanTypeName = "unknown";
}

void ReportCoilSelection::finishCoilSummaryReportTable(EnergyPlusData &state)
{
    doFinalProcessingOfCoilData(state);
    writeCoilSelectionOutput(state);
    writeCoilSelectionOutput2(state);
}

void ReportCoilSelection::writeCoilSelectionOutput(EnergyPlusData &state)
{

    // make calls to fill out predefined tabular report entries for each coil selection report object
    for (auto &c : coilSelectionDataObjs) {

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilType, c->coilName_, c->coilObjName);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilLocation, c->coilName_, c->coilLocation);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilHVACType, c->coilName_, c->typeHVACname);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilHVACName, c->coilName_, c->userNameforHVACsystem);

        // begin std 229 existing heating coil table adding new variables
        if (c->isHeating) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchHeatCoilUsedAsSupHeat, c->coilName_, c->isSupplementalHeater ? "Yes" : "No");
            OutputReportPredefined::PreDefTableEntry(state,
                                                     state.dataOutRptPredefined->pdchHeatCoilAirloopName,
                                                     c->coilName_,
                                                     c->airloopNum > 0 && c->airloopNum <= state.dataHVACGlobal->NumPrimaryAirSys
                                                         ? state.dataAirSystemsData->PrimaryAirSystems(c->airloopNum).Name
                                                         : "N/A");
            OutputReportPredefined::PreDefTableEntry(state,
                                                     state.dataOutRptPredefined->pdchHeatCoilPlantloopName,
                                                     c->coilName_,
                                                     c->waterLoopNum > 0 ? state.dataPlnt->PlantLoop(c->waterLoopNum).Name : "N/A");
        }
        // end std 229 existing heating coil table adding new variables

        // begin std 229 New coil connections table entries
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilName_CCs, c->coilName_, c->coilName_);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilType_CCs, c->coilName_, c->coilObjName);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilLoc_CCs, c->coilName_, c->coilLocation);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilHVACType_CCs, c->coilName_, c->typeHVACname);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilHVACName_CCs, c->coilName_, c->userNameforHVACsystem);
        // end of std 229 New coil connections table entries

        if (c->zoneName.size() == 1) {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilZoneName, c->coilName_, c->zoneName[0]);
            // begin std 229 New coil connections table entries
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilZoneNames_CCs, c->coilName_, c->zoneName[0]);
            // end of std 229 New coil connections table entries
        } else if (c->zoneName.size() > 1) {
            // make list of zone names
            std::string tmpZoneList;
            for (std::size_t vecLoop = 0; vecLoop < c->zoneName.size(); ++vecLoop) {
                tmpZoneList += c->zoneName[vecLoop] + "; ";
            }
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilZoneName, c->coilName_, tmpZoneList);
            // begin std 229 New coil connections table entries
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilZoneNames_CCs, c->coilName_, tmpZoneList);
            // end of std 229 New coil connections table entries
        } else {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilZoneName, c->coilName_, "N/A");
            // begin std 229 New coil connections table entries
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilZoneNames_CCs, c->coilName_, "N/A");
            // end of std 229 New coil connections table entries
        }

        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchSysSizingMethCoinc, c->coilName_, c->coilSizingMethodConcurrenceName);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchSysSizingMethCap, c->coilName_, c->coilSizingMethodCapacityName);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchSysSizingMethAir, c->coilName_, c->coilSizingMethodAirFlowName);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilIsCapAutosized, c->coilName_, c->coilCapAutoMsg);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilIsAirFlowAutosized, c->coilName_, c->coilVolFlowAutoMsg);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilIsWaterFlowAutosized, c->coilName_, c->coilWaterFlowAutoMsg);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilIsOATreated, c->coilName_, c->coilOAPretreatMsg);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilFinalTotalCap, c->coilName_, c->coilTotCapFinal, 3);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilFinalSensCap, c->coilName_, c->coilSensCapFinal, 3);
        if (c->coilRefAirVolFlowFinal == -999.0 || c->coilRefAirVolFlowFinal == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilFinalAirVolFlowRate, c->coilName_, c->coilRefAirVolFlowFinal, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilFinalAirVolFlowRate, c->coilName_, c->coilRefAirVolFlowFinal, 6);
        }

        if (c->coilRefWaterVolFlowFinal == -999.0 || c->coilRefWaterVolFlowFinal == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilFinalPlantVolFlowRate, c->coilName_, c->coilRefWaterVolFlowFinal, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilFinalPlantVolFlowRate, c->coilName_, c->coilRefWaterVolFlowFinal, 8);
        }

        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchFanAssociatedWithCoilName, c->coilName_, c->fanAssociatedWithCoilName);
        // begin std 229 New coil connections table entries
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilSupFanName_CCs, c->coilName_, c->fanAssociatedWithCoilName);
        // end of std 229 New coil connections table entries

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFanAssociatedWithCoilType, c->coilName_, c->fanTypeName);
        // begin std 229 New coil connections table entries
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilSupFanType_CCs, c->coilName_, c->fanTypeName);
        // end of std 229 New coil connections table entries

        if (c->fanSizeMaxAirVolumeFlow == -999.0 || c->fanSizeMaxAirVolumeFlow == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchFanAssociatedVdotSize, c->coilName_, c->fanSizeMaxAirVolumeFlow, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchFanAssociatedVdotSize, c->coilName_, c->fanSizeMaxAirVolumeFlow, 6);
        }
        if (c->fanSizeMaxAirMassFlow == -999.0 || c->fanSizeMaxAirMassFlow == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchFanAssociatedMdotSize, c->coilName_, c->fanSizeMaxAirMassFlow, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchFanAssociatedMdotSize, c->coilName_, c->fanSizeMaxAirMassFlow, 8);
        }

        // begin std 229 New coil connections table entries
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilPlantName_CCs, c->coilName_, c->plantLoopName);
        OutputReportPredefined::PreDefTableEntry(state,
                                                 state.dataOutRptPredefined->pdchCoilAirloopName_CCs,
                                                 c->coilName_,
                                                 c->airloopNum > 0 && c->airloopNum <= state.dataHVACGlobal->NumPrimaryAirSys
                                                     ? state.dataAirSystemsData->PrimaryAirSystems(c->airloopNum).Name
                                                     : "N/A");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilPlantloopName_CCs, c->coilName_, c->plantLoopName);
        // end of std 229 New coil connections table entries

        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilDDnameSensIdealPeak, c->coilName_, c->desDayNameAtSensPeak);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilDateTimeSensIdealPeak, c->coilName_, c->coilSensePeakHrMin);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilDDnameTotIdealPeak, c->coilName_, c->desDayNameAtTotalPeak);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilDateTimeTotIdealPeak, c->coilName_, c->coilTotalPeakHrMin);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilDDnameAirFlowIdealPeak, c->coilName_, c->desDayNameAtAirFlowPeak);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilDateTimeAirFlowIdealPeak, c->coilName_, c->airPeakHrMin);

        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilPeakLoadTypeToSizeOn, c->coilName_, c->coilPeakLoadTypeToSizeOnName);

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilTotalCapIdealPeak, c->coilName_, c->coilTotCapAtPeak, 2);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilSensCapIdealPeak, c->coilName_, c->coilSensCapAtPeak, 2);
        if (c->coilDesMassFlow == -999.0 || c->coilDesMassFlow == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilAirMassFlowIdealPeak, c->coilName_, c->coilDesMassFlow, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilAirMassFlowIdealPeak, c->coilName_, c->coilDesMassFlow, 8);
        }
        if (c->coilDesVolFlow == -999.0 || c->coilDesVolFlow == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilAirVolumeFlowIdealPeak, c->coilName_, c->coilDesVolFlow, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilAirVolumeFlowIdealPeak, c->coilName_, c->coilDesVolFlow, 6);
        }
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilEntDryBulbIdealPeak, c->coilName_, c->coilDesEntTemp, 2);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilEntWetBulbIdealPeak, c->coilName_, c->coilDesEntWetBulb, 2);
        if (c->coilDesEntHumRat == -999.0 || c->coilDesEntHumRat == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilEntHumRatIdealPeak, c->coilName_, c->coilDesEntHumRat, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilEntHumRatIdealPeak, c->coilName_, c->coilDesEntHumRat, 8);
        }
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilEntEnthalpyIdealPeak, c->coilName_, c->coilDesEntEnth, 1);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilLvgDryBulbIdealPeak, c->coilName_, c->coilDesLvgTemp, 2);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilLvgWetBulbIdealPeak, c->coilName_, c->coilDesLvgWetBulb, 2);
        if (c->coilDesLvgHumRat == -999.0 || c->coilDesLvgHumRat == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilLvgHumRatIdealPeak, c->coilName_, c->coilDesLvgHumRat, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilLvgHumRatIdealPeak, c->coilName_, c->coilDesLvgHumRat, 8);
        }
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilLvgEnthalpyIdealPeak, c->coilName_, c->coilDesLvgEnth, 1);
        if (c->coilDesWaterMassFlow == -999.0 || c->coilDesWaterMassFlow == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilWaterMassFlowIdealPeak, c->coilName_, c->coilDesWaterMassFlow, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilWaterMassFlowIdealPeak, c->coilName_, c->coilDesWaterMassFlow, 8);
        }

        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilEntWaterTempIdealPeak, c->coilName_, c->coilDesWaterEntTemp, 2);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilLvgWaterTempIdealPeak, c->coilName_, c->coilDesWaterLvgTemp, 2);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilWaterDeltaTempIdealPeak, c->coilName_, c->coilDesWaterTempDiff, 2);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchFanHeatGainIdealPeak, c->coilName_, c->fanHeatGainIdealPeak, 3);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilNetTotalCapacityIdealPeak, c->coilName_, c->coilAndFanNetTotalCapacityIdealPeak, 2);

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilRatedTotalCap, c->coilName_, c->coilRatedTotCap, 2);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilRatedSensCap, c->coilName_, c->coilRatedSensCap, 2);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilOffRatingCapacityModifierIdealPeak, c->coilName_, c->coilCapFTIdealPeak, 4);
        if (c->ratedAirMassFlow == -999.0 || c->ratedAirMassFlow == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilRatedAirMass, c->coilName_, c->ratedAirMassFlow, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilRatedAirMass, c->coilName_, c->ratedAirMassFlow, 8);
        }

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilRatedEntDryBulb, c->coilName_, c->ratedCoilInDb, 2);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilRatedEntWetBulb, c->coilName_, c->ratedCoilInWb, 2);
        if (c->ratedCoilInHumRat == -999.0 || c->ratedCoilInHumRat == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilRatedEntHumRat, c->coilName_, c->ratedCoilInHumRat, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilRatedEntHumRat, c->coilName_, c->ratedCoilInHumRat, 8);
        }

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilRatedEntEnthalpy, c->coilName_, c->ratedCoilInEnth, 1);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilRatedLvgDryBulb, c->coilName_, c->ratedCoilOutDb, 2);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilRatedLvgWetBulb, c->coilName_, c->ratedCoilOutWb, 2);
        if (c->ratedCoilOutHumRat == -999.0 || c->ratedCoilOutHumRat == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilRatedLvgHumRat, c->coilName_, c->ratedCoilOutHumRat, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilRatedLvgHumRat, c->coilName_, c->ratedCoilOutHumRat, 8);
        }

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilRatedLvgEnthalpy, c->coilName_, c->ratedCoilOutEnth, 1);

        if (c->plantDesMaxMassFlowRate == -999.0 || c->plantDesMaxMassFlowRate == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchPlantMassFlowMaximum, c->coilName_, c->plantDesMaxMassFlowRate, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchPlantMassFlowMaximum, c->coilName_, c->plantDesMaxMassFlowRate, 8);
        }
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchPlantRetTempDesign, c->coilName_, c->plantDesRetTemp, 2);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchPlantSupTempDesign, c->coilName_, c->plantDesSupTemp, 2);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchPlantDeltaTempDesign, c->coilName_, c->plantDesDeltaTemp, 2);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchPlantCapacity, c->coilName_, c->plantDesCapacity, 2);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchCoilCapPrcntPlantCapacity, c->coilName_, c->coilCapPrcntPlantCap, 4);
        if (c->coilFlowPrcntPlantFlow == -999.0 || c->coilFlowPrcntPlantFlow == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilFlowPrcntPlantFlow, c->coilName_, c->coilFlowPrcntPlantFlow, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchCoilFlowPrcntPlantFlow, c->coilName_, c->coilFlowPrcntPlantFlow, 6);
        }

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOADryBulbIdealPeak, c->coilName_, c->oaPeakTemp, 2);
        if (c->oaPeakHumRat == -999.0 || c->oaPeakHumRat == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOAHumRatIdealPeak, c->coilName_, c->oaPeakHumRat, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOAHumRatIdealPeak, c->coilName_, c->oaPeakHumRat, 8);
        }

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOAWetBulbatIdealPeak, c->coilName_, c->oaPeakWetBulb, 2);
        if (c->oaPeakVolFlow == -999.0 || c->oaPeakVolFlow == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOAVolFlowIdealPeak, c->coilName_, c->oaPeakVolFlow, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOAVolFlowIdealPeak, c->coilName_, c->oaPeakVolFlow, 8);
        }

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOAFlowPrcntIdealPeak, c->coilName_, c->oaPeakVolFrac, 4);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchAirSysRADryBulbIdealPeak, c->coilName_, c->raPeakTemp, 2);
        if (c->raPeakHumRat == -999.0 || c->raPeakHumRat == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchAirSysRAHumRatIdealPeak, c->coilName_, c->raPeakHumRat, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchAirSysRAHumRatIdealPeak, c->coilName_, c->raPeakHumRat, 8);
        }

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchZoneAirDryBulbIdealPeak, c->coilName_, c->rmPeakTemp, 2);
        if (c->rmPeakHumRat == -999.0 || c->rmPeakHumRat == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchZoneAirHumRatIdealPeak, c->coilName_, c->rmPeakHumRat, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchZoneAirHumRatIdealPeak, c->coilName_, c->rmPeakHumRat, 8);
        }

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchZoneAirRelHumIdealPeak, c->coilName_, c->rmPeakRelHum, 4);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchCoilUA, c->coilName_, c->coilUA, 3);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchZoneSensibleLoadIdealPeak, c->coilName_, c->rmSensibleAtPeak, 2);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchZoneLatentLoadIdealPeak, c->coilName_, c->rmLatentAtPeak);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchReheatCoilMultiplier, c->coilName_, c->reheatLoadMult, 4);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchFlowCapRatioLowCapIncreaseRatio, c->coilName_, c->maxRatio, 5);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFlowCapRatioHiCapDecreaseRatio, c->coilName_, c->minRatio, 5);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchPlantFluidSpecificHeat, c->coilName_, c->cpFluid, 4);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchPlantFluidDensity, c->coilName_, c->rhoFluid, 4);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMoistAirSpecificHeat, c->coilName_, c->cpMoistAir, 4);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDryAirSpecificHeat, c->coilName_, c->cpDryAir, 4);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchStandRhoAir, c->coilName_, c->rhoStandAir, 4);
    }
}

void ReportCoilSelection::writeCoilSelectionOutput2(EnergyPlusData &state)
{

    // make calls to fill out predefined tabular report entries for each coil selection report object
    for (auto &c : coilSelectionDataObjs) {
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2CoilType, c->coilName_, c->coilObjName);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2CoilHVACType, c->coilName_, c->typeHVACname);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2CoilHVACName, c->coilName_, c->userNameforHVACsystem);

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2CoilFinalTotalCap, c->coilName_, c->coilTotCapFinal, 3);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2CoilFinalSensCap, c->coilName_, c->coilSensCapFinal, 3);
        if (c->coilRefAirVolFlowFinal == -999.0 || c->coilRefAirVolFlowFinal == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdch2CoilFinalAirVolFlowRate, c->coilName_, c->coilRefAirVolFlowFinal, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdch2CoilFinalAirVolFlowRate, c->coilName_, c->coilRefAirVolFlowFinal, 6);
        }

        if (c->coilRefWaterVolFlowFinal == -999.0 || c->coilRefWaterVolFlowFinal == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdch2CoilFinalPlantVolFlowRate, c->coilName_, c->coilRefWaterVolFlowFinal, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdch2CoilFinalPlantVolFlowRate, c->coilName_, c->coilRefWaterVolFlowFinal, 8);
        }

        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdch2CoilDDnameSensIdealPeak, c->coilName_, c->desDayNameAtSensPeak);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdch2CoilDateTimeSensIdealPeak, c->coilName_, c->coilSensePeakHrMin);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdch2CoilDDnameAirFlowIdealPeak, c->coilName_, c->desDayNameAtAirFlowPeak);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2CoilDateTimeAirFlowIdealPeak, c->coilName_, c->airPeakHrMin);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2CoilTotalCapIdealPeak, c->coilName_, c->coilTotCapAtPeak, 2);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2CoilSensCapIdealPeak, c->coilName_, c->coilSensCapAtPeak, 2);
        if (c->coilDesVolFlow == -999.0 || c->coilDesVolFlow == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdch2CoilAirVolumeFlowIdealPeak, c->coilName_, c->coilDesVolFlow, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdch2CoilAirVolumeFlowIdealPeak, c->coilName_, c->coilDesVolFlow, 6);
        }
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2CoilEntDryBulbIdealPeak, c->coilName_, c->coilDesEntTemp, 2);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdch2CoilEntWetBulbIdealPeak, c->coilName_, c->coilDesEntWetBulb, 2);
        if (c->coilDesEntHumRat == -999.0 || c->coilDesEntHumRat == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdch2CoilEntHumRatIdealPeak, c->coilName_, c->coilDesEntHumRat, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdch2CoilEntHumRatIdealPeak, c->coilName_, c->coilDesEntHumRat, 8);
        }
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2CoilLvgDryBulbIdealPeak, c->coilName_, c->coilDesLvgTemp, 2);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdch2CoilLvgWetBulbIdealPeak, c->coilName_, c->coilDesLvgWetBulb, 2);
        if (c->coilDesLvgHumRat == -999.0 || c->coilDesLvgHumRat == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdch2CoilLvgHumRatIdealPeak, c->coilName_, c->coilDesLvgHumRat, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdch2CoilLvgHumRatIdealPeak, c->coilName_, c->coilDesLvgHumRat, 8);
        }

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2CoilRatedTotalCap, c->coilName_, c->coilRatedTotCap, 2);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2CoilRatedSensCap, c->coilName_, c->coilRatedSensCap, 2);

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2OADryBulbIdealPeak, c->coilName_, c->oaPeakTemp, 2);
        if (c->oaPeakHumRat == -999.0 || c->oaPeakHumRat == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2OAHumRatIdealPeak, c->coilName_, c->oaPeakHumRat, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2OAHumRatIdealPeak, c->coilName_, c->oaPeakHumRat, 8);
        }

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2OAWetBulbatIdealPeak, c->coilName_, c->oaPeakWetBulb, 2);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2OAFlowPrcntIdealPeak, c->coilName_, c->oaPeakVolFrac, 4);

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2ZoneAirDryBulbIdealPeak, c->coilName_, c->rmPeakTemp, 2);
        if (c->rmPeakHumRat == -999.0 || c->rmPeakHumRat == -99999.0) {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdch2ZoneAirHumRatIdealPeak, c->coilName_, c->rmPeakHumRat, 1);
        } else {
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdch2ZoneAirHumRatIdealPeak, c->coilName_, c->rmPeakHumRat, 8);
        }

        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2ZoneAirRelHumIdealPeak, c->coilName_, c->rmPeakRelHum, 4);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2CoilUA, c->coilName_, c->coilUA, 3);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdch2ZoneSensibleLoadIdealPeak, c->coilName_, c->rmSensibleAtPeak, 2);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdch2ZoneLatentLoadIdealPeak, c->coilName_, c->rmLatentAtPeak);
    }
}

void ReportCoilSelection::setCoilFinalSizes(EnergyPlusData &state,
                                            std::string const &coilName,    // user-defined name of the coil
                                            std::string const &coilObjName, //  coil object name, e.g., Coil:Cooling:Water
                                            Real64 const totGrossCap,       // total capacity [W]
                                            Real64 const sensGrossCap,      // sensible capacity [W]
                                            Real64 const airFlowRate,       // design or reference or rated air flow rate [m3/s]
                                            Real64 const waterFlowRate      // design or reference or rated water flow rate [m3/s]
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilObjName);
    auto &c(coilSelectionDataObjs[index]);
    if (c != nullptr) {
        c->coilTotCapFinal = totGrossCap;
        c->coilSensCapFinal = sensGrossCap;
        c->coilRefAirVolFlowFinal = airFlowRate;
        c->coilRefWaterVolFlowFinal = waterFlowRate;
    }
}

void ReportCoilSelection::doAirLoopSetup(EnergyPlusData &state, int const coilVecIndex)
{
    // this routine sets up some things for central air systems, needs to follow setting of an airloop num
    auto &c(coilSelectionDataObjs[coilVecIndex]);
    if (c->airloopNum > 0 && c->airloopNum <= int(state.dataAirSystemsData->PrimaryAirSystems.size())) {
        // see if there is an OA controller
        if (state.dataAirSystemsData->PrimaryAirSystems(c->airloopNum).OASysExists) {
            // loop over OA controllers and match node num ?
            for (int loop = 1; loop <= state.dataMixedAir->NumOAControllers; ++loop) {
                if (state.dataAirSystemsData->PrimaryAirSystems(c->airloopNum).OASysInletNodeNum == state.dataMixedAir->OAController(loop).RetNode) {
                    c->oaControllerNum = loop;
                }
            }
        }
        // fill list of zones connected to this air loop
        // this could be reworked to use different structure which is available now since std 62.1 changes
        if (allocated(state.dataAirLoop->AirToZoneNodeInfo)) {
            if (state.dataAirLoop->AirToZoneNodeInfo(c->airloopNum).NumZonesCooled > 0) {
                int zoneCount = state.dataAirLoop->AirToZoneNodeInfo(c->airloopNum).NumZonesCooled;
                c->zoneNum.resize(zoneCount);
                c->zoneName.resize(zoneCount);
                for (int loopZone = 1; loopZone <= state.dataAirLoop->AirToZoneNodeInfo(c->airloopNum).NumZonesCooled; ++loopZone) {
                    c->zoneNum[loopZone - 1] = state.dataAirLoop->AirToZoneNodeInfo(c->airloopNum).CoolCtrlZoneNums(loopZone);
                    c->zoneName[loopZone - 1] = state.dataHeatBal->Zone(c->zoneNum[loopZone - 1]).Name;
                }
            }

            if (state.dataAirLoop->AirToZoneNodeInfo(c->airloopNum).NumZonesHeated > 0) {
                int zoneCount = state.dataAirLoop->AirToZoneNodeInfo(c->airloopNum).NumZonesHeated;
                for (int loopZone = 1; loopZone <= zoneCount; ++loopZone) {
                    int zoneIndex = state.dataAirLoop->AirToZoneNodeInfo(c->airloopNum).HeatCtrlZoneNums(loopZone);
                    // see if this zone is new or already in list
                    bool found = false;
                    for (auto &z : c->zoneNum) {
                        if (z == zoneIndex) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) { // add it
                        c->zoneNum.emplace_back(zoneIndex);
                        c->zoneName.emplace_back(state.dataHeatBal->Zone(zoneIndex).Name);
                    }
                }
            }
        }
    }
}

void ReportCoilSelection::doZoneEqSetup(EnergyPlusData &state, int const coilVecIndex)
{
    auto &c(coilSelectionDataObjs[coilVecIndex]);
    c->coilLocation = "Zone";
    c->zoneNum.resize(1);
    c->zoneNum[0] = c->zoneEqNum;
    c->zoneName.resize(1);
    c->zoneName[0] = state.dataHeatBal->Zone(c->zoneNum[0]).Name;
    c->typeHVACname = "Zone Equipment"; // init

    // find the system and get   c->oaControllerNum

    // need to rework for new multiple air handler in zone

    // going to need the zone inlet node index for this now... how to find it??

    // maybe not needed, would be set in other calls   c->airloopNum = DataZoneEquipment::ZoneEquipConfig( c->zoneEqNum  ).AirLoopNum;

    if (c->airloopNum > 0) {
        if (state.dataAirSystemsData->PrimaryAirSystems(c->airloopNum).OASysExists) {
            // loop over OA controllers and match node num ?
            for (int loop = 1; loop <= state.dataMixedAir->NumOAControllers; ++loop) {
                if (state.dataAirSystemsData->PrimaryAirSystems(c->airloopNum).OASysInletNodeNum == state.dataMixedAir->OAController(loop).RetNode) {
                    c->oaControllerNum = loop;
                }
            }
        }
        // fill out supply fan info
        switch (state.dataAirSystemsData->PrimaryAirSystems(c->airloopNum).supFanModelType) {
        case DataAirSystems::StructArrayLegacyFanModels: {

            state.dataRptCoilSelection->coilSelectionReportObj->setCoilSupplyFanInfo(
                state,
                c->coilName_,
                c->coilObjName,
                state.dataFans->Fan(state.dataAirSystemsData->PrimaryAirSystems(c->airloopNum).SupFanNum).FanName,
                DataAirSystems::StructArrayLegacyFanModels,
                state.dataAirSystemsData->PrimaryAirSystems(c->airloopNum).SupFanNum);
            break;
        }
        case DataAirSystems::ObjectVectorOOFanSystemModel: {

            state.dataRptCoilSelection->coilSelectionReportObj->setCoilSupplyFanInfo(
                state,
                c->coilName_,
                c->coilObjName,
                state.dataHVACFan->fanObjs[state.dataAirSystemsData->PrimaryAirSystems(c->airloopNum).supFanVecIndex]->name,
                DataAirSystems::ObjectVectorOOFanSystemModel,
                state.dataAirSystemsData->PrimaryAirSystems(c->airloopNum).supFanVecIndex);
            break;
        }
        default:
            // do nothing
            break;
        } // end switch
    }

    if (c->zoneEqNum > 0) {
        c->coilLocation = "Unknown";
        c->typeHVACname = "Unknown";
        c->userNameforHVACsystem = "Unknown";
        // now search equiment
        if (state.dataZoneEquip->ZoneEquipList(c->zoneEqNum).NumOfEquipTypes == 1) { // this must be it, fill strings for type and name
            c->typeHVACname = state.dataZoneEquip->ZoneEquipList(c->zoneEqNum).EquipTypeName(1);
            c->userNameforHVACsystem = state.dataZoneEquip->ZoneEquipList(c->zoneEqNum).EquipName(1);
            c->coilLocation = "Zone Equipment";
            c->zoneHVACTypeNum = state.dataZoneEquip->ZoneEquipList(c->zoneEqNum).EquipType(1);
            c->zoneHVACIndex = state.dataZoneEquip->ZoneEquipList(c->zoneEqNum).EquipIndex(1);
        } else if (state.dataZoneEquip->ZoneEquipList(c->zoneEqNum).NumOfEquipTypes > 1) {
            bool foundOne(false);
            for (int equipLoop = 1; equipLoop <= state.dataZoneEquip->ZoneEquipList(c->zoneEqNum).NumOfEquipTypes; ++equipLoop) {
                // go with the first ZoneHVAC device in the list
                DataZoneEquipment::ZoneEquipType equipType = state.dataZoneEquip->ZoneEquipList(c->zoneEqNum).EquipType(equipLoop);
                if (equipType == DataZoneEquipment::ZoneEquipType::VariableRefrigerantFlowTerminal ||
                    equipType == DataZoneEquipment::ZoneEquipType::EnergyRecoveryVentilator ||
                    equipType == DataZoneEquipment::ZoneEquipType::FourPipeFanCoil || equipType == DataZoneEquipment::ZoneEquipType::OutdoorAirUnit ||
                    equipType == DataZoneEquipment::ZoneEquipType::PackagedTerminalAirConditioner ||
                    equipType == DataZoneEquipment::ZoneEquipType::PackagedTerminalHeatPump ||
                    equipType == DataZoneEquipment::ZoneEquipType::UnitHeater || equipType == DataZoneEquipment::ZoneEquipType::UnitVentilator ||
                    equipType == DataZoneEquipment::ZoneEquipType::VentilatedSlab ||
                    equipType == DataZoneEquipment::ZoneEquipType::PackagedTerminalHeatPumpWaterToAir ||
                    equipType == DataZoneEquipment::ZoneEquipType::WindowAirConditioner ||
                    equipType == DataZoneEquipment::ZoneEquipType::DehumidifierDX) {
                    if (!foundOne) {
                        c->typeHVACname = state.dataZoneEquip->ZoneEquipList(c->zoneEqNum).EquipTypeName(equipLoop);
                        c->userNameforHVACsystem = state.dataZoneEquip->ZoneEquipList(c->zoneEqNum).EquipName(equipLoop);
                        foundOne = true;
                        c->coilLocation = "Zone Equipment";
                        c->zoneHVACTypeNum = state.dataZoneEquip->ZoneEquipList(c->zoneEqNum).EquipType(equipLoop);
                        c->zoneHVACIndex = state.dataZoneEquip->ZoneEquipList(c->zoneEqNum).EquipIndex(equipLoop);
                    } else { // or may have found another
                        c->typeHVACname += " or " + state.dataZoneEquip->ZoneEquipList(c->zoneEqNum).EquipTypeName(equipLoop);
                        c->userNameforHVACsystem += " or " + state.dataZoneEquip->ZoneEquipList(c->zoneEqNum).EquipName(equipLoop);
                    }
                }
            }
        }
    }
}

void ReportCoilSelection::doFinalProcessingOfCoilData(EnergyPlusData &state)
{
    // this routine does some final processing in preparation for writing out results
    for (auto &c : coilSelectionDataObjs) {

        // mine final/hard values from coil models

        if (c->zoneEqNum > 0) {
            c->coilLocation = "Unknown";
            c->typeHVACname = "Unknown";
            c->userNameforHVACsystem = "Unknown";
            // now search equipment
            auto const &zoneEquipList = state.dataZoneEquip->ZoneEquipList(c->zoneEqNum);
            if (zoneEquipList.NumOfEquipTypes == 1) { // this must be it, fill strings for type and name
                c->typeHVACname = zoneEquipList.EquipTypeName(1);
                c->userNameforHVACsystem = zoneEquipList.EquipName(1);
                c->coilLocation = "Zone Equipment";
            } else if (zoneEquipList.NumOfEquipTypes > 1) {
                bool foundOne(false);
                for (int equipLoop = 1; equipLoop <= zoneEquipList.NumOfEquipTypes; ++equipLoop) {
                    // go with the first ZoneHVAC device in the list
                    DataZoneEquipment::ZoneEquipType equipType = zoneEquipList.EquipType(equipLoop);
                    if (equipType == DataZoneEquipment::ZoneEquipType::VariableRefrigerantFlowTerminal ||
                        equipType == DataZoneEquipment::ZoneEquipType::EnergyRecoveryVentilator ||
                        equipType == DataZoneEquipment::ZoneEquipType::FourPipeFanCoil ||
                        equipType == DataZoneEquipment::ZoneEquipType::OutdoorAirUnit ||
                        equipType == DataZoneEquipment::ZoneEquipType::PackagedTerminalAirConditioner ||
                        equipType == DataZoneEquipment::ZoneEquipType::PackagedTerminalHeatPump ||
                        equipType == DataZoneEquipment::ZoneEquipType::UnitHeater || equipType == DataZoneEquipment::ZoneEquipType::UnitVentilator ||
                        equipType == DataZoneEquipment::ZoneEquipType::VentilatedSlab ||
                        equipType == DataZoneEquipment::ZoneEquipType::PackagedTerminalHeatPumpWaterToAir ||
                        equipType == DataZoneEquipment::ZoneEquipType::WindowAirConditioner ||
                        equipType == DataZoneEquipment::ZoneEquipType::DehumidifierDX) {
                        if (!foundOne) {
                            c->typeHVACname = zoneEquipList.EquipTypeName(equipLoop);
                            c->userNameforHVACsystem = zoneEquipList.EquipName(equipLoop);
                            foundOne = true;
                            c->coilLocation = "Zone Equipment";
                        } else { // or may have found another
                            c->typeHVACname += " or " + zoneEquipList.EquipTypeName(equipLoop);
                            c->userNameforHVACsystem += " or " + zoneEquipList.EquipName(equipLoop);
                        }
                    }
                } // for (equipLoop)
            }     // if (zoneEquipList.numOfEquipTypes > 0)
        }         // if (c->ZoneEqNum > 0)

        if (c->airloopNum > state.dataHVACGlobal->NumPrimaryAirSys && c->oASysNum > 0) {
            c->coilLocation = "DOAS AirLoop";
            c->typeHVACname = "AirLoopHVAC:DedicatedOutdoorAirSystem";
            int DOASSysNum = state.dataAirLoop->OutsideAirSys(c->oASysNum).AirLoopDOASNum;
            c->userNameforHVACsystem = state.dataAirLoopHVACDOAS->airloopDOAS[DOASSysNum].Name;
        } else if (c->airloopNum > 0 && c->zoneEqNum == 0) {
            c->coilLocation = "AirLoop";
            c->typeHVACname = "AirLoopHVAC";
            c->userNameforHVACsystem = state.dataAirSystemsData->PrimaryAirSystems(c->airloopNum).Name;
        } else if (c->zoneEqNum > 0 && c->airloopNum > 0) { // e.g. reheat coil, has a system and is zone equipment
            c->userNameforHVACsystem += " on air system named " + state.dataAirSystemsData->PrimaryAirSystems(c->airloopNum).Name;
            c->coilLocation = "Zone Equipment";
        }

        if (c->coilDesVolFlow > 0) {
            c->oaPeakVolFrac = (c->oaPeakVolFlow / c->coilDesVolFlow) * 100.0; // make into percentage
        } else {
            c->oaPeakVolFrac = -999.0;
        }

        if (c->coilSizingMethodConcurrence == DataSizing::NonCoincident) {
            c->coilSizingMethodConcurrenceName = "Non-Coincident";
        } else if (c->coilSizingMethodConcurrence == DataSizing::Coincident) {
            c->coilSizingMethodConcurrenceName = "Coincident";
        } else if (c->coilSizingMethodConcurrence == DataSizing::Combination) {
            c->coilSizingMethodConcurrenceName = "Combination";
        } else {
            c->coilSizingMethodConcurrenceName = "N/A";
        }

        if (c->coilSizingMethodCapacity == DataSizing::CoolingDesignCapacity) {
            c->coilSizingMethodCapacityName = "CoolingDesignCapacity";
        } else if (c->coilSizingMethodCapacity == DataSizing::HeatingDesignCapacity) {
            c->coilSizingMethodCapacityName = "HeatingDesignCapacity";
        } else if (c->coilSizingMethodCapacity == DataSizing::CapacityPerFloorArea) {
            c->coilSizingMethodCapacityName = "CapacityPerFloorArea";
        } else if (c->coilSizingMethodCapacity == DataSizing::FractionOfAutosizedCoolingCapacity) {
            c->coilSizingMethodCapacityName = "FractionOfAutosizedCoolingCapacity";
        } else if (c->coilSizingMethodCapacity == DataSizing::FractionOfAutosizedHeatingCapacity) {
            c->coilSizingMethodCapacityName = "FractionOfAutosizedHeatingCapacity";
        }

        if (c->coilSizingMethodAirFlow == DataSizing::SupplyAirFlowRate) {
            c->coilSizingMethodAirFlowName = "SupplyAirFlowRate";
        } else if (c->coilSizingMethodAirFlow == DataSizing::FlowPerFloorArea) {
            c->coilSizingMethodAirFlowName = "FlowPerFloorArea";
        } else if (c->coilSizingMethodAirFlow == DataSizing::FractionOfAutosizedCoolingAirflow) {
            c->coilSizingMethodAirFlowName = "FractionOfAutosizedCoolingAirflow";
        } else if (c->coilSizingMethodAirFlow == DataSizing::FractionOfAutosizedHeatingAirflow) {
            c->coilSizingMethodAirFlowName = "FractionOfAutosizedHeatingAirflow";
        }

        if (c->isCoilSizingForTotalLoad) {
            c->coilPeakLoadTypeToSizeOnName = "Total";
        } else {
            c->coilPeakLoadTypeToSizeOnName = "Sensible";
        }

        if (c->capIsAutosized) {
            c->coilCapAutoMsg = "Yes";
        } else {
            c->coilCapAutoMsg = "No";
        }

        if (c->volFlowIsAutosized) {
            c->coilVolFlowAutoMsg = "Yes";
        } else {
            c->coilVolFlowAutoMsg = "No";
        }

        if (c->oaPretreated) {
            c->coilOAPretreatMsg = "Yes";
        } else {
            c->coilOAPretreatMsg = "No";
        }

        // call psych routine to flush out moist air metrics from those available
        if (c->coilDesEntTemp != -999.0 && c->coilDesEntHumRat != -999.0) {
            c->coilDesEntWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                state, c->coilDesEntTemp, c->coilDesEntHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::doFinalProcessingOfCoilData");
            if (c->coilDesEntHumRat != -999.0) {
                c->coilDesEntEnth = Psychrometrics::PsyHFnTdbW(c->coilDesEntTemp, c->coilDesEntHumRat);
            }
        }
        if (c->oaPeakTemp != -999.0 && c->oaPeakHumRat != -999.0) {
            c->oaPeakWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                state, c->oaPeakTemp, c->oaPeakHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::doFinalProcessingOfCoilData");
        }

        if (c->waterLoopNum > 0 && c->pltSizNum > 0) {

            c->plantLoopName = state.dataPlnt->PlantLoop(c->waterLoopNum).Name;
            if (state.dataSize->PlantSizData(c->pltSizNum).LoopType != DataSizing::TypeOfPlantLoop::Steam) {
                c->rhoFluid = FluidProperties::GetDensityGlycol(state,
                                                                state.dataPlnt->PlantLoop(c->waterLoopNum).FluidName,
                                                                Constant::InitConvTemp,
                                                                state.dataPlnt->PlantLoop(c->waterLoopNum).FluidIndex,
                                                                "ReportCoilSelection::doFinalProcessingOfCoilData");

                c->cpFluid = FluidProperties::GetSpecificHeatGlycol(state,
                                                                    state.dataPlnt->PlantLoop(c->waterLoopNum).FluidName,
                                                                    Constant::InitConvTemp,
                                                                    state.dataPlnt->PlantLoop(c->waterLoopNum).FluidIndex,
                                                                    "ReportCoilSelection::doFinalProcessingOfCoilData");
            } else { // steam loop
                c->rhoFluid = FluidProperties::GetSatDensityRefrig(state,
                                                                   state.dataPlnt->PlantLoop(c->waterLoopNum).FluidName,
                                                                   100.0,
                                                                   1.0,
                                                                   state.dataPlnt->PlantLoop(c->waterLoopNum).FluidIndex,
                                                                   "ReportCoilSelection::doFinalProcessingOfCoilData");
                c->cpFluid = FluidProperties::GetSatSpecificHeatRefrig(state,
                                                                       state.dataPlnt->PlantLoop(c->waterLoopNum).FluidName,
                                                                       100.0,
                                                                       0.0,
                                                                       state.dataPlnt->PlantLoop(c->waterLoopNum).FluidIndex,
                                                                       "ReportCoilSelection::doFinalProcessingOfCoilData");
            }
            c->plantDesMaxMassFlowRate = state.dataPlnt->PlantLoop(c->waterLoopNum).MaxMassFlowRate;
            if (c->plantDesMaxMassFlowRate > 0.0 && c->coilDesWaterMassFlow > 0.0) {
                c->coilFlowPrcntPlantFlow = (c->coilDesWaterMassFlow / c->plantDesMaxMassFlowRate) * 100.0; // convert to percentage.
            }
        }
        // fill out some fan information
        switch (c->supFanModelType) {
        case DataAirSystems::StructArrayLegacyFanModels: {
            int locFanTypeNum(0);
            bool errorsFound(false);
            Fans::GetFanType(state, c->fanAssociatedWithCoilName, locFanTypeNum, errorsFound);
            if (locFanTypeNum == DataHVACGlobals::FanType_SimpleConstVolume) {
                c->fanTypeName = "Fan:ConstantVolume";
            } else if (locFanTypeNum == DataHVACGlobals::FanType_SimpleVAV) {
                c->fanTypeName = "Fan:VariableVolume";
            } else if (locFanTypeNum == DataHVACGlobals::FanType_SimpleOnOff) {
                c->fanTypeName = "Fan:OnOff";
            } else if (locFanTypeNum == DataHVACGlobals::FanType_ZoneExhaust) {
                c->fanTypeName = "Fan:ZoneExhaust";
            } else if (locFanTypeNum == DataHVACGlobals::FanType_ComponentModel) {
                c->fanTypeName = "Fan:ComponentModel";
            }
            if (c->supFanNum <= 0) {
                Fans::GetFanIndex(state, c->fanAssociatedWithCoilName, c->supFanNum, errorsFound, c->fanTypeName);
            }
            c->fanSizeMaxAirVolumeFlow =
                Fans::GetFanDesignVolumeFlowRate(state, c->fanTypeName, c->fanAssociatedWithCoilName, errorsFound, c->supFanNum);
            c->fanSizeMaxAirMassFlow = state.dataFans->Fan(c->supFanNum).MaxAirMassFlowRate;
            break;
        }
        case DataAirSystems::ObjectVectorOOFanSystemModel: {
            c->fanTypeName = "Fan:SystemModel";
            if (c->supFanVecIndex < 0) {
                c->supFanVecIndex = HVACFan::getFanObjectVectorIndex(state, c->fanAssociatedWithCoilName);
            }
            c->fanSizeMaxAirVolumeFlow = state.dataHVACFan->fanObjs[c->supFanVecIndex]->designAirVolFlowRate;
            c->fanSizeMaxAirMassFlow = state.dataHVACFan->fanObjs[c->supFanVecIndex]->maxAirMassFlowRate();
            break;
        }
        default:
            // do nothing
            break;
        } // end switch

        c->coilAndFanNetTotalCapacityIdealPeak = c->coilTotCapAtPeak - c->fanHeatGainIdealPeak;

        // fill out some plant design info
        if (c->pltSizNum > 0) {
            c->plantDesSupTemp = state.dataSize->PlantSizData(c->pltSizNum).ExitTemp;
            c->plantDesDeltaTemp = state.dataSize->PlantSizData(c->pltSizNum).DeltaT;
            if (state.dataSize->PlantSizData(c->pltSizNum).LoopType == DataSizing::TypeOfPlantLoop::Heating) {
                c->plantDesRetTemp = c->plantDesSupTemp - c->plantDesDeltaTemp;
            } else if (state.dataSize->PlantSizData(c->pltSizNum).LoopType == DataSizing::TypeOfPlantLoop::Cooling ||
                       state.dataSize->PlantSizData(c->pltSizNum).LoopType == DataSizing::TypeOfPlantLoop::Condenser) {
                c->plantDesRetTemp = c->plantDesSupTemp + c->plantDesDeltaTemp;
            }

            if (state.dataSize->PlantSizData(c->pltSizNum).LoopType != DataSizing::TypeOfPlantLoop::Steam) {
                c->plantDesCapacity = c->cpFluid * c->rhoFluid * state.dataSize->PlantSizData(c->pltSizNum).DeltaT *
                                      state.dataSize->PlantSizData(c->pltSizNum).DesVolFlowRate;
            } else {
                // find boiler on this plant loop and get capacity from it
                if (allocated(state.dataBoilerSteam->Boiler)) {
                    for (int boilerIndex = 1; boilerIndex <= (int)state.dataBoilerSteam->Boiler.size(); ++boilerIndex) {
                        if (state.dataBoilerSteam->Boiler(boilerIndex).plantLoc.loopNum == c->waterLoopNum) { // steam boiler on this loop
                            c->plantDesSupTemp = state.dataBoilerSteam->Boiler(boilerIndex).TempUpLimitBoilerOut;
                            c->plantDesRetTemp = state.dataBoilerSteam->Boiler(boilerIndex).TempUpLimitBoilerOut - c->plantDesDeltaTemp;
                            c->plantDesCapacity = state.dataBoilerSteam->Boiler(boilerIndex).NomCap;
                        }
                    }
                }
            }

            if (c->plantDesCapacity > 0.0) {
                c->coilCapPrcntPlantCap = (c->coilTotCapAtPeak / c->plantDesCapacity) * 100.0; // convert to percentage.
            }
        }

        if (c->pltSizNum == 0 && c->waterLoopNum == 0) {
            c->rhoFluid = -999.0;
            c->cpFluid = -999.0;
            c->plantDesMaxMassFlowRate = -999.0;
            c->coilFlowPrcntPlantFlow = -999.0;
            c->plantDesSupTemp = -999.0;
            c->plantDesDeltaTemp = -999.0;
            c->plantDesRetTemp = -999.0;
            c->coilDesWaterMassFlow = -999.0;
            c->coilDesWaterEntTemp = -999.0;
            c->coilDesWaterLvgTemp = -999.0;
            c->coilDesWaterTempDiff = -999.0;
            c->plantDesCapacity = -999.0;
            c->coilCapPrcntPlantCap = -999.0;
            c->coilFlowPrcntPlantFlow = -999.0;
        }

        c->cpDryAir = Psychrometrics::PsyCpAirFnW(0.0);
        c->rhoStandAir = state.dataEnvrn->StdRhoAir;

        // apply ADP method to find an SHR for Ideal loads peak, calculate sensible capacity for cooling coils
        if (c->coilDesEntTemp > c->coilDesLvgTemp) { // cooling coil
            Real64 CoilADPTemp =
                Psychrometrics::PsyTdpFnWPb(state, c->coilDesLvgHumRat, state.dataEnvrn->StdBaroPress); // apparatus dewpoint temperature
            Real64 CoilADPHumRat =
                Psychrometrics::PsyWFnTdpPb(state, CoilADPTemp, state.dataEnvrn->StdBaroPress); // humidity ratio at apparatus dewpoint temperaure
            Real64 CoilTinwADPEnthalpy = Psychrometrics::PsyHFnTdbW(
                c->coilDesEntTemp, CoilADPHumRat); // Enthalpy at inlet drybulb and humidity ratio at apparatus dewpoint temperature
            Real64 CoilADPEnthalpy =
                Psychrometrics::PsyHFnTdbW(CoilADPTemp, CoilADPHumRat); // Enthalpy at apparatus dewpoint, with Tdb set at apparatus dewpoint
            Real64 SHRatIdealPeak(1.0);
            if ((c->coilDesEntEnth - CoilADPEnthalpy) > 1.e-10) {
                SHRatIdealPeak = min((CoilTinwADPEnthalpy - CoilADPEnthalpy) / (c->coilDesEntEnth - CoilADPEnthalpy), 1.0); // calculate SHR
            } else {
                SHRatIdealPeak = 1.0;
            }
            c->coilSensCapAtPeak = SHRatIdealPeak * c->coilTotCapAtPeak;
        }
    } // end for loop over each coil
}

int ReportCoilSelection::getIndexForOrCreateDataObjFromCoilName(EnergyPlusData &state,
                                                                std::string const &coilName, // user-defined name of the coil
                                                                std::string const &coilType  // idf input object class name of coil
)
{
    int index(-1);
    for (int i = 0; i < numCoilsReported_; i++) {
        if (coilSelectionDataObjs[i] != nullptr) {
            if (Util::SameString(coilSelectionDataObjs[i]->coilName_, coilName)) {
                if (Util::SameString(coilSelectionDataObjs[i]->coilObjName, coilType)) {
                    return index = i;
                } else {
                    // throw error  coil type does not match coil name, check for unique names across coil types
                    ShowWarningError(state,
                                     format("check for unique coil names across different coil types: {} occurs in both {} and {}",
                                            coilName,
                                            coilType,
                                            coilSelectionDataObjs[i]->coilObjName));
                }
            }
        }
    }

    if (index == -1) { // then did not find it
        // check if really a coil type
        bool found(false);
        bool locIsCooling(false);
        bool locIsHeating(false);
        for (int loop = 1; loop <= DataHVACGlobals::NumAllCoilTypes; ++loop) {
            if (Util::SameString(coilType, DataHVACGlobals::cAllCoilTypes(loop))) {
                found = true;
                locIsCooling = Util::SameString(coilType, DataHVACGlobals::cCoolingCoilTypes(loop));
                locIsHeating = Util::SameString(coilType, DataHVACGlobals::cHeatingCoilTypes(loop));
                break;
            }
        }
        if (found) {
            coilSelectionDataObjs.emplace_back(new CoilSelectionData(coilName));
            index = coilSelectionDataObjs.size() - 1;
            coilSelectionDataObjs[index]->coilObjName = coilType;
            ++numCoilsReported_;
            coilSelectionDataObjs[index]->isCooling = locIsCooling;
            coilSelectionDataObjs[index]->isHeating = locIsHeating;
        }
    }

    if (index == -1) {
        ShowFatalError(state, format("getIndexForOrCreateDataObjFromCoilName: Developer error - not a coil: {} = {}", coilType, coilName));
    }
    return index;
}

void ReportCoilSelection::setRatedCoilConditions(EnergyPlusData &state,
                                                 std::string const &coilName,     // ! user-defined name of the coil
                                                 std::string const &coilObjName,  //  coil object name, e.g., Coil:Cooling:Water
                                                 Real64 const RatedCoilTotCap,    // ! rated coil total capacity [W]
                                                 Real64 const RatedCoilSensCap,   // rated coil sensible capacity [W]
                                                 Real64 const RatedAirMassFlow,   // rated coil design air mass flow rate [m3/s]
                                                 Real64 const RatedCoilInDb,      // rated coil inlet air dry bulb at time of peak [C]
                                                 Real64 const RatedCoilInHumRat,  // rated coil inlet air humidity ratio [kgWater/kgDryAir]
                                                 Real64 const RatedCoilInWb,      // rated coil inlet air wet bulb [C]
                                                 Real64 const RatedCoilOutDb,     // rated coil outlet air dry bulb [C]
                                                 Real64 const RatedCoilOutHumRat, // rated coil outlet air humidity ratio, [kgWater/kgDryAir]
                                                 Real64 const RatedCoilOutWb,     // rated coil outlet air wet bulb [C]
                                                 Real64 const RatedCoilOadbRef,   // rated DX coil outside air dry bulb reference [C]
                                                 Real64 const RatedCoilOawbRef,   // rated DX coil outside air wet bulb reference [C]
                                                 Real64 const RatedCoilBpFactor,  // rated coil bypass factor
                                                 Real64 const RatedCoilEff        // rated coil effectiveness
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilObjName);
    auto &c(coilSelectionDataObjs[index]);
    c->coilRatedTotCap = RatedCoilTotCap;
    c->coilRatedSensCap = RatedCoilSensCap;
    c->ratedAirMassFlow = RatedAirMassFlow;
    c->ratedCoilInDb = RatedCoilInDb;
    c->ratedCoilInWb = RatedCoilInWb;
    c->ratedCoilInHumRat = RatedCoilInHumRat;
    if ((RatedCoilInDb == -999.0) || (RatedCoilInHumRat == -999.0)) {
        c->ratedCoilInEnth = -999.0;
    } else {
        c->ratedCoilInEnth = Psychrometrics::PsyHFnTdbW(RatedCoilInDb, RatedCoilInHumRat);
    }

    c->ratedCoilOutDb = RatedCoilOutDb;
    c->ratedCoilOutWb = RatedCoilOutWb;
    c->ratedCoilOutHumRat = RatedCoilOutHumRat;
    if ((RatedCoilOutDb == -999.0) || (RatedCoilOutHumRat == -999.0)) {
        c->ratedCoilOutEnth = -999.0;
    } else {
        c->ratedCoilOutEnth = Psychrometrics::PsyHFnTdbW(RatedCoilOutDb, RatedCoilOutHumRat);
    }

    c->ratedCoilEff = RatedCoilEff;
    c->ratedCoilBpFactor = RatedCoilBpFactor;
    // TODO    //c->ratedCoilAppDewPt =
    c->ratedCoilOadbRef = RatedCoilOadbRef;
    c->ratedCoilOawbRef = RatedCoilOawbRef;
}

void ReportCoilSelection::setCoilAirFlow(EnergyPlusData &state,
                                         std::string const &coilName, // user-defined name of the coil
                                         std::string const &coilType, // idf input object class name of coil
                                         Real64 const airVdot,        // air flow rate in m3/s
                                         bool const isAutoSized       // true if air flow was autosized
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->coilDesVolFlow = airVdot;
    c->volFlowIsAutosized = isAutoSized;

    c->coilDesMassFlow = airVdot * state.dataEnvrn->StdRhoAir;
}

void ReportCoilSelection::setCoilWaterFlowNodeNums(EnergyPlusData &state,
                                                   std::string const &coilName, // user-defined name of the coil
                                                   std::string const &coilType, // idf input object class name of coil
                                                   Real64 const waterVdot,      // plant fluid flow rate in m3/s
                                                   bool const isAutoSized,      // true if water flow was autosized
                                                   int const inletNodeNum,      // coil chw inlet node num
                                                   int const outletNodeNum,     // coil chw outlet node num
                                                   int const plantLoopNum       // plant loop structure index
)
{
    int plantSizNum = -999;
    if ((state.dataSize->NumPltSizInput > 0) && (inletNodeNum > 0) && (outletNodeNum > 0)) {
        bool errorsfound = false;
        plantSizNum = PlantUtilities::MyPlantSizingIndex(state, "water coil", coilName, inletNodeNum, outletNodeNum, errorsfound);
    }
    state.dataRptCoilSelection->coilSelectionReportObj->setCoilWaterFlowPltSizNum(
        state, coilName, coilType, waterVdot, isAutoSized, plantSizNum, plantLoopNum);
}

void ReportCoilSelection::setCoilWaterFlowPltSizNum(EnergyPlusData &state,
                                                    std::string const &coilName, // user-defined name of the coil
                                                    std::string const &coilType, // idf input object class name of coil
                                                    Real64 const waterVdot,      // plant fluid flow rate in m3/s
                                                    bool const isAutoSized,      // true if water flow was autosized
                                                    int const plantSizNum,       // plant sizing structure index
                                                    int const plantLoopNum       // plant loop structure index
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->pltSizNum = plantSizNum;
    c->waterLoopNum = plantLoopNum;
    if (c->waterLoopNum > 0) {
        c->plantLoopName = state.dataPlnt->PlantLoop(c->waterLoopNum).Name;
    }

    if (c->waterLoopNum > 0 && c->pltSizNum > 0) {
        if (state.dataSize->PlantSizData(c->pltSizNum).LoopType != DataSizing::TypeOfPlantLoop::Steam) {
            c->rhoFluid = FluidProperties::GetDensityGlycol(state,
                                                            state.dataPlnt->PlantLoop(c->waterLoopNum).FluidName,
                                                            Constant::InitConvTemp,
                                                            state.dataPlnt->PlantLoop(c->waterLoopNum).FluidIndex,
                                                            "ReportCoilSelection::setCoilWaterFlow");

            c->cpFluid = FluidProperties::GetSpecificHeatGlycol(state,
                                                                state.dataPlnt->PlantLoop(c->waterLoopNum).FluidName,
                                                                Constant::InitConvTemp,
                                                                state.dataPlnt->PlantLoop(c->waterLoopNum).FluidIndex,
                                                                "ReportCoilSelection::setCoilWaterFlow");
        } else { // steam loop
            c->rhoFluid = FluidProperties::GetSatDensityRefrig(state,
                                                               state.dataPlnt->PlantLoop(c->waterLoopNum).FluidName,
                                                               100.0,
                                                               1.0,
                                                               state.dataPlnt->PlantLoop(c->waterLoopNum).FluidIndex,
                                                               "ReportCoilSelection::setCoilWaterFlow");
            c->cpFluid = FluidProperties::GetSatSpecificHeatRefrig(state,
                                                                   state.dataPlnt->PlantLoop(c->waterLoopNum).FluidName,
                                                                   100.0,
                                                                   0.0,
                                                                   state.dataPlnt->PlantLoop(c->waterLoopNum).FluidIndex,
                                                                   "ReportCoilSelection::setCoilWaterFlow");
        }
    }
    if (c->rhoFluid > 0.0) {
        c->coilDesWaterMassFlow = waterVdot * c->rhoFluid;
    }
    if (isAutoSized) {
        c->coilWaterFlowAutoMsg = "Yes";
    } else {
        c->coilWaterFlowAutoMsg = "No";
    }
}

void ReportCoilSelection::setCoilEntAirTemp(EnergyPlusData &state,
                                            std::string const &coilName,    // user-defined name of the coil
                                            std::string const &coilType,    // idf input object class name of coil
                                            Real64 const entAirDryBulbTemp, // degree C air entering coil
                                            int const curSysNum,            // airloop system number index, if non zero
                                            int const curZoneEqNum          // zone equipment list index, if non-zero
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->coilDesEntTemp = entAirDryBulbTemp;
    c->airloopNum = curSysNum;
    doAirLoopSetup(state, index);
    c->zoneEqNum = curZoneEqNum;
}

void ReportCoilSelection::setCoilEntAirHumRat(EnergyPlusData &state,
                                              std::string const &coilName, // user-defined name of the coil
                                              std::string const &coilType, // idf input object class name of coil
                                              Real64 const entAirHumrat    //
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->coilDesEntHumRat = entAirHumrat;
}

void ReportCoilSelection::setCoilEntWaterTemp(EnergyPlusData &state,
                                              std::string const &coilName, // user-defined name of the coil
                                              std::string const &coilType, // idf input object class name of coil
                                              Real64 const entWaterTemp    //
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->coilDesWaterEntTemp = entWaterTemp;
}

void ReportCoilSelection::setCoilLvgWaterTemp(EnergyPlusData &state,
                                              std::string const &coilName, // user-defined name of the coil
                                              std::string const &coilType, // idf input object class name of coil
                                              Real64 const lvgWaterTemp    //
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->coilDesWaterLvgTemp = lvgWaterTemp;
}

void ReportCoilSelection::setCoilWaterDeltaT(EnergyPlusData &state,
                                             std::string const &coilName, // user-defined name of the coil
                                             std::string const &coilType, // idf input object class name of coil
                                             Real64 const CoilWaterDeltaT // degree C temperature difference used to size coil
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->coilDesWaterTempDiff = CoilWaterDeltaT;
}

void ReportCoilSelection::setCoilLvgAirTemp(EnergyPlusData &state,
                                            std::string const &coilName,   // user-defined name of the coil
                                            std::string const &coilType,   // idf input object class name of coil
                                            Real64 const lvgAirDryBulbTemp //
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->coilDesLvgTemp = lvgAirDryBulbTemp;
}

void ReportCoilSelection::setCoilLvgAirHumRat(EnergyPlusData &state,
                                              std::string const &coilName, // user-defined name of the coil
                                              std::string const &coilType, // idf input object class name of coil
                                              Real64 const lvgAirHumRat    //
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->coilDesLvgHumRat = lvgAirHumRat;
}

std::string PeakHrMinString(EnergyPlusData &state, const int designDay, const int timeStepAtPeak)
{
    return fmt::format("{}/{} {}",
                       state.dataWeather->DesDayInput(designDay).Month,
                       state.dataWeather->DesDayInput(designDay).DayOfMonth,
                       ReportCoilSelection::getTimeText(state, timeStepAtPeak));
}

void ReportCoilSelection::setCoilCoolingCapacity(
    EnergyPlusData &state,
    std::string const &coilName,       // user-defined name of the coil
    std::string const &coilType,       // idf input object class name of coil
    Real64 const TotalCoolingCap,      // {W} coil cooling capacity, sizing result
    bool const isAutoSize,             // true if value was autosized
    int const curSysNum,               // airloop system number index, if non zero
    int const curZoneEqNum,            // zone equipment list index, if non-zero
    int const curOASysNum,             // OA system equipment list index, if non-zero
    Real64 const fanCoolLoad,          // {W} fan load used in ideal loads coil sizing
    Real64 const coilCapFunTempFac,    // {W} curve result for modification factor for capacity as a function of temperature
    Real64 const DXFlowPerCapMinRatio, // non dimensional ratio, capacity adjustment ratio min
    Real64 const DXFlowPerCapMaxRatio  // non dimensional ratio, capacity adjustment ratio max
)
{
    auto &ZoneEqSizing(state.dataSize->ZoneEqSizing);
    auto &SysSizPeakDDNum(state.dataSize->SysSizPeakDDNum);

    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    // no this is adjusted back to ratings    c->coilTotCapAtPeak = TotalCoolingCap;
    c->coilCapFTIdealPeak = coilCapFunTempFac;
    c->coilTotCapAtPeak = TotalCoolingCap * c->coilCapFTIdealPeak;
    c->capIsAutosized = isAutoSize;
    c->minRatio = DXFlowPerCapMinRatio;
    c->maxRatio = DXFlowPerCapMaxRatio;

    c->fanHeatGainIdealPeak = fanCoolLoad;
    c->airloopNum = curSysNum;
    doAirLoopSetup(state, index);
    c->zoneEqNum = curZoneEqNum;
    //    if ( c->zoneEqNum > 0 ) doZoneEqSetup( index );
    c->oASysNum = curOASysNum;

    if (curSysNum > 0 && c->zoneEqNum == 0 && allocated(state.dataSize->FinalSysSizing) && allocated(SysSizPeakDDNum) &&
        curSysNum <= state.dataHVACGlobal->NumPrimaryAirSys) {

        // These next blocks does not always work with SizingPeriod:WeatherFileDays or SizingPeriod:WeatherFileConditionType, protect against hard
        // crash
        if (SysSizPeakDDNum(curSysNum).SensCoolPeakDD > 0 && SysSizPeakDDNum(curSysNum).SensCoolPeakDD <= state.dataEnvrn->TotDesDays) {
            c->desDayNameAtSensPeak = state.dataWeather->DesDayInput(SysSizPeakDDNum(curSysNum).SensCoolPeakDD).Title;
            c->coilSensePeakHrMin = PeakHrMinString(state,
                                                    SysSizPeakDDNum(curSysNum).SensCoolPeakDD,
                                                    SysSizPeakDDNum(curSysNum).TimeStepAtSensCoolPk(SysSizPeakDDNum(curSysNum).SensCoolPeakDD));
        }
        if (SysSizPeakDDNum(curSysNum).TotCoolPeakDD > 0 && SysSizPeakDDNum(curSysNum).TotCoolPeakDD <= state.dataEnvrn->TotDesDays) {
            c->desDayNameAtTotalPeak = state.dataWeather->DesDayInput(SysSizPeakDDNum(curSysNum).TotCoolPeakDD).Title;
            c->coilTotalPeakHrMin = PeakHrMinString(state,
                                                    SysSizPeakDDNum(curSysNum).TotCoolPeakDD,
                                                    SysSizPeakDDNum(curSysNum).TimeStepAtTotCoolPk(SysSizPeakDDNum(curSysNum).TotCoolPeakDD));
        }

        if (SysSizPeakDDNum(curSysNum).CoolFlowPeakDD > 0 && SysSizPeakDDNum(curSysNum).CoolFlowPeakDD <= state.dataEnvrn->TotDesDays) {
            c->desDayNameAtAirFlowPeak = state.dataWeather->DesDayInput(SysSizPeakDDNum(curSysNum).CoolFlowPeakDD).Title;
            c->airPeakHrMin = PeakHrMinString(state,
                                              SysSizPeakDDNum(curSysNum).CoolFlowPeakDD,
                                              SysSizPeakDDNum(curSysNum).TimeStepAtCoolFlowPk(SysSizPeakDDNum(curSysNum).CoolFlowPeakDD));
        }

        auto &finalSysSizing = state.dataSize->FinalSysSizing(curSysNum);
        c->isCoilSizingForTotalLoad = (finalSysSizing.coolingPeakLoad == DataSizing::PeakLoad::TotalCooling);
        c->oaPeakTemp = finalSysSizing.OutTempAtCoolPeak;
        c->oaPeakVolFlow = finalSysSizing.DesOutAirVolFlow;
        c->oaPeakHumRat = finalSysSizing.OutHumRatAtCoolPeak;
        c->raPeakTemp = finalSysSizing.RetTempAtCoolPeak;
        c->raPeakHumRat = finalSysSizing.RetHumRatAtCoolPeak;
        c->coilSizingMethodConcurrence = finalSysSizing.SizingOption;
        c->coilSizingMethodCapacity = finalSysSizing.CoolingCapMethod;
        c->coilSizingMethodAirFlow = finalSysSizing.ScaleCoolSAFMethod;
        // DesOutAirVolFlow

        // loop over cooled zones attached to this airloop to find average Room condition
        // change weighting to use supply air flow rate rather than zone air volume for all the zones on this coil's air system
        Real64 sumT_Vdot(0.0);   // numerator for average zone temperature, zone temperature values times zone supply air volume flow rate
        Real64 sumW_Vdot(0.0);   // numerator average zone humidity ratio, zone hum rat value times zone supply air volume flow rate
        Real64 sumSensLoad(0.0); // straight total for zone design loads
        Real64 sumVdot(0.0);     // denominator for supply air flow rate weighted averages

        // Decide what day and time to use for zone/room averages
        int SysPeakDDnum(0);
        int SysPeakTimeStepInDay(0);
        if (finalSysSizing.coolingPeakLoad == DataSizing::PeakLoad::TotalCooling) {
            SysPeakDDnum = SysSizPeakDDNum(curSysNum).TotCoolPeakDD;
            if (SysPeakDDnum > 0) SysPeakTimeStepInDay = SysSizPeakDDNum(curSysNum).TimeStepAtTotCoolPk(SysSizPeakDDNum(curSysNum).TotCoolPeakDD);
        } else if (finalSysSizing.coolingPeakLoad == DataSizing::PeakLoad::SensibleCooling) {
            SysPeakDDnum = SysSizPeakDDNum(curSysNum).SensCoolPeakDD;
            if (SysPeakDDnum > 0) SysPeakTimeStepInDay = SysSizPeakDDNum(curSysNum).TimeStepAtSensCoolPk(SysSizPeakDDNum(curSysNum).SensCoolPeakDD);
        }

        if (SysPeakDDnum > 0 && SysPeakTimeStepInDay > 0) {
            for (auto &z : c->zoneNum) {
                auto const &thisCalcZoneSizing = state.dataSize->CalcZoneSizing(SysPeakDDnum, z);
                auto const &thisFinalZoneSizing = state.dataSize->FinalZoneSizing(z);
                Real64 mult = state.dataHeatBal->Zone(z).Multiplier * state.dataHeatBal->Zone(z).ListMultiplier;
                Real64 Tz = thisCalcZoneSizing.CoolZoneTempSeq(SysPeakTimeStepInDay);
                Real64 Vdot_z = thisCalcZoneSizing.CoolFlowSeq(SysPeakTimeStepInDay);
                if (Vdot_z == 0.0) { // take value from final zone sizing
                    Vdot_z = thisFinalZoneSizing.CoolMassFlow;
                    if (Vdot_z == 0.0) {
                        Vdot_z = finalSysSizing.DesCoolVolFlow * state.dataEnvrn->StdRhoAir / c->zoneNum.size();
                    }
                }
                Real64 Wz = thisCalcZoneSizing.CoolZoneHumRatSeq(SysPeakTimeStepInDay);
                sumT_Vdot += Tz * Vdot_z * mult;
                sumW_Vdot += Wz * Vdot_z * mult;
                sumVdot += Vdot_z * mult;
                Real64 Qdot_z = thisCalcZoneSizing.CoolLoadSeq(SysPeakTimeStepInDay);
                if (Qdot_z > 0.0) {
                    sumSensLoad += Qdot_z * mult;
                } else {
                    sumSensLoad += thisFinalZoneSizing.DesCoolLoad * mult;
                }
            }
        }
        if (c->zoneNum.size() > 0 && sumVdot > 0.0) {
            c->rmPeakTemp = (sumT_Vdot / sumVdot);
            c->rmPeakHumRat = (sumW_Vdot / sumVdot);
            c->rmPeakRelHum =
                Psychrometrics::PsyRhFnTdbWPb(state, c->rmPeakTemp, c->rmPeakHumRat, state.dataEnvrn->StdBaroPress) * 100.0; // convert to percentage
        } else {
            c->rmPeakTemp = -999.0;
            c->rmPeakHumRat = -999.0;
            c->rmPeakRelHum = -999.0;
        }

        if (c->coilSizingMethodConcurrence == DataSizing::Coincident) {
            c->rmSensibleAtPeak = finalSysSizing.SysCoolCoinSpaceSens;
        } else if (c->coilSizingMethodConcurrence == DataSizing::NonCoincident) {
            c->rmSensibleAtPeak = sumSensLoad;
        } else { // DataSizing::Combination or other
            c->rmSensibleAtPeak = sumSensLoad;
        }

        // now set Coil Ent And Lvg Conditions
        if (curOASysNum > 0) {                 // then this system coil is part of OA system
            if (c->coilDesEntTemp == -999.0) { // don't overwrite if already set directly
                c->coilDesEntTemp = finalSysSizing.OutTempAtCoolPeak;
            }
            if (c->coilDesEntHumRat == -999.0) { // don't overwrite if already set directly
                c->coilDesEntHumRat = finalSysSizing.OutHumRatAtCoolPeak;
            }
            c->coilDesEntWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                state, c->coilDesEntTemp, c->coilDesEntHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::setCoilCoolingCapacity");
            c->coilDesEntEnth = Psychrometrics::PsyHFnTdbW(c->coilDesEntTemp, c->coilDesEntHumRat);
            if (c->coilDesLvgTemp == -999.0) { // don't overwrite if already set directly
                c->coilDesLvgTemp = finalSysSizing.PrecoolTemp;
            }
            if (c->coilDesLvgHumRat == -999.0) { // don't overwrite if already set directly
                c->coilDesLvgHumRat = finalSysSizing.PrecoolHumRat;
            }
            c->coilDesLvgWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                state, c->coilDesLvgTemp, c->coilDesLvgHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::setCoilCoolingCapacity");
            c->coilDesLvgEnth = Psychrometrics::PsyHFnTdbW(c->coilDesLvgTemp, c->coilDesLvgHumRat);

        } else {                               // part of main air loop
            if (c->coilDesEntTemp == -999.0) { // don't overwrite if already set directly
                c->coilDesEntTemp = finalSysSizing.MixTempAtCoolPeak;
            }
            if (c->coilDesEntHumRat == -999.0) { // don't overwrite if already set directly
                c->coilDesEntHumRat = finalSysSizing.MixHumRatAtCoolPeak;
            }
            c->coilDesEntWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                state, c->coilDesEntTemp, c->coilDesEntHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::setCoilCoolingCapacity");
            c->coilDesEntEnth = Psychrometrics::PsyHFnTdbW(c->coilDesEntTemp, c->coilDesEntHumRat);
            if (c->coilDesLvgTemp == -999.0) {
                c->coilDesLvgTemp = finalSysSizing.CoolSupTemp;
            }
            if (c->coilDesLvgHumRat == -999.0) { // don't overwrite if already set directly
                c->coilDesLvgHumRat = finalSysSizing.CoolSupHumRat;
            }
            c->coilDesLvgWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                state, c->coilDesLvgTemp, c->coilDesLvgHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::setCoilCoolingCapacity");
            c->coilDesLvgEnth = Psychrometrics::PsyHFnTdbW(c->coilDesLvgTemp, c->coilDesLvgHumRat);
            if (state.dataAirSystemsData->PrimaryAirSystems(curSysNum).NumOACoolCoils > 0) { // there is precooling of the OA stream
                c->oaPretreated = true;
            }
        }

    } else if (curZoneEqNum > 0 && allocated(state.dataSize->FinalZoneSizing)) {
        c->zoneNum.resize(1);
        c->zoneName.resize(1);
        c->zoneNum[0] = curZoneEqNum;
        if (allocated(state.dataZoneEquip->ZoneEquipConfig)) c->zoneName[0] = state.dataZoneEquip->ZoneEquipConfig(curZoneEqNum).ZoneName;
        auto const &thisFinalZoneSizing = state.dataSize->FinalZoneSizing(curZoneEqNum);
        c->desDayNameAtSensPeak = thisFinalZoneSizing.CoolDesDay;
        c->oaPeakTemp = thisFinalZoneSizing.OutTempAtCoolPeak;
        c->oaPeakHumRat = thisFinalZoneSizing.OutHumRatAtCoolPeak;
        c->raPeakTemp = thisFinalZoneSizing.ZoneTempAtCoolPeak;
        c->raPeakHumRat = thisFinalZoneSizing.ZoneHumRatAtCoolPeak;
        c->rmPeakTemp = thisFinalZoneSizing.ZoneTempAtCoolPeak;
        c->rmPeakHumRat = thisFinalZoneSizing.ZoneHumRatAtCoolPeak;
        c->rmPeakRelHum =
            Psychrometrics::PsyRhFnTdbWPb(state, c->rmPeakTemp, c->rmPeakHumRat, state.dataEnvrn->StdBaroPress) * 100.0; // convert to percentage
        if (thisFinalZoneSizing.CoolDDNum > 0 && thisFinalZoneSizing.CoolDDNum <= state.dataEnvrn->TotDesDays) {
            c->coilSensePeakHrMin = PeakHrMinString(state, thisFinalZoneSizing.CoolDDNum, thisFinalZoneSizing.TimeStepNumAtCoolMax);
            c->airPeakHrMin = PeakHrMinString(state, thisFinalZoneSizing.CoolDDNum, thisFinalZoneSizing.TimeStepNumAtCoolMax);
        }

        c->rmSensibleAtPeak = thisFinalZoneSizing.DesCoolLoad;

        if (ZoneEqSizing(curZoneEqNum).OAVolFlow > 0.0) {
            c->oaPeakVolFlow = ZoneEqSizing(curZoneEqNum).OAVolFlow;
        } else {
            c->oaPeakVolFlow = 0.0;
        }
        // coil entering conditions depend on the type of zone equipment involved
        // set typeof_Coil integer
        if (state.dataSize->TermUnitIU) { // an unpowered induction terminal unit
            // should be picked up by CoolingWaterDesAirInletHumRatSizing and CoolingWaterDesWaterInletTempSizing
            // c->coilDesEntTemp = DataSizing::FinalZoneSizing( curZoneEqNum ).ZoneTempAtCoolPeak;
            // c->coilDesEntHumRat = DataSizing::FinalZoneSizing( curZoneEqNum ).ZoneHumRatAtCoolPeak;
        } else if (state.dataSize->ZoneEqFanCoil) {
            // should be picked up by CoolingWaterDesAirInletHumRatSizing and CoolingWaterDesWaterInletTempSizing
            // if ( DataSizing::FinalZoneSizing( curZoneEqNum ).DesCoolMassFlow > 0.0 ) {
            //    c->oaPeakVolFrac = min( (DataEnvironment::StdRhoAir * c->oaPeakVolFlow)/DataSizing::FinalZoneSizing( curZoneEqNum
            //).DesCoolMassFlow, 1.0 ); } else {     c->oaPeakVolFrac = 0.0;
            //}
            // c->coilDesEntTemp = c->oaPeakVolFrac * DataSizing::FinalZoneSizing( curZoneEqNum ).OutTempAtCoolPeak + ( 1.0 - c->oaPeakVolFrac ) *
            // DataSizing::FinalZoneSizing( curZoneEqNum ).ZoneTempAtCoolPeak;  c->coilDesEntHumRat =  c->oaPeakVolFrac *
            // DataSizing::FinalZoneSizing( curZoneEqNum ).OutHumRatAtCoolPeak + ( 1.0 - c->oaPeakVolFrac ) * DataSizing::FinalZoneSizing(
            // curZoneEqNum ).ZoneHumRatAtCoolPeak;
        } else if (state.dataSize->ZoneEqDXCoil) {
            if (ZoneEqSizing(curZoneEqNum).OAVolFlow > 0.0) {
                if (c->coilDesEntTemp == -999.0) { // don't overwrite if already set directly by setCoilEntAirTemp
                    c->coilDesEntTemp = thisFinalZoneSizing.DesCoolCoilInTemp;
                }
                if (c->coilDesEntHumRat == -999.0) { // don't overwrite if already set directly by setCoilEntAirHumRat
                    c->coilDesEntHumRat = thisFinalZoneSizing.DesCoolCoilInHumRat;
                }
            } else {
                if (c->coilDesEntTemp == -999.0) { // don't overwrite if already set directly by setCoilEntAirTemp
                    c->coilDesEntTemp = thisFinalZoneSizing.ZoneTempAtCoolPeak;
                }
                if (c->coilDesEntHumRat == -999.0) { // don't overwrite if already set directly by setCoilEntAirHumRat
                    c->coilDesEntHumRat = thisFinalZoneSizing.ZoneHumRatAtCoolPeak;
                }
            }
        } else {
            if (c->coilDesEntTemp == -999.0) { // don't overwrite if already set directly by setCoilEntAirTemp
                c->coilDesEntTemp = thisFinalZoneSizing.DesCoolCoilInTemp;
            }
            if (c->coilDesEntHumRat == -999.0) { // don't overwrite if already set directly by setCoilEntAirHumRat
                c->coilDesEntHumRat = thisFinalZoneSizing.DesCoolCoilInHumRat;
            }
        }

        if (c->coilDesLvgTemp == -999.0) { // don't overwrite if already set directly by setCoilLvgAirTemp
            c->coilDesLvgTemp = thisFinalZoneSizing.CoolDesTemp;
        }
        if (c->coilDesLvgHumRat == -999.0) { // don't overwrite if already set directly by setCoilLvgAirHumRat
            c->coilDesLvgHumRat = thisFinalZoneSizing.CoolDesHumRat;
        }
        c->coilDesLvgWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
            state, c->coilDesLvgTemp, c->coilDesLvgHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::setCoilCoolingCapacity");
        c->coilDesLvgEnth = Psychrometrics::PsyHFnTdbW(c->coilDesLvgTemp, c->coilDesLvgHumRat);
    } else if (curOASysNum > 0 && c->airloopNum > state.dataHVACGlobal->NumPrimaryAirSys) {
        if (!state.dataAirLoopHVACDOAS->airloopDOAS.empty()) {
            int DOASSysNum = state.dataAirLoop->OutsideAirSys(curOASysNum).AirLoopDOASNum;
            c->coilDesEntTemp = state.dataAirLoopHVACDOAS->airloopDOAS[DOASSysNum].SizingCoolOATemp;
            c->coilDesEntHumRat = state.dataAirLoopHVACDOAS->airloopDOAS[DOASSysNum].SizingCoolOAHumRat;
            if (c->coilDesEntTemp > -999.0 && c->coilDesEntHumRat > -999.0) {
                c->coilDesEntWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                    state, c->coilDesEntTemp, c->coilDesEntHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::setCoilCoolingCapacity");
                c->coilDesEntEnth = Psychrometrics::PsyHFnTdbW(c->coilDesEntTemp, c->coilDesEntHumRat);
            }
            c->coilDesLvgTemp = state.dataAirLoopHVACDOAS->airloopDOAS[DOASSysNum].PrecoolTemp;
            c->coilDesLvgHumRat = state.dataAirLoopHVACDOAS->airloopDOAS[DOASSysNum].PrecoolHumRat;
            if (c->coilDesLvgTemp > -999.0 && c->coilDesLvgHumRat > -999.0) {
                c->coilDesLvgWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                    state, c->coilDesLvgTemp, c->coilDesLvgHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::setCoilCoolingCapacity");
                c->coilDesLvgEnth = Psychrometrics::PsyHFnTdbW(c->coilDesLvgTemp, c->coilDesLvgHumRat);
            }
            int sizMethod = 0;
            bool sizMethodsAreTheSame = true;
            for (int airLoopNum = 0; airLoopNum < state.dataAirLoopHVACDOAS->airloopDOAS[DOASSysNum].NumOfAirLoops; ++airLoopNum) {
                int actualAirLoopNum = state.dataAirLoopHVACDOAS->airloopDOAS[DOASSysNum].m_AirLoopNum[airLoopNum];
                if (airLoopNum == 0) {
                    sizMethod = state.dataSize->FinalSysSizing(actualAirLoopNum).SizingOption;
                } else {
                    if (sizMethod != state.dataSize->FinalSysSizing(actualAirLoopNum).SizingOption) {
                        sizMethodsAreTheSame = false;
                    }
                }
            }
            if (sizMethodsAreTheSame) {
                c->coilSizingMethodConcurrence = sizMethod;
            } else {
                c->coilSizingMethodConcurrence = DataSizing::Combination;
            }
        }
    } else {
        // do nothing
    }

    // calc sensible capacity from inlet outlet
    c->cpMoistAir = Psychrometrics::PsyCpAirFnW(c->coilDesEntHumRat);
}

void ReportCoilSelection::setCoilHeatingCapacity(
    EnergyPlusData &state,
    std::string const &coilName,       // user-defined name of the coil
    std::string const &coilType,       // idf input object class name of coil
    Real64 const totalHeatingCap,      // {W} coil Heating capacity
    bool const isAutoSize,             // true if value was autosized
    int const curSysNum,               // airloop system number index, if non zero
    int const curZoneEqNum,            // zone equipment list index, if non-zero
    int const curOASysNum,             // OA system equipment list index, if non-zero
    Real64 const fanHeatGain,          // {W} fan load used in ideal loads coil sizing
    Real64 const coilCapFunTempFac,    // {W} curve result for modification factor for capacity as a function of temperature
    Real64 const DXFlowPerCapMinRatio, // non dimensional ratio, capacity adjustment ratio min
    Real64 const DXFlowPerCapMaxRatio  // non dimensional ratio, capacity adjustment ratio max
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->capIsAutosized = isAutoSize;
    c->coilCapFTIdealPeak = coilCapFunTempFac;
    c->coilTotCapAtPeak = totalHeatingCap * c->coilCapFTIdealPeak;
    c->minRatio = DXFlowPerCapMinRatio;
    c->maxRatio = DXFlowPerCapMaxRatio;

    c->fanHeatGainIdealPeak = fanHeatGain;
    c->airloopNum = curSysNum;
    doAirLoopSetup(state, index);
    c->zoneEqNum = curZoneEqNum;
    //    if ( c->zoneEqNum > 0 ) doZoneEqSetup( index );
    if (curSysNum > 0 && c->zoneEqNum == 0 && curSysNum <= int(state.dataSize->FinalSysSizing.size())) {
        auto &finalSysSizing = state.dataSize->FinalSysSizing(curSysNum);
        c->desDayNameAtSensPeak = finalSysSizing.HeatDesDay;

        c->oaPeakTemp = finalSysSizing.HeatOutTemp;
        c->oaPeakHumRat = finalSysSizing.HeatOutHumRat;
        c->oaPeakVolFlow = finalSysSizing.DesOutAirVolFlow;
        c->raPeakTemp = finalSysSizing.HeatRetTemp;
        c->raPeakHumRat = finalSysSizing.HeatRetHumRat;
        c->coilSizingMethodConcurrence = finalSysSizing.SizingOption;
        c->coilSizingMethodCapacity = finalSysSizing.HeatingCapMethod;
        c->coilSizingMethodAirFlow = finalSysSizing.ScaleHeatSAFMethod;

        // Central Heating Coils are always sized at the conditions at the peak Sensible Heating Load
        c->isCoilSizingForTotalLoad = false;

        // DesOutAirVolFlow

        // loop over heated zones attached to this airloop to find average Room condition, if none heated use cooled zones
        // weighted average by zone supply air volume flow rate for all the zones on this coil's air system
        Real64 sumT_Vdot(0.0); // numerator for average zone temperature, zone temperature values times zone air volume
        Real64 sumW_Vdot(0.0); // numerator average zone humidity ratio, zone hum rat value times zone air volume
        Real64 sumLoad(0.0);   // straight total for zone design loads
        Real64 sumVdot(0.0);   // denominator for zone-volume weighted averages

        int SysPeakDDnum = finalSysSizing.HeatDDNum;
        int SysPeakTimeStepInDay = finalSysSizing.SysHeatCoilTimeStepPk;
        if (SysPeakDDnum > 0 && SysPeakTimeStepInDay > 0) { // may be zero if no peak found because of zero system load
            for (auto &z : c->zoneNum) {
                auto const &thisCalcZoneSizing = state.dataSize->CalcZoneSizing(SysPeakDDnum, z);
                auto const &thisFinalZoneSizing = state.dataSize->FinalZoneSizing(z);
                Real64 mult = state.dataHeatBal->Zone(z).Multiplier * state.dataHeatBal->Zone(z).ListMultiplier;
                Real64 Tz = thisCalcZoneSizing.HeatZoneTempSeq(SysPeakTimeStepInDay);
                Real64 Vdot_z = thisCalcZoneSizing.HeatFlowSeq(SysPeakTimeStepInDay);
                if (Vdot_z == 0.0) { // take value from final zone sizing
                    Vdot_z = thisFinalZoneSizing.HeatMassFlow;
                    if (Vdot_z == 0.0) {
                        Vdot_z = finalSysSizing.DesHeatVolFlow * state.dataEnvrn->StdRhoAir / c->zoneNum.size();
                    }
                }
                Real64 Wz = thisCalcZoneSizing.HeatZoneHumRatSeq(SysPeakTimeStepInDay);
                sumT_Vdot += Tz * Vdot_z * mult;
                sumW_Vdot += Wz * Vdot_z * mult;
                sumVdot += Vdot_z * mult;
                Real64 Qdot_z = thisCalcZoneSizing.HeatLoadSeq(SysPeakTimeStepInDay);
                if (Qdot_z > 0.0) {
                    sumLoad += Qdot_z * mult;
                } else {
                    sumLoad += thisFinalZoneSizing.DesHeatLoad * mult;
                }
            }
        }

        if (c->zoneNum.size() > 0 && sumVdot > 0.0) {
            c->rmPeakTemp = (sumT_Vdot / sumVdot);
            c->rmPeakHumRat = (sumW_Vdot / sumVdot);
            c->rmPeakRelHum =
                Psychrometrics::PsyRhFnTdbWPb(state, c->rmPeakTemp, c->rmPeakHumRat, state.dataEnvrn->StdBaroPress) * 100.0; // convert to percentage
        } else {
            c->rmPeakTemp = -999.0;
            c->rmPeakHumRat = -999.0;
            c->rmPeakRelHum = -999.0;
        }

        if (c->coilSizingMethodConcurrence == DataSizing::Coincident) {
            c->rmSensibleAtPeak = finalSysSizing.SysHeatCoinSpaceSens;
        } else if (c->coilSizingMethodConcurrence == DataSizing::NonCoincident) {
            c->rmSensibleAtPeak = sumLoad;
        }

        if (finalSysSizing.HeatDDNum > 0 && finalSysSizing.HeatDDNum <= state.dataEnvrn->TotDesDays) {
            c->coilSensePeakHrMin = PeakHrMinString(state, finalSysSizing.HeatDDNum, finalSysSizing.SysHeatCoilTimeStepPk);

            c->airPeakHrMin = PeakHrMinString(state, finalSysSizing.HeatDDNum, finalSysSizing.SysHeatAirTimeStepPk);

            c->desDayNameAtAirFlowPeak = state.dataWeather->DesDayInput(finalSysSizing.HeatDDNum).Title;
        }

        // now set Coil Ent And Lvg Conditions

        if (curOASysNum > 0) { // then this system coil is part of OA system
            if (c->coilDesEntTemp == -999.0) c->coilDesEntTemp = finalSysSizing.HeatOutTemp;
            if (c->coilDesEntHumRat == -999.0) c->coilDesEntHumRat = finalSysSizing.HeatOutHumRat;
            c->coilDesEntWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                state, c->coilDesEntTemp, c->coilDesEntHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::setCoilHeatingCapacity");
            c->coilDesEntEnth = Psychrometrics::PsyHFnTdbW(c->coilDesEntTemp, c->coilDesEntHumRat);
            if (c->coilDesLvgTemp == -999.0) c->coilDesLvgTemp = finalSysSizing.PreheatTemp;
            if (c->coilDesLvgHumRat == -999.0) c->coilDesLvgHumRat = finalSysSizing.PreheatHumRat;
            c->coilDesLvgWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                state, c->coilDesLvgTemp, c->coilDesLvgHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::setCoilHeatingCapacity");
            c->coilDesLvgEnth = Psychrometrics::PsyHFnTdbW(c->coilDesLvgTemp, c->coilDesLvgHumRat);

        } else { // part of main air loop
            if (c->coilDesEntTemp == -999.0) c->coilDesEntTemp = finalSysSizing.HeatMixTemp;
            if (c->coilDesEntHumRat == -999.0) c->coilDesEntHumRat = finalSysSizing.HeatMixHumRat;
            c->coilDesEntWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                state, c->coilDesEntTemp, c->coilDesEntHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::setCoilHeatingCapacity");
            c->coilDesEntEnth = Psychrometrics::PsyHFnTdbW(c->coilDesEntTemp, c->coilDesEntHumRat);
            if (c->coilDesLvgTemp == -999.0) c->coilDesLvgTemp = finalSysSizing.HeatSupTemp;
            if (c->coilDesLvgHumRat == -999.0) c->coilDesLvgHumRat = finalSysSizing.HeatSupHumRat;
            c->coilDesLvgWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                state, c->coilDesLvgTemp, c->coilDesLvgHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::setCoilHeatingCapacity");
            c->coilDesLvgEnth = Psychrometrics::PsyHFnTdbW(c->coilDesLvgTemp, c->coilDesLvgHumRat);
            if (state.dataAirSystemsData->PrimaryAirSystems(curSysNum).NumOAHeatCoils > 0) { // there is preHeating of the OA stream
                c->oaPretreated = true;
            }
        }

    } else if (curZoneEqNum > 0 && allocated(state.dataSize->FinalZoneSizing)) {
        auto &finalZoneSizing = state.dataSize->FinalZoneSizing(curZoneEqNum);
        c->zoneNum.resize(1);
        c->zoneName.resize(1);
        c->zoneNum[0] = curZoneEqNum;
        if (allocated(state.dataZoneEquip->ZoneEquipConfig)) c->zoneName[0] = state.dataZoneEquip->ZoneEquipConfig(curZoneEqNum).ZoneName;
        c->desDayNameAtSensPeak = finalZoneSizing.HeatDesDay;
        c->oaPeakTemp = finalZoneSizing.OutTempAtHeatPeak;
        c->oaPeakHumRat = finalZoneSizing.OutHumRatAtHeatPeak;
        c->raPeakTemp = finalZoneSizing.ZoneRetTempAtHeatPeak;
        c->raPeakHumRat = finalZoneSizing.ZoneHumRatAtHeatPeak;
        c->rmPeakTemp = finalZoneSizing.ZoneTempAtHeatPeak;
        c->rmPeakHumRat = finalZoneSizing.ZoneHumRatAtHeatPeak;
        c->rmPeakRelHum =
            Psychrometrics::PsyRhFnTdbWPb(state, c->rmPeakTemp, c->rmPeakHumRat, state.dataEnvrn->StdBaroPress) * 100.0; // convert to percentage
        if (finalZoneSizing.HeatDDNum > 0 && finalZoneSizing.HeatDDNum <= state.dataEnvrn->TotDesDays) {
            c->coilSensePeakHrMin = PeakHrMinString(state, finalZoneSizing.HeatDDNum, finalZoneSizing.TimeStepNumAtHeatMax);
            c->airPeakHrMin = PeakHrMinString(state, finalZoneSizing.HeatDDNum, finalZoneSizing.TimeStepNumAtHeatMax);
        }
        c->desDayNameAtAirFlowPeak = finalZoneSizing.HeatDesDay;

        c->rmSensibleAtPeak = finalZoneSizing.DesHeatLoad;

        auto &zoneEqSizing = state.dataSize->ZoneEqSizing(curZoneEqNum);
        if (zoneEqSizing.OAVolFlow > 0.0) {
            c->oaPeakVolFlow = zoneEqSizing.OAVolFlow;
        } else if (zoneEqSizing.ATMixerVolFlow > 0.0) {
            c->oaPeakVolFlow = zoneEqSizing.ATMixerVolFlow;
        } else {
            c->oaPeakVolFlow = 0.0;
        }
        // coil entering conditions depend on the type of zone equipment involved
        // set typeof_Coil integer
        if (state.dataSize->TermUnitIU) {      // an unpowered induction terminal unit
            if (c->coilDesEntTemp == -999.0) { // don't overwrite if already set directly by setCoilEntAirTemp
                auto const &thisTermUnitFinalZoneSizing = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum);
                c->coilDesEntTemp = thisTermUnitFinalZoneSizing.DesHeatCoilInTempTU;
                c->coilDesEntHumRat = thisTermUnitFinalZoneSizing.DesHeatCoilInHumRatTU;
            }
        } else if (state.dataSize->TermUnitSingDuct) {
            if (c->coilDesEntTemp == -999.0) { // don't overwrite if already set directly by setCoilEntAirTemp
                auto const &thisTermUnitFinalZoneSizing = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum);
                c->coilDesEntTemp = thisTermUnitFinalZoneSizing.DesHeatCoilInTempTU;
                c->coilDesEntHumRat = thisTermUnitFinalZoneSizing.DesHeatCoilInHumRatTU;
            }
        } else if (state.dataSize->TermUnitPIU) {
            auto const &thisTermUnitSizing = state.dataSize->TermUnitSizing(state.dataSize->CurTermUnitSizingNum);
            Real64 MinPriFlowFrac = thisTermUnitSizing.MinFlowFrac;
            if (thisTermUnitSizing.InducesPlenumAir) {
                if (c->coilDesEntTemp == -999.0) { // don't overwrite if already set directly by setCoilEntAirTemp
                    auto const &termUnitFinalZoneSizing = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum);
                    c->coilDesEntTemp = (termUnitFinalZoneSizing.DesHeatCoilInTempTU * MinPriFlowFrac) +
                                        (termUnitFinalZoneSizing.ZoneRetTempAtHeatPeak * (1.0 - MinPriFlowFrac));
                    c->coilDesEntHumRat = (termUnitFinalZoneSizing.DesHeatCoilInHumRatTU * MinPriFlowFrac) +
                                          (termUnitFinalZoneSizing.ZoneHumRatAtHeatPeak * (1.0 - MinPriFlowFrac));
                }
            } else {
                if (c->coilDesEntTemp == -999.0) { // don't overwrite if already set directly by setCoilEntAirTemp
                    auto const &thisTermUnitFinalZoneSizing = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum);
                    c->coilDesEntTemp = (thisTermUnitFinalZoneSizing.DesHeatCoilInTempTU * MinPriFlowFrac) +
                                        (thisTermUnitFinalZoneSizing.ZoneTempAtHeatPeak * (1.0 - MinPriFlowFrac));
                    c->coilDesEntHumRat = (thisTermUnitFinalZoneSizing.DesHeatCoilInHumRatTU * MinPriFlowFrac) +
                                          (thisTermUnitFinalZoneSizing.ZoneHumRatAtHeatPeak * (1.0 - MinPriFlowFrac));
                }
            }
        } else if (state.dataSize->ZoneEqFanCoil) {
            if (c->coilDesEntTemp == -999.0) { // don't overwrite if already set directly by setCoilEntAirTemp
                Real64 desOAFlowFrac = 0.0;
                if (zoneEqSizing.OAVolFlow > 0.0 && finalZoneSizing.DesHeatMassFlow > 0.0) {
                    desOAFlowFrac = std::min(state.dataEnvrn->StdRhoAir * zoneEqSizing.OAVolFlow / finalZoneSizing.DesHeatMassFlow, 1.0);
                } else {
                    desOAFlowFrac = finalZoneSizing.DesHeatOAFlowFrac;
                }
                c->coilDesEntTemp = desOAFlowFrac * finalZoneSizing.OutTempAtHeatPeak + (1.0 - desOAFlowFrac) * finalZoneSizing.ZoneTempAtHeatPeak;
                c->coilDesEntHumRat =
                    desOAFlowFrac * finalZoneSizing.OutHumRatAtHeatPeak + (1.0 - desOAFlowFrac) * finalZoneSizing.ZoneHumRatAtHeatPeak;
            }
        } else if (state.dataSize->ZoneEqDXCoil) {
            if (zoneEqSizing.OAVolFlow > 0.0) {
                if (c->coilDesEntTemp == -999.0) { // don't overwrite if already set directly by setCoilEntAirTemp
                    c->coilDesEntTemp = finalZoneSizing.DesHeatCoilInTemp;
                }
                if (c->coilDesEntHumRat == -999.0) { // don't overwrite if already set directly by setCoilEntAirHumRat
                    c->coilDesEntHumRat = finalZoneSizing.DesHeatCoilInHumRat;
                }
            } else {
                if (c->coilDesEntTemp == -999.0) { // don't overwrite if already set directly by setCoilEntAirTemp
                    c->coilDesEntTemp = finalZoneSizing.ZoneTempAtHeatPeak;
                }
                if (c->coilDesEntHumRat == -999.0) { // don't overwrite if already set directly by setCoilEntAirHumRat
                    c->coilDesEntHumRat = finalZoneSizing.ZoneHumRatAtHeatPeak;
                }
            }
        } else {
            if (c->coilDesEntTemp == -999.0) { // don't overwrite if already set directly by setCoilEntAirTemp
                c->coilDesEntTemp = finalZoneSizing.DesHeatCoilInTemp;
            }
            if (c->coilDesEntHumRat == -999.0) { // don't overwrite if already set directly by setCoilEntAirHumRat
                c->coilDesEntHumRat = finalZoneSizing.DesHeatCoilInHumRat;
            }
        }

        if (c->coilDesEntTemp > -999.0 && c->coilDesEntHumRat > -999.0) {
            c->coilDesEntWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                state, c->coilDesEntTemp, c->coilDesEntHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::setCoilHeatingCapacity");
            c->coilDesEntEnth = Psychrometrics::PsyHFnTdbW(c->coilDesEntTemp, c->coilDesEntHumRat);
        }

        if (c->coilDesLvgTemp == -999.0) { // don't overwrite if already set directly by setCoilLvgAirTemp
            c->coilDesLvgTemp = finalZoneSizing.HeatDesTemp;
        }
        if (c->coilDesLvgHumRat == -999.0) { // don't overwrite if already set directly by setCoilLvgAirHumRat
            c->coilDesLvgHumRat = finalZoneSizing.HeatDesHumRat;
        }
        c->coilDesLvgWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
            state, c->coilDesLvgTemp, c->coilDesLvgHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::setCoilHeatingCapacity");
        c->coilDesLvgEnth = Psychrometrics::PsyHFnTdbW(c->coilDesLvgTemp, c->coilDesLvgHumRat);
    } else if (curOASysNum > 0 && c->airloopNum > int(state.dataSize->FinalSysSizing.size())) {
        if (!state.dataAirLoopHVACDOAS->airloopDOAS.empty()) {
            c->oASysNum = curOASysNum; // where should this get set? It's -999 here.
            int DOASSysNum = state.dataAirLoop->OutsideAirSys(curOASysNum).AirLoopDOASNum;
            c->coilDesEntTemp = state.dataAirLoopHVACDOAS->airloopDOAS[DOASSysNum].HeatOutTemp;
            c->coilDesEntHumRat = state.dataAirLoopHVACDOAS->airloopDOAS[DOASSysNum].HeatOutHumRat;
            if (c->coilDesEntTemp > -999.0 && c->coilDesEntHumRat > -999.0) {
                c->coilDesEntWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                    state, c->coilDesEntTemp, c->coilDesEntHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::setCoilHeatingCapacity");
                c->coilDesEntEnth = Psychrometrics::PsyHFnTdbW(c->coilDesEntTemp, c->coilDesEntHumRat);
            }
            c->coilDesLvgTemp = state.dataAirLoopHVACDOAS->airloopDOAS[DOASSysNum].PreheatTemp;
            c->coilDesLvgHumRat = state.dataAirLoopHVACDOAS->airloopDOAS[DOASSysNum].PreheatHumRat;
            if (c->coilDesLvgTemp > -999.0 && c->coilDesLvgHumRat > -999.0) {
                c->coilDesLvgWetBulb = Psychrometrics::PsyTwbFnTdbWPb(
                    state, c->coilDesLvgTemp, c->coilDesLvgHumRat, state.dataEnvrn->StdBaroPress, "ReportCoilSelection::setCoilHeatingCapacity");
                c->coilDesLvgEnth = Psychrometrics::PsyHFnTdbW(c->coilDesLvgTemp, c->coilDesLvgHumRat);
            }
            int sizMethod = 0;
            bool sizMethodsAreTheSame = true;
            for (int airLoopNum = 0; airLoopNum < state.dataAirLoopHVACDOAS->airloopDOAS[DOASSysNum].NumOfAirLoops; ++airLoopNum) {
                int actualAirLoopNum = state.dataAirLoopHVACDOAS->airloopDOAS[DOASSysNum].m_AirLoopNum[airLoopNum];
                if (airLoopNum == 0) {
                    sizMethod = state.dataSize->FinalSysSizing(actualAirLoopNum).SizingOption;
                } else {
                    if (sizMethod != state.dataSize->FinalSysSizing(actualAirLoopNum).SizingOption) {
                        sizMethodsAreTheSame = false;
                    }
                }
            }
            if (sizMethodsAreTheSame) {
                c->coilSizingMethodConcurrence = sizMethod;
            } else {
                c->coilSizingMethodConcurrence = DataSizing::Combination;
            }
        }
    } else {
        // do nothing
    }

    if (state.dataSize->DataCoilIsSuppHeater) {
        c->isSupplementalHeater = true;
    }
    // some heating coils only use this routine, so set air flow if not yet set
    if (c->coilDesVolFlow <= 0.0) {
        if (state.dataSize->DataFlowUsedForSizing > 0.0) { // flow has been set in global, so use it
            c->coilDesVolFlow = state.dataSize->DataFlowUsedForSizing;
        } else if (curZoneEqNum > 0 && allocated(state.dataSize->FinalZoneSizing)) {
            auto &finalZoneSizing = state.dataSize->FinalZoneSizing(curZoneEqNum);
            if (finalZoneSizing.DesHeatMassFlow >= DataHVACGlobals::SmallMassFlow) {
                c->coilDesMassFlow = finalZoneSizing.DesHeatMassFlow;
                c->coilDesVolFlow = c->coilDesMassFlow / state.dataEnvrn->StdRhoAir;
            }
        } else if (curSysNum > 0 && curSysNum <= int(state.dataSize->FinalSysSizing.size())) {
            auto &finalSysSizing = state.dataSize->FinalSysSizing(curSysNum);
            if (curOASysNum > 0 && allocated(state.dataSize->OASysEqSizing)) {
                auto &oASysEqSizing = state.dataSize->OASysEqSizing(curSysNum);
                if (oASysEqSizing.AirFlow) {
                    c->coilDesVolFlow = oASysEqSizing.AirVolFlow;
                } else if (oASysEqSizing.HeatingAirFlow) {
                    c->coilDesVolFlow = oASysEqSizing.HeatingAirVolFlow;
                } else {
                    c->coilDesVolFlow = finalSysSizing.DesOutAirVolFlow;
                }
            } else {
                if (state.dataSize->DataFlowUsedForSizing > 0.0) {
                    c->coilDesVolFlow = state.dataSize->DataFlowUsedForSizing;
                } else if (curSysNum > 0 && allocated(state.dataSize->UnitarySysEqSizing)) {
                    auto &unitarySysEqSizing = state.dataSize->UnitarySysEqSizing(curSysNum);
                    if (unitarySysEqSizing.AirFlow) {
                        c->coilDesVolFlow = unitarySysEqSizing.AirVolFlow;
                    } else if (unitarySysEqSizing.HeatingAirFlow) {
                        c->coilDesVolFlow = unitarySysEqSizing.HeatingAirVolFlow;
                    }
                } else {
                    if (state.dataSize->CurDuctType == DataHVACGlobals::AirDuctType::Main) {
                        if (finalSysSizing.SysAirMinFlowRat > 0.0 && !state.dataSize->DataDesicRegCoil) {
                            c->coilDesVolFlow = finalSysSizing.SysAirMinFlowRat * finalSysSizing.DesMainVolFlow;
                        } else {
                            c->coilDesVolFlow = finalSysSizing.DesMainVolFlow;
                        }
                    } else if (state.dataSize->CurDuctType == DataHVACGlobals::AirDuctType::Cooling) {
                        if (finalSysSizing.SysAirMinFlowRat > 0.0 && !state.dataSize->DataDesicRegCoil) {
                            c->coilDesVolFlow = finalSysSizing.SysAirMinFlowRat * finalSysSizing.DesCoolVolFlow;
                        } else {
                            c->coilDesVolFlow = finalSysSizing.DesCoolVolFlow;
                        }
                    } else if (state.dataSize->CurDuctType == DataHVACGlobals::AirDuctType::Heating) {
                        c->coilDesVolFlow = finalSysSizing.DesHeatVolFlow;
                    } else if (state.dataSize->CurDuctType == DataHVACGlobals::AirDuctType::Other) {
                        c->coilDesVolFlow = finalSysSizing.DesMainVolFlow;
                    } else {
                        c->coilDesVolFlow = finalSysSizing.DesMainVolFlow;
                    }
                }
            }
        }
        c->coilDesMassFlow = c->coilDesVolFlow * state.dataEnvrn->StdRhoAir;
    }

    // calc sensible capacity from inlet outlet
    c->cpMoistAir = Psychrometrics::PsyCpAirFnW(c->coilDesLvgHumRat);
    // this is not generally correct but okay for heating coils
    c->coilSensCapAtPeak = std::abs(c->cpMoistAir * c->coilDesMassFlow * (c->coilDesLvgTemp - c->coilDesEntTemp));
    c->coilSensCapAtPeak = min(c->coilSensCapAtPeak, c->coilTotCapAtPeak);
}

void ReportCoilSelection::setCoilWaterCoolingCapacity(EnergyPlusData &state,
                                                      std::string const &coilName,  // user-defined name of the coil
                                                      std::string const &coilType,  // idf input object class name of coil
                                                      Real64 const totalCoolingCap, // {W} coil cooling capacity
                                                      bool const isAutoSize,        // true if value was autosized
                                                      int const inletNodeNum,       // coil chw inlet node num
                                                      int const outletNodeNum,      // coil chw outlet node num
                                                      int const dataWaterLoopNum    // plant loop structure index
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->coilTotCapAtPeak = totalCoolingCap;
    c->capIsAutosized = isAutoSize;
    if ((state.dataSize->NumPltSizInput > 0) && (inletNodeNum > 0) && (outletNodeNum > 0)) {
        bool errorsfound = false;
        c->pltSizNum = PlantUtilities::MyPlantSizingIndex(state, "chilled water coil", coilName, inletNodeNum, outletNodeNum, errorsfound);
    } else {
        c->pltSizNum = -999;
    }
    c->waterLoopNum = dataWaterLoopNum;
}

void ReportCoilSelection::setCoilWaterHeaterCapacityNodeNums(EnergyPlusData &state,
                                                             std::string const &coilName,  // user-defined name of the coil
                                                             std::string const &coilType,  // idf input object class name of coil
                                                             Real64 const totalHeatingCap, // {W} coil Heating capacity
                                                             bool const isAutoSize,        // true if value was autosized
                                                             int const inletNodeNum,       // coil chw inlet node num
                                                             int const outletNodeNum,      // coil chw outlet node num
                                                             int const dataWaterLoopNum    // plant loop structure index
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->coilTotCapAtPeak = totalHeatingCap;
    c->capIsAutosized = isAutoSize;
    if ((state.dataSize->NumPltSizInput > 0) && (inletNodeNum > 0) && (outletNodeNum > 0)) {
        bool errorsfound = false;
        c->pltSizNum = PlantUtilities::MyPlantSizingIndex(state, "hot water coil", coilName, inletNodeNum, outletNodeNum, errorsfound);
    } else {
        c->pltSizNum = -999;
    }
    c->waterLoopNum = dataWaterLoopNum;
}

void ReportCoilSelection::setCoilWaterHeaterCapacityPltSizNum(EnergyPlusData &state,
                                                              std::string const &coilName,  // user-defined name of the coil
                                                              std::string const &coilType,  // idf input object class name of coil
                                                              Real64 const totalHeatingCap, // {W} coil Heating capacity
                                                              bool const isAutoSize,        // true if value was autosized
                                                              int const dataPltSizNum,      // plant sizing structure index
                                                              int const dataWaterLoopNum    // plant loop structure index
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->coilTotCapAtPeak = totalHeatingCap;
    c->capIsAutosized = isAutoSize;
    c->pltSizNum = dataPltSizNum;
    c->waterLoopNum = dataWaterLoopNum;
}

void ReportCoilSelection::setCoilUA(EnergyPlusData &state,
                                    std::string const &coilName,            // user-defined name of the coil
                                    std::string const &coilType,            // idf input object class name of coil
                                    Real64 const UAvalue,                   // [W/k] UA value for coil,
                                    Real64 const dataCapacityUsedForSizing, // [W] sizing global
                                    bool const isAutoSize,                  // true if value was autosized
                                    int const curSysNum,                    // airloop system number index, if non zero
                                    int const curZoneEqNum                  // zone equipment list index, if non-zero
)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->coilUA = UAvalue;
    c->coilTotCapAtPeak = dataCapacityUsedForSizing;
    c->capIsAutosized = isAutoSize;
    c->airloopNum = curSysNum;
    doAirLoopSetup(state, index);
    c->zoneEqNum = curZoneEqNum;
}

void ReportCoilSelection::setCoilReheatMultiplier(EnergyPlusData &state,
                                                  std::string const &coilName, // user-defined name of the coil
                                                  std::string const &coilType, // idf input object class name of coil
                                                  Real64 const multiplierReheatLoad)
{
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->reheatLoadMult = multiplierReheatLoad;
}

void ReportCoilSelection::setCoilSupplyFanInfo(EnergyPlusData &state,
                                               std::string const &coilName, // user-defined name of the coil
                                               std::string const &coilType, // idf input object class name of coil
                                               std::string const &fanName,
                                               DataAirSystems::FanModelType fanEnumType,
                                               int fanIndex)
{
    if (fanName.empty()) {
        return;
    }
    int index = getIndexForOrCreateDataObjFromCoilName(state, coilName, coilType);
    auto &c(coilSelectionDataObjs[index]);
    c->fanAssociatedWithCoilName = fanName;
    c->supFanModelType = fanEnumType;
    int locFanIndex(-1);
    if (fanEnumType == DataAirSystems::StructArrayLegacyFanModels) {
        if (fanIndex <= 0) {
            bool errorsFound(false);
            Fans::GetFanIndex(state, fanName, locFanIndex, errorsFound);
        } else {
            locFanIndex = fanIndex;
        }
        c->supFanNum = locFanIndex;
    } else if (fanEnumType == DataAirSystems::ObjectVectorOOFanSystemModel) {
        if (fanIndex < 0) {
            locFanIndex = HVACFan::getFanObjectVectorIndex(state, fanName);
        } else {
            locFanIndex = fanIndex;
        }
        c->supFanVecIndex = locFanIndex;
    }
}

std::string ReportCoilSelection::getTimeText(EnergyPlusData &state, int const timeStepAtPeak)
{
    std::string returnString = "";

    if (timeStepAtPeak == 0) {
        return returnString;
    }

    // Scan timesteps of 24-hr day to match up correct hour and minute
    int minutes(0);
    int timeStepIndex(0);
    int hourPrint;
    for (int hourCounter = 1; hourCounter <= 24; ++hourCounter) {
        for (int timeStepCounter = 1; timeStepCounter <= state.dataGlobal->NumOfTimeStepInHour; ++timeStepCounter) {
            ++timeStepIndex;
            minutes += state.dataGlobal->MinutesPerTimeStep;
            if (minutes == 60) {
                minutes = 0;
                hourPrint = hourCounter;
            } else {
                hourPrint = hourCounter - 1;
            }
            if (timeStepIndex == timeStepAtPeak) {
                returnString = format(DataSizing::PeakHrMinFmt, hourPrint, minutes);
            }
        }
    }

    return returnString;
}

bool ReportCoilSelection::isCompTypeFan(std::string const &compType // string component type, input object class name
)
{
    // if compType name is one of the fan objects, then return true
    if (Util::SameString(compType, "Fan:SystemModel")) {
        return true;
    } else if (Util::SameString(compType, "Fan:ComponentModel")) {
        return true;
    } else if (Util::SameString(compType, "Fan:OnOff")) {
        return true;
    } else if (Util::SameString(compType, "Fan:ConstantVolume")) {
        return true;
    } else if (Util::SameString(compType, "Fan:VariableVolume")) {
        return true;
    } else {
        return false;
    }
}

bool ReportCoilSelection::isCompTypeCoil(std::string const &compType // string component type, input object class name
)
{
    // if compType name is one of the coil objects, then return true
    bool found(false);
    for (int loop = 1; loop <= DataHVACGlobals::NumAllCoilTypes; ++loop) {
        if (Util::SameString(compType, DataHVACGlobals::cAllCoilTypes(loop))) {
            found = true;
            break;
        }
    }
    return found;
}

void ReportCoilSelection::setZoneLatentLoadCoolingIdealPeak(int const zoneIndex, Real64 const zoneCoolingLatentLoad)
{
    // loop over all the coils and the zones in the coils and if this zone index is in the coil
    for (auto &c : coilSelectionDataObjs) {

        if (c->isCooling) {
            for (std::size_t zoneInd = 0; zoneInd < c->zoneNum.size(); ++zoneInd) {
                if (zoneIndex == c->zoneNum[zoneInd]) {
                    c->rmLatentAtPeak += zoneCoolingLatentLoad;
                    break;
                }
            }
        }
    }
}

void ReportCoilSelection::setZoneLatentLoadHeatingIdealPeak(int const zoneIndex, Real64 const zoneHeatingLatentLoad)
{
    // loop over all the coils and the zones in the coils and if this zone index is in the coil
    for (auto &c : coilSelectionDataObjs) {

        if (c->isHeating) {
            for (std::size_t zoneInd = 0; zoneInd < c->zoneNum.size(); ++zoneInd) {
                if (zoneIndex == c->zoneNum[zoneInd]) {
                    c->rmLatentAtPeak += zoneHeatingLatentLoad;
                    break;
                }
            }
        }
    }
}

} // namespace EnergyPlus
