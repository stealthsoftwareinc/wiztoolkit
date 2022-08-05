#! /bin/bash

# Copyright (C) 2021 Stealth Software Technologies, Inc.

if [ -n "${CI_JOB_TOKEN}" ]
then
  git clone "https://gitlab-ci-token:${CI_JOB_TOKEN}@gitlab.stealthsoftwareinc.com/stealth/loggingtools.git" logging/
else
  git clone git@github.com/stealthsoftwareinc/stealth_logging.git logging/
fi
