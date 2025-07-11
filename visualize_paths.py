import sys
import re
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
from matplotlib.lines import Line2D

# Usage: python3 visualize_paths.py [path_file]

def parse_paths(filename):
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

def animate_paths(paths, starts, goals, nrows, ncols):
    makespan = max(len(p) for p in paths)
    n_agents = len(paths)
    colors = plt.cm.get_cmap('tab20', n_agents)

    fig, ax = plt.subplots(figsize=(max(8, ncols/2), max(8, nrows/2)))
    ax.set_xlim(-0.5, ncols-0.5)
    ax.set_ylim(-0.5, nrows-0.5)
    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_aspect('equal')
    ax.set_title('Multi-Agent Pathfinding Visualization', fontsize=16, pad=20)
    # Light grid
    for x in range(ncols):
        ax.axvline(x-0.5, color='lightgray', linewidth=0.7, zorder=0)
    for y in range(nrows):
        ax.axhline(y-0.5, color='lightgray', linewidth=0.7, zorder=0)

    # Start and goal markers (small faded dot/star)
    for i, (s, g) in enumerate(zip(starts, goals)):
        ax.scatter(s[1], s[0], marker='o', color=colors(i), s=40, alpha=0.3, edgecolors='none', zorder=2)
        ax.scatter(g[1], g[0], marker='*', color=colors(i), s=80, alpha=0.5, edgecolors='none', zorder=2)

    # Agent current position (large dot) and trail (faint line)
    agent_dots = [ax.plot([], [], 'o', color=colors(i), markersize=18, zorder=3)[0] for i in range(n_agents)]
    agent_trails = [ax.plot([], [], '-', color=colors(i), linewidth=2, alpha=0.3, zorder=1)[0] for i in range(n_agents)]
    # Agent number text
    agent_texts = [ax.text(0, 0, str(i), color='white', fontsize=8, ha='center', va='center', weight='bold', zorder=4) for i in range(n_agents)]

    # Legend: one entry per agent
    handles = [Line2D([0], [0], marker='o', color='w', markerfacecolor=colors(i), markersize=10, label=f'Agent {i}') for i in range(n_agents)]
    ax.legend(handles=handles, loc='upper right', bbox_to_anchor=(1.15, 1.0), fontsize='small', title='Agents')

    def update(frame):
        for i, path in enumerate(paths):
            pos = path[min(frame, len(path)-1)]
            agent_dots[i].set_data([pos[1]], [pos[0]])
            # Trail
            trail = path[:min(frame+1, len(path))]
            if len(trail) > 1:
                agent_trails[i].set_data([c for r, c in trail], [r for r, c in trail])
            else:
                agent_trails[i].set_data([], [])
            # Agent number
            agent_texts[i].set_position((pos[1], pos[0]))
        ax.set_title(f'Multi-Agent Pathfinding Visualization\nTimestep {frame}', fontsize=16, pad=20)
        return agent_dots + agent_trails + agent_texts

    ani = animation.FuncAnimation(fig, update, frames=makespan, interval=500, blit=True, repeat=True)
    plt.tight_layout()
    plt.show()

def main():
    path_file = sys.argv[1] if len(sys.argv) > 1 else 'paths.txt'
    paths, starts, goals, nrows, ncols = parse_paths(path_file)
    animate_paths(paths, starts, goals, nrows, ncols)

if __name__ == '__main__':
    main() 