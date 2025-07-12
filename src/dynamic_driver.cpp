#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include "DynamicInstance.h"
#include "LNS.h"
#include "AnytimeBCBS.h"
#include "AnytimeEECBS.h"
#include "PIBT/pibt.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

int main(int argc, char** argv)
{
    namespace po = boost::program_options;
    
    // Declare the supported options.
    po::options_description desc("Dynamic MAPF - Real-time Goal Assignment");
    desc.add_options()
        ("help", "produce help message")
        
        // Input parameters
        ("map,m", po::value<string>()->default_value("warehouse-20-40-10-2-2.map"), "input file for map")
        ("agents,a", po::value<string>()->default_value("warehouse-20-40-10-2-2-10000agents-1.scen"), "input file for agents")
        ("agentNum,k", po::value<int>()->default_value(10), "number of agents")
        
        // Simulation parameters
        ("simulationTime,t", po::value<double>()->default_value(60.0), "simulation time in seconds")
        ("simulationSpeed,s", po::value<double>()->default_value(1.0), "simulation speed multiplier")
        ("taskInterval,i", po::value<double>()->default_value(5.0), "interval between new task assignments (seconds)")
        ("warehouseMode,w", po::value<bool>()->default_value(true), "use warehouse task generation")
        
        // Output parameters
        ("output,o", po::value<string>()->default_value("dynamic_output"), "output file name")
        ("visualize,v", po::value<bool>()->default_value(true), "enable real-time visualization")
        
        // Algorithm parameters
        ("solver", po::value<string>()->default_value("LNS"), "solver (LNS, A-BCBS, A-EECBS)")
        ("initAlgo", po::value<string>()->default_value("PP"), "initial algorithm")
        ("replanAlgo", po::value<string>()->default_value("PP"), "replanning algorithm")
        ("neighborSize", po::value<int>()->default_value(8), "neighborhood size")
        ("sipp", po::value<bool>()->default_value(true), "use SIPP")
        ("screen", po::value<int>()->default_value(1), "screen output level")
    ;
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    
    if (vm.count("help")) {
        cout << desc << endl;
        return 1;
    }
    
    po::notify(vm);
    
    // Initialize dynamic instance
    cout << "Initializing Dynamic MAPF Simulation..." << endl;
    DynamicInstance dynamic_instance(
        vm["map"].as<string>(),
        vm["agents"].as<string>(),
        vm["agentNum"].as<int>()
    );
    
    cout << "Map: " << vm["map"].as<string>() << endl;
    cout << "Agents: " << vm["agentNum"].as<int>() << endl;
    cout << "Simulation Time: " << vm["simulationTime"].as<double>() << " seconds" << endl;
    cout << "Simulation Speed: " << vm["simulationSpeed"].as<double>() << "x" << endl;
    
    // Start simulation
    dynamic_instance.startSimulation(vm["simulationSpeed"].as<double>());
    
    // Main simulation loop
    auto start_time = chrono::high_resolution_clock::now();
    double simulation_duration = vm["simulationTime"].as<double>();
    double task_interval = vm["taskInterval"].as<double>();
    bool warehouse_mode = vm["warehouseMode"].as<bool>();
    
    double last_task_time = 0.0;
    int task_counter = 0;
    
    cout << "\n=== Starting Dynamic Simulation ===" << endl;
    cout << "Press Ctrl+C to stop simulation\n" << endl;
    
    while (true) {
        auto current_time = chrono::high_resolution_clock::now();
        double elapsed = chrono::duration<double>(current_time - start_time).count();
        
        if (elapsed >= simulation_duration) {
            cout << "\nSimulation time limit reached!" << endl;
            break;
        }
        
        // Assign new tasks periodically
        if (elapsed - last_task_time >= task_interval) {
            if (warehouse_mode) {
                // Generate warehouse tasks
                dynamic_instance.generateWarehouseTasks(2); // Assign 2 new tasks
                cout << "[" << elapsed << "s] Assigned " << 2 << " new warehouse tasks" << endl;
            } else {
                // Assign random goals to random agents
                for (int i = 0; i < 3; i++) {
                    int agent_id = rand() % vm["agentNum"].as<int>();
                    dynamic_instance.assignRandomGoal(agent_id, rand() % 5 + 1);
                }
                cout << "[" << elapsed << "s] Assigned 3 random goals" << endl;
            }
            last_task_time = elapsed;
            task_counter += 2;
        }
        
        // Print status every 10 seconds
        if (int(elapsed) % 10 == 0 && int(elapsed) != int(last_task_time)) {
            cout << "[" << elapsed << "s] Status: ";
            int active_agents = 0;
            for (int i = 0; i < vm["agentNum"].as<int>(); i++) {
                if (dynamic_instance.hasAgentTask(i)) {
                    active_agents++;
                }
            }
            cout << active_agents << "/" << vm["agentNum"].as<int>() << " agents active" << endl;
        }
        
        // Sleep for a short interval
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    
    // Stop simulation
    dynamic_instance.stopSimulation();
    
    // Print final statistics
    cout << "\n=== Simulation Complete ===" << endl;
    cout << "Total tasks assigned: " << task_counter << endl;
    cout << "Simulation duration: " << simulation_duration << " seconds" << endl;
    
    // Save final agent positions
    if (vm.count("output")) {
        string output_file = vm["output"].as<string>() + "_final_positions.txt";
        ofstream out(output_file);
        out << "Final Agent Positions:" << endl;
        for (int i = 0; i < vm["agentNum"].as<int>(); i++) {
            int loc = dynamic_instance.getAgentLocation(i);
            auto [row, col] = dynamic_instance.getCoordinate(loc);
            out << "Agent " << i << ": (" << row << ", " << col << ")" << endl;
        }
        out.close();
        cout << "Final positions saved to: " << output_file << endl;
    }
    
    return 0;
} 