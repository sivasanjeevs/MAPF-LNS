#!/usr/bin/env python3

import sys
import os
import tempfile
import subprocess

def test_pathfinding():
    """Test that the pathfinding system works"""
    print("Testing dynamic MAPF system...")
    
    # Test map and scenario files
    map_file = "random-32-32-20.map"
    if not os.path.exists(map_file):
        print(f"Error: Map file {map_file} not found!")
        return False
    
    # Test lns executable
    lns_exec = "./lns"
    if not os.path.exists(lns_exec):
        print(f"Error: lns executable not found!")
        return False
    
    # Create a simple test scenario
    with tempfile.TemporaryDirectory() as tmpdir:
        scen_path = os.path.join(tmpdir, 'test.scen')
        out_path = os.path.join(tmpdir, 'test_paths.txt')
        
        # Create a simple scenario with 2 agents
        with open(scen_path, 'w') as f:
            f.write('version 1\n')
            f.write('0\trandom-32-32-20.map\t32\t32\t5\t5\t10\t10\t0\n')
            f.write('1\trandom-32-32-20.map\t32\t32\t15\t15\t20\t20\t0\n')
        
        # Call lns
        cmd = [lns_exec, '--map', map_file, '--agents', scen_path, 
               '--agentNum', '2', '--outputPaths', out_path, '--cutoffTime', '10']
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, check=True, timeout=15)
            print("Pathfinding test successful!")
            print("Output:", result.stdout.strip())
            
            if os.path.exists(out_path):
                with open(out_path, 'r') as f:
                    paths_content = f.read()
                print("Generated paths:")
                print(paths_content)
                return True
            else:
                print("Error: No output file generated")
                return False
                
        except subprocess.TimeoutExpired:
            print("Error: Pathfinding timed out")
            return False
        except subprocess.CalledProcessError as e:
            print(f"Error: Pathfinding failed with return code {e.returncode}")
            print("Stderr:", e.stderr)
            return False

def test_visualizer_import():
    """Test that the visualizer can be imported"""
    try:
        from dynamic_visualizer import DynamicMAPFVisualizer
        print("Dynamic visualizer import successful!")
        return True
    except ImportError as e:
        print(f"Error importing dynamic visualizer: {e}")
        return False

if __name__ == "__main__":
    print("=== Dynamic MAPF System Test ===\n")
    
    # Test 1: Pathfinding
    print("1. Testing pathfinding system...")
    pathfinding_ok = test_pathfinding()
    
    # Test 2: Visualizer import
    print("\n2. Testing visualizer import...")
    visualizer_ok = test_visualizer_import()
    
    # Summary
    print("\n=== Test Results ===")
    print(f"Pathfinding: {'✓ PASS' if pathfinding_ok else '✗ FAIL'}")
    print(f"Visualizer:  {'✓ PASS' if visualizer_ok else '✗ FAIL'}")
    
    if pathfinding_ok and visualizer_ok:
        print("\n🎉 All tests passed! The dynamic MAPF system is ready to use.")
        print("Run './run_dynamic.sh' to start the interactive visualization.")
    else:
        print("\n❌ Some tests failed. Please check the errors above.")
        sys.exit(1) 