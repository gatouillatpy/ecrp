@echo off
cls
set PATH="C:\Program Files (x86)\PuTTY";%PATH%
plink ovh -t -pw %OVH_PW% -m stop.sh
echo The server have been stopped. Ready to restart?
pause
plink ovh -pw %OVH_PW% -m start.sh
echo All clear.
pause
