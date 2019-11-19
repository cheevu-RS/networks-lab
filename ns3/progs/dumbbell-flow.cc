#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/gnuplot.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/point-to-point-module.h"
#include <fstream>
#include <iostream>
using namespace ns3;

Ptr<PacketSink> sink, sink1, sink2,
    sink3;                /* Pointer to the packet sink application */
uint64_t lastTotalRx = 0; /* The value of the last total received bytes */

void CalculateThroughput() {
  Time now = Simulator::Now(); /* Return the simulator's virtual time. */
  double cur = (sink->GetTotalRx() - lastTotalRx) * (double)8 /
               1e5; /* Convert Application RX Packets to MBits. */
  std::cout << now.GetSeconds() << "s: \t" << cur << " Mbit/s" << std::endl;
  lastTotalRx = sink->GetTotalRx();
  Simulator::Schedule(MilliSeconds(100), &CalculateThroughput);
}

void Create2DPlotFile() {
  std::string fileNameWithNoExtension = "plot-2d";
  std::string graphicsFileName = fileNameWithNoExtension + ".png";
  std::string plotFileName = fileNameWithNoExtension + ".plt";
  std::string plotTitle = "2-D Plot";
  std::string dataTitle = "2-D Data";

  // Instantiate the plot and set its title.
  Gnuplot plot(graphicsFileName);
  plot.SetTitle(plotTitle);

  // Make the graphics file, which the plot file will create when it
  // is used with Gnuplot, be a PNG file.
  plot.SetTerminal("png");

  // Set the labels for each axis.
  plot.SetLegend("X Values", "Y Values");

  // Set the range for the x axis.
  plot.AppendExtra("set xrange [-6:+6]");

  // Instantiate the dataset, set its title, and make the points be
  // plotted along with connecting lines.
  Gnuplot2dDataset dataset;
  dataset.SetTitle(dataTitle);
  dataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);

  double x;
  double y;

  // Create the 2-D dataset.
  for (x = -5.0; x <= +5.0; x += 1.0) {
    // Calculate the 2-D curve
    //
    //            2
    //     y  =  x   .
    //
    y = x * x;

    // Add this point.
    dataset.Add(x, y);
  }

  // Add the dataset to the plot.
  plot.AddDataset(dataset);

  // Open the plot file.
  std::ofstream plotFile(plotFileName.c_str());

  // Write the plot file.
  plot.GenerateOutput(plotFile);

  // Close the plot file.
  plotFile.close();
}

int main(int argc, char *argv[]) {
  Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(100));
  Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("500kb/s"));

  uint32_t nLeftLeaf = 4;
  uint32_t nRightLeaf = 2;
  uint32_t nLeaf = 0; // If non-zero, number of both left and right
  std::string animFile = "lab-1.xml";

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
  pointToPointRouter.SetChannelAttribute("Delay", StringValue("200ms"));

  PointToPointHelper pointToPointLeaf;
  pointToPointLeaf.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
  pointToPointLeaf.SetChannelAttribute("Delay", StringValue("200ms"));

  PointToPointDumbbellHelper dumbbell(nLeftLeaf, pointToPointLeaf, nRightLeaf,
                                      pointToPointLeaf, pointToPointRouter);

  // Install Stack
  InternetStackHelper stack;
  dumbbell.InstallStack(stack);

  // Assign IP Addresses
  dumbbell.AssignIpv4Addresses(Ipv4AddressHelper("10.1.1.0", "255.255.255.0"),
                               Ipv4AddressHelper("10.2.1.0", "255.255.255.0"),
                               Ipv4AddressHelper("10.3.1.0", "255.255.255.0"));

  const int PORT = 1000;
  // Create the OnOff applications to send TCP to the server
  OnOffHelper tcpClientHelper("ns3::TcpSocketFactory", Address());
  tcpClientHelper.SetAttribute("OnTime",
                               StringValue("ns3::UniformRandomVariable"));
  tcpClientHelper.SetAttribute("OffTime",
                               StringValue("ns3::UniformRandomVariable"));
  AddressValue tcpRemoteAddress(
      InetSocketAddress(dumbbell.GetRightIpv4Address(0), PORT));
  // packet size
  tcpClientHelper.SetAttribute("PacketSize", UintegerValue(256));

  tcpClientHelper.SetAttribute("Remote", tcpRemoteAddress);

  // Create the OnOff applications to send UDP to the server
  OnOffHelper udpClientHelper("ns3::UdpSocketFactory", Address());
  udpClientHelper.SetAttribute("OnTime",
                               StringValue("ns3::UniformRandomVariable"));
  udpClientHelper.SetAttribute("OffTime",
                               StringValue("ns3::UniformRandomVariable"));
  AddressValue udpRemoteAddress(
      InetSocketAddress(dumbbell.GetRightIpv4Address(1), PORT));
  // packet size
  udpClientHelper.SetAttribute("PacketSize", UintegerValue(256));

  udpClientHelper.SetAttribute("Remote", udpRemoteAddress);

  ApplicationContainer tcpClientApps, udpClientApps;
  for (uint32_t i = 0; i < dumbbell.LeftCount() / 2; ++i) {
    std::cout << "TCP: " << i << '\n';
    // Create an on/off app sending packets to the same leaf right side
    tcpClientApps.Add(tcpClientHelper.Install(dumbbell.GetLeft(i)));
    sink = StaticCast<PacketSink>(tcpClientApps.Get(i));
  }

  for (uint32_t i = dumbbell.LeftCount() / 2; i < dumbbell.LeftCount(); ++i) {
    std::cout << "UDP: " << i << '\n';
    // Create an on/off app sending packets to the same leaf right side
    udpClientApps.Add(udpClientHelper.Install(dumbbell.GetLeft(i)));
    // sink1 = StaticCast<PacketSink> ();
  }

  UdpServerHelper udpServer(PORT);
  udpClientApps.Add(udpServer.Install(dumbbell.GetRight(1)));

  tcpClientApps.Start(Seconds(1.0));
  udpClientApps.Start(Seconds(5.0));

  tcpClientApps.Stop(Seconds(10.0));
  udpClientApps.Stop(Seconds(10.0));

  // Set the bounding box for animation
  dumbbell.BoundingBox(1, 1, 100, 100);

  // Create the animation object and configure for specified output
  AnimationInterface anim(animFile);
  anim.EnablePacketMetadata();                                // Optional
  anim.EnableIpv4L3ProtocolCounters(Seconds(0), Seconds(10)); // Optional

  // Set up the actual simulation
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Calculate Throughput using Flowmonitor
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

  // Now, do the actual simulation.
  // NS_LOG_INFO ("Run Simulation.");
  // for(uint32_t i = 0; i < 12; ++i)
  Simulator::Schedule(Seconds(1.1), &CalculateThroughput);
  Simulator::Stop(Seconds(11.0));

  AsciiTraceHelper ascii;
  pointToPointRouter.EnableAsciiAll(ascii.CreateFileStream("lab-1.tr"));

  Simulator::Run();

  monitor->CheckForLostPackets();

  Ptr<Ipv4FlowClassifier> classifier =
      DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i =
           stats.begin();
       i != stats.end(); ++i) {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
    //  if ((t.sourceAddress=="10.1.1.1" && t.destinationAddress == "10.1.2.2"))
    //    {
    std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> "
              << t.destinationAddress << ")\n";
    std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
    std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
    std::cout << "  Throughput: "
              << i->second.rxBytes * 8.0 /
                     (i->second.timeLastRxPacket.GetSeconds() -
                      i->second.timeFirstTxPacket.GetSeconds()) /
                     1024 / 1024
              << " Mbps\n";
    // }
  }

  monitor->SerializeToXmlFile("lab-1.flowmon", true, true);

  // Create a 2-D plot file.
  Create2DPlotFile();

  Simulator::Destroy();
  // NS_LOG_INFO ("Done.");

  Simulator::Run();
  std::cout << "Animation Trace file created:" << animFile.c_str() << std::endl;
  Simulator::Destroy();
  return 0;
}