#!/bin/sh
SCRIPT=`readlink -f $0`
SCRIPTPATH=`dirname $SCRIPT`
echo $SCRIPTPATH
cd $SCRIPTPATH
java -jar ant-ivy/ant-launcher.jar -nouserlib -noclasspath $@
