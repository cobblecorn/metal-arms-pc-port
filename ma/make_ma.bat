echo off
if "%1"=="" GOTO ERROR
if "%2"=="" GOTO ERROR
cd \code\src\lib\smallamx
call make_small_%1.bat %2
cd \code\src\lib\fang2\dx\%1
call make_fang_%1.bat %2
cd \code\src\app\ma\%1
call make_ma_%1.bat %2
goto DIDIT

if "%3"=="" GOTO DIDIT
cd \code\src\lib\smallamx
call make_small_%1.bat %3
cd \code\src\lib\fang2\dx\%1
call make_fang_%1.bat %3
cd \code\src\app\ma\%1
call make_ma_%1.bat %3
goto DIDIT

if "%4"=="" GOTO DIDIT
cd \code\src\lib\smallamx
call make_small_%1.bat %4
cd \code\src\lib\fang2\dx\%1
call make_fang_%1.bat %4
cd \code\src\app\ma\%1
call make_ma_%1.bat %4
goto DIDIT

if "%5"=="" GOTO DIDIT
cd \code\src\lib\smallamx
call make_small_%1.bat %5
cd \code\src\lib\fang2\dx\%1
call make_fang_%1.bat %5
cd \code\src\app\ma\%1
call make_ma_%1.bat %5
goto DIDIT


:ERROR
echo "ERROR:: make_ma win|xb <debug> <release> <production> <test>"
GOTO EXIT
:DIDIT
GOTO EXIT
:EXIT
cd \code\src