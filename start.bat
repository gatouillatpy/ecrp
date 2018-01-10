@echo off
cls
set PATH="C:\Program Files (x86)\PuTTY";%PATH%
plink ovh -pw %OVH_PW% -m start.sh
echo All clear.
pause
