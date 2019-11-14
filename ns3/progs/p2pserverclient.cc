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
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FirstScriptExample");

int main(int argc, char *argv[]) {
  CommandLine cmd;
  cmd.Parse(argc, argv);

  Time::SetResolution(Time::NS);
  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create(3);
  NodeContainer n0n1 = NodeContainer(nodes.Get(0), nodes.Get(1));
  NodeContainer n0n2 = NodeContainer(nodes.Get(0), nodes.Get(2));

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer devices01, devices02;
  devices01 = pointToPoint.Install(n0n1);
  devices02 = pointToPoint.Install(n0n2);
  InternetStackHelper stack;
  stack.Install(nodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces01 = address.Assign(devices01);
  address.SetBase("10.2.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces02 = address.Assign(devices02);

  UdpEchoServerHelper echoServer(9);

  ApplicationContainer serverApps = echoServer.Install(nodes.Get(0));
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(100.0));

  // std::cout << interfaces01.GetAddress(0) << std::endl;
  // std::cout << interfaces01.GetAddress(1) << std::endl;
  // std::cout << interfaces02.GetAddress(0) << std::endl;
  // std::cout << interfaces02.GetAddress(1) << std::endl;
  // client 1
  UdpEchoClientHelper echoClient0(interfaces01.GetAddress(0), 9);
  echoClient0.SetAttribute("MaxPackets", UintegerValue(1));
  echoClient0.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  echoClient0.SetAttribute("PacketSize", UintegerValue(1024));

  ApplicationContainer clientApps0 = echoClient0.Install(nodes.Get(1));
  clientApps0.Start(Seconds(3.0));
  clientApps0.Stop(Seconds(100.0));

  // client 2
  UdpEchoClientHelper echoClient2(interfaces02.GetAddress(0), 9);
  echoClient2.SetAttribute("MaxPackets", UintegerValue(1));
  echoClient2.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  echoClient2.SetAttribute("PacketSize", UintegerValue(1024));

  ApplicationContainer clientApps2 = echoClient2.Install(nodes.Get(2));
  clientApps2.Start(Seconds(4.0));
  clientApps2.Stop(Seconds(100.0));

  AnimationInterface anim("p2p.xml");
  anim.SetConstantPosition(nodes.Get(0), 100, 100);
  anim.SetConstantPosition(nodes.Get(1), 20, 200);
  anim.SetConstantPosition(nodes.Get(2), 20, 0);
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
