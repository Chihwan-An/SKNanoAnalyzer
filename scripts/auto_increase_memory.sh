#!/bin/bash
# Auto-increase memory for HTCondor jobs that were held and released
# Usage: ./auto_increase_memory.sh [interval_seconds] [memory_increment_mb]
#
# This script monitors jobs that:
# 1. Were held (NumJobStarts > 1)
# 2. Are currently running or idle (JobStatus 1 or 2)
# 3. Haven't been processed yet in this session
#
# When found, it increases RequestMemory by the specified increment

INTERVAL=${1:-30}           # Check every 30 seconds by default
INCREMENT=${2:-2048}        # Increase by 2GB by default
MAX_MEMORY=${3:-16384}      # Maximum 16GB

# Track processed jobs to avoid duplicate increases
declare -A PROCESSED_JOBS

echo "=== HTCondor Memory Auto-Scaler ==="
echo "Interval: ${INTERVAL}s | Increment: ${INCREMENT}MB | Max: ${MAX_MEMORY}MB"
echo "Press Ctrl+C to stop"
echo "=================================="

while true; do
    # Find jobs that have been restarted (NumJobStarts > 1) and are idle/running
    # Format: ClusterId.ProcId NumJobStarts CurrentMemory HoldReason
    while IFS=' ' read -r cluster proc status starts memory reason; do
        [ -z "$cluster" ] && continue

        JOB_ID="${cluster}.${proc}"

        # Skip if already processed this session
        [ "${PROCESSED_JOBS[$JOB_ID]}" == "1" ] && continue

        # Check if job was restarted (held then released)
        if [ "$starts" -gt 1 ]; then
            CURRENT_MEM=$memory
            NEW_MEM=$((CURRENT_MEM + INCREMENT))

            # Cap at maximum
            if [ "$NEW_MEM" -gt "$MAX_MEMORY" ]; then
                NEW_MEM=$MAX_MEMORY
            fi

            # Skip if already at max
            if [ "$CURRENT_MEM" -ge "$MAX_MEMORY" ]; then
                echo "[$(date '+%H:%M:%S')] $JOB_ID: Already at max memory (${CURRENT_MEM}MB), skipping"
                PROCESSED_JOBS[$JOB_ID]=1
                continue
            fi

            echo "[$(date '+%H:%M:%S')] $JOB_ID: Restarted $starts times, increasing memory: ${CURRENT_MEM}MB -> ${NEW_MEM}MB"

            # Increase memory
            condor_qedit "$cluster.$proc" RequestMemory "$NEW_MEM" 2>/dev/null

            if [ $? -eq 0 ]; then
                echo "                  -> Success"
                PROCESSED_JOBS[$JOB_ID]=1
            else
                echo "                  -> Failed (job may have completed)"
            fi
        fi
    done < <(condor_q -constraint "Owner==\"$USER\" && NumJobStarts>1 && (JobStatus==1 || JobStatus==2)" \
             -format "%d " ClusterId \
             -format "%d " ProcId \
             -format "%d " JobStatus \
             -format "%d " NumJobStarts \
             -format "%d " RequestMemory \
             -format "%s\n" HoldReason 2>/dev/null)

    # Also check currently held jobs and release them with more memory
    while IFS=' ' read -r cluster proc memory holdreason; do
        [ -z "$cluster" ] && continue

        JOB_ID="${cluster}.${proc}"

        # Check if hold reason is memory-related
        if [[ "$holdreason" == *"memory"* ]] || [[ "$holdreason" == *"Memory"* ]] || [[ "$holdreason" == *"OOM"* ]] || [[ "$holdreason" == *"SIGKILL"* ]]; then
            CURRENT_MEM=$memory
            NEW_MEM=$((CURRENT_MEM + INCREMENT))

            if [ "$NEW_MEM" -gt "$MAX_MEMORY" ]; then
                NEW_MEM=$MAX_MEMORY
            fi

            if [ "$CURRENT_MEM" -ge "$MAX_MEMORY" ]; then
                echo "[$(date '+%H:%M:%S')] $JOB_ID: HELD (memory) but at max, cannot increase further"
                continue
            fi

            echo "[$(date '+%H:%M:%S')] $JOB_ID: HELD due to memory, increasing: ${CURRENT_MEM}MB -> ${NEW_MEM}MB and releasing"

            condor_qedit "$cluster.$proc" RequestMemory "$NEW_MEM" 2>/dev/null
            condor_release "$cluster.$proc" 2>/dev/null

            if [ $? -eq 0 ]; then
                echo "                  -> Released with new memory"
            fi
        fi
    done < <(condor_q -constraint "Owner==\"$USER\" && JobStatus==5" \
             -format "%d " ClusterId \
             -format "%d " ProcId \
             -format "%d " RequestMemory \
             -format "%s\n" HoldReason 2>/dev/null)

    sleep "$INTERVAL"
done
