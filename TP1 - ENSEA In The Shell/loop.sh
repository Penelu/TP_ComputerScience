#!/bin/bash

# Infinite loop
while true; do
    # Print "test script"
    echo "test script"
    
    # Check if 'q' is pressed
    read -t 1 -n 1 key
    if [[ $key == "q" ]]; then
        echo "Terminating script..."
        break
    fi
done
