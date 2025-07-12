import sys
import re
import pygame
import time
import numpy as np
import os
import tempfile
import shutil
import threading
import queue
import subprocess
from typing import List, Tuple, Optional

class DynamicMAPFVisualizer:
    def __init__(self, map_file: str, initial_scen_file: str = None, initial_agent_num: int = 0):
        self.map_file = map_file
        self.obstacles, self.nrows, self.ncols = self.parse_map(map_file)
        
        # Agent data
        self.agents = []  # List of (start, goal, path, color, agent_id)
        self.next_agent_id = 0
        
        # Visualization state
        self.running = True
        self.paused = False
        self.frame = 0
        self.speed = 1
        self.makespan = 1
        
        # UI state
        self.selecting = False
        self.select_stage = 0  # 0: not selecting, 1: select start, 2: select goal
        self.new_start = None
        self.new_goal = None
        
        # Threading for pathfinding
        self.pathfinding_queue = queue.Queue()
        self.pathfinding_thread = None
        self.pathfinding_busy = False
        
        # Initialize pygame
        pygame.init()
        self.setup_display()
        
        # Load initial agents if provided
        if initial_scen_file and initial_agent_num > 0:
            self.load_initial_agents(initial_scen_file, initial_agent_num)
    
    def parse_map(self, map_filename):
        """Parse map file to get obstacles and dimensions"""
        obstacles = set()
        nrows = ncols = 0
        with open(map_filename, 'r') as f:
            lines = f.readlines()
            for i, line in enumerate(lines):
                if line.startswith('height'):
                    nrows = int(line.strip().split()[1])
                elif line.startswith('width'):
                    ncols = int(line.strip().split()[1])
                elif line.strip() == 'map':
                    map_start = i + 1
                    break
            for r, line in enumerate(lines[map_start:map_start + nrows]):
                for c, ch in enumerate(line.strip()):
                    if ch == '@':
                        obstacles.add((r, c))
        return obstacles, nrows, ncols
    
    def setup_display(self):
        """Setup pygame display and UI elements"""
        info = pygame.display.Info()
        screen_w, screen_h = info.current_w, info.current_h
        
        # Use 90% of the screen for the grid+legend
        max_grid_w = int(screen_w * 0.9)
        max_grid_h = int(screen_h * 0.9)
        legend_width = 250
        
        # Compute cell size and margin so grid fits
        cell_size_w = (max_grid_w - legend_width) // self.ncols
        cell_size_h = max_grid_h // self.nrows
        self.cell_size = min(cell_size_w, cell_size_h, 60)
        self.margin = max(30, min(60, self.cell_size // 2))
        
        self.grid_w = self.ncols * self.cell_size
        self.grid_h = self.nrows * self.cell_size
        self.width = self.grid_w + self.margin * 2 + legend_width
        self.height = self.grid_h + self.margin * 2
        
        self.screen = pygame.display.set_mode((self.width, self.height))
        pygame.display.set_caption('Dynamic Multi-Agent Pathfinding Visualization')
        
        self.font = pygame.font.SysFont('Arial', max(12, self.cell_size // 3))
        self.small_font = pygame.font.SysFont('Arial', max(10, self.cell_size // 4))
        self.clock = pygame.time.Clock()
        
        self.grid_color = (200, 200, 200)
        self.bg_color = (255, 255, 255)
    
    def load_initial_agents(self, scen_file: str, agent_num: int):
        """Load initial agents from scenario file"""
        starts, goals = self.parse_scen_file(scen_file, agent_num)
        for i, (start, goal) in enumerate(zip(starts, goals)):
            self.add_agent(start, goal)
    
    def parse_scen_file(self, scen_file: str, agent_num: int) -> Tuple[List[Tuple[int, int]], List[Tuple[int, int]]]:
        """Parse scenario file to get start and goal positions"""
        starts = []
        goals = []
        with open(scen_file, 'r') as f:
            lines = f.readlines()
            for line in lines[1:agent_num+1]:  # Skip version line
                parts = line.strip().split('\t')
                if len(parts) >= 8:
                    start_col, start_row = int(parts[4]), int(parts[5])
                    goal_col, goal_row = int(parts[6]), int(parts[7])
                    starts.append((start_row, start_col))
                    goals.append((goal_row, goal_col))
        return starts, goals
    
    def get_agent_colors(self, n_agents):
        """Generate distinct colors for agents"""
        base_colors = [
            (31, 119, 180), (255, 127, 14), (44, 160, 44), (214, 39, 40),
            (148, 103, 189), (140, 86, 75), (227, 119, 194), (127, 127, 127),
            (188, 189, 34), (23, 190, 207), (255, 152, 150), (197, 176, 213)
        ]
        colors = []
        for i in range(n_agents):
            colors.append(base_colors[i % len(base_colors)])
        return colors
    
    def grid_pos_from_mouse(self, pos):
        """Convert mouse position to grid coordinates"""
        mx, my = pos
        c = (mx - self.margin) // self.cell_size
        r = (my - self.margin) // self.cell_size
        if 0 <= r < self.nrows and 0 <= c < self.ncols:
            return (r, c)
        return None
    
    def write_scen_file(self, scen_path: str, starts: List[Tuple[int, int]], goals: List[Tuple[int, int]]):
        """Write scenario file for pathfinding"""
        with open(scen_path, 'w') as f:
            f.write('version 1\n')
            for i, (s, g) in enumerate(zip(starts, goals)):
                f.write(f"{i}\t{self.map_file}\t{self.ncols}\t{self.nrows}\t{s[1]}\t{s[0]}\t{g[1]}\t{g[0]}\t0\n")
    
    def parse_paths_file(self, filename: str):
        """Parse paths from output file"""
        paths = []
        with open(filename, 'r') as f:
            for line in f:
                m = re.match(r'Agent (\d+):(.*)', line.strip())
                if not m:
                    continue
                path_str = m.group(2)
                coords = re.findall(r'\((\d+),(\d+)\)', path_str)
                path = [(int(r), int(c)) for r, c in coords]
                paths.append(path)
        return paths
    
    def call_pathfinder(self, starts: List[Tuple[int, int]], goals: List[Tuple[int, int]]) -> Optional[List[List[Tuple[int, int]]]]:
        """Call the C++ pathfinder with given starts and goals"""
        num_agents = len(starts)
        if num_agents == 0:
            return []
        
        # Create temporary files
        with tempfile.TemporaryDirectory() as tmpdir:
            scen_path = os.path.join(tmpdir, 'temp.scen')
            out_path = os.path.join(tmpdir, 'temp_paths.txt')
            
            self.write_scen_file(scen_path, starts, goals)
            
            # Call the existing C++ executable
            lns_exec = './lns'
            cmd = [
                lns_exec, '--map', self.map_file, '--agents', scen_path, 
                '--agentNum', str(num_agents), '--outputPaths', out_path, 
                '--cutoffTime', '30'  # Shorter timeout for dynamic planning
            ]
            
            try:
                result = subprocess.run(cmd, capture_output=True, text=True, check=True, timeout=35)
                if os.path.exists(out_path):
                    paths = self.parse_paths_file(out_path)
                    
                    # Adjust timing for new agents to start from timestep 0
                    if len(self.agents) > 0 and len(paths) > len(self.agents):
                        # This means we added new agents
                        current_time = self.frame
                        adjusted_paths = []
                        
                        for i, path in enumerate(paths):
                            if i < len(self.agents):
                                # Existing agents keep their paths
                                adjusted_paths.append(path)
                            else:
                                # New agents start from timestep 0
                                # Pad the beginning with their start position
                                start_pos = starts[i]
                                padding = [start_pos] * current_time
                                adjusted_path = padding + path
                                adjusted_paths.append(adjusted_path)
                        
                        return adjusted_paths
                    else:
                        return paths
            except (subprocess.TimeoutExpired, subprocess.CalledProcessError, FileNotFoundError) as e:
                print(f"Pathfinding error: {e}")
                return None
        
        return None
    
    def add_agent(self, start: Tuple[int, int], goal: Tuple[int, int]):
        """Add a new agent and replan all paths"""
        if start in self.obstacles or goal in self.obstacles:
            print("Cannot place agent on obstacle")
            return False
        
        # Check if positions are already occupied
        for agent in self.agents:
            if start == agent[0] or start == agent[1] or goal == agent[0] or goal == agent[1]:
                print("Position already occupied")
                return False
        
        # Add new agent with temporary path
        agent_id = self.next_agent_id
        self.next_agent_id += 1
        color = self.get_agent_colors(1)[0]
        temp_path = [start]  # Temporary path until replanning
        
        self.agents.append((start, goal, temp_path, color, agent_id))
        
        # Replan all paths
        self.replan_all_paths()
        return True
    
    def check_collisions(self):
        """Check for collisions between agents and log them"""
        collisions = []
        
        for i, (start1, goal1, path1, color1, agent_id1) in enumerate(self.agents):
            for j, (start2, goal2, path2, color2, agent_id2) in enumerate(self.agents):
                if i >= j:  # Avoid checking same pair twice
                    continue
                
                # Check vertex collisions (same position at same time)
                for t in range(min(len(path1), len(path2))):
                    if path1[t] == path2[t]:
                        collisions.append({
                            'type': 'vertex',
                            'agents': (agent_id1, agent_id2),
                            'position': path1[t],
                            'timestep': t
                        })
                
                # Check edge collisions (swapping positions)
                for t in range(min(len(path1) - 1, len(path2) - 1)):
                    if (path1[t] == path2[t + 1] and path1[t + 1] == path2[t]):
                        collisions.append({
                            'type': 'edge',
                            'agents': (agent_id1, agent_id2),
                            'positions': (path1[t], path1[t + 1]),
                            'timestep': t
                        })
        
        # Log collisions if any found
        if collisions:
            print(f"🚨 COLLISION DETECTED! Found {len(collisions)} collision(s):")
            for collision in collisions:
                if collision['type'] == 'vertex':
                    print(f"   Vertex collision: Agents {collision['agents']} at position {collision['position']} at timestep {collision['timestep']}")
                else:  # edge collision
                    print(f"   Edge collision: Agents {collision['agents']} swapping positions {collision['positions']} at timestep {collision['timestep']}")
            return True
        else:
            print("✅ No collisions detected - all paths are collision-free!")
            return False
    
    def replan_all_paths(self):
        """Replan paths for all agents"""
        if not self.agents:
            return
        
        # Extract current starts and goals
        starts = [agent[0] for agent in self.agents]
        goals = [agent[1] for agent in self.agents]
        
        # Call pathfinder
        new_paths = self.call_pathfinder(starts, goals)
        
        if new_paths and len(new_paths) == len(self.agents):
            # Update paths
            for i, (start, goal, old_path, color, agent_id) in enumerate(self.agents):
                if i < len(new_paths) and new_paths[i]:
                    self.agents[i] = (start, goal, new_paths[i], color, agent_id)
            
            # Update makespan
            self.makespan = max(len(agent[2]) for agent in self.agents) if self.agents else 1
            self.frame = min(self.frame, self.makespan - 1)
            
            print(f"Replanned paths for {len(self.agents)} agents, makespan: {self.makespan}")
            
            # Check for collisions after replanning
            self.check_collisions()
        else:
            print("Pathfinding failed, keeping existing paths")
    
    def handle_events(self):
        """Handle pygame events"""
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_SPACE:
                    self.paused = not self.paused
                elif event.key == pygame.K_RIGHT:
                    self.frame = min(self.frame + 1, self.makespan - 1)
                elif event.key == pygame.K_LEFT:
                    self.frame = max(self.frame - 1, 0)
                elif event.key == pygame.K_ESCAPE:
                    self.running = False
                elif event.key == pygame.K_a:
                    # Start selecting a new agent
                    self.selecting = True
                    self.select_stage = 1
                    self.new_start = None
                    self.new_goal = None
                elif event.key == pygame.K_r:
                    # Replan all paths
                    self.replan_all_paths()
                elif event.key == pygame.K_c:
                    # Manual collision check
                    print(f"\n🔍 Manual collision check at timestep {self.frame}:")
                    self.check_collisions()
            elif event.type == pygame.MOUSEBUTTONDOWN and self.selecting:
                pos = pygame.mouse.get_pos()
                grid_pos = self.grid_pos_from_mouse(pos)
                if grid_pos and grid_pos not in self.obstacles:
                    if self.select_stage == 1:
                        self.new_start = grid_pos
                        self.select_stage = 2
                    elif self.select_stage == 2:
                        self.new_goal = grid_pos
                        # Add new agent
                        if self.add_agent(self.new_start, self.new_goal):
                            print(f"Added agent {self.next_agent_id - 1} from {self.new_start} to {self.new_goal}")
                        self.selecting = False
                        self.select_stage = 0
                        self.new_start = None
                        self.new_goal = None
    
    def draw_grid(self):
        """Draw the grid"""
        for x in range(self.ncols + 1):
            pygame.draw.line(self.screen, self.grid_color, 
                           (self.margin + x * self.cell_size, self.margin), 
                           (self.margin + x * self.cell_size, self.margin + self.nrows * self.cell_size), 1)
        for y in range(self.nrows + 1):
            pygame.draw.line(self.screen, self.grid_color, 
                           (self.margin, self.margin + y * self.cell_size), 
                           (self.margin + self.ncols * self.cell_size, self.margin + y * self.cell_size), 1)
    
    def draw_obstacles(self):
        """Draw obstacles"""
        for (r, c) in self.obstacles:
            pygame.draw.rect(self.screen, (0, 0, 0), 
                           (self.margin + c * self.cell_size, self.margin + r * self.cell_size, 
                            self.cell_size, self.cell_size))
    
    def draw_agents(self):
        """Draw all agents"""
        for start, goal, path, color, agent_id in self.agents:
            # Draw start and goal markers
            start_pos = (self.margin + start[1] * self.cell_size + self.cell_size // 2, 
                        self.margin + start[0] * self.cell_size + self.cell_size // 2)
            goal_pos = (self.margin + goal[1] * self.cell_size + self.cell_size // 2, 
                       self.margin + goal[0] * self.cell_size + self.cell_size // 2)
            
            # Start: circle
            pygame.draw.circle(self.screen, color, start_pos, self.cell_size // 4, 0)
            pygame.draw.circle(self.screen, (255, 255, 255), start_pos, self.cell_size // 4, 2)
            
            # Goal: star
            for angle in range(0, 360, 72):
                x1 = int(goal_pos[0] + self.cell_size // 4 * np.cos(np.radians(angle)))
                y1 = int(goal_pos[1] + self.cell_size // 4 * np.sin(np.radians(angle)))
                x2 = int(goal_pos[0] + self.cell_size // 8 * np.cos(np.radians(angle + 36)))
                y2 = int(goal_pos[1] + self.cell_size // 8 * np.sin(np.radians(angle + 36)))
                pygame.draw.line(self.screen, color, goal_pos, (x1, y1), 2)
                pygame.draw.line(self.screen, color, goal_pos, (x2, y2), 2)
            
            # Draw path trail
            if len(path) > 1:
                trail = path[:min(self.frame + 1, len(path))]
                if len(trail) >= 2:  # Only draw if we have at least 2 points
                    points = [(self.margin + c * self.cell_size + self.cell_size // 2, 
                              self.margin + r * self.cell_size + self.cell_size // 2) for r, c in trail]
                    pygame.draw.lines(self.screen, color, False, points, max(2, self.cell_size // 15))
            
            # Draw current position
            if path:
                current_pos = path[min(self.frame, len(path) - 1)]
                pos_pix = (self.margin + current_pos[1] * self.cell_size + self.cell_size // 2, 
                          self.margin + current_pos[0] * self.cell_size + self.cell_size // 2)
                pygame.draw.circle(self.screen, color, pos_pix, max(8, self.cell_size // 2 - 2))
                
                # Agent number
                text = self.font.render(str(agent_id), True, (255, 255, 255))
                text_rect = text.get_rect(center=pos_pix)
                self.screen.blit(text, text_rect)
    
    def draw_legend(self):
        """Draw legend and UI elements"""
        legend_x = self.grid_w + self.margin * 2
        legend_y = self.margin
        
        # Title
        legend_title = self.font.render('Agents', True, (0, 0, 0))
        self.screen.blit(legend_title, (legend_x, legend_y))
        
        # Agent list
        for i, (start, goal, path, color, agent_id) in enumerate(self.agents):
            y_pos = legend_y + 35 + i * 30
            pygame.draw.circle(self.screen, color, (legend_x + 20, y_pos), 12)
            agent_label = self.small_font.render(f'Agent {agent_id}', True, (0, 0, 0))
            self.screen.blit(agent_label, (legend_x + 40, y_pos - 10))
        
        # Timestep
        timestep_text = self.font.render(f'Timestep: {self.frame}', True, (0, 0, 0))
        self.screen.blit(timestep_text, (self.margin, 10))
        
        # Instructions
        instr = self.small_font.render('SPACE: Pause/Play   ←/→: Step   ESC: Quit   A: Add Agent   R: Replan   C: Check Collisions', True, (80, 80, 80))
        self.screen.blit(instr, (self.margin, self.height - 30))
        
        # Selection feedback
        if self.selecting:
            if self.select_stage == 1 and self.new_start:
                pygame.draw.rect(self.screen, (0, 255, 0), 
                               (self.margin + self.new_start[1] * self.cell_size, 
                                self.margin + self.new_start[0] * self.cell_size, 
                                self.cell_size, self.cell_size), 3)
            if self.select_stage == 2 and self.new_start:
                pygame.draw.rect(self.screen, (0, 255, 0), 
                               (self.margin + self.new_start[1] * self.cell_size, 
                                self.margin + self.new_start[0] * self.cell_size, 
                                self.cell_size, self.cell_size), 3)
            if self.select_stage == 2 and self.new_goal:
                pygame.draw.rect(self.screen, (255, 0, 0), 
                               (self.margin + self.new_goal[1] * self.cell_size, 
                                self.margin + self.new_goal[0] * self.cell_size, 
                                self.cell_size, self.cell_size), 3)
    
    def update(self):
        """Update simulation state"""
        if not self.paused:
            self.frame = (self.frame + 1) % self.makespan
            
            # Check for collisions at current timestep (only log occasionally to avoid spam)
            if self.frame % 10 == 0:  # Check every 10 timesteps
                self.check_collisions_at_timestep(self.frame)
            
            # Check if all agents have reached their goals
            all_at_goals = True
            for start, goal, path, color, agent_id in self.agents:
                if path and len(path) > 0:
                    current_pos = path[min(self.frame, len(path) - 1)]
                    if current_pos != goal:
                        all_at_goals = False
                        break
                else:
                    all_at_goals = False
                    break
            
            # If all agents are at goals, you could add a completion message or restart
            if all_at_goals and self.agents:
                # Optional: Add a completion indicator
                pass
    
    def check_collisions_at_timestep(self, timestep):
        """Check for collisions at a specific timestep"""
        collisions = []
        
        for i, (start1, goal1, path1, color1, agent_id1) in enumerate(self.agents):
            for j, (start2, goal2, path2, color2, agent_id2) in enumerate(self.agents):
                if i >= j:  # Avoid checking same pair twice
                    continue
                
                # Get positions at current timestep
                if timestep < len(path1) and timestep < len(path2):
                    pos1 = path1[timestep]
                    pos2 = path2[timestep]
                    
                    # Check vertex collision
                    if pos1 == pos2:
                        collisions.append({
                            'type': 'vertex',
                            'agents': (agent_id1, agent_id2),
                            'position': pos1,
                            'timestep': timestep
                        })
                    
                    # Check edge collision (swapping positions)
                    if timestep + 1 < len(path1) and timestep + 1 < len(path2):
                        next_pos1 = path1[timestep + 1]
                        next_pos2 = path2[timestep + 1]
                        if pos1 == next_pos2 and pos2 == next_pos1:
                            collisions.append({
                                'type': 'edge',
                                'agents': (agent_id1, agent_id2),
                                'positions': (pos1, pos2),
                                'timestep': timestep
                            })
        
        # Log collisions if any found
        if collisions:
            print(f"🚨 COLLISION AT TIMESTEP {timestep}! Found {len(collisions)} collision(s):")
            for collision in collisions:
                if collision['type'] == 'vertex':
                    print(f"   Vertex collision: Agents {collision['agents']} at position {collision['position']}")
                else:  # edge collision
                    print(f"   Edge collision: Agents {collision['agents']} swapping positions {collision['positions']}")
            return True
        
        return False
    
    def draw(self):
        """Draw everything"""
        self.screen.fill(self.bg_color)
        self.draw_obstacles()
        self.draw_grid()
        self.draw_agents()
        self.draw_legend()
        pygame.display.flip()
    
    def run(self):
        """Main game loop"""
        while self.running:
            self.handle_events()
            self.update()
            self.draw()
            
            if not self.paused:
                self.clock.tick(self.speed)
            else:
                self.clock.tick(15)
        
        pygame.quit()

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 dynamic_visualizer.py <map_file> [scen_file] [agent_num]")
        sys.exit(1)
    
    map_file = sys.argv[1]
    scen_file = sys.argv[2] if len(sys.argv) > 2 else None
    agent_num = int(sys.argv[3]) if len(sys.argv) > 3 else 0
    
    visualizer = DynamicMAPFVisualizer(map_file, scen_file, agent_num)
    visualizer.run()

if __name__ == '__main__':
    main() 