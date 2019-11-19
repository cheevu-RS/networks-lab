/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"

// Default Network Topology
//
// Number of wifi or csma nodes can be increased up to 250
//                          |
//                 Rank 0   |   Rank 1
// -------------------------|----------------------------
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThirdScriptExample");

int main(int argc, char *argv[]) {
  bool verbose = true;
  uint32_t nCsma = 4;
  uint32_t nWifi = 4;
  bool tracing = false;

  CommandLine cmd;
  cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue("tracing", "Enable pcap tracing", tracing);

  cmd.Parse(argc, argv);

  // Check for valid number of csma or wifi nodes
  // 250 should be enough, otherwise IP addresses
  // soon become an issue
  if (nWifi > 250 || nCsma > 250) {
    std::cout << "Too many wifi or csma nodes, no more than 250 each."
              << std::endl;
    return 1;
  }

  if (verbose) {
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
  }
  NodeContainer p2pNodes;
  p2pNodes.Create(5);
  NodeContainer p2pNodes01, p2pNodes12, p2pNodes23, p2pNodes34, p2pNodes41;
  // NodeContainer n0n1 = NodeContainer(nodes.Get(0), nodes.Get(1));
  p2pNodes01 = NodeContainer(p2pNodes.Get(0), p2pNodes.Get(1));
  p2pNodes12 = NodeContainer(p2pNodes.Get(1), p2pNodes.Get(2));
  p2pNodes23 = NodeContainer(p2pNodes.Get(2), p2pNodes.Get(3));
  p2pNodes34 = NodeContainer(p2pNodes.Get(3), p2pNodes.Get(4));
  p2pNodes41 = NodeContainer(p2pNodes.Get(4), p2pNodes.Get(1));

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("1.5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer p2pDevices01, p2pDevices12, p2pDevices23, p2pDevices34,
      p2pDevices41;
  p2pDevices01 = pointToPoint.Install(p2pNodes01);
  p2pDevices12 = pointToPoint.Install(p2pNodes12);
  p2pDevices23 = pointToPoint.Install(p2pNodes23);
  p2pDevices34 = pointToPoint.Install(p2pNodes34);
  p2pDevices41 = pointToPoint.Install(p2pNodes41);

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create(nWifi);
  NodeContainer wifiApNode = p2pNodes01.Get(0);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
  phy.SetChannel(channel.Create());

  WifiHelper wifi;
  wifi.SetRemoteStationManager("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid("ns-3-ssid");
  mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing",
              BooleanValue(false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install(phy, mac, wifiStaNodes);

  mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install(phy, mac, wifiApNode);

  MobilityHelper mobility;

  // mobility.SetPositionAllocator(
  //     "ns3::GridPositionAllocator", "MinX", DoubleValue(0.0), "MinY",
  //     DoubleValue(0.0), "DeltaX", DoubleValue(5.0), "DeltaY",
  //     DoubleValue(10.0), "GridWidth", UintegerValue(3), "LayoutType",
  //     StringValue("RowFirst"));

  // mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Bounds",
  //                           RectangleValue(Rectangle(-50, 50, -25, 50)));

  mobility.Install(wifiStaNodes);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiApNode);
  NodeContainer new1 =
      NodeContainer(p2pNodes.Get(1), p2pNodes.Get(2), p2pNodes.Get(3));

  InternetStackHelper stack;

  stack.Install(p2pNodes);
  stack.Install(wifiStaNodes);
  Ipv4AddressHelper address;

  Ipv4InterfaceContainer p2pInterfaces01, p2pInterfaces12, p2pInterfaces23,
      p2pInterfaces34, p2pInterfaces41;
  address.SetBase("10.1.1.0", "255.255.255.0");
  p2pInterfaces01 = address.Assign(p2pDevices01);
  address.SetBase("10.1.2.0", "255.255.255.0");
  p2pInterfaces12 = address.Assign(p2pDevices12);
  address.SetBase("10.1.3.0", "255.255.255.0");
  p2pInterfaces23 = address.Assign(p2pDevices23);
  address.SetBase("10.1.4.0", "255.255.255.0");
  p2pInterfaces34 = address.Assign(p2pDevices34);
  address.SetBase("10.1.5.0", "255.255.255.0");
  p2pInterfaces41 = address.Assign(p2pDevices41);
  // Ipv4InterfaceContainer csmaInterfaces;
  // csmaInterfaces = address.Assign (csmaDevices);

  address.SetBase("10.1.6.0", "255.255.255.0");
  address.Assign(staDevices);
  address.Assign(apDevices);

  p2pInterfaces01.SetMetric(1, 100);
  p2pInterfaces01.SetMetric(0, 0);
  p2pInterfaces23.SetMetric(0, 2);
  p2pInterfaces34.SetMetric(0, 8);
  p2pInterfaces41.SetMetric(0, 10);

  // Create router nodes, initialize routing database and set up the routing
  // tables in the nodes.
  // Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  UdpEchoServerHelper echoServer(9);

  ApplicationContainer serverApps = echoServer.Install(p2pNodes.Get(4));
  serverApps.Start(Seconds(0.0));
  serverApps.Stop(Seconds(13.0));

  UdpEchoClientHelper echoClient(p2pInterfaces41.GetAddress(0), 9);
  echoClient.SetAttribute("MaxPackets", UintegerValue(1000));
  echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  echoClient.SetAttribute("PacketSize", UintegerValue(500));

  ApplicationContainer clientApps =
      echoClient.Install(wifiStaNodes.Get(nWifi - 1));
  clientApps.Start(Seconds(1.0));
  clientApps.Stop(Seconds(13.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  RipHelper routingHelper;

  Ptr<OutputStreamWrapper> routingStream =
      Create<OutputStreamWrapper>(&std::cout);

  routingHelper.PrintRoutingTableAt(Seconds(7.0), p2pNodes.Get(0),
                                    routingStream);
  routingHelper.PrintRoutingTableAt(Seconds(7.0), p2pNodes.Get(1),
                                    routingStream);
  routingHelper.PrintRoutingTableAt(Seconds(7.0), p2pNodes.Get(2),
                                    routingStream);
  routingHelper.PrintRoutingTableAt(Seconds(7.0), p2pNodes.Get(3),
                                    routingStream);
  routingHelper.PrintRoutingTableAt(Seconds(7.0), p2pNodes.Get(4),
                                    routingStream);

  routingHelper.PrintRoutingTableAt(Seconds(11.0), p2pNodes.Get(0),
                                    routingStream);
  routingHelper.PrintRoutingTableAt(Seconds(11.0), p2pNodes.Get(1),
                                    routingStream);
  routingHelper.PrintRoutingTableAt(Seconds(11.0), p2pNodes.Get(2),
                                    routingStream);
  routingHelper.PrintRoutingTableAt(Seconds(11.0), p2pNodes.Get(3),
                                    routingStream);
  routingHelper.PrintRoutingTableAt(Seconds(11.0), p2pNodes.Get(4),
                                    routingStream);

  Simulator::Stop(Seconds(13.0));

  // if (tracing == true)
  //   {
  pointToPoint.EnablePcapAll("third");
  //     phy.EnablePcap ("third", apDevices.Get (0));
  //     csma.EnablePcap ("third", csmaDevices.Get (0), true);
  //   }

  // breaking link n2-n3 at t=5s
  Ptr<Node> n7 = p2pNodes.Get(2);
  Ptr<Ipv4> n7i = n7->GetObject<Ipv4>();
  Simulator::Schedule(Seconds(5), &Ipv4::SetDown, n7i, 2);

  Ptr<Node> n8 = p2pNodes.Get(3);
  Ptr<Ipv4> n8i = n8->GetObject<Ipv4>();
  Simulator::Schedule(Seconds(5), &Ipv4::SetDown, n8i, 1);

  // breaking link n3-n4 at t=5s
  Simulator::Schedule(Seconds(7), &Ipv4::SetDown, n8i, 2);

  Ptr<Node> n6 = p2pNodes.Get(4);
  Ptr<Ipv4> n6i = n6->GetObject<Ipv4>();
  Simulator::Schedule(Seconds(7), &Ipv4::SetDown, n6i, 1);

  AnimationInterface anim("new3.xml");                     // Mandatory
  anim.EnablePacketMetadata(true);                         // Optional
  anim.SetConstantPosition(wifiApNode.Get(0), 0, 0);       // n0
  anim.SetConstantPosition(wifiStaNodes.Get(0), -50, 50);  // n5
  anim.SetConstantPosition(wifiStaNodes.Get(1), 50, 50);   // n6
  anim.SetConstantPosition(wifiStaNodes.Get(2), -50, -50); // n5
  anim.SetConstantPosition(wifiStaNodes.Get(3), 50, -50);  // n6
  anim.SetConstantPosition(p2pNodes.Get(1), 100, 0);       // n1
  anim.SetConstantPosition(p2pNodes.Get(2), 150, 0);       // n2
  anim.SetConstantPosition(p2pNodes.Get(3), 150, 50);      // n3
  anim.SetConstantPosition(p2pNodes.Get(4), 100, 50);      // n3
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
