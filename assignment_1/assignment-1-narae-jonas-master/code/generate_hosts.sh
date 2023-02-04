#!/bin/bash

/share/apps/ifi/available-nodes.sh | grep compute | shuf | head -n $1 > hostfile
