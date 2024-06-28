#!/usr/bin/env bash

target_dir=${1:?"Error: No directory provided. Usage: $0 <directory>"}

cd $target_dir

mv WebGPU.html index.html