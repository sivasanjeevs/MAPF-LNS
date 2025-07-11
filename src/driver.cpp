#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include "LNS.h"
#include "AnytimeBCBS.h"
#include "AnytimeEECBS.h"
#include "PIBT/pibt.h"


/* Main function */
int main(int argc, char** argv)
{
	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")

		// params for the input instance and experiment settings
		("map,m", po::value<string>()->required(), "input file for map")
		("agents,a", po::value<string>()->required(), "input file for agents")
		("agentNum,k", po::value<int>()->default_value(0), "number of agents")
        ("output,o", po::value<string>(), "output file name (no extension)")
        ("outputPaths", po::value<string>(), "output file for paths")
        ("cutoffTime,t", po::value<double>()->default_value(7200), "cutoff time (seconds)")
		("screen,s", po::value<int>()->default_value(0),
		        "screen option (0: none; 1: LNS results; 2:LNS detailed results; 3: MAPF detailed results)")
		("stats", po::value<string>(), "output stats file")
        ("neighborSize", po::value<int>()->default_value(8), "Size of the neighborhood")
        ("maxIterations", po::value<int>()->default_value(0), "maximum number of iterations")
        ("initDestoryStrategy", po::value<string>()->default_value("Adaptive"), "initial destroy strategy (Adaptive)")
		;
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);

	if (vm.count("help")) {
		cout << desc << endl;
		return 1;
	}

    po::notify(vm);

	srand((int)time(0));

	Instance instance(vm["map"].as<string>(), vm["agents"].as<string>(),
		vm["agentNum"].as<int>());
    double time_limit = vm["cutoffTime"].as<double>();
    int screen = vm["screen"].as<int>();
	srand(vm["seed"].as<int>());

    // Only allow LNS(PP;PP)
    string solver = "LNS";
    string initAlgo = "PP";
    string replanAlgo = "PP";
    string destoryStrategy = "Adaptive";
    string initDestoryStrategy = vm["initDestoryStrategy"].as<string>();
    int neighborSize = vm["neighborSize"].as<int>();
    int maxIterations = vm["maxIterations"].as<int>();
    PIBTPPS_option pipp_option; // Not used, but required by LNS constructor
    LNS lns(instance, time_limit, initAlgo, replanAlgo, destoryStrategy, neighborSize, maxIterations, false, initDestoryStrategy, true, screen, pipp_option);
    bool succ = lns.run();
    if (succ)
    {
        lns.validateSolution();
        if (vm.count("outputPaths"))
            lns.writePathsToFile(vm["outputPaths"].as<string>());
    }
    if (vm.count("output"))
        lns.writeResultToFile(vm["output"].as<string>());
    if (vm.count("stats"))
        lns.writeIterStatsToFile(vm["stats"].as<string>());
	return 0;

}