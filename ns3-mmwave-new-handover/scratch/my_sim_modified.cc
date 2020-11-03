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
#include "ns3/epc-helper.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
//#include "ns3/gtk-config-store.h"
#include <ns3/buildings-helper.h>
#include <ns3/random-variable-stream.h>
#include <ns3/lte-ue-net-device.h>
#include <iostream>
#include <ctime>
#include <stdlib.h>
#include <list>
#include <ns3/flow-monitor-module.h>

using namespace ns3;
using namespace mmwave;
NS_LOG_COMPONENT_DEFINE("MmWaveConfigurableSimulation");

//Custom Global Variables
std::chrono::system_clock::time_point startTime;
std::chrono::system_clock::time_point endTime;
static ns3::GlobalValue g_x2Latency ("x2Latency", "Latency on X2 interface (us)",
                                     ns3::DoubleValue (500), ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_mmeLatency ("mmeLatency", "Latency on MME interface (us)",
                                      ns3::DoubleValue (10000), ns3::MakeDoubleChecker<double> ());


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

bool stopSent = false;


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
  std::cout<<"ENB will be layed out on a "<<gridxy<<" * "<<gridxy<<" grid (With Random Elevations)."<<std::endl;
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

NodeContainer makeUE(NodeContainer nodes, uint64_t num, int32_t mindim, int32_t maxx, int32_t maxy, int32_t maxz, int32_t maxxvel, int32_t maxyvel, int32_t maxzvel)
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
}

void checkSimulation(double simTime, double interval)
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
	   std::cout << "Stopfile Line Is: " << line <<std::endl;
       if ( !::stopSent && line == "1" )
       {
           // output a priont line for us to review
           std::cout << "Kill the clients at " << simTime << "\n";
           // stop the simulation
           Simulator::Stop(Seconds(simTime+interval));
		   ::stopSent = true;
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
	fieldNames.push_back("Time,UE,CurXPos,CurYPos,CurZPos,CurXVel,CurYVel,CurZVel,CalcClosestENB,CalcClosestEnbDist,enbX,enbY,enbZ");

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

void logDetails(double curSimTime, uint32_t num, uint32_t eNum, NodeContainer nodes, NodeContainer eNodes, Ptr<MmWaveHelper> helper)
{
	std::cout<<"SimTime: "<<curSimTime<<std::endl;
	std::ofstream outFile;
	std::stringstream combine;
	std::time_t realtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
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

		double x = 0;
		double y = 0;
		double z = 0;
		int closestEnb = 0;
		double closestDist=0;
		double newDist=0;
		for(uint32_t j=0; j<eNum; j++){
			Ptr<Node> enb = eNodes.Get(j);
			Ptr<MobilityModel> eMmPtr = enb->GetObject<MobilityModel>();
			ns3::Vector enbxyz = eMmPtr->GetPosition();
			if(j==0){
				x = enbxyz.x;
				y = enbxyz.y;
				z = enbxyz.z;
				closestEnb = j;
				closestDist = sqrt(pow(x-xyzpositions.x,2)+pow(y-xyzpositions.y,2)+pow(z-xyzpositions.z,2));
			}else{
				newDist = sqrt(pow(enbxyz.x-xyzpositions.x,2)+pow(enbxyz.y-xyzpositions.y,2)+pow(enbxyz.z-xyzpositions.z,2));
				if(newDist<closestDist){
					std::cout<<"UE"<<i<<" is closer to ENB"<<j<<"("<<newDist<<") than ENB"<<closestEnb<<"("<<closestDist<<")"<<std::endl;
					closestEnb = j;
					closestDist = newDist;
					x = enbxyz.x;
					y = enbxyz.y;
					z = enbxyz.z;
				}
			}
		}

		if(outFile.is_open())
		{
			outFile<<curSimTime<<","<<i<<","<<xyzpositions.x<<","<<xyzpositions.y<<","<<xyzpositions.z<<","<<xyzvelocities.x<<","<<xyzvelocities.y<<","<<xyzvelocities.z<<","<<closestEnb<<","<<closestDist<<","<<x<<","<<y<<","<<z<<"\n";
		}
		combine.str(std::string());
		outFile.close();
	}
	outFile.open("timelog.txt",std::ios_base::app);
	if(outFile.is_open()){
		outFile<<"\n\n"<<std::ctime(&realtime)<<curSimTime<<"\n";
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

  GlobalValue::GetValueByName ("x2Latency", doubleValue);
  double x2Latency = doubleValue.Get ();
  GlobalValue::GetValueByName ("mmeLatency", doubleValue);
  double mmeLatency = doubleValue.Get ();

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

  GlobalValue::GetValueByName ("maxXVel", integerValue);
  int32_t maxXVel = integerValue.Get ();

  GlobalValue::GetValueByName ("maxYVel", integerValue);
  int32_t maxYVel = integerValue.Get ();

  GlobalValue::GetValueByName ("maxZVel", integerValue);
  int32_t maxZVel = integerValue.Get ();

  //Use this block to avoid unused variable warnings/errors
  //std::cout<<numEnb<<numUe<<std::endl;

  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::X2LinkDelay", TimeValue (MicroSeconds (x2Latency)));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::X2LinkDataRate", DataRateValue (DataRate ("1000Gb/s")));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::X2LinkMtu",  UintegerValue (10000));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::S1uLinkDelay", TimeValue (MicroSeconds (1000)));
  Config::SetDefault ("ns3::MmWavePointToPointEpcHelper::S1apLinkDelay", TimeValue (MicroSeconds (mmeLatency)));

  Config::SetDefault ("ns3::LteEnbRrc::FixedTttValue", UintegerValue (150));
  Config::SetDefault ("ns3::LteEnbRrc::CrtPeriod", IntegerValue (1600));



  LogComponentEnable("MmWaveBearerStatsConnector",LOG_LEVEL_INFO);
  LogComponentEnable ("MmWaveSpectrumPhy", LOG_LEVEL_INFO);
  LogComponentEnable ("MmWaveEnbPhy", LOG_LEVEL_INFO);
  LogComponentEnable ("MmWavePhy", LOG_LEVEL_INFO);

  LogComponentEnable ("MmWaveUeMac", LOG_LEVEL_INFO);
  LogComponentEnable ("MmWaveEnbMac", LOG_LEVEL_INFO);
  //LogComponentEnable ("MmWaveRxMacScheduler", LOG_LEVEL_INFO);

  LogComponentEnable("MmWaveBeamforming",LOG_LEVEL_ALL);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  LogComponentEnable ("MmWaveUePhy", LOG_LEVEL_INFO);
  LogComponentEnable ("mmWaveControlMessage", LOG_LEVEL_INFO);
  LogComponentEnable ("MmWaveAmc", LOG_LEVEL_INFO);

  std::cout<<"NumUE: "<<numUe<<std::endl;

  Ptr<MmWaveHelper> ptr_mmWave = CreateObject<MmWaveHelper> ();
  Ptr<MmWaveBearerStatsConnector> ptr_stats = CreateObject<MmWaveBearerStatsConnector>();
  ptr_mmWave->SetAttribute ("PathlossModel", StringValue ("ns3::MmWavePropagationLossModel"));








  Ptr<MmWavePointToPointEpcHelper> epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
  ptr_mmWave->SetEpcHelper (epcHelper);







  ptr_mmWave->Initialize ();











  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet by connecting remoteHost to pgw. Setup routing too
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (2500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);










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
  std::ostringstream maxVeloc;
  maxVeloc << std::sqrt(maxXVel*maxXVel + maxYVel*maxYVel + maxZVel*maxZVel);
  std::string mVel = "ns3::UniformRandomVariable[Min=0|Max=" + maxVeloc.str() + "]";
  uemobility.SetMobilityModel ("ns3::GaussMarkovMobilityModel","Bounds", BoxValue (Box (0, maxX, 0, maxY, 0, maxZ)),"TimeStep", TimeValue (Seconds (0.5)),"Alpha", DoubleValue (0.85), "MeanVelocity", StringValue ("ns3::ConstantRandomVariable[Constant=8.33]"), "MeanDirection", StringValue ("ns3::UniformRandomVariable[Min=0|Max=6.283185307]"), "MeanPitch", StringValue ("ns3::UniformRandomVariable[Min=0.005|Max=0.005]"), "NormalVelocity", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=1.0|Bound=12.0]"), "NormalDirection", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.2|Bound=3.14]"), "NormalPitch", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.0002|Bound=0.0004]"));

  uemobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator",
  "X", StringValue ("ns3::UniformRandomVariable[Min=0|Max="+std::to_string(maxX)+"]"),
  "Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max="+std::to_string(maxY)+"]"),
  "Z", StringValue ("ns3::UniformRandomVariable[Min=0|Max="+std::to_string(maxZ)+"]"));

  uemobility.Install (ueNodes);

  //ueNodes = makeUE(ueNodes, numUe, mindim, maxX, maxY, maxZ, maxXVel, maxYVel, maxZVel);

  BuildingsHelper::Install (enbNodes);
  BuildingsHelper::Install (ueNodes);

  NetDeviceContainer enbNetDev = ptr_mmWave->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueNetDev = ptr_mmWave->InstallUeDevice (ueNodes);

  //Config::SetDefault ("ns3::UdpClient::Interval", TimeValue (NanoSeconds (2500000)));
  makeLogs(numUe);
  std::cout<<"SimTime: "<<simTime<<std::endl;
  if(simTime>0.0001){
	Simulator::Stop (Seconds (simTime));
    for (double i = 0; i <= simTime; i=i+interval) {
      // log positions every interval
      Simulator::Schedule ( Seconds(i), &logDetails, i, numUe, numEnb, ueNodes, enbNodes, ptr_mmWave);
      Simulator::Schedule ( Seconds(i), &checkSimulation, i, interval );
    }
  }
  else{
	for (double i = 0; i <= 9999; i=i+interval) {
      // log positions every interval
      Simulator::Schedule ( Seconds(i), &logDetails, i, numUe, numEnb, ueNodes, enbNodes, ptr_mmWave);
      Simulator::Schedule ( Seconds(i), &checkSimulation, i, interval );
    }
  }










  //NetDeviceContainer mmWaveEnbDevs = ptr_mmWave->InstallEnbDevice (enbNodes);
  //NetDeviceContainer mcUeDevs;
  //mcUeDevs = ptr_mmWave->InstallMcUeDevice (ueNodes);
  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }


  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  bool dl = 1;
  bool ul = 0;

  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      if (dl)
        {
          UdpServerHelper dlPacketSinkHelper (dlPort);
          dlPacketSinkHelper.SetAttribute ("PacketWindowSize", UintegerValue (256));
          serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (u)));

          // Simulator::Schedule(MilliSeconds(20), &PrintLostUdpPackets, DynamicCast<UdpServer>(serverApps.Get(serverApps.GetN()-1)), lostFilename);

          UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
          dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds (interval*100000)));
          dlClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
          clientApps.Add (dlClient.Install (remoteHost));

        }
      if (ul)
        {
          ++ulPort;
          PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
          ulPacketSinkHelper.SetAttribute ("PacketWindowSize", UintegerValue (256));
          serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
          UdpClientHelper ulClient (remoteHostAddr, ulPort);
          ulClient.SetAttribute ("Interval", TimeValue (MicroSeconds (interval*100000)));
          ulClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
          clientApps.Add (ulClient.Install (ueNodes.Get (u)));
        }
    }


  serverApps.Start (Seconds (0.00001));
  clientApps.Start (Seconds (0.00001));
  //clientApps.Stop (Seconds (simTime - 1));


  //Activate X2 Interface ~ Requires EPC
  ptr_mmWave->AddX2Interface(enbNodes);
  ptr_mmWave->AttachToClosestEnb (ueNetDev, enbNetDev);
  ptr_mmWave->SetLteHandoverAlgorithmType("ns3::A2A4RsrqHandoverAlgorithm");
  ptr_mmWave->EnableTraces ();

  // Activate a data radio bearer
  //enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  //EpsBearer bearer (q);
  //ptr_mmWave->ActivateDataRadioBearer (ueNetDev, bearer);
  BuildingsHelper::MakeMobilityModelConsistent ();


  Ptr<FlowMonitor> monitor;
  FlowMonitorHelper flowmon;
  monitor = flowmon.InstallAll();

  startTime = std::chrono::system_clock::now();
  Simulator::Run ();

  monitor->CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::chrono::seconds sec(1);
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  /*for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i){
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if (t.sourceAddress=="1.0.0.2" & t.destinationAddress=="7.0.0.2" )
      {
    	  NS_LOG_DEBUG ("Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")");
    	  if (i->second.rxPackets > 0){
			  std::cout <<"Delay Down = " << i->second.delaySum.GetSeconds() / i->second.rxPackets << "\n";
          }
      }
      if (t.sourceAddress=="7.0.0.2" & t.destinationAddress=="1.0.0.2" )
      {
    	  NS_LOG_DEBUG ("Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")");
    	  if (i->second.rxPackets > 0){
			  std::cout <<"Delay Up = " << i->second.delaySum.GetSeconds() / i->second.rxPackets <<"\n";
          }
      }
  }*/

  Simulator::Destroy ();
  endTime = std::chrono::system_clock::now();
  std::cout<<"Execution Time: "<<difftime(std::chrono::system_clock::to_time_t(endTime),std::chrono::system_clock::to_time_t(startTime))<<std::endl;
  return 0;
}
