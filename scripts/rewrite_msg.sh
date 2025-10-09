#!/bin/bash

# Read message from stdin
msg=$(cat)

# Get first line
first_line=$(echo "$msg" | head -n1)

# Clean: lowercase and truncate to 60 chars
clean_first=$(echo "$first_line" | tr '[:upper:]' '[:lower:]' | cut -c1-60)

# Replace first line in message
echo "$msg" | sed "1s/.*/$clean_first/"