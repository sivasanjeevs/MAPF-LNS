#!/usr/bin/env python3
"""
Warehouse Scenario Analyzer
Analyzes warehouse MAPF scenarios and helps with dynamic goal assignment
"""

import matplotlib.pyplot as plt
import numpy as np
import sys
import os
from typing import List, Tuple, Dict
import argparse

class WarehouseAnalyzer:
    def __init__(self, map_file: str, scenario_file: str):
        self.map_file = map_file
        self.scenario_file = scenario_file
        self.map_data = None
        self.scenario_data = []
        self.map_width = 0
        self.map_height = 0
        
    def load_map(self):
        """Load the warehouse map"""
        print(f"Loading map: {self.map_file}")
        
        with open(self.map_file, 'r') as f:
            lines = f.readlines()
        
        # Parse header
        map_type = lines[0].strip()
        height = int(lines[1].split()[1])
        width = int(lines[2].split()[1])
        
        self.map_height = height
        self.map_width = width
        
        # Load map data (skip header lines)
        map_lines = lines[3:]
        self.map_data = []
        
        for line in map_lines:
            if line.strip() and line.strip() != 'map':
                self.map_data.append(list(line.strip()))
        
        print(f"Map loaded: {width}x{height}")
        print(f"Map type: {map_type}")
        
    def load_scenario(self, max_agents: int = None):
        """Load the scenario file"""
        print(f"Loading scenario: {self.scenario_file}")
        
        with open(self.scenario_file, 'r') as f:
            lines = f.readlines()
        
        # Skip version line
        for line in lines[1:]:
            if line.strip():
                parts = line.strip().split('\t')
                if len(parts) >= 7:
                    agent_id = int(parts[0])
                    map_path = parts[1]
                    width = int(parts[2])
                    height = int(parts[3])
                    start_x = int(parts[4])
                    start_y = int(parts[5])
                    goal_x = int(parts[6])
                    goal_y = int(parts[7])
                    optimal_cost = int(parts[8]) if len(parts) > 8 else 0
                    
                    self.scenario_data.append({
                        'id': agent_id,
                        'start': (start_x, start_y),
                        'goal': (goal_x, goal_y),
                        'optimal_cost': optimal_cost
                    })
                    
                    if max_agents and len(self.scenario_data) >= max_agents:
                        break
        
        print(f"Loaded {len(self.scenario_data)} agents from scenario")
        
    def analyze_scenario(self):
        """Analyze the scenario data"""
        if not self.scenario_data:
            print("No scenario data loaded!")
            return
        
        print("\n=== Scenario Analysis ===")
        
        # Basic statistics
        total_agents = len(self.scenario_data)
        print(f"Total agents: {total_agents}")
        
        # Distance analysis
        distances = []
        for agent in self.scenario_data:
            start = agent['start']
            goal = agent['goal']
            dist = np.sqrt((goal[0] - start[0])**2 + (goal[1] - start[1])**2)
            distances.append(dist)
        
        print(f"Average distance: {np.mean(distances):.2f}")
        print(f"Min distance: {np.min(distances):.2f}")
        print(f"Max distance: {np.max(distances):.2f}")
        
        # Goal distribution
        goals = [agent['goal'] for agent in self.scenario_data]
        unique_goals = set(goals)
        print(f"Unique goals: {len(unique_goals)}")
        
        # Start distribution
        starts = [agent['start'] for agent in self.scenario_data]
        unique_starts = set(starts)
        print(f"Unique starts: {len(unique_starts)}")
        
    def visualize_scenario(self, num_agents: int = 100, save_plot: bool = True):
        """Visualize the scenario"""
        if not self.map_data or not self.scenario_data:
            print("Map or scenario data not loaded!")
            return
        
        # Create figure
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6))
        
        # Plot 1: Map with start/goal positions
        ax1.set_title(f"Warehouse Map with {num_agents} Agents")
        
        # Convert map to numpy array for visualization
        map_array = np.array([[1 if cell == '.' else 0 for cell in row] for row in self.map_data])
        ax1.imshow(map_array, cmap='gray', origin='upper')
        
        # Plot start and goal positions
        for i, agent in enumerate(self.scenario_data[:num_agents]):
            start = agent['start']
            goal = agent['goal']
            
            # Plot start (blue) and goal (red)
            ax1.plot(start[0], start[1], 'bo', markersize=3, alpha=0.7)
            ax1.plot(goal[0], goal[1], 'ro', markersize=3, alpha=0.7)
            
            # Draw arrow from start to goal
            ax1.arrow(start[0], start[1], goal[0]-start[0], goal[1]-start[1], 
                     head_width=1, head_length=1, fc='green', ec='green', alpha=0.3)
        
        ax1.set_xlim(0, self.map_width)
        ax1.set_ylim(self.map_height, 0)
        
        # Plot 2: Distance distribution
        distances = []
        for agent in self.scenario_data[:num_agents]:
            start = agent['start']
            goal = agent['goal']
            dist = np.sqrt((goal[0] - start[0])**2 + (goal[1] - start[1])**2)
            distances.append(dist)
        
        ax2.hist(distances, bins=20, alpha=0.7, color='skyblue', edgecolor='black')
        ax2.set_title("Distance Distribution")
        ax2.set_xlabel("Distance")
        ax2.set_ylabel("Number of Agents")
        ax2.axvline(np.mean(distances), color='red', linestyle='--', label=f'Mean: {np.mean(distances):.1f}')
        ax2.legend()
        
        plt.tight_layout()
        
        if save_plot:
            plt.savefig('warehouse_analysis.png', dpi=300, bbox_inches='tight')
            print("Analysis plot saved as: warehouse_analysis.png")
        
        plt.show()
        
    def generate_dynamic_tasks(self, num_agents: int = 100, task_duration: int = 60) -> List[Dict]:
        """Generate dynamic tasks for warehouse simulation"""
        if not self.scenario_data:
            print("No scenario data loaded!")
            return []
        
        tasks = []
        for i in range(min(num_agents, len(self.scenario_data))):
            agent = self.scenario_data[i]
            
            # Create dynamic task
            task = {
                'agent_id': agent['id'],
                'start_pos': agent['start'],
                'goal_pos': agent['goal'],
                'priority': np.random.randint(1, 10),  # Random priority 1-9
                'deadline': np.random.randint(10, task_duration),  # Random deadline
                'task_type': 'pickup' if np.random.random() > 0.5 else 'delivery'
            }
            tasks.append(task)
        
        return tasks
    
    def save_dynamic_config(self, num_agents: int = 100, filename: str = "warehouse_dynamic_config.txt"):
        """Save configuration for dynamic simulation"""
        tasks = self.generate_dynamic_tasks(num_agents)
        
        with open(filename, 'w') as f:
            f.write(f"# Dynamic Warehouse Configuration\n")
            f.write(f"# Generated from: {self.scenario_file}\n")
            f.write(f"# Number of agents: {num_agents}\n\n")
            
            for task in tasks:
                f.write(f"Agent {task['agent_id']}: ")
                f.write(f"Start({task['start_pos'][0]},{task['start_pos'][1]}) ")
                f.write(f"Goal({task['goal_pos'][0]},{task['goal_pos'][1]}) ")
                f.write(f"Priority:{task['priority']} ")
                f.write(f"Deadline:{task['deadline']} ")
                f.write(f"Type:{task['task_type']}\n")
        
        print(f"Dynamic configuration saved to: {filename}")

def main():
    parser = argparse.ArgumentParser(description="Warehouse Scenario Analyzer")
    parser.add_argument("--map", default="instances/warehouse-20-40-10-2-2.map", help="Map file")
    parser.add_argument("--scenario", default="instances/warehouse-20-40-10-2-2-10000agents-1.scen", help="Scenario file")
    parser.add_argument("--agents", type=int, default=100, help="Number of agents to analyze")
    parser.add_argument("--visualize", action="store_true", help="Generate visualization")
    parser.add_argument("--config", action="store_true", help="Generate dynamic configuration")
    
    args = parser.parse_args()
    
    # Check if files exist
    if not os.path.exists(args.map):
        print(f"Error: Map file not found: {args.map}")
        return
    
    if not os.path.exists(args.scenario):
        print(f"Error: Scenario file not found: {args.scenario}")
        return
    
    # Create analyzer
    analyzer = WarehouseAnalyzer(args.map, args.scenario)
    
    # Load data
    analyzer.load_map()
    analyzer.load_scenario(args.agents)
    
    # Analyze
    analyzer.analyze_scenario()
    
    # Generate visualization
    if args.visualize:
        analyzer.visualize_scenario(args.agents)
    
    # Generate dynamic configuration
    if args.config:
        analyzer.save_dynamic_config(args.agents)
    
    print("\nAnalysis complete!")

if __name__ == "__main__":
    main()