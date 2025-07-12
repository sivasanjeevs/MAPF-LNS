#include <iostream>
#include <string>
#include "DynamicInstance.h"

int main() {
    std::cout << "=== Dynamic MAPF Debug Test ===" << std::endl;
    
    try {
        std::cout << "Creating DynamicInstance..." << std::endl;
        
        // Test with default warehouse map
        DynamicInstance instance("warehouse-20-40-10-2-2.map", 
                               "warehouse-20-40-10-2-2-10000agents-1.scen", 
                               15); // 15 agents
        
        std::cout << "DynamicInstance created successfully!" << std::endl;
        std::cout << "Map size: " << instance.map_size << std::endl;
        std::cout << "Rows: " << instance.num_of_rows << ", Cols: " << instance.num_of_cols << std::endl;
        std::cout << "Agents: " << instance.num_of_agents << std::endl;
        
        // Test warehouse functions
        std::cout << "\nTesting warehouse functions..." << std::endl;
        auto [pickup_row, pickup_col] = instance.getWarehousePickupLocation();
        std::cout << "Pickup location: (" << pickup_row << ", " << pickup_col << ")" << std::endl;
        
        auto [dropoff_row, dropoff_col] = instance.getWarehouseDropoffLocation();
        std::cout << "Dropoff location: (" << dropoff_row << ", " << dropoff_col << ")" << std::endl;
        
        // Test goal assignment
        std::cout << "\nTesting goal assignment..." << std::endl;
        instance.assignRandomGoal(0, 1);
        std::cout << "Random goal assigned to agent 0" << std::endl;
        
        std::cout << "\nAll tests passed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception caught!" << std::endl;
        return 1;
    }
    
    return 0;
} 