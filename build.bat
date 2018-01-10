@echo off
cls
set PATH="C:\Program Files (x86)\PuTTY";%PATH%
plink ovh -t -pw %OVH_PW% -m build.sh
echo All clear.
pause
