#!/usr/bin/env python3
"""
Test script for Dynamic MAPF Goal Assignment
Demonstrates real-time goal assignment and visualization
"""

import time
import random
import threading
from visualize_dynamic_paths import DynamicMAPFVisualizer

def test_dynamic_goal_assignment():
    """Test dynamic goal assignment with visualization"""
    
    print("=== Dynamic MAPF Goal Assignment Test ===")
    
    # Create visualizer
    visualizer = DynamicMAPFVisualizer()
    
    # Add initial agents
    print("Adding initial agents...")
    for i in range(5):
        start_row = random.randint(0, 15)
        start_col = random.randint(0, 15)
        goal_row = random.randint(0, 15)
        goal_col = random.randint(0, 15)
        visualizer.add_agent(i, start_row, start_col, goal_row, goal_col)
    
    # Start visualization in a separate thread
    print("Starting visualization...")
    viz_thread = threading.Thread(target=visualizer.start_visualization)
    viz_thread.daemon = True
    viz_thread.start()
    
    # Wait for visualization to start
    time.sleep(2)
    
    # Simulate dynamic goal assignment
    print("Simulating dynamic goal assignment...")
    for step in range(20):
        # Update agent positions (simulate movement)
        for agent_id in range(5):
            current_pos = visualizer.agent_positions.get(agent_id, (0, 0))
            goal_pos = visualizer.agent_goals.get(agent_id, (0, 0))
            
            # Simple movement towards goal
            new_row = current_pos[0]
            new_col = current_pos[1]
            
            if current_pos[0] < goal_pos[0]:
                new_row += 1
            elif current_pos[0] > goal_pos[0]:
                new_row -= 1
                
            if current_pos[1] < goal_pos[1]:
                new_col += 1
            elif current_pos[1] > goal_pos[1]:
                new_col -= 1
            
            # Update position
            visualizer.update_agent_position(agent_id, new_row, new_col)
        
        # Assign new goals every 5 steps
        if step % 5 == 0 and step > 0:
            for agent_id in range(3):  # Update goals for first 3 agents
                new_goal_row = random.randint(0, 15)
                new_goal_col = random.randint(0, 15)
                visualizer.update_agent_goal(agent_id, new_goal_row, new_goal_col)
                print(f"Step {step}: Assigned new goal to agent {agent_id}: ({new_goal_row}, {new_goal_col})")
        
        # Update simulation time
        visualizer.current_time = step * 0.5
        
        time.sleep(0.5)
    
    print("Test completed!")
    print("Press Ctrl+C to stop visualization")

def test_warehouse_scenario():
    """Test warehouse-specific scenario"""
    
    print("\n=== Warehouse Scenario Test ===")
    
    # Create visualizer with warehouse layout
    visualizer = DynamicMAPFVisualizer()
    
    # Add warehouse agents (pickup and dropoff areas)
    print("Setting up warehouse agents...")
    
    # Pickup area (left side)
    for i in range(3):
        start_row = random.randint(0, 19)
        start_col = random.randint(0, 9)  # Left quarter
        goal_row = random.randint(0, 19)
        goal_col = random.randint(30, 39)  # Right quarter (dropoff)
        visualizer.add_agent(i, start_row, start_col, goal_row, goal_col)
    
    # Dropoff area (right side)
    for i in range(3, 6):
        start_row = random.randint(0, 19)
        start_col = random.randint(30, 39)  # Right quarter
        goal_row = random.randint(0, 19)
        goal_col = random.randint(0, 9)  # Left quarter (pickup)
        visualizer.add_agent(i, start_row, start_col, goal_row, goal_col)
    
    # Start visualization
    viz_thread = threading.Thread(target=visualizer.start_visualization)
    viz_thread.daemon = True
    viz_thread.start()
    
    time.sleep(2)
    
    # Simulate warehouse operations
    print("Simulating warehouse operations...")
    for step in range(30):
        # Update positions
        for agent_id in range(6):
            current_pos = visualizer.agent_positions.get(agent_id, (0, 0))
            goal_pos = visualizer.agent_goals.get(agent_id, (0, 0))
            
            # Move towards goal
            new_row = current_pos[0]
            new_col = current_pos[1]
            
            if current_pos[0] < goal_pos[0]:
                new_row += 1
            elif current_pos[0] > goal_pos[0]:
                new_row -= 1
                
            if current_pos[1] < goal_pos[1]:
                new_col += 1
            elif current_pos[1] > goal_pos[1]:
                new_col -= 1
            
            visualizer.update_agent_position(agent_id, new_row, new_col)
        
        # Assign new warehouse tasks
        if step % 8 == 0 and step > 0:
            # Even agents go to pickup, odd agents go to dropoff
            for agent_id in range(6):
                if agent_id % 2 == 0:
                    # Go to pickup area
                    new_goal_row = random.randint(0, 19)
                    new_goal_col = random.randint(0, 9)
                else:
                    # Go to dropoff area
                    new_goal_row = random.randint(0, 19)
                    new_goal_col = random.randint(30, 39)
                
                visualizer.update_agent_goal(agent_id, new_goal_row, new_goal_col)
                print(f"Step {step}: Agent {agent_id} assigned new warehouse task")
        
        visualizer.current_time = step * 0.5
        time.sleep(0.5)
    
    print("Warehouse test completed!")

def main():
    """Main test function"""
    
    print("Dynamic MAPF Goal Assignment Test Suite")
    print("=======================================")
    
    try:
        # Test 1: Basic dynamic goal assignment
        test_dynamic_goal_assignment()
        
        # Wait between tests
        time.sleep(3)
        
        # Test 2: Warehouse scenario
        test_warehouse_scenario()
        
    except KeyboardInterrupt:
        print("\nTest interrupted by user")
    except Exception as e:
        print(f"Test error: {e}")

if __name__ == "__main__":
    main() 