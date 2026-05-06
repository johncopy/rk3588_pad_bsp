#!/bin/bash

repo forall -c "pwd; git reset; git checkout .; git clean -df"
