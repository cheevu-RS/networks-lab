// Network topology
// //
// //             n0   r    n1-n2-n3-n4n-n5
// //             |    _    |
// //             ====|_|====
// //                router
// //
// // - Tracing of queues and packet receptions to file

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("oct23");

int main(int argc, char **argv) {
  NS_LOG_INFO("Create nodes.");
  NodeContainer p2pNodes;
  p2pNodes.Create(2);
  //   Ptr<Node> n0 = CreateObject<Node>();
  //   Ptr<Node> r = CreateObject<Node>();
  //   Ptr<Node> n1 = CreateObject<Node>();

  uint32_t nCsma = 5;
  NodeContainer csmaNodes;
  csmaNodes.Add(p2pNodes.Get(1));
  csmaNodes.Create(nCsma);

  //   NodeContainer net1(p2pNodes.Get(0), p2pNodes.Get(1));
  //   NodeContainer net2(p2pNodes.Get(1), p2pNodes.Get(2));
  //   NodeContainer all(n0, r, n1);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install(p2pNodes);

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install(csmaNodes);

  InternetStackHelper stack;
  stack.Install(p2pNodes.Get(0));
  stack.Install(csmaNodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign(p2pDevices);
  address.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign(csmaDevices);

  UdpEchoServerHelper echoServer(9);
  ApplicationContainer serverApps = echoServer.Install(p2pNodes.Get(0));
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(6.0));

  UdpEchoClientHelper echoClient(p2pInterfaces.GetAddress(0), 9);
  echoClient.SetAttribute("MaxPackets", UintegerValue(1));
  echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  echoClient.SetAttribute("PacketSize", UintegerValue(1024));

  ApplicationContainer clientApps = echoClient.Install(csmaNodes.Get(5));
  clientApps.Start(Seconds(4.0));
  clientApps.Stop(Seconds(6.0));

  //   ApplicationContainer clientApps1 = echoClient.Install(csmaNodes.Get(1));
  //   clientApps1.Start(Seconds(4.0));
  //   clientApps1.Stop(Seconds(6.0));

  // Install on/off app on all left side nodes
  OnOffHelper clientTCPHelper("ns3::TcpSocketFactory", Address());
  clientTCPHelper.SetAttribute("OnTime",
                               StringValue("ns3::UniformRandomVariable"));
  clientTCPHelper.SetAttribute("OffTime",
                               StringValue("ns3::UniformRandomVariable"));
  ApplicationContainer clientTCPApps;

  AddressValue remoteAddress1(
      InetSocketAddress(p2pInterfaces.GetAddress(0), 1000));
  clientTCPHelper.SetAttribute("Remote", remoteAddress1);
  clientTCPApps.Add(clientTCPHelper.Install(csmaNodes.Get(1)));
  //   clientTCPApps.Add(clientTCPHelper.Install(csmaNodes.Get(5));

  // for (uint32_t i = 2; i < 4; ++i) {
  //   // Create an on/off app sending packets to the same leaf right side
  // }

  clientTCPApps.Start(Seconds(2.0));
  clientTCPApps.Stop(Seconds(4.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  pointToPoint.EnablePcapAll("routing0");
  csma.EnablePcap("routing1", csmaDevices.Get(1), true);
  csma.EnablePcap("routing5", csmaDevices.Get(5), true);

  AnimationInterface anim("routing.xml");
  anim.EnablePacketMetadata(); // Optional
  anim.SetConstantPosition(p2pNodes.Get(0), 0, 0);
  //   anim.SetConstantPosition(p2pNodes.Get(1), 5, 5);
  anim.SetConstantPosition(csmaNodes.Get(0), 50, 0);
  anim.SetConstantPosition(csmaNodes.Get(1), 100, 100);
  anim.SetConstantPosition(csmaNodes.Get(2), 100, 50);
  anim.SetConstantPosition(csmaNodes.Get(3), 100, 0);
  anim.SetConstantPosition(csmaNodes.Get(4), 100, -50);
  anim.SetConstantPosition(csmaNodes.Get(5), 100, -100);

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("routing.tr"));

  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");
}
