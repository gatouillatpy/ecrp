@echo off
cls
set PATH="C:\Program Files (x86)\PuTTY";%PATH%
plink ovh -t -pw %OVH_PW% -m stop.sh
echo All clear.
pause
