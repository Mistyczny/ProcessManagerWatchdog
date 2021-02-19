#!/bin/bash
set -e
set -u

unset HOME

artifact=${1}
download_dir=${2}
artifactory_path=${3}

mkdir -p ${download_dir}
cd ${download_dir}

jfrog rt download --flat=true "${artifactory_path}/${artifact}"