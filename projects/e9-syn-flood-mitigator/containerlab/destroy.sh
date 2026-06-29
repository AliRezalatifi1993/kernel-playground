#!/bin/bash
LAB_NAME="e9-syn-lab"
IMAGE="clab-softnet-routing:latest"
TOPOLOGY="e9-syn-lab.clab.yml"
LAB_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${LAB_DIR}/../lib/destroy.sh"
