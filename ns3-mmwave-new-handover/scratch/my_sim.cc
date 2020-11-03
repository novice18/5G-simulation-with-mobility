/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation;
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   Author: Marco Miozzo <marco.miozzo@cttc.es>
*           Nicola Baldo  <nbaldo@cttc.es>
*
*   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
*                         Sourjya Dutta <sdutta@nyu.edu>
*                         Russell Ford <russell.ford@nyu.edu>
*                         Menglei Zhang <menglei@nyu.edu>
*/


#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store.h"
#include "ns3/mmwave-helper.h"
#include "ns3/log.h"
#include <ns3/buildings-module.h>
#include <chrono>
//Changes
//#include "ns3/epc-helper.h"

using namespace ns3;
using namespace mmwave;
NS_LOG_COMPONENT_DEFINE("MmWaveConfigurableSimulation");

//Custom Global Variables
std::chrono::system_clock::time_point startTime;
static ns3::GlobalValue g_simTime ("simTime",
                                   "Total duration of the simulation [s]",
                                   ns3::DoubleValue (0.25),
                                   ns3::MakeDoubleChecker<double> ());
								   
static ns3::GlobalValue g_interval ("interval",
                                   "Total duration of the simulation [s]",
                                   ns3::DoubleValue (0.01),
                                   ns3::MakeDoubleChecker<double> ());
								  
static ns3::GlobalValue g_numEnb ("numEnb",
                                   "The number of ENB to simulate",
                                   ns3::UintegerValue (1),
                                   ns3::MakeUintegerChecker<uint64_t> ());
								   
static ns3::GlobalValue g_numUe ("numUe",
                                   "The number of UE to simulate",
                                   ns3::UintegerValue (1),
                                   ns3::MakeUintegerChecker<uint64_t> ());
								   
static ns3::GlobalValue g_maxX ("maxX",
                                   "The largest value allowed on the X axis.",
                                   ns3::IntegerValue (100),
                                   MakeIntegerChecker<int32_t> ());
								   
static ns3::GlobalValue g_maxY ("maxY",
                                   "The largest value allowed on the Y axis.",
                                   ns3::IntegerValue (100),
                                   MakeIntegerChecker<int32_t> ());
								   
static ns3::GlobalValue g_maxZ ("maxZ",
                                   "The largest value allowed on the Z axis.",
                                   ns3::IntegerValue (100),
                                   MakeIntegerChecker<int32_t> ());
								   
static ns3::GlobalValue g_maxXVel ("maxXVel",
                                   "The maximum velocity along the X axis.",
                                   ns3::IntegerValue (100),
                                   MakeIntegerChecker<int32_t> ());
								   
static ns3::GlobalValue g_maxYVel ("maxYVel",
                                   "The maximum velocity along the Y axis.",
                                   ns3::IntegerValue (100),
                                   MakeIntegerChecker<int32_t> ());
								   
static ns3::GlobalValue g_maxZVel ("maxZVel",
                                   "The maximum velocity along the Z axis.",
                                   ns3::IntegerValue (100),
                                   MakeIntegerChecker<int32_t> ());


//Custom Functions
Ptr<ListPositionAllocator>  makeENB(uint64_t num, int32_t mindim, int32_t maxx, int32_t maxy, int32_t maxz, Ptr<ListPositionAllocator> lpa)
{
  int32_t diff = maxx-mindim;
  std::cout<<"Creating "<<num<<" ENB in Simulation"<<std::endl;
  int32_t x = mindim;
  int32_t y = mindim;
  //int32_t z = abs((rand() % diff)+mindim);
  int32_t z = 0;
  int gridxy = int(ceil(pow(num,0.5)));
  int gridbreak = floor(diff/gridxy);
  std::cout<<"ENB will be layed out on a "<<gridxy<<" * "<<gridxy<<" grid."<<std::endl;
  int pos_count=0;
  for(uint64_t i=1;i<=num;i++){
    std::cout<<"Creating ENB "<<i<<" at ("<<x<<", "<<y<<", "<<z<<")"<<std::endl;
    
	pos_count++;
    lpa->Add (Vector (x,y,z));
	
	if(pos_count==gridxy){
		x = mindim;
		y = y+gridbreak;
		pos_count=0;
	}else{
		x = x+gridbreak;
	}
    //z = abs((rand() % diff)+mindim);
  }
  return lpa;
}

/*NodeContainer makeUE(NodeContainer nodes, uint64_t num, int32_t mindim, int32_t maxx, int32_t maxy, int32_t maxz, int32_t maxxvel, int32_t maxyvel, int32_t maxzvel)
{
  int32_t diff = maxx-mindim;
  int32_t x;
  int32_t y;
  int32_t z;
  int32_t xvel;
  int32_t yvel;
  int32_t zvel;
  
  std::cout<<"Creating "<<num<<" UE in Simulation"<<std::endl;
  for(uint32_t i=0; i<num; i++){
    x = (rand() % diff)+mindim;
	y = (rand() % diff)+mindim;
    //z = abs((rand() % diff)+mindim);
	z=0;
	xvel = (rand()%(maxxvel*2))-maxxvel;
	yvel = (rand()%(maxyvel*2))-maxyvel;
	//zvel = (rand()%maxzvel);
	zvel=0;
	
	std::cout<<"Creating UE "<<i<<" at [X: "<<x<<", Y: "<<y<<", Z: "<<z<<"]"<<" with velocity [X: "<<xvel<<", Y: "<<yvel<<", Z: "<<zvel<<"]"<<std::endl;
	
	Ptr<MobilityModel> ueMmPtr = (nodes.Get(i))->GetObject<MobilityModel>();
	ueMmPtr->SetPosition(Vector(x, y, z));
	Ptr<>nodes.Get (i)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (xvel, yvel, zvel));
  }
  
  int32_t x;
  int32_t y;
  int32_t z;
  int32_t xvel;
  int32_t yvel;
  int32_t zvel;
  for(uint64_t i=0;i<num;i++){
	x = (rand() % diff)+mindim;
	y = (rand() % diff)+mindim;
    z = abs((rand() % diff)+mindim);
	xvel = (rand()%(maxxvel*2))-maxxvel;
	yvel = (rand()%(maxyvel*2))-maxyvel;
	zvel = (rand()%maxzvel);
    std::cout<<"Creating UE "<<i<<" at ("<<x<<", "<<y<<", "<<z<<") with velocity ("<<xvel<<", "<<yvel<<", "<<zvel<<")"<<std::endl;
  
    nodes.Get (i)->GetObject<MobilityModel> ()->SetPosition (Vector (x,y,z));
    nodes.Get (i)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (xvel, yvel, zvel));
  }
  
  return nodes;
}*/

void checkSimulation(double simTime)
{
   //std::ofstream outfile("testDYLANpatienceHELP.txt");
   //outfile<<"FILLED WITH NONSENSE"<<std::endl;
   //outfile.close();
   // read from the input file
   std::ifstream inputFile("mmwaveStopFile.txt");
   std::string line;

   // if the file exists and is open
   if ( inputFile.is_open() )
   {
       // get the first file from the file
       std::getline(inputFile, line);

       // output a print line for us to review
       std::cout << "At real time " << std::chrono::duration<double, std::milli>(std::chrono::system_clock::now()-startTime).count()/1000    << " s and simulation time " << simTime << " s the content of the file is " << line <<  "\n";

       // if the input from the file is 1
       if ( line == "1" )
       {
           // output a priont line for us to review
           std::cout << "Kill the clients at " << simTime << "\n";
           // stop the simulation
           Simulator::Stop(Simulator::Now());
       }
       // close the file
       inputFile.close();

   }
   // if the file does not exist or can not be opened
   else
   {
       std::cout << "File not opening" <<  "\n";
   }
}

void makeLogs(uint32_t numUe)
{
	uint32_t numLogFiles = 1;
	std::vector <std::string> logFileNames;
	logFileNames.push_back("mmwavePositions");
	std::vector <std::string> fieldNames;
	fieldNames.push_back("Time,UE,CurXPos,CurYPos,CurZPos,CurXVel,CurYVel,CurZVel");
	
	std::stringstream combine;
	for(uint32_t ue =0;ue<numUe;ue++){
		for(uint32_t i=0;i<numLogFiles;i++){
			std::ofstream logFile;
			combine<<logFileNames[i]<<ue<<".csv";
			logFile.open(combine.str());
			logFile<<fieldNames[i]<<"\n";
			combine.str(std::string());
		}
	}
}

void logDetails(double curSimTime, uint32_t num, NodeContainer nodes, Ptr<MmWaveHelper> helper)
{
	std::cout<<"SimTime: "<<curSimTime<<std::endl;
	std::ofstream outFile;
	std::stringstream combine;
	//Ptr<MmWaveDlCqiMessage> ctrlmsg = (nodes.Get(0))->GetObject<MmWaveDlCqiMessage>();
	//std::cout<<"CQI???:"<<GetDlCqi()<<std::endl;
	for(uint32_t i=0; i<num; i++)
	{	
		Ptr<Node> node = nodes.Get(i);
		Ptr<MobilityModel> ueMmPtr = node->GetObject<MobilityModel>();
		ns3::Vector xyzpositions = ueMmPtr->GetPosition();
		ns3::Vector xyzvelocities = ueMmPtr->GetVelocity();

		//uint32_t nDevs = node->GetNDevices();
		//Ptr<NetDevice> uedev= node->GetDevice(nDevs-1);
		//Ptr<MmWaveAmc> amc = CreateObject<MmWaveAmc>(helper->GetCcPhyParams().at(0).GetConfigurationParameters());
		
		//se = log2 ( 1 + ( sinr_ / ( (-std::log (5.0 * m_ber )) / 1.5) ));
		//cqi = amc->GetCqiFromSpectralEfficiency(se)
		
		//std::cout<<"AMC: "<<amc<<std::endl;
		combine<<"mmwavePositions"<<i<<".csv";
		outFile.open(combine.str(),std::ios_base::app);
		if(outFile.is_open())
		{
			outFile<<curSimTime<<","<<i<<","<<xyzpositions.x<<","<<xyzpositions.y<<","<<xyzpositions.z<<","<<xyzvelocities.x<<","<<xyzvelocities.y<<","<<xyzvelocities.z<<"\n";
		}
		combine.str(std::string());
		outFile.close();
	}
	outFile.open("timelog.txt",std::ios_base::app);
	if(outFile.is_open()){
		outFile<<curSimTime<<"\n";
	}
	outFile.close();
}

//Main
int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  srand(time(NULL));
  
  UintegerValue uintegerValue;
  IntegerValue integerValue;
  DoubleValue doubleValue;
  BooleanValue booleanValue;
  StringValue stringValue;
  
  GlobalValue::GetValueByName ("simTime", doubleValue);
  double simTime = doubleValue.Get ();
  
  GlobalValue::GetValueByName ("interval", doubleValue);
  double interval = doubleValue.Get ();
  
  GlobalValue::GetValueByName ("numEnb", uintegerValue);
  uint64_t numEnb = uintegerValue.Get ();
  
  GlobalValue::GetValueByName ("numUe", uintegerValue);
  uint64_t numUe = uintegerValue.Get ();
  
  int32_t mindim = 0;
  
  GlobalValue::GetValueByName ("maxX", integerValue);
  int32_t maxX = integerValue.Get ();
  
  GlobalValue::GetValueByName ("maxY", integerValue);
  int32_t maxY = integerValue.Get ();
  
  GlobalValue::GetValueByName ("maxZ", integerValue);
  int32_t maxZ = integerValue.Get ();
  
  /*GlobalValue::GetValueByName ("maxXVel", integerValue);
  int32_t maxXVel = integerValue.Get ();
  
  GlobalValue::GetValueByName ("maxYVel", integerValue);
  int32_t maxYVel = integerValue.Get ();
  
  GlobalValue::GetValueByName ("maxZVel", integerValue);
  int32_t maxZVel = integerValue.Get ();*/
  
  //Use this block to avoid unused variable warnings/errors
  //std::cout<<numEnb<<numUe<<std::endl;

  /*LogComponentEnable ("LteRlcAm", LOG_LEVEL_LOGIC);
  LogComponentEnable("MmWaveBearerStatsConnector",LOG_LEVEL_INFO);
  LogComponentEnable ("MmWaveSpectrumPhy", LOG_LEVEL_INFO);
  LogComponentEnable ("MmWaveEnbPhy", LOG_LEVEL_INFO);
  LogComponentEnable ("MmWavePhy", LOG_LEVEL_INFO);

  LogComponentEnable ("MmWaveUeMac", LOG_LEVEL_INFO);
  LogComponentEnable ("MmWaveEnbMac", LOG_LEVEL_INFO);
  //LogComponentEnable ("MmWaveRxMacScheduler", LOG_LEVEL_INFO);

  LogComponentEnable ("LteUeRrc", LOG_LEVEL_ALL);
  LogComponentEnable ("LteEnbRrc", LOG_LEVEL_ALL);
  LogComponentEnable("PropagationLossModel",LOG_LEVEL_ALL);
  LogComponentEnable("mmWaveInterference",LOG_LEVEL_ALL);
  LogComponentEnable("MmWaveBeamforming",LOG_LEVEL_ALL);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  */
  //LogComponentEnable ("MmWaveUePhy", LOG_LEVEL_INFO);
  //LogComponentEnable ("LteUePhy", LOG_LEVEL_INFO);
  //LogComponentEnable ("mmWaveControlMessage", LOG_LEVEL_INFO);
  LogComponentEnable ("MmWaveAmc", LOG_LEVEL_INFO);
  
  std::cout<<"NumUE: "<<numUe<<std::endl;

  Ptr<MmWaveHelper> ptr_mmWave = CreateObject<MmWaveHelper> ();
  Ptr<MmWaveBearerStatsConnector> ptr_stats = CreateObject<MmWaveBearerStatsConnector>();
  ptr_mmWave->SetAttribute ("PathlossModel", StringValue ("ns3::MmWavePropagationLossModel"));
  ptr_mmWave->Initialize ();
  
  //Make ENBs
  NodeContainer enbNodes;
  enbNodes.Create (numEnb);
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  MobilityHelper enbmobility;
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbPositionAlloc = makeENB(numEnb,mindim,maxX, maxY, maxZ, enbPositionAlloc);//////////
  enbmobility.SetPositionAllocator(enbPositionAlloc);
  //enbmobility.SetPositionAllocator ("ns3::GridPositionAllocator","MinX", DoubleValue(mindim),"GridWidth", );
  enbmobility.Install (enbNodes);
  
  //Make UEs
  NodeContainer ueNodes;
  ueNodes.Create (numUe);
  MobilityHelper uemobility;
  //int boxBound=150;
  uemobility.SetMobilityModel ("ns3::GaussMarkovMobilityModel","Bounds", BoxValue (Box (0, maxX, 0, maxY, 0, maxZ)),"TimeStep", TimeValue (Seconds (0.5)),"Alpha", DoubleValue (0.85), "MeanVelocity", StringValue ("ns3::ConstantRandomVariable[Constant=8.33]"), "MeanDirection", StringValue ("ns3::UniformRandomVariable[Min=0|Max=6.283185307]"), "MeanPitch", StringValue ("ns3::UniformRandomVariable[Min=0.005|Max=0.005]"), "NormalVelocity", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=1.0|Bound=12.0]"), "NormalDirection", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.2|Bound=3.14]"), "NormalPitch", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.0002|Bound=0.0004]"));
  uemobility.Install (ueNodes);
  
  uemobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator",
  "X", StringValue ("ns3::UniformRandomVariable[Min=0|Max="+std::to_string(maxX)+"]"),
  "Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max="+std::to_string(maxY)+"]"),
  "Z", StringValue ("ns3::UniformRandomVariable[Min=0|Max="+std::to_string(maxZ)+"]"));
  
  //ueNodes = makeUE(ueNodes, numUe, mindim, maxX, maxY, maxZ, maxXVel, maxYVel, maxZVel);

  BuildingsHelper::Install (enbNodes);
  BuildingsHelper::Install (ueNodes);

  NetDeviceContainer enbNetDev = ptr_mmWave->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueNetDev = ptr_mmWave->InstallUeDevice (ueNodes);


  ptr_mmWave->AttachToClosestEnb (ueNetDev, enbNetDev);
  ptr_mmWave->EnableTraces ();
  
  //Activate X2 Interface ~ Requires EPC
  //Ptr<MmWavePointToPointEpcHelper> epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  //ptr_mmWave->SetEpcHelper(epcHelper);
  //ptr_mmWave->AddX2Interface(enbNodes);
  // Activate a data radio bearer
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  ptr_mmWave->ActivateDataRadioBearer (ueNetDev, bearer);
  BuildingsHelper::MakeMobilityModelConsistent ();

  //Config::SetDefault ("ns3::UdpClient::Interval", TimeValue (NanoSeconds (2500000)));
  if(simTime!=-1){
	Simulator::Stop (Seconds (simTime));  
  }
  makeLogs(numUe);
  for (uint32_t u = 1; u <= simTime; ++u) {
    // stop the simulator
    Simulator::Schedule ( Seconds(u), &checkSimulation, u );
  }
  
  for (double i = 0; i <= simTime; i=i+interval) {
    // log positions every interval
    Simulator::Schedule ( Seconds(i), &logDetails, i, numUe, ueNodes, ptr_mmWave);
  }
  startTime = std::chrono::system_clock::now();
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}