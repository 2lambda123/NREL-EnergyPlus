// EnergyPlus, Copyright (c) 1996-2021, The Board of Trustees of the University of Illinois,
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

#ifndef PlantOperationEnums_hh_INCLUDED
#define PlantOperationEnums_hh_INCLUDED

namespace EnergyPlus::DataPlant {

// Parameters for loop flow request priority,
//     used in logic to deal with Node%MassFlowRequest for determining overall loop flow rate
int const LoopFlowStatus_Unknown(21);             // component's status is not yet set
int const LoopFlowStatus_NeedyAndTurnsLoopOn(22); // component is a "winner" for loop flow requests
// active valve inside component that modulates flow
//  gets the loop going under most conditions
int const LoopFlowStatus_NeedyIfLoopOn(23); // component is a "winner" for loop flow requests
// but doesn't normally get the loop going to start with
//  once loop is going, may increase needs, non-zero minimums
int const LoopFlowStatus_TakesWhatGets(24); // component is a "loser" for loop flow requests,
// but if the loop is on it
// it does make flow requests (for s/m resolution)

// Parameters for scheme types
// Used in TYPE(OperationData)%OpSchemeType
// As in PlantLoop(:)%OpScheme(:)%OpSchemeType
// Also in PlantLoop()LoopSide()Branch()Comp()%CurOpSchemeType
// this may be changed later...
enum OpSchemeType
{ // Changed to enum: Better semantic fit and allows use in switch statements: Suggest this migration throughout EnergyPlus (and probably C++11
  // enum "class")
    UnknownStatusOpSchemeType = -1,
    NoControlOpSchemeType = 0,          // Scheme Type placeholder for items such as pipes
    HeatingRBOpSchemeType = 1,          // Scheme Type for Heating Load Range Based Operation
    CoolingRBOpSchemeType = 2,          // Scheme Type for Cooling  Load Range Based Operation
    WetBulbRBOpSchemeType = 3,          // Scheme Type for Wet bulb range based Operation
    DryBulbRBOpSchemeType = 4,          // Scheme Type for Dry bulb range based Operation
    DewPointRBOpSchemeType = 5,         // Scheme Type for Dewpoint range based Operation
    RelHumRBOpSchemeType = 6,           // Scheme Type for relative humidity range based Operation
    DryBulbTDBOpSchemeType = 7,         // Scheme Type for relative humidity range based Operation
    WetBulbTDBOpSchemeType = 8,         // Scheme Type for Wet bulb range based Operation
    DewPointTDBOpSchemeType = 9,        // Scheme Type for Wet bulb range based Operation
    CompSetPtBasedSchemeType = 10,      // Temp Based Control
    UncontrolledOpSchemeType = 11,      // Scheme Type for Uncontrolled Operation
    EMSOpSchemeType = 12,               // Scheme Type for EMS based operation user Define scheme
    PumpOpSchemeType = 13,              // Not really an OpScheme, just a placeholder
    DemandOpSchemeType = 14,            // Placeholder for demand side equipment such as coils
    FreeRejectionOpSchemeType = 15,     // Scheme Type for waterside economizers and the like
    WSEconOpSchemeType = 16,            // Scheme Type for waterside economizers and the like
    ThermalEnergyStorageSchemeType = 17 // Scheme Type for Simplified Thermal Energy Storage operation
};

enum class PlantEquipmentType
{
    Invalid = -1,
    Boiler_Simple = 0,
    Boiler_Steam = 1,
    Chiller_Absorption = 2,          // older BLAST absorption chiller
    Chiller_Indirect_Absorption = 3, // revised absorption chiller
    Chiller_CombTurbine = 4,
    Chiller_ConstCOP = 5,
    Chiller_DFAbsorption = 6,
    Chiller_Electric = 7, // basic BLAST Chiller
    Chiller_ElectricEIR = 8,
    Chiller_ElectricReformEIR = 9,
    Chiller_EngineDriven = 10,
    CoolingTower_SingleSpd = 11,
    CoolingTower_TwoSpd = 12,
    CoolingTower_VarSpd = 13,
    Generator_FCExhaust = 14,
    HeatPumpWtrHeaterPumped = 15,
    HPWaterEFCooling = 16,
    HPWaterEFHeating = 17,
    HPWaterPECooling = 18,
    HPWaterPEHeating = 19,
    Pipe = 20,
    PipeSteam = 21,
    PipeExterior = 22,
    PipeInterior = 23,
    PipeUnderground = 24,
    PurchChilledWater = 25,
    PurchHotWater = 26,
    TS_IceDetailed = 27,
    TS_IceSimple = 28,
    ValveTempering = 29,
    WtrHeaterMixed = 30,
    WtrHeaterStratified = 31,
    PumpVariableSpeed = 32,
    PumpConstantSpeed = 33,
    PumpCondensate = 34,
    PumpBankVariableSpeed = 35,
    PumpBankConstantSpeed = 36,
    WaterUseConnection = 37,
    CoilWaterCooling = 38,             // demand side component
    CoilWaterDetailedFlatCooling = 39, // demand side component
    CoilWaterSimpleHeating = 40,       // demand side component
    CoilSteamAirHeating = 41,          // demand side component
    SolarCollectorFlatPlate = 42,      // demand side component
    PlantLoadProfile = 43,             // demand side component
    GrndHtExchgSystem = 44,
    GrndHtExchgSurface = 45,
    GrndHtExchgPond = 46,
    Generator_MicroTurbine = 47, // newer FSEC turbine
    Generator_ICEngine = 48,
    Generator_CTurbine = 49, // older BLAST turbine
    Generator_MicroCHP = 50,
    Generator_FCStackCooler = 51,
    FluidCooler_SingleSpd = 52,
    FluidCooler_TwoSpd = 53,
    EvapFluidCooler_SingleSpd = 54,
    EvapFluidCooler_TwoSpd = 55,
    ChilledWaterTankMixed = 56,
    ChilledWaterTankStratified = 57,
    PVTSolarCollectorFlatPlate = 58,
    Baseboard_Conv_Water = 59,
    Baseboard_Rad_Conv_Steam = 60,
    Baseboard_Rad_Conv_Water = 61,
    LowTempRadiant_VarFlow = 62,
    LowTempRadiant_ConstFlow = 63,
    CooledBeamAirTerminal = 64,
    CoilWAHPHeatingEquationFit = 65,
    CoilWAHPCoolingEquationFit = 66,
    CoilWAHPHeatingParamEst = 67,
    CoilWAHPCoolingParamEst = 68,
    RefrigSystemWaterCondenser = 69,
    RefrigerationWaterCoolRack = 70,
    MultiSpeedHeatPumpRecovery = 71,
    Chiller_ExhFiredAbsorption = 72,
    PipingSystemPipeCircuit = 73,
    SolarCollectorICS = 74,
    CoilVSWAHPHeatingEquationFit = 75,
    CoilVSWAHPCoolingEquationFit = 76,
    PlantComponentUserDefined = 77,
    CoilUserDefined = 78,
    ZoneHVACAirUserDefined = 79,
    AirTerminalUserDefined = 80,
    HeatPumpVRF = 81,
    GrndHtExchgHorizTrench = 82,
    FluidToFluidPlantHtExchg = 83,
    WaterSource = 84,
    CentralGroundSourceHeatPump = 85,
    UnitarySysRecovery = 86,
    PackagedTESCoolingCoil = 87,
    CoolingTower_VarSpdMerkel = 88,
    SwimmingPool_Indoor = 89,
    GrndHtExchgSlinky = 90,
    HeatPumpWtrHeaterWrapped = 91,
    FourPipeBeamAirTerminal = 92,
    CoolingPanel_Simple = 93,
    HeatPumpEIRCooling = 94,
    HeatPumpEIRHeating = 95,
    Num = 96
};

// Parameters for component character wrt how load gets met (or not)
//  used in %HowLoadServed to facilitate load dispatch logic
int const HowMet_Unknown(50);                              // not yet set
int const HowMet_NoneDemand(51);                           // does not meet a load, demand component
int const HowMet_PassiveCap(52);                           // Passive machine, does what conditions allow but
int const HowMet_ByNominalCap(53);                         // MaxLoad, MinLoad, OptLoad should work
int const HowMet_ByNominalCapLowOutLimit(54);              // MaxLoad, MinLoad, OptLoad but with low limit temp on outlet
int const HowMet_ByNominalCapHiOutLimit(55);               // MaxLoad, MinLoad, OptLoad but with high limit temp on outlet
int const HowMet_ByNominalCapFreeCoolCntrl(56);            // HowMet_ByNominalCap with free cool shutdown
int const HowMet_ByNominalCapLowOutLimitFreeCoolCntrl(57); // HowMet_ByNominalCapLowOutLimit with free cool shutdown

enum class LoadingScheme
{
    Unassigned = -1,
    Optimal = 0,              // Optimal Load Distribution Scheme
    Sequential = 1,           // Sequential Load Distribution Scheme
    Uniform = 2,              // Uniform Load Distribution Scheme
    UniformPLR = 3,           // Uniform PLR Load Distribution Scheme
    SequentialUniformPLR = 4, // Sequential Uniform PLR Load Distribution Scheme
};

enum class FlowMode
{
    Unassigned = -1,
    Constant = 0,
    NotModulated = 1,
    LeavingSetpointModulated = 2,
};

enum class CondenserType
{
    Unassigned = -1,
    AirCooled = 0,
    WaterCooled = 1,
    EvapCooled = 2,
};

// SimFlagCriteriaTypes for use in performing interconnect re-sim checks
enum class CriteriaType
{
    MassFlowRate = -1,
    Temperature = 0,
    HeatTransferRate = 1,
};

enum class FreeCoolControlMode
{
    Unassigned = -1,
    WetBulb = 0, // HeatExchanger:Hydronic model control type mode, outdoor wetbulb sensor
    DryBulb = 1, // HeatExchanger:Hydronic model control type mode, outdoor drybulb sensor
    Loop = 2,    // HeatExchanger:Hydronic model control type mode, loop setpoint sensor
};

enum class LoopDemandCalcScheme
{
    Unassigned = -1,
    SingleSetPoint = 0,       // Uses a single temp setpoint to calculate loop demand
    DualSetPointDeadBand = 1, // Uses a dual temp setpoint with a deadband between the high
};

enum class CommonPipeType
{
    No = -1,
    Single = 0,
    TwoWay = 1,
};

enum class FlowLock
{
    PumpQuery = -1, // Used to ask the pumps for their min/max avail based on no constraints
    Unlocked = 0,   // components request flow
    Locked = 1,     // components take their inlet flow
};

enum class PressureCall
{
    Init = -1,
    Calc = 0,
    Update = 1,
};

enum class PressSimType
{
    NoPressure = -1,         // Nothing for that particular loop
    PumpPowerCorrection = 0, // Only updating the pump power
    FlowCorrection = 1,      // Update pump flow rate based on pump curve
    FlowSimulation = 2,      // Full pressure network simulation
};

enum class CtrlType
{
    Unassigned = -1,
    HeatingOp = 0, // Constant for Heating Operation
    CoolingOp = 1, // Constant for Cooling Operation
    DualOp = 2,    // Constant for Cooling or Heating Operation
};

// branch loop type for absorption chillerheater models
enum class BrLoopType
{
    Chiller = -1,
    Heater = 0,
    Condenser = 1,
    NoMatch = 2
};

} // namespace EnergyPlus::DataPlant

#endif
