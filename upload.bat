@echo off
cls
set PATH="C:\Program Files (x86)\PuTTY";%PATH%
psftp ovh -pw %OVH_PW% -b upload.sh
echo All clear.
pause
