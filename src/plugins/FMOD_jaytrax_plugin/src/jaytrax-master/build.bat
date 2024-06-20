@echo off

call getcomp rosbe

set opts=-std=c99 -mconsole -Os -s -Wall -Wextra
set link=-lwinmm
set bin=bin

set ljayold=lib_oldjaytrax
set c_liboldjay=%ljayold%\jxs.c %ljayold%\jaytrax.c %ljayold%\mixcore.c
set cli=clitools
set c_cli=%cli%\main.c %cli%\winmmout.c

set includes=-I%ljayold% -I%cli%
set compiles=%c_cli% %c_liboldjay%
set errlog=.\jaytraxcli_err.log

set outname=oldjaytrax_cli
del %bin%\%outname%.exe
gcc -o %bin%\%outname%.exe %includes% %compiles% %opts% %link% 2> %errlog%

IF %ERRORLEVEL% NEQ 0 (
    echo oops!
    notepad %errlog%
    goto :end
)
for %%R in (%errlog%) do if %%~zR lss 1 del %errlog%
:end