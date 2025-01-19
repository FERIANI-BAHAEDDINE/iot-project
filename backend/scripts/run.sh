#!/bin/bash

# Activate virtual environment (if any)
# source /path/to/your/venv/bin/activate

# Define default host and port
HOST="0.0.0.0"
PORT="8000"

# Check if a specific host and port are passed as arguments
if [ ! -z "$1" ]; then
  HOST=$1
fi

if [ ! -z "$2" ]; then
  PORT=$2
fi
docker start postgres
# Run Uvicorn to start the FastAPI application
echo "Starting FastAPI app on $HOST:$PORT..."
uvicorn main:app --host $HOST --port $PORT --reload
