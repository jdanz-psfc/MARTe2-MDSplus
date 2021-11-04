#!/bin/bash
# script to run RABBIT test directly from the command line (toki)
if [[ -z "$1" ]]
then
 mytest="all"
else
 mytest=$1
fi

source set_RABBIT_env.bash
$matlab850cmd -r "try, flag = run_RABBIT_tests('$mytest'); exit(~flag); catch ME, disp(getReport(ME)), exit(1); end"
CODE=$? # exit code of matlab run
echo "exit with CODE" $CODE
exit $CODE
