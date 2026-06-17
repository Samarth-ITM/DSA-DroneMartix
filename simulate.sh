#!/bin/bash

# COURIER DISPATCH AIRSPACE DRONE MATRIX
# Automated CLI Simulation Script for Presentations/Demos.
# Runs each CLI menu option sequentially with a 7-second pause so the output is easily readable.

# Stop script on errors during compilation
set -e

# Resolve absolute path to the project root directory
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_ROOT"

# Text Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

clear
echo -e "${BLUE}========================================================================${NC}"
echo -e "${BLUE}            COURIER DISPATCH DRONE MATRIX - CLI AUTOMATED DEMO          ${NC}"
echo -e "${BLUE}========================================================================${NC}"
echo -e "${YELLOW}[Build] Recompiling C++ codebase...${NC}"
make clean
make all
echo -e "${GREEN}[Success] Compilation complete.${NC}"
echo -e "${YELLOW}[Demo] Starting simulation. Pause duration between actions: 7 seconds.${NC}"
echo -e "${BLUE}========================================================================${NC}"
sleep 3

# Helper function to print action and feed input with a delay
perform_action() {
  local action_desc="$1"
  local action_input="$2"
  echo -e "\n${YELLOW}>>> [DEMO ACTION] ${action_desc} (Feeding Input: '${action_input}') <<<${NC}" >&2
  echo "${action_input}"
  sleep 7
}

# Run the C++ CLI in the foreground with piped inputs
(
  # Option 1: Airspace Overview
  perform_action "Display Airspace Overview & Node Connections" "1"

  # Option 2: Fleet Dashboard
  perform_action "Open Fleet Dashboard (Sub-choice: List available drones)" "2"
  # Sub-choice 2: Merge Sort Fleet by Battery Level
  perform_action "Sort Fleet by Battery using Merge Sort" "2"

  # Option 10: Charging Pad Dashboard
  perform_action "Display Charging Pad Status & Locations" "10"

  # Option 4: Address Lookup Demo
  perform_action "Address Lookup (Select Lookup Demo)" "4"
  # Order ID: 1001
  perform_action "Resolve Address for Order ID 1001 to Node 5" "1001"

  # Option 3: Package Queue - Enqueue Package
  perform_action "Open Package Queue" "3"
  # Choose yes to enqueue
  perform_action "Confirm Enqueuing a New Package (Yes)" "y"
  # Select sample package 1 (Warehouse_Alpha to Customer_Zone_A)
  perform_action "Select Preloaded Sample Package 1 (Warehouse -> Node 5)" "1"

  # Option 5: Dispatch Delivery
  perform_action "Dispatch Next Pending Package (Assigns Package 501 to Best Drone)" "5"

  # Option 6: Advance simulation by one tick
  perform_action "Advance Simulation Tick by 1 step (Starts flight)" "6"

  # Option 7: Advance simulation by five ticks
  perform_action "Advance Simulation Tick by 5 steps (Progresses flight)" "7"

  # Option 8: Trigger Storm Emergency
  perform_action "Trigger Storm weather warning (Forces emergency rerouting/rollback)" "8"

  # Option 9: Undo Flight Step
  perform_action "Select Undo/Rollback Flight Step option" "9"
  # Rollback Drone 103
  perform_action "Enter Drone ID 103 to undo its last flight step" "103"

  # Option 11: Run Full Demonstration
  perform_action "Launch the Full Pre-programmed Demonstration" "11"
  
  # The full demo has 12 stages, each prompting "Press Enter to continue..."
  for i in {1..12}; do
    perform_action "Advance Demonstration Stage ($i/12)" ""
  done

  # Option 12: Exit
  perform_action "Save simulator state and Exit" "12"
) | ./drone_matrix
