# Copyright 2021, Kacper Waśniowski
# All rights reserved.

string(REPLACE "." "_" BoostDownloadVersion ${BoostVersion})

configure_file(Dockerfile.ubuntu.in Dockerfile.ubuntu)