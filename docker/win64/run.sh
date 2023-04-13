#!/bin/bash
docker run -it \
  --name=elinks-win64-dev \
  -v /tmp:/tmp/host \
  elinks-win64-dev:latest \
  /bin/bash
