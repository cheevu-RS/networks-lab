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
 *
 * Author: George F. Riley<riley@ece.gatech.edu>
 */

#include <iostream>

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

int main(int argc, char *argv[]) {
  Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(100));
  Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("500kb/s"));

  uint32_t nLeftLeaf = 4;
  uint32_t nRightLeaf = 2;
  uint32_t nLeaf = 0; // If non-zero, number of both left and right
  std::string animFile =
      "dumbbell-animation.xml"; // Name of file for animation output

  CommandLine cmd;
  cmd.AddValue("nLeftLeaf", "Number of left side leaf nodes", nLeftLeaf);
  cmd.AddValue("nRightLeaf", "Number of right side leaf nodes", nRightLeaf);
  cmd.AddValue("nLeaf", "Number of left and right side leaf nodes", nLeaf);
  cmd.AddValue("animFile", "File Name for Animation Output", animFile);

  cmd.Parse(argc, argv);
  if (nLeaf > 0) {
    nLeftLeaf = nLeaf;
    nRightLeaf = nLeaf;
  }

  // Create the point-to-point link helpers
  PointToPointHelper pointToPointRouter;
  pointToPointRouter.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
  pointToPointRouter.SetChannelAttribute("Delay", StringValue("1ms"));
  PointToPointHelper pointToPointLeaf;
  pointToPointLeaf.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
  pointToPointLeaf.SetChannelAttribute("Delay", StringValue("1ms"));

  PointToPointDumbbellHelper d(nLeftLeaf, pointToPointLeaf, nRightLeaf,
                               pointToPointLeaf, pointToPointRouter);

  // Install Stack
  InternetStackHelper stack;
  d.InstallStack(stack);

  // Assign IP Addresses
  d.AssignIpv4Addresses(Ipv4AddressHelper("10.1.1.0", "255.255.255.0"),
                        Ipv4AddressHelper("10.2.1.0", "255.255.255.0"),
                        Ipv4AddressHelper("10.3.1.0", "255.255.255.0"));

  // udp - node 2,3

  // Install on/off app on all right side nodes
  OnOffHelper clientUDPHelper("ns3::UdpSocketFactory", Address());
  // clientUDPHelper.SetAttribute("OnTime",zs3::UniformRandomVariable"));
  ApplicationContainer clientUDPApps;
  AddressValue remoteAddress(InetSocketAddress(d.GetRightIpv4Address(1), 1000));
  clientUDPHelper.SetAttribute("Remote", remoteAddress);
  clientUDPApps.Add(clientUDPHelper.Install(d.GetLeft(2)));
  clientUDPApps.Add(clientUDPHelper.Install(d.GetLeft(3)));

  UdpEchoServerHelper udpServer(1000);
  clientUDPApps.Add(udpServer.Install(d.GetRight(1)));

  // for (uint32_t i = 4; i < 6; ++i) {
  //   // Create an on/off app sending packets to the same leaf right side
  // }

  clientUDPApps.Start(Seconds(5.0));
  clientUDPApps.Stop(Seconds(10.0));

  // Install on/off app on all left side nodes
  OnOffHelper clientTCPHelper("ns3::TcpSocketFactory", Address());
  // clientTCPHelper.SetAttribute("OnTime",
  //                              StringValue("ns3::UniformRandomVariable"));
  // clientTCPHelper.SetAttribute("OffTime",
  //                              StringValue("ns3::UniformRandomVariable"));
  ApplicationContainer clientTCPApps;

  AddressValue remoteAddress1(
      InetSocketAddress(d.GetRightIpv4Address(0), 1000));
  clientTCPHelper.SetAttribute("Remote", remoteAddress1);
  clientTCPApps.Add(clientTCPHelper.Install(d.GetLeft(0)));
  clientTCPApps.Add(clientTCPHelper.Install(d.GetLeft(1)));

  // for (uint32_t i = 2; i < 4; ++i) {
  //   // Create an on/off app sending packets to the same leaf right side
  // }

  clientTCPApps.Start(Seconds(1.0));
  clientTCPApps.Stop(Seconds(10.0));

  // Set the bounding box for animation
  d.BoundingBox(1, 1, 100, 100);

  // Create the animation object and configure for specified output
  AnimationInterface anim(animFile);
  anim.EnablePacketMetadata(); // Optional
  // anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(10)); // Optional

  // Set up the actual simulation
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Simulator::Run();
  std::cout << "Animation Trace file created:" << animFile.c_str() << std::endl;
  Simulator::Destroy();
  return 0;
}
