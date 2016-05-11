#!/bin/bash

ProgramName=$(basename $0)
ProgramNameEnv=$(basename ${ProgramName} .sh).env
ProgramDir=$(dirname $0)

S2_TEST_FILE=${ProgramDir}/$(basename ${ProgramName} .sh).s2
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

Source_descend() {
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

Source() {
  local descend=""
  local src_file="$2"
  local i="$1"
  local not_found=1

  while test $i -ne 0
  do
    local env_file="${ProgramDir}/$descend$src_file"
    if test -r "${env_file}" ; then
      source "${env_file}"
      not_found=0
    fi

    i=$(expr $i - 1)
    descend="../$descend"
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
  echo "Usage: $ProgramName [options]
options: --help         this help
         --superfast    run with no diagnostics
         --fast         run with no logging and debugging (only warnings/errors)
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
      --[Ss][Uu][Pp][Ee][Rr][Ff][Aa][Ss][Tt]) S2_RUN=superfast
      ;;
      --[Ff][Aa][Ss][Tt]) S2_RUN=fast
      ;;
      --[Gg][Dd][Bb]) S2_RUN=gdb
      ;;
      --[Vv][Aa][Ll][Gg][Rr][Ii][Nn][Dd]) S2_RUN=valgrind
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

  # invocation by make
  while true
  do
    case "$1" in
      test)
      ;;
      superfast) S2_RUN=superfast
      ;;
      fast)      S2_RUN=fast
      ;;
      gdb)       S2_RUN=gdb
      ;;
      valgrind)  S2_RUN=valgrind
      ;;
      *) break
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

  test -r ${ProgramNameEnv} && source ${ProgramNameEnv}
  Source 6 ${ENV_SH} || Die 3 "Couldn't source ${ENV_SH}"

  if test x"${S2_SUPRESS_PROGRESS}" != x; then
      EXTRA_OPTIONS="--progress=0"
  fi

  case "$S2_RUN" in
    superfast)
      rm -f ${S2_P} ${S2_D} ${S2_E} ${S2_L} ${S2_W} ${S2_OUT} ${S2_LOG} ${S2_E0} ${S2_E1} ${S2_E2}
      echo -e "${S2_TEST_FILE}:"
      time DG_DIAGNOSE=0 ${S2_BIN}\
        --simple-name\
        --file=${S2_TEST_FILE}\
        $@\
        > ${S2_OUT}
      err=$?
      echo -e "$err\n"
      return $err
    ;;

    fast)
      rm -f ${S2_P} ${S2_D} ${S2_E} ${S2_L} ${S2_W} ${S2_OUT} ${S2_LOG} ${S2_E0} ${S2_E1} ${S2_E2}
      echo -e "${S2_TEST_FILE}:"
      time DG_DBG=0 DG_LOG=0 ${S2_BIN}\
        --simple-name\
        --file=${S2_TEST_FILE}\
        $@\
        > ${S2_OUT}
      err=$?
      echo -e "$err\n"
      return $err
    ;;

    gdb)
      gdb $@ ${S2_BIN}
    ;;

    valgrind)
      # --gen-suppressions=all
      rm -f ${S2_P} ${S2_D} ${S2_E} ${S2_L} ${S2_W} ${S2_OUT} ${S2_LOG} ${S2_E0} ${S2_E1} ${S2_E2}
      echo -e "${S2_TEST_FILE}:" >&2
      valgrind\
        --gen-suppressions=all\
        --show-reachable=yes\
        --suppressions=${S2_SRC_DIR}/valgrind.supp\
        ${S2_BIN}\
        --progress=0\
        --simple-name\
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
    ;;

    *)
      # Normal S2 run
      rm -f ${S2_P} ${S2_D} ${S2_E} ${S2_L} ${S2_W} ${S2_OUT} ${S2_LOG} ${S2_E0} ${S2_E1} ${S2_E2}
      echo -e "${S2_TEST_FILE}:"
      time DG_DBG=0 ${S2_BIN}\
        ${EXTRA_OPTIONS}\
        --simple-name\
        --file=${S2_TEST_FILE}\
	--pp-out-file=/dev/null\
        --err-file=${S2_E}\
        --log-file=/dev/null\
        --warn-file=${S2_W}\
        --e0-file=/dev/null\
        --e1-file=${S2_E1}\
        --e2-file=/dev/null\
        $@\
        > ${S2_OUT}
      err=$?
      echo -e "$err\n"
      return $err
    ;;
  esac
}

main $@
