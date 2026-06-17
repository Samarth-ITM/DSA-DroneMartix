#!/bin/bash

# COURIER DISPATCH AIRSPACE DRONE MATRIX
# Helper startup script to run C++ backend binaries and the React visualization layer.

# Ensure script stops on error during compile/build steps
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

show_help() {
  echo -e "${BLUE}========================================================================${NC}"
  echo -e "${BLUE}                 COURIER DISPATCH AIRSPACE DRONE MATRIX                 ${NC}"
  echo -e "${BLUE}========================================================================${NC}"
  echo "Usage:"
  echo -e "  ./run.sh             ${GREEN}Run All${NC} (Start React Web Server in background + C++ CLI)"
  echo -e "  ./run.sh --cli       ${GREEN}Run C++ CLI Only${NC}"
  echo -e "  ./run.sh --web       ${GREEN}Run React Web Server Only${NC}"
  echo -e "  ./run.sh --demo      ${GREEN}Run Automated CLI Presentation Demo${NC} (simulate.sh)"
  echo -e "  ./run.sh --help      ${GREEN}Show this help message${NC}"
  echo -e "${BLUE}========================================================================${NC}"
}

run_cli() {
  echo -e "\n${YELLOW}[Build] Compiling C++ simulator...${NC}"
  make all
  echo -e "${GREEN}[Success] Compilation complete.${NC}"
  
  echo -e "\n${YELLOW}[CLI] Launching C++ CLI Simulation...${NC}\n"
  ./drone_matrix
}

run_web() {
  cd visualization
  if [ ! -d "node_modules" ]; then
    echo -e "\n${YELLOW}[Web] node_modules not found. Installing dependencies (npm install)...${NC}"
    npm install
  fi
  echo -e "\n${GREEN}[Web] Starting Vite local development server...${NC}"
  echo -e "${YELLOW}Press Ctrl+C to stop the web server.${NC}\n"
  npm run dev
}

run_all() {
  # 1. Compile C++ Code
  echo -e "${YELLOW}[Build] Recompiling C++ binaries...${NC}"
  make all
  echo -e "${GREEN}[Build] Compilation complete.${NC}"

  # 2. Check and start Web Server if not already active
  VITE_ALREADY_RUNNING=false
  VITE_PID=""

  if nc -z 127.0.0.1 5173 2>/dev/null || nc -z localhost 5173 2>/dev/null; then
    echo -e "${GREEN}[Web] React Web Server is already running on http://localhost:5173${NC}"
    VITE_ALREADY_RUNNING=true
  else
    echo -e "${YELLOW}[Web] Starting React Web Server (Vite) on port 5173 in the background...${NC}"
    
    cd visualization
    if [ ! -d "node_modules" ]; then
      echo -e "${YELLOW}[Web] node_modules not found. Installing dependencies (npm install)...${NC}"
      npm install
    fi
    # Start Vite in background, redirecting output
    npm run dev -- --port 5173 > /tmp/vite_dev_server.log 2>&1 &
    VITE_PID=$!
    cd ..
    
    # Wait a brief moment to confirm it binds
    sleep 2
    if nc -z 127.0.0.1 5173 2>/dev/null || nc -z localhost 5173 2>/dev/null; then
      echo -e "${GREEN}[Web] Web Server started successfully!${NC}"
      echo -e "${GREEN}[Web] View the Visualization Dashboard at: http://localhost:5173${NC}"
    else
      echo -e "${RED}[Web] Warning: Web Server did not bind to port 5173 in time.${NC}"
      echo -e "${RED}[Web] Check logs at /tmp/vite_dev_server.log${NC}"
    fi
  fi

  # Setup trap to clean up the background Vite server on exit if we started it
  if [ "$VITE_ALREADY_RUNNING" != "true" ] && [ ! -z "$VITE_PID" ]; then
    trap 'echo -e "\n${RED}[Shutdown] Stopping background React server (PID $VITE_PID)...${NC}"; kill $VITE_PID 2>/dev/null; exit 0' EXIT INT TERM
  fi

  # 3. Launch C++ Simulator in foreground
  echo -e "\n${YELLOW}[CLI] Launching C++ CLI Simulation in foreground...${NC}"
  echo -e "${BLUE}------------------------------------------------------------${NC}"
  ./drone_matrix
  echo -e "${BLUE}------------------------------------------------------------${NC}"
}

# Parse Command Line Arguments
if [ $# -eq 0 ]; then
  # Default action: run it all!
  run_all
else
  case "$1" in
    --cli|-c)
      run_cli
      ;;
    --web|-w)
      run_web
      ;;
    --demo|-d)
      ./simulate.sh
      ;;
    --help|-h)
      show_help
      ;;
    *)
      echo -e "${RED}Error: Unknown argument '$1'${NC}"
      show_help
      exit 1
      ;;
  esac
fi
