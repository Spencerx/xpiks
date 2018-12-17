#!/bin/bash

docker exec flying_unicorn find /home/ftpuser \( -name '*.jpg' -or -name '*.zip' \) -type f -exec rm -v {} +
