import sys
import re
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import time
import threading
from collections import defaultdict

class DynamicMAPFVisualizer:
    def __init__(self, map_file=None, agent_file=None):
        self.map_data = None
        self.agent_positions = {}
        self.agent_goals = {}
        self.agent_paths = {}
        self.agent_colors = {}
        self.agent_trails = defaultdict(list)
        self.current_time = 0
        self.simulation_running = False
        
        # Load map if provided
        if map_file:
            self.load_map(map_file)
        if agent_file:
            self.load_agents(agent_file)
    
    def load_map(self, map_file):
        """Load map from file"""
        try:
            with open(map_file, 'r') as f:
                lines = f.readlines()
                
            # Parse map dimensions
            if lines[0].startswith('type'):
                # Standard map format
                height = int(lines[1].split()[1])
                width = int(lines[2].split()[1])
                map_lines = lines[4:4+height]
            else:
                # Simple format
                height = len(lines)
                width = len(lines[0].strip())
                map_lines = lines
            
            self.map_data = np.zeros((height, width))
            for i, line in enumerate(map_lines):
                for j, char in enumerate(line.strip()):
                    if char in ['@', 'T', 'O']:  # Obstacles
                        self.map_data[i, j] = 1
                        
            print(f"Loaded map: {width}x{height}")
        except Exception as e:
            print(f"Error loading map: {e}")
            self.map_data = np.zeros((20, 20))  # Default empty map
    
    def load_agents(self, agent_file):
        """Load initial agent positions from file"""
        try:
            with open(agent_file, 'r') as f:
                lines = f.readlines()
            
            for line in lines[1:]:  # Skip header
                parts = line.strip().split('\t')
                if len(parts) >= 8:
                    agent_id = int(parts[0])
                    start_col = int(parts[4])
                    start_row = int(parts[5])
                    goal_col = int(parts[6])
                    goal_row = int(parts[7])
                    
                    self.agent_positions[agent_id] = (start_row, start_col)
                    self.agent_goals[agent_id] = (goal_row, goal_col)
                    self.agent_colors[agent_id] = plt.cm.tab20(agent_id % 20)
                    
            print(f"Loaded {len(self.agent_positions)} agents")
        except Exception as e:
            print(f"Error loading agents: {e}")
    
    def update_agent_position(self, agent_id, row, col):
        """Update agent position in real-time"""
        self.agent_positions[agent_id] = (row, col)
        self.agent_trails[agent_id].append((row, col))
        
        # Keep only last 50 positions for trail
        if len(self.agent_trails[agent_id]) > 50:
            self.agent_trails[agent_id] = self.agent_trails[agent_id][-50:]
    
    def update_agent_goal(self, agent_id, row, col):
        """Update agent goal in real-time"""
        self.agent_goals[agent_id] = (row, col)
    
    def add_agent(self, agent_id, start_row, start_col, goal_row, goal_col):
        """Add a new agent"""
        self.agent_positions[agent_id] = (start_row, start_col)
        self.agent_goals[agent_id] = (goal_row, goal_col)
        self.agent_colors[agent_id] = plt.cm.tab20(agent_id % 20)
        self.agent_trails[agent_id] = [(start_row, start_col)]
    
    def remove_agent(self, agent_id):
        """Remove an agent"""
        if agent_id in self.agent_positions:
            del self.agent_positions[agent_id]
        if agent_id in self.agent_goals:
            del self.agent_goals[agent_id]
        if agent_id in self.agent_colors:
            del self.agent_colors[agent_id]
        if agent_id in self.agent_trails:
            del self.agent_trails[agent_id]
    
    def start_visualization(self):
        """Start the real-time visualization"""
        self.simulation_running = True
        
        # Create figure and axes
        self.fig, self.ax = plt.subplots(figsize=(12, 8))
        self.ax.set_title('Dynamic Multi-Agent Pathfinding', fontsize=16, pad=20)
        
        # Set up the plot
        if self.map_data is not None:
            height, width = self.map_data.shape
            self.ax.set_xlim(-0.5, width-0.5)
            self.ax.set_ylim(-0.5, height-0.5)
        else:
            self.ax.set_xlim(-0.5, 20-0.5)
            self.ax.set_ylim(-0.5, 20-0.5)
        
        self.ax.set_xticks([])
        self.ax.set_yticks([])
        self.ax.set_aspect('equal')
        
        # Draw obstacles if map is loaded
        if self.map_data is not None:
            obstacle_positions = np.where(self.map_data == 1)
            self.ax.scatter(obstacle_positions[1], obstacle_positions[0], 
                           c='black', s=100, marker='s', alpha=0.7, zorder=1)
        
        # Initialize agent elements
        self.agent_dots = {}
        self.agent_goals_markers = {}
        self.agent_trail_lines = {}
        self.agent_texts = {}
        
        # Create legend
        legend_elements = []
        
        for agent_id in self.agent_positions:
            # Agent current position (large dot)
            dot, = self.ax.plot([], [], 'o', color=self.agent_colors[agent_id], 
                               markersize=15, zorder=4)
            self.agent_dots[agent_id] = dot
            
            # Agent goal (star)
            goal_marker = self.ax.scatter([], [], marker='*', 
                                        color=self.agent_colors[agent_id], 
                                        s=200, alpha=0.6, zorder=2)
            self.agent_goals_markers[agent_id] = goal_marker
            
            # Agent trail (faint line)
            trail_line, = self.ax.plot([], [], '-', color=self.agent_colors[agent_id], 
                                      linewidth=2, alpha=0.3, zorder=1)
            self.agent_trail_lines[agent_id] = trail_line
            
            # Agent number text
            text = self.ax.text(0, 0, str(agent_id), color='white', fontsize=10, 
                               ha='center', va='center', weight='bold', zorder=5)
            self.agent_texts[agent_id] = text
            
            # Add to legend
            legend_elements.append(plt.Line2D([0], [0], marker='o', color='w', 
                                            markerfacecolor=self.agent_colors[agent_id], 
                                            markersize=10, label=f'Agent {agent_id}'))
        
        self.ax.legend(handles=legend_elements, loc='upper right', 
                      bbox_to_anchor=(1.15, 1.0), fontsize='small', title='Agents')
        
        # Start animation
        self.ani = animation.FuncAnimation(self.fig, self.update_animation, 
                                         frames=None, interval=100, blit=False, repeat=True)
        
        plt.tight_layout()
        plt.show()
    
    def update_animation(self, frame):
        """Update animation frame"""
        if not self.simulation_running:
            return
        
        # Update agent positions and goals
        for agent_id in self.agent_positions:
            if agent_id in self.agent_dots:
                # Update current position
                pos = self.agent_positions[agent_id]
                self.agent_dots[agent_id].set_data([pos[1]], [pos[0]])
                
                # Update goal position
                if agent_id in self.agent_goals:
                    goal = self.agent_goals[agent_id]
                    self.agent_goals_markers[agent_id].set_offsets([[goal[1], goal[0]]])
                
                # Update trail
                if agent_id in self.agent_trails and len(self.agent_trails[agent_id]) > 1:
                    trail = self.agent_trails[agent_id]
                    trail_x = [pos[1] for pos in trail]
                    trail_y = [pos[0] for pos in trail]
                    self.agent_trail_lines[agent_id].set_data(trail_x, trail_y)
                
                # Update agent number text
                self.agent_texts[agent_id].set_position((pos[1], pos[0]))
        
        # Update title with current time
        self.ax.set_title(f'Dynamic Multi-Agent Pathfinding\nTime: {self.current_time:.1f}s', 
                         fontsize=16, pad=20)
        
        return []
    
    def stop_visualization(self):
        """Stop the visualization"""
        self.simulation_running = False
        if hasattr(self, 'ani'):
            self.ani.event_source.stop()
        plt.close('all')

def parse_dynamic_paths(filename):
    """Parse dynamic path file with real-time updates"""
    paths = []
    starts = []
    goals = []
    max_row = 0
    max_col = 0
    
    with open(filename, 'r') as f:
        for line in f:
            m = re.match(r'Agent (\d+):(.*)', line.strip())
            if not m:
                continue
            path_str = m.group(2)
            coords = re.findall(r'\((\d+),(\d+)\)', path_str)
            path = [(int(r), int(c)) for r, c in coords]
            if path:
                starts.append(path[0])
                goals.append(path[-1])
                max_row = max(max_row, max(r for r, _ in path))
                max_col = max(max_col, max(c for _, c in path))
            paths.append(path)
    
    return paths, starts, goals, max_row + 1, max_col + 1

def main():
    # Create visualizer
    visualizer = DynamicMAPFVisualizer()
    
    # Add some example agents for demonstration
    for i in range(5):
        visualizer.add_agent(i, i*2, i*2, i*2+5, i*2+5)
    
    # Start visualization
    print("Starting dynamic visualization...")
    print("Press Ctrl+C to stop")
    
    try:
        visualizer.start_visualization()
    except KeyboardInterrupt:
        print("\nVisualization stopped by user")
        visualizer.stop_visualization()

if __name__ == '__main__':
    main() 