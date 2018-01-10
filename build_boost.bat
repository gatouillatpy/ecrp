@echo off
c:
cd c:\boost_1_66_0
call bootstrap
call b2 variant=debug,release link=shared,static runtime-link=shared address-model=32,64,32_64 -j8 --toolset=msvc-14.0 --build-type=complete stage
pause
