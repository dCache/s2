#!/bin/bash

set -x
$@
err=$?
set +x
exit $err
