#!/bin/bash

echo ""
echo "oorail-system test script"
echo ""

# test docker
echo "Testing Docker installation..."
docker run hello-world
echo ""

# free
echo "The following should display 0 for swap"
free
echo ""
