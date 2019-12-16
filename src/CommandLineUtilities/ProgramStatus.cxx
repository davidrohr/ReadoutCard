// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file ProgramStatus.cxx
/// \brief Tool that returns current configuration information about RoCs.
///
/// \author Kostas Alexopoulos (kostas.alexopoulos@cern.ch)

#include <iostream>
#include "Cru/Common.h"
#include "Cru/Constants.h"
#include "Cru/CruBar.h"
#include "ReadoutCard/ChannelFactory.h"
#include "CommandLineUtilities/Options.h"
#include "CommandLineUtilities/Program.h"
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace AliceO2::roc::CommandLineUtilities;
using namespace AliceO2::roc;
namespace pt = boost::property_tree;
namespace po = boost::program_options;

class ProgramStatus : public Program
{
 public:
  virtual Description getDescription()
  {
    return { "Status", "Return current RoC configuration status",
             "roc-status --id 42:00.0\n"
             "roc-status --id 42:00.0 --json" };
  }

  virtual void addOptions(boost::program_options::options_description& options)
  {
    Options::addOptionCardId(options);
    options.add_options()("json-out",
                          po::bool_switch(&mOptions.jsonOut),
                          "Toggle json-formatted output");
  }

  virtual void run(const boost::program_options::variables_map& map)
  {
    auto enforcePrecision = [](auto flo) {
      std::ostringstream precisionEnforcer;
      precisionEnforcer << std::fixed;
      precisionEnforcer << std::setprecision(2);
      precisionEnforcer << flo;
      return precisionEnforcer.str();
    };

    std::ostringstream table;
    auto formatHeader = "  %-9s %-16s %-10s %-14s %-15s %-10s %-14s %-14s %-8s %-19s\n";
    auto formatRow = "  %-9s %-16s %-10s %-14s %-15s %-10s %-14.2f %-14.2f %-8s %-19.1f\n";
    auto header = (boost::format(formatHeader) % "Link ID" % "GBT Mode Tx/Rx" % "Loopback" % "GBT MUX" % "Datapath Mode" % "Datapath" % "RX freq(MHz)" % "TX freq(MHz)" % "Status" % "Optical power(uW)").str();
    auto lineFat = std::string(header.length(), '=') + '\n';
    auto lineThin = std::string(header.length(), '-') + '\n';

    if (!mOptions.jsonOut) {
      table << lineFat << header << lineThin;
    }

    // initialize ptree
    pt::ptree root;

    auto cardId = Options::getOptionCardId(map);
    auto params = Parameters::makeParameters(cardId, 2); //status available on BAR2
    // We care for all of the links
    //params.setLinkMask(Parameters::linkMaskFromString("0-23"));
    auto bar2 = ChannelFactory().getBar(params);

    CardType::type cardType = bar2->getCardType();
    if (cardType == CardType::type::Crorc) {
      std::cout << "CRORC status report not yet supported" << std::endl;
      return;
    } else if (cardType != CardType::type::Cru) {
      std::cout << "Invalid card type" << std::endl;
      return;
    }

    auto cruBar2 = std::dynamic_pointer_cast<CruBar>(bar2);

    Cru::ReportInfo reportInfo = cruBar2->report();

    std::string clock = (reportInfo.ttcClock == 0 ? "TTC" : "Local");

    if (!mOptions.jsonOut) {
      std::cout << "----------------------------" << std::endl;
      std::cout << clock << " clock | ";
      if (reportInfo.dynamicOffset) {
        std::cout << "Dynamic offset" << std::endl;
      } else {
        std::cout << "Fixed offset" << std::endl;
      }
      std::cout << "----------------------------" << std::endl;
    } else {
      root.put("clock", clock);
      if (reportInfo.dynamicOffset) {
        root.put("offset", "Dynamic");
      } else {
        root.put("offset", "Fixed");
      }
    }

    for (const auto& el : reportInfo.linkMap) {
      auto link = el.second;
      int globalId = el.first; //Use the "new" link mapping
      std::string gbtTxMode = GbtMode::toString(link.gbtTxMode);
      std::string gbtRxMode = GbtMode::toString(link.gbtRxMode);
      std::string gbtTxRxMode = gbtTxMode + "/" + gbtRxMode;
      std::string loopback = (link.loopback == false ? "None" : "Enabled");

      std::string downstreamData;
      if (reportInfo.downstreamData == Cru::DATA_CTP) {
        downstreamData = "CTP";
      } else if (reportInfo.downstreamData == Cru::DATA_PATTERN) {
        downstreamData = "PATTERN";
      } else if (reportInfo.downstreamData == Cru::DATA_MIDTRG) {
        downstreamData = "MIDTRG";
      }

      std::string gbtMux = GbtMux::toString(link.gbtMux);
      if (gbtMux == "TTC") {
        gbtMux += ":" + downstreamData;
      }

      std::string datapathMode = DatapathMode::toString(link.datapathMode);

      std::string enabled = (link.enabled) ? "Enabled" : "Disabled";

      float rxFreq = link.rxFreq;
      float txFreq = link.txFreq;

      std::string linkStatus;
      if (link.stickyBit == Cru::LinkStatus::Up) {
        linkStatus = "UP";
      } else if (link.stickyBit == Cru::LinkStatus::UpWasDown) {
        linkStatus = "UP (was DOWN)";
      } else if (link.stickyBit == Cru::LinkStatus::Down) {
        linkStatus = "DOWN";
      }

      float opticalPower = link.opticalPower;

      if (!mOptions.jsonOut) {
        auto format = boost::format(formatRow) % globalId % gbtTxRxMode % loopback % gbtMux % datapathMode % enabled % rxFreq % txFreq % linkStatus % opticalPower;
        table << format;
      } else {
        pt::ptree linkNode;

        // add kv pairs for this card
        linkNode.put("gbtMode", gbtTxRxMode);
        linkNode.put("loopback", loopback);
        linkNode.put("gbtMux", gbtMux);
        linkNode.put("datapathMode", datapathMode);
        linkNode.put("datapath", enabled);
        linkNode.put("rxFreq", enforcePrecision(rxFreq));
        linkNode.put("txFreq", enforcePrecision(txFreq));
        linkNode.put("status", linkStatus);
        linkNode.put("opticalPower", enforcePrecision(opticalPower));

        // add the link node to the tree
        root.add_child(std::to_string(globalId), linkNode);
      }
    }

    if (!mOptions.jsonOut) {
      auto lineFat = std::string(header.length(), '=') + '\n';
      table << lineFat;
      std::cout << table.str();
    } else {
      pt::write_json(std::cout, root);
    }
  }

 private:
  struct OptionsStruct {
    bool jsonOut = false;
  } mOptions;
};

int main(int argc, char** argv)
{
  return ProgramStatus().execute(argc, argv);
}
