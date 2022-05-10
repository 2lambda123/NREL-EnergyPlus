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

#ifndef AirflowNetworkBalanceManager_hh_INCLUDED
#define AirflowNetworkBalanceManager_hh_INCLUDED

// define this variable to get new code, commenting should yield original
#define SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS

// C++ Headers
#include <unordered_map>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <AirflowNetwork/Elements.hpp>
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace AirflowNetwork {

    struct AirflowNetworkReportVars
    {
        // Members
        Real64 InfilVolume;         // Volume of Air {m3} due to infiltration
        Real64 InfilMass;           // Mass of Air {kg} due to infiltration
        Real64 InfilAirChangeRate;  // Infiltration air change rate {ach}
        Real64 VentilHeatLoss;      // Heat Gain {W} due to ventilation
        Real64 VentilHeatGain;      // Heat Loss {W} due to ventilation
        Real64 VentilVolume;        // Volume of Air {m3} due to ventilation
        Real64 VentilMass;          // Mass of Air {kg} due to ventilation
        Real64 VentilAirChangeRate; // Ventilation air change rate {ach}
        Real64 VentilFanElec;       // Fan Electricity {W} due to ventilation
        Real64 VentilAirTemp;       // Air Temp {C} of ventilation
        Real64 MixVolume;           // Mixing volume of Air {m3}
        Real64 MixMass;             // Mixing mass of air {kg}
        Real64 ExfilSensiLoss;      // Sensible heat Loss rate {W} due to exfiltration
        Real64 ExfilLatentLoss;     // Latent heat Loss rate {W} due to exfiltration
        Real64 ExfilTotalLoss;      // Total heat Loss rate {W} due to exfiltration
        Real64 ExfilMass;           // Mass of Air {kg} due to exfiltration
        Real64 InletMass;           // Total zone inlet mass of air {kg}
        Real64 OutletMass;          // Total zone outlet mass of air {kg}

        // Default Constructor
        AirflowNetworkReportVars()
            : InfilVolume(0.0), InfilMass(0.0), InfilAirChangeRate(0.0), VentilHeatLoss(0.0), VentilHeatGain(0.0), VentilVolume(0.0), VentilMass(0.0),
              VentilAirChangeRate(0.0), VentilFanElec(0.0), VentilAirTemp(0.0), MixVolume(0.0), MixMass(0.0), ExfilSensiLoss(0.0),
              ExfilLatentLoss(0.0), ExfilTotalLoss(0.0), ExfilMass(0.0), InletMass(0.0), OutletMass(0.0)
        {
        }
    };

    // Functions

    void ManageAirflowNetworkBalance(EnergyPlusData &state,
                                     Optional_bool_const FirstHVACIteration = _, // True when solution technique on first iteration
                                     Optional_int_const Iter = _,                // Iteration number
                                     Optional_bool ResimulateAirZone = _         // True when solution technique on third iteration
    );

    void GetAirflowNetworkInput(EnergyPlusData &state);

    void AllocateAndInitData(EnergyPlusData &state);

    void CalcAirflowNetworkAirBalance(EnergyPlusData &state);

    Real64 CalcDuctInsideConvResist(Real64 Tair, // Average air temperature
                                    Real64 mdot, // Mass flow rate
                                    Real64 Dh,   // Hydraulic diameter
                                    Real64 hIn   // User defined convection coefficient
    );

    Real64 CalcDuctOutsideConvResist(EnergyPlusData &state,
                                     Real64 Ts,      // Surface temperature
                                     Real64 Tamb,    // Free air temperature
                                     Real64 Wamb,    // Free air humidity ratio
                                     Real64 Pamb,    // Free air barometric pressure
                                     Real64 Dh,      // Hydraulic diameter
                                     Real64 ZoneNum, // Zone number
                                     Real64 hOut     // User defined convection coefficient
    );

    Real64 CalcWindPressure(EnergyPlusData &state,
                            int curve,           // Curve index, change this to pointer after curve refactor
                            bool symmetricCurve, // True if the curve is symmetric (0 to 180)
                            bool relativeAngle,  // True if the Cp curve angle is measured relative to the surface
                            Real64 azimuth,      // Azimuthal angle of surface
                            Real64 windSpeed,    // Wind velocity
                            Real64 windDir,      // Wind direction
                            Real64 dryBulbTemp,  // Air node dry bulb temperature
                            Real64 humRat        // Air node humidity ratio
    );

    void CalcAirflowNetworkHeatBalance(EnergyPlusData &state);

    void CalcAirflowNetworkMoisBalance(EnergyPlusData &state);

    void CalcAirflowNetworkCO2Balance(EnergyPlusData &state);

    void CalcAirflowNetworkGCBalance(EnergyPlusData &state);

    void MRXINV(EnergyPlusData &state, int NORDER);

    void ReportAirflowNetwork(EnergyPlusData &state);

    void UpdateAirflowNetwork(EnergyPlusData &state, Optional_bool_const FirstHVACIteration = _); // True when solution technique on first iteration

    void AirflowNetworkVentingControl(EnergyPlusData &state,
                                      int i,             // AirflowNetwork surface number
                                      Real64 &OpenFactor // Window or door opening factor (used to calculate airflow)
    );

    void AssignFanAirLoopNum(EnergyPlusData &state);

    void ValidateDistributionSystem(EnergyPlusData &state);

    void ValidateFanFlowRate(EnergyPlusData &state); // Catch a fan flow rate from EPlus input file and add a flag for VAV terminal damper

    void ValidateExhaustFanInput(EnergyPlusData &state);

    void HybridVentilationControl(EnergyPlusData &state);

    void CalcSingleSidedCps(EnergyPlusData &state, std::vector<std::vector<Real64>> &valsByFacade, int numWindDirs = 36);

    Real64 GetZoneOutdoorAirChangeRate(EnergyPlusData &state, int ZoneNum); // hybrid ventilation system controlled zone number

    int GetAirLoopNumber(EnergyPlusData &state, int NodeNumber); // Get air loop number for each distribution node and linkage

    Real64 AFNPressureResidual(EnergyPlusData &state, Real64 ExFanMassFlowRate,
                               Array1D<Real64> const &Par); // Residual function using Regula Falsi

    // derived class or struct
    struct OccupantVentilationControlProp
    {

        std::string Name;                     // Provide a unique object name
        Real64 MinOpeningTime;                // Minimum Opening Time
        Real64 MinClosingTime;                // Minimum Closing Time
        std::string ComfortLowTempCurveName;  // Thermal Comfort Low Temperature Curve Name
        std::string ComfortHighTempCurveName; // Thermal Comfort High Temperature Curve Name
        int ComfortLowTempCurveNum;           // Thermal Comfort Low Temperature Curve number
        int ComfortHighTempCurveNum;          // Thermal Comfort high Temperature Curve number
        int OpeningProbSchNum;                // Opening probability schedule pointer
        int ClosingProbSchNum;                // Closing probability schedule pointer
        Real64 ComfortBouPoint;               // Thermal Comfort Temperature Boundary Point
        bool OccupancyCheck;                  // Occupancy check
        std::string OpeningProbSchName;       // Opening probability schedule name
        std::string ClosingProbSchName;       // Closing probability schedule name
        Real64 MaxPPD;                        // Maximum PPD used to calculate comfort band (%)
        bool MinTimeControlOnly;              // Chach minimum opening and closing time only

        // Default Constructor
        OccupantVentilationControlProp()
            : MinOpeningTime(0.0), MinClosingTime(0.0), ComfortLowTempCurveNum(0), ComfortHighTempCurveNum(0), OpeningProbSchNum(0),
              ClosingProbSchNum(0), ComfortBouPoint(10.0), OccupancyCheck(false), MaxPPD(10.0), MinTimeControlOnly(false)
        {
        }

        void calc(EnergyPlusData &state,
                  int ZoneNum,
                  Real64 TimeOpenDuration,
                  Real64 TimeCloseDuration,
                  int &OpeningStatus,
                  int &OpeningProbStatus,
                  int &ClosingProbStatus); // function to perform calculations

        bool openingProbability(EnergyPlusData &state,
                                int ZoneNum,
                                Real64 TimeCloseDuration); // function to perform calculations of opening probability

        bool closingProbability(EnergyPlusData &state, Real64 TimeCloseDuration); // function to perform calculations of closing probability
    };

struct AirflowNetworkSolverData : BaseGlobalStruct
    {

    void initialize(EnergyPlusData &state);
    void calculateWindPressureCoeffs(EnergyPlusData &state);
    void allocate(EnergyPlusData &state);
    void initialize_calculation();
    void setsky();
    void airmov(EnergyPlusData &state);
    void solvzp(EnergyPlusData &state, int &ITER); // number of iterations
    void filjac(EnergyPlusData &state,
                int const NNZE,  // number of nonzero entries in the "AU" array.
                bool const LFLAG // if = 1, use laminar relationship (initialization).
    );
    void facsky(EnergyPlusData &state,
                Array1D<Real64> &AU,   // the upper triangle of [A] before and after factoring
                Array1D<Real64> &AD,   // the main diagonal of [A] before and after factoring
                Array1D<Real64> &AL,   // the lower triangle of [A] before and after factoring
                const Array1D_int &IK, // pointer to the top of column/row "K"
                int const NEQ,         // number of equations
                int const NSYM         // symmetry:  0 = symmetric matrix, 1 = non-symmetric
    );

    void slvsky(const Array1D<Real64> &AU, // the upper triangle of [A] before and after factoring
                const Array1D<Real64> &AD, // the main diagonal of [A] before and after factoring
                const Array1D<Real64> &AL, // the lower triangle of [A] before and after factoring
                Array1D<Real64> &B,        // "B" vector (input); "X" vector (output).
                const Array1D_int &IK,     // pointer to the top of column/row "K"
                int const NEQ,             // number of equations
                int const NSYM             // symmetry:  0 = symmetric matrix, 1 = non-symmetric
    );

    void filsky(const Array1D<Real64> &X,    // element array (row-wise sequence)
                std::array<int, 2> const LM, // location matrix
                const Array1D_int &IK,       // pointer to the top of column/row "K"
                Array1D<Real64> &AU,         // the upper triangle of [A] before and after factoring
                Array1D<Real64> &AD,         // the main diagonal of [A] before and after factoring
                int const FLAG               // mode of operation
    );

    EPVector<AirflowNetwork::OccupantVentilationControlProp> OccupantVentilationControl;
    Array1D_int SplitterNodeNumbers;
    int AirflowNetworkNumOfExtSurfaces = 0;
    // Inverse matrix
    Array1D<Real64> MA;
    Array1D<Real64> MV;
    Array1D_int IVEC;
    int VentilationCtrl = 0;  // Hybrid ventilation control type
    int NumOfExhaustFans = 0; // Number of exhaust fans
    int NumAirflowNetwork = 0;
    int AirflowNetworkNumOfDetOpenings = 0;
    int AirflowNetworkNumOfSimOpenings = 0;
    int AirflowNetworkNumOfHorOpenings = 0;
    int AirflowNetworkNumOfSurCracks = 0;
    int AirflowNetworkNumOfSurELA = 0;
    int AirflowNetworkNumOfSFR = 0;
    int AirflowNetworkNumOfExtNode = 0;
    int AirflowNetworkNumOfOutAirNode = 0;
    int AirflowNetworkNumOfSingleSideZones = 0; // Total number of zones with advanced single sided wind pressure coefficient calculation
    int DisSysNumOfNodes = 0;
    int DisSysNumOfLeaks = 0;
    int DisSysNumOfELRs = 0;
    int DisSysNumOfDucts = 0;
    int DisSysNumOfDuctViewFactors = 0;
    int DisSysNumOfDampers = 0;
    int DisSysNumOfCVFs = 0;
    int DisSysNumOfDetFans = 0;
    int DisSysNumOfCoils = 0;
    int DisSysNumOfHXs = 0;
    int DisSysNumOfCPDs = 0;
    int DisSysNumOfTermUnits = 0;
    int DisSysNumOfLinks = 0;
    int NumOfExtNodes = 0;
    Real64 IncAng = 0.0;                     // Wind incidence angle relative to facade normal (deg)
    int SupplyFanType = 0;                   // Supply air fan type
    Real64 MaxOnOffFanRunTimeFraction = 0.0; // max Run time fraction for an On/Off fan flow rate among airloops
    Real64 CurrentEndTimeLast = 0.0;         // last end time
    Real64 TimeStepSysLast = 0.0;            // last system time step
    int AirflowNetworkNumOfOccuVentCtrls = 0;
    int IntraZoneNumOfNodes = 0;
    int IntraZoneNumOfLinks = 0;
    int IntraZoneNumOfZones = 0;
    int NumOfPressureControllers = 0; // number of pressure controllers
    int NumOfOAFans = 0;              // number of OutdoorAir fans
    int NumOfReliefFans = 0;          // number of OutdoorAir relief fans
    bool AirflowNetworkGetInputFlag = true;
    bool AssignFanAirLoopNumFlag = true;
    bool ValidateDistributionSystemFlag = true;
    Array1D<Real64> FacadeAng =
        Array1D<Real64>(5); // Facade azimuth angle (for walls, angle of outward normal to facade measured clockwise from North) (deg)
    Array1D<Real64> LoopPartLoadRatio;
    Array1D<Real64> LoopOnOffFanRunTimeFraction;
    Array1D<bool> LoopOnOffFlag;

    bool ValidateExhaustFanInputOneTimeFlag = true;
    bool initializeOneTimeFlag = true;
    bool initializeMyEnvrnFlag = true;
    bool CalcAirflowNetworkAirBalanceOneTimeFlag = true;
    bool CalcAirflowNetworkAirBalanceErrorsFound = false;
    bool UpdateAirflowNetworkMyOneTimeFlag = true;
    bool UpdateAirflowNetworkMyOneTimeFlag1 = true;

    // CalcAirflowNetworkAirBalance variables
    int ErrCountVar = 0;
    int ErrCountHighPre = 0;
    int ErrCountLowPre = 0;
    int ErrIndexHighPre = 0;
    int ErrIndexVar = 0;
    int ErrIndexLowPre = 0;

    // Object Data
    EPVector<AirflowNetwork::AirflowNetworkReportVars> AirflowNetworkZnRpt;
    std::unordered_map<std::string, std::string> UniqueAirflowNetworkSurfaceName;

    // AirflowNetwork::Solver solver;

    // Output and reporting
    EPVector<AirflowNetwork::AirflowNetworkExchangeProp> exchangeData;
    EPVector<AirflowNetwork::AirflowNetworkExchangeProp> multiExchangeData;
    EPVector<AirflowNetwork::AirflowNetworkLinkReportData> linkReport;
    EPVector<AirflowNetwork::AirflowNetworkNodeReportData> nodeReport;
    EPVector<AirflowNetwork::AirflowNetworkLinkReportData> linkReport1;

    // used to be statics
    Array1D<bool> onceZoneFlag;
    Array1D<bool> onceSurfFlag;
    bool onetime = false;
    int HybridGlobalErrIndex = 0;
    int HybridGlobalErrCount = 0;
    int AFNNumOfExtOpenings = 0; // Total number of external openings in the model
    int OpenNuminZone = 0;       // Counts which opening this is in the zone, 1 or 2

    std::unordered_map<std::string, AirflowElement *> elements;
    std::unordered_map<std::string, int> compnum; // Stopgap until all the introspection is dealt with

    std::vector<AirProperties> properties;

    // int const NrInt; // Number of intervals for a large opening

    AirflowNetwork::DetailedOpeningSolver dos;

    // Data
    int ActualNumOfLinks = 0;
    int ActualNumOfNodes = 0;

    // Common block AFEDAT
    Array1D<Real64> AFECTL;
    Array1D<Real64> AFLOW2;
    Array1D<Real64> AFLOW;
    Array1D<Real64> PS;
    Array1D<Real64> PW;

    // Common block CONTRL
    Real64 PB = 0.0;

    // Common block ZONL
    // Array1D<Real64> RHOZ;
    // Array1D<Real64> SQRTDZ;
    // Array1D<Real64> VISCZ;
    Array1D<Real64> SUMAF;
    // Array1D<Real64> TZ; // Temperature [C]
    // Array1D<Real64> WZ; // Humidity ratio [kg/kg]
    Array1D<Real64> PZ; // Pressure [Pa]

    // Other array variables
    Array1D_int ID;
    Array1D_int IK;
    Array1D<Real64> AD;
    Array1D<Real64> AU;

#ifdef SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS
    Array1D_int newIK;     // noel
    Array1D<Real64> newAU; // noel
#endif

    // REAL(r64), ALLOCATABLE, DIMENSION(:) :: AL
    Array1D<Real64> SUMF;

    int SimulateAirflowNetwork = 1;
    Array1D_bool AirflowNetworkZoneFlag;
    int NumOfNodesMultiZone = 0;    // Number of nodes for multizone calculation
    int NumOfNodesDistribution = 0; // Number of nodes for distribution system calculation
    int NumOfLinksMultiZone = 0;    // Number of links for multizone calculation
    int NumOfLinksDistribution = 0; // Number of links for distribution system calculation
    int NumOfNodesIntraZone = 0;    // Number of nodes for intrazone calculation
    int NumOfLinksIntraZone = 0;    // Number of links for intrazone calculation

    int AirflowNetworkNumOfNodes = 0; // Number of nodes for AirflowNetwork calculation
    // = NumOfNodesMultiZone+NumOfNodesDistribution
    int AirflowNetworkNumOfComps = 0; // Number of components for AirflowNetwork calculation
    int AirflowNetworkNumOfLinks = 0; // Number of links for AirflowNetwork calculation
    // = NumOfLinksMultiZone+NumOfLinksDistribution
    // RoomAirManager use
    int AirflowNetworkNumOfSurfaces = 0; // The number of surfaces for multizone calculation
    int AirflowNetworkNumOfZones = 0;    // The number of zones for multizone calculation

    bool RollBackFlag = false;                 // Roll back flag when system time step down shifting
    Array1D<Real64> ANZT;                      // Local zone air temperature for roll back use
    Array1D<Real64> ANZW;                      // Local zone air humidity ratio for roll back use
    Array1D<Real64> ANCO;                      // Local zone air CO2 for roll back use
    Array1D<Real64> ANGC;                      // Local zone air generic contaminant for roll back use
    int AirflowNetworkNumOfExhFan = 0;         // Number of zone exhaust fans
    Array1D_bool AirflowNetworkZoneExhaustFan; // Logical to use zone exhaust fans
    bool AirflowNetworkFanActivated = false;   // Supply fan activation flag
    bool AirflowNetworkUnitarySystem = false;  // set to TRUE for unitary systems (to make answers equal, will remove eventually)
    // Multispeed HP only
    int MultiSpeedHPIndicator = 0; // Indicator for multispeed heat pump use
    // Additional airflow needed for an VAV fan to compensate the leakage losses and supply pathway pressure losses [kg/s]
    Real64 VAVTerminalRatio = 0.0;       // The terminal flow ratio when a supply VAV fan reach its max flow rate
    bool VAVSystem = false;              // This flag is used to represent a VAV system
    Real64 ExhaustFanMassFlowRate = 0.0; // Exhaust fan flow rate used in PressureStat
    int PressureSetFlag = 0;             // PressureSet flag
    Real64 ReliefMassFlowRate = 0.0;     // OA Mixer relief node flow rate used in PressureStat
    bool AFNDefaultControlFlag = false;  // Default simulation control flag

    Array1D<AirflowNetwork::AirflowNetworkNodeSimuData> AirflowNetworkNodeSimu;
    Array1D<AirflowNetwork::AirflowNetworkLinkSimuData> AirflowNetworkLinkSimu;

    AirflowNetwork::AirflowNetworkSimuProp AirflowNetworkSimu;
    // unique object name | AirflowNetwork control | Wind pressure coefficient input control | Integer equivalent for WPCCntr
    // field | CP Array name at WPCCntr = "INPUT" | Building type | Height Selection | Maximum number of iteration |
    // Initialization flag | Relative airflow convergence | Absolute airflow convergence | Convergence acceleration limit |
    // Maximum pressure change in an element [Pa] | Azimuth Angle of Long Axis of Building | Ratio of Building Width Along
    // Short Axis to Width Along Long Axis | Number of wind directions | Minimum pressure difference | Exterior large opening
    // error count during HVAC system operation | Exterior large opening error index during HVAC system operation | Large
    // opening error count at Open factor > 1.0 | Large opening error error index at Open factor > 1.0 | Initialization flag
    // type
    Array1D<AirflowNetwork::AirflowNetworkNodeProp> AirflowNetworkNodeData;
    Array1D<AirflowNetwork::AirflowNetworkCompProp> AirflowNetworkCompData;
    Array1D<AirflowNetwork::AirflowNetworkLinkageProp> AirflowNetworkLinkageData;
    Array1D<AirflowNetwork::MultizoneZoneProp> MultizoneZoneData;
    Array1D<AirflowNetwork::MultizoneSurfaceProp> MultizoneSurfaceData;
    Array1D<AirflowNetwork::DetailedOpening> MultizoneCompDetOpeningData;
    Array1D<AirflowNetwork::SimpleOpening> MultizoneCompSimpleOpeningData;
    Array1D<AirflowNetwork::HorizontalOpening> MultizoneCompHorOpeningData;
    Array1D<AirflowNetwork::SurfaceCrack> MultizoneSurfaceCrackData;
    Array1D<AirflowNetwork::EffectiveLeakageArea> MultizoneSurfaceELAData;
    Array1D<AirflowNetwork::SpecifiedMassFlow> SpecifiedMassFlowData;
    Array1D<AirflowNetwork::SpecifiedVolumeFlow> SpecifiedVolumeFlowData;
    Array1D<AirflowNetwork::MultizoneExternalNodeProp> MultizoneExternalNodeData;
    Array1D<AirflowNetwork::DeltaCpProp> DeltaCp;
    Array1D<AirflowNetwork::DeltaCpProp> EPDeltaCP;
    Array1D<AirflowNetwork::ZoneExhaustFan> MultizoneCompExhaustFanData;
    Array1D<AirflowNetwork::IntraZoneNodeProp> IntraZoneNodeData;       // Intra zone data set
    Array1D<AirflowNetwork::IntraZoneLinkageProp> IntraZoneLinkageData; // Intra zone linkage adat set
    Array1D<AirflowNetwork::DisSysNodeProp> DisSysNodeData;
    Array1D<AirflowNetwork::DuctLeak> DisSysCompLeakData;
    Array1D<AirflowNetwork::EffectiveLeakageRatio> DisSysCompELRData;
    Array1D<AirflowNetwork::Duct> DisSysCompDuctData;
    Array1D<AirflowNetwork::Damper> DisSysCompDamperData;
    Array1D<AirflowNetwork::ConstantVolumeFan> DisSysCompCVFData;
    Array1D<AirflowNetwork::DetailedFan> DisSysCompDetFanData;
    Array1D<AirflowNetwork::DisSysCompCoilProp> DisSysCompCoilData;
    Array1D<AirflowNetwork::DisSysCompHXProp> DisSysCompHXData;
    Array1D<AirflowNetwork::DisSysCompTermUnitProp> DisSysCompTermUnitData;
    Array1D<AirflowNetwork::ConstantPressureDrop> DisSysCompCPDData;
    Array1D<AirflowNetwork::AiflowNetworkReportProp> AirflowNetworkReportData;
    Array1D<AirflowNetwork::PressureControllerProp> PressureControllerData;
    Array1D<AirflowNetwork::OutdoorAirFan> DisSysCompOutdoorAirData;
    Array1D<AirflowNetwork::ReliefFlow> DisSysCompReliefAirData;
    Array1D<AirflowNetwork::AirflowNetworkLinkageViewFactorProp> AirflowNetworkLinkageViewFactorData;

    void clear_state() override
    {
        this->OccupantVentilationControl.deallocate();
        this->SplitterNodeNumbers.deallocate();
        this->AirflowNetworkNumOfExtSurfaces = 0;
        this->MA.deallocate();
        this->MV.deallocate();
        this->IVEC.deallocate();
        this->VentilationCtrl = 0;
        this->NumOfExhaustFans = 0;
        this->NumAirflowNetwork = 0;
        this->AirflowNetworkNumOfDetOpenings = 0;
        this->AirflowNetworkNumOfSimOpenings = 0;
        this->AirflowNetworkNumOfHorOpenings = 0;
        this->AirflowNetworkNumOfSurCracks = 0;
        this->AirflowNetworkNumOfSurELA = 0;
        this->AirflowNetworkNumOfSFR = 0;
        this->AirflowNetworkNumOfExtNode = 0;
        this->AirflowNetworkNumOfOutAirNode = 0;
        this->AirflowNetworkNumOfSingleSideZones = 0;
        this->DisSysNumOfNodes = 0;
        this->DisSysNumOfLeaks = 0;
        this->DisSysNumOfELRs = 0;
        this->DisSysNumOfDucts = 0;
        this->DisSysNumOfDuctViewFactors = 0;
        this->DisSysNumOfDampers = 0;
        this->DisSysNumOfCVFs = 0;
        this->DisSysNumOfDetFans = 0;
        this->DisSysNumOfCoils = 0;
        this->DisSysNumOfHXs = 0;
        this->DisSysNumOfCPDs = 0;
        this->DisSysNumOfTermUnits = 0;
        this->DisSysNumOfLinks = 0;
        this->NumOfExtNodes = 0;
        this->IncAng = 0.0;
        this->SupplyFanType = 0;
        this->MaxOnOffFanRunTimeFraction = 0.0;
        this->CurrentEndTimeLast = 0.0;
        this->TimeStepSysLast = 0.0;
        this->AirflowNetworkNumOfOccuVentCtrls = 0;
        this->IntraZoneNumOfNodes = 0;
        this->IntraZoneNumOfLinks = 0;
        this->IntraZoneNumOfZones = 0;
        this->NumOfPressureControllers = 0;
        this->NumOfOAFans = 0;
        this->NumOfReliefFans = 0;
        this->AirflowNetworkGetInputFlag = true;
        this->AssignFanAirLoopNumFlag = true;
        this->ValidateDistributionSystemFlag = true;
        this->FacadeAng = Array1D<Real64>(5);
        this->AirflowNetworkZnRpt.deallocate();
        this->LoopPartLoadRatio.deallocate();
        this->LoopOnOffFanRunTimeFraction.deallocate();
        this->LoopOnOffFlag.deallocate();
        this->UniqueAirflowNetworkSurfaceName.clear();
        this->ValidateExhaustFanInputOneTimeFlag = true;
        this->initializeOneTimeFlag = true;
        this->initializeMyEnvrnFlag = true;
        this->CalcAirflowNetworkAirBalanceOneTimeFlag = true;
        this->CalcAirflowNetworkAirBalanceErrorsFound = false;
        this->UpdateAirflowNetworkMyOneTimeFlag = true;
        this->UpdateAirflowNetworkMyOneTimeFlag1 = true;
        this->exchangeData.deallocate();
        this->multiExchangeData.deallocate();
        this->linkReport.deallocate();
        this->nodeReport.deallocate();
        this->linkReport1.deallocate();
        this->ErrCountVar = 0;
        this->ErrCountHighPre = 0;
        this->ErrCountLowPre = 0;
        this->ErrIndexHighPre = 0;
        this->ErrIndexVar = 0;
        this->ErrIndexLowPre = 0;
        this->onceZoneFlag.clear();
        this->onceSurfFlag.clear();
        this->onetime = false;
        this->HybridGlobalErrIndex = 0;
        this->HybridGlobalErrCount = 0;
        this->AFNNumOfExtOpenings = 0;
        this->OpenNuminZone = 0;

        ActualNumOfLinks = 0;
        ActualNumOfNodes = 0;
        AFECTL.clear();
        AFLOW2.clear();
        AFLOW.clear();
        PS.clear();
        PW.clear();
        PB = 0.0;
        SUMAF.clear();
        PZ.clear();
        ID.clear();
        IK.clear();
        AD.clear();
        AU.clear();
        elements.clear();
        compnum.clear();
        properties.clear();

#ifdef SKYLINE_MATRIX_REMOVE_ZERO_COLUMNS
        newIK.deallocate();
        newAU.deallocate();
#endif
        SUMF.deallocate();
        dos.clear();

        this->SimulateAirflowNetwork = 1;
        this->AirflowNetworkNodeSimu.clear();
        this->AirflowNetworkLinkSimu.clear();
        this->AirflowNetworkZoneFlag.clear();
        this->NumOfNodesMultiZone = 0;
        this->NumOfNodesDistribution = 0;
        this->NumOfLinksMultiZone = 0;
        this->NumOfLinksDistribution = 0;
        this->NumOfNodesIntraZone = 0;
        this->NumOfLinksIntraZone = 0;
        this->AirflowNetworkNumOfNodes = 0;
        this->AirflowNetworkNumOfComps = 0;
        this->AirflowNetworkNumOfLinks = 0;
        this->AirflowNetworkNumOfSurfaces = 0;
        this->AirflowNetworkNumOfZones = 0;
        this->RollBackFlag = false;
        this->ANZT.clear();
        this->ANZW.clear();
        this->ANCO.clear();
        this->ANGC.clear();
        this->AirflowNetworkNumOfExhFan = 0;
        this->AirflowNetworkZoneExhaustFan.clear();
        this->AirflowNetworkFanActivated = false;
        this->AirflowNetworkUnitarySystem = false;
        this->MultiSpeedHPIndicator = 0;
        this->VAVTerminalRatio = 0.0;
        this->VAVSystem = false;
        this->AirflowNetworkSimu = AirflowNetwork::AirflowNetworkSimuProp();
        this->AirflowNetworkNodeData.clear();
        this->AirflowNetworkCompData.clear();
        this->AirflowNetworkLinkageData.clear();
        this->MultizoneZoneData.clear();
        this->MultizoneSurfaceData.clear();
        this->MultizoneCompDetOpeningData.clear();
        this->MultizoneCompSimpleOpeningData.clear();
        this->MultizoneCompHorOpeningData.clear();
        this->MultizoneSurfaceCrackData.clear();
        this->SpecifiedMassFlowData.clear();
        this->SpecifiedVolumeFlowData.clear();
        this->MultizoneSurfaceELAData.clear();
        this->MultizoneExternalNodeData.clear();
        this->DeltaCp.clear();
        this->EPDeltaCP.clear();
        this->MultizoneCompExhaustFanData.clear();
        this->IntraZoneNodeData.clear();    // Intra zone data set
        this->IntraZoneLinkageData.clear(); // Intra zone linkage adat set
        this->DisSysNodeData.clear();
        this->DisSysCompLeakData.clear();
        this->DisSysCompELRData.clear();
        this->DisSysCompDuctData.clear();
        this->DisSysCompDamperData.clear();
        this->DisSysCompCVFData.clear();
        this->DisSysCompDetFanData.clear();
        this->DisSysCompCoilData.clear();
        this->DisSysCompHXData.clear();
        this->DisSysCompTermUnitData.clear();
        this->DisSysCompCPDData.clear();
        this->AirflowNetworkReportData.clear();
        this->PressureControllerData.clear();
        this->DisSysCompOutdoorAirData.clear();
        this->DisSysCompReliefAirData.clear();
        this->AirflowNetworkLinkageViewFactorData.clear();
    }
};

} // namespace AirflowNetwork

} // namespace EnergyPlus

#endif
