@echo off
setlocal
cd /d %~dp0
java -jar ant-ivy\ant-launcher.jar -nouserlib -noclasspath %*
REM setlocal causes the cd above to be undone when batch file exits
