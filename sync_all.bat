@echo off
cls
set PATH="C:\Program Files (x86)\PuTTY";%PATH%
plink ovh -t -pw %OVH_PW% -m stop.sh
echo The server have been stopped. Ready to upload?
pause
psftp ovh -pw %OVH_PW% -b upload.sh
echo All files have been uploaded. Build?
pause
plink ovh -t -pw %OVH_PW% -m build.sh
echo Build complete. Restart?
pause
plink ovh -pw %OVH_PW% -m start.sh
echo All clear.
pause
