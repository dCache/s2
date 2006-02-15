#!/bin/bash

### Basic vars #######################################################
export cdate=`date '+%Y%m%d-%H%M%S'`
export S2_P=`basename ${ProgramName} .sh`.p
export S2_D=`basename ${ProgramName} .sh`.d
export S2_E=${S2_D}
export S2_L=`basename ${ProgramName} .sh`.l
export S2_W=${S2_E}
export S2_OUT=`basename ${ProgramName} .sh`.out
export S2_ERR=`basename ${ProgramName} .sh`.err
export S2_LOG=`basename ${ProgramName} .sh`.log
export S2_E0=`basename ${ProgramName} .sh`.e0
export S2_E1=`basename ${ProgramName} .sh`.e1
export S2_E2=`basename ${ProgramName} .sh`.e2
export SRM2_SUPPORTED=srm2_supported.log
#export S2_ERR=/dev/stderr
#export S2_OUT=/dev/stdout

### Debug ############################################################
# diagnose library related variables
#export DG_DIAGNOSE=0	# no diagnostics at all (logging/debug/warnings/errors)
#			# recompile without diagnostics if you need even better
#			# performance
#export DG_DBG_FILEv="(parse\.cpp|s2\.cpp|var_table\.cpp)"
export DG_DBG_FILEv="(timeout\.c)"
export DG_DBG_FUNCv="(dq_param|Node)"
export DG_LOG=7		# limit logging a bit

### SRM2-relate vars #################################################
ENDPOINT=4
if test ${ENDPOINT} -eq 0 ; then
  export SRM_HOST=mencak.esc.rl.ac.uk
  export SRM_PORT=8444
  export SERVER_PATH=/dpm/esc.rl.ac.uk/home/dteam
elif test ${ENDPOINT} -eq 1 ; then
  export SRM_HOST=lxb1389.cern.ch
  export SRM_PORT=9000
  export SERVER_PATH=/castor/cern.ch/user/j/jmencak/castor
elif test ${ENDPOINT} -eq 2 ; then
  export SRM_HOST=lxb2079.cern.ch
  export SRM_PORT=9000
  export SERVER_PATH=/castor/cern.ch/user/j/jmencak/castor
elif test ${ENDPOINT} -eq 3 ; then
  export SRM_HOST=wn3.epcc.ed.ac.uk
  export SRM_PORT=8443
  export SERVER_PATH=/dpm/epcc.ed.ac.uk/home/dteam
else
  Die 3 "Please choose an ENDPOINT." 
fi
export SRM_ENDPOINT="srm://${SRM_HOST}:${SRM_PORT}"

export TIMEOUT=60000000			# 1 minute
export SLEEP_SOR=1			# 1 second (Status of Request)
export FILE_TO_PUT="/etc/group"
export FILE_TO_PUT_SIZE=`ls -l ${FILE_TO_PUT} | awk '{print $5}'`
export FILE_TO_GET="/tmp/group.$$"
export NEW_DIR="${SERVER_PATH}/${cdate}"
export NEW_FILE="${SERVER_PATH}/${cdate}.txt"
export NEW_FILE01="${SERVER_PATH}/${cdate}_1.txt"
export NEW_FILE02="${SERVER_PATH}/${cdate}_2.txt"
export NEW_SRM_DIR="srm://${SRM_HOST}:${SRM_PORT}/${NEW_DIR}"
export NEW_SRM_FILE="srm://${SRM_HOST}:${SRM_PORT}/${NEW_FILE}"
export NEW_SRM_FILE01="srm://${SRM_HOST}:${SRM_PORT}/${NEW_FILE01}"
export NEW_SRM_FILE02="srm://${SRM_HOST}:${SRM_PORT}/${NEW_FILE02}"

# space functions
export SPACE_TOKEN_DESCR="space-${cdate}"
export LIFETIME=60
export LIFETIME_NEW=120
export RESERVE_SPACE=1048576
export RESERVE_SPACE_NEW=2097152
