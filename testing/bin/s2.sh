#!/bin/bash

ProgramName=$(basename $0)
ProgramNameEnv=$(basename ${ProgramName} .sh).env
ProgramDir=$(dirname $0) ; cd ${ProgramDir}

S2_TEST_FILE=$(basename ${ProgramName} .sh).s2
S2_BIN=s2
ENV_SH=s2.env

### Functions ########################################################
Fail() {
  echo $@ >&2
}

Die() {
  err=$1
  shift
  Fail "$ProgramName: $@"
  exit $err
}

Which() {
  local descend=""
  local src_file="$2"
  local i="$1"
  local not_found=1

  while test $i -ne 0
  do
    i=$(expr $i - 1)
    which_file="${ProgramDir}/$descend$src_file"
    if test -r "${which_file}" ; then
      not_found=0
      break
    fi
    descend="../$descend"
  done

  return $not_found
}

Which_s2() {
  Which 7 src/${S2_BIN}
  if test $? -eq 0 ; then
    S2_BIN=${which_file}
  fi
}

Source() {
  local descend=""
  local src_file="$2"
  local i="$1"
  local not_found=1

  while test $i -ne 0
  do
    i=$(expr $i - 1)
    descend="../$descend"
  done

  while test x$descend != x
  do
    descend="${descend:3}"
    local env_file="${ProgramDir}/$descend$src_file"
    if test -r "${env_file}" ; then
      source "${env_file}"
      not_found=0
    fi
  done

  return $not_found
}

Defined() {
  env | grep "^$1=" >/dev/null
  return $?
}

Export() {
  Defined $1
  if test $? -eq 0 ; then
    export $1
  else
    export $1="$2"
  fi
}

Usage() {
  echo "Usage: $ProgramName
options: --help         this help
         --gdb          run a gdb session
         --valgrind     run with valgrind
	 --             s2 options separator

e.g.: $ProgramName --valgrind -- --timeout=5000000 --show-defaults
      $ProgramName -- ${ProgramName%.sh}_arg1 ${ProgramName%.sh}_arg2 ${ProgramName%.sh}_arg3" >&2
  exit 0
}

main() {
  while true
  do
    case "$1" in
      --[Hh][Ee][Ll][Pp]) Usage
      ;;
      --[Gg][Dd][Bb]) S2_GDB=1
      ;;
      --[Vv][Aa][Ll][Gg][Rr][Ii][Nn][Dd]) S2_VALGRIND=1
      ;;
      --[Ss][2]-[Bb][Ii][Nn])
        Which_s2
        S2_BIN=`which ${S2_BIN} 2>/dev/null` || S2_BIN="false"
	echo -n $S2_BIN
        exit
      ;;
      --) shift; break
      ;;
      -*) Die 1 "invalid option \`$1'"
      ;;
      *)  break 
      ;;
    esac
    shift
  done

  if test x"${ProgramName}" = xs2.sh && ! test -L ${ProgramName} ; then
    Fail "This script is not meant to be run here.  Please see README."
    Usage
  fi

  Which_s2
  S2_BIN=`which ${S2_BIN} 2>/dev/null` || Die 3 "Couldn't find s2 binary."
  S2_SRC_DIR=`dirname ${S2_BIN}`

  Source 5 ${ENV_SH} || Die 3 "Couldn't source ${ENV_SH}"
  test -r ${ProgramNameEnv} && source ${ProgramNameEnv}

  if test   x${S2_GDB} != x; then
    gdb $@ ${S2_BIN}
  elif test x${S2_VALGRIND} != x; then
    # --gen-suppressions=all
    rm -f ${S2_P} ${S2_D} ${S2_E} ${S2_L} ${S2_W} ${S2_OUT} ${S2_LOG} ${S2_E0} ${S2_E1} ${S2_E2}
    valgrind\
      --show-reachable=yes\
      --suppressions=${S2_SRC_DIR}/valgrind.supp\
      ${S2_BIN}\
      --file=${S2_TEST_FILE}\
      --pp-out-file=${S2_P}\
      --dbg-file=${S2_D}\
      --err-file=${S2_E}\
      --log-file=${S2_L}\
      --warn-file=${S2_W}\
      --e0-file=${S2_E0}\
      --e1-file=${S2_E1}\
      --e2-file=${S2_E2}\
      $@\
      >${S2_OUT}
  else
    # Normal S2 run
    rm -f ${S2_P} ${S2_D} ${S2_E} ${S2_L} ${S2_W} ${S2_OUT} ${S2_LOG} ${S2_E0} ${S2_E1} ${S2_E2}
    echo -e "${S2_TEST_FILE}:"
    time ${S2_BIN}\
      --file=${S2_TEST_FILE}\
      --pp-out-file=${S2_P}\
      --dbg-file=${S2_D}\
      --err-file=${S2_E}\
      --log-file=${S2_L}\
      --warn-file=${S2_W}\
      --e0-file=${S2_E0}\
      --e1-file=${S2_E1}\
      --e2-file=${S2_E2}\
      $@\
      > ${S2_OUT}
    err=$?
    echo -e "$err\n"
    return $err
  fi
}

main $@
