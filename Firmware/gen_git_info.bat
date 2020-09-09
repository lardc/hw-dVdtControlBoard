@echo off
setlocal

set file_output=git_info.h

(
echo #ifndef __GIT_INFO_H
echo #define __GIT_INFO_H
echo.
echo #include "stdinc.h"
echo.
) > %file_output%

set brace="
set dot_comma=;

set branch_var=static const char git_branch[] =

for /f %%i in ('git branch --show-current') do set git_branch=%%i
set branch_str=%branch_var% %brace%%git_branch%%brace%%dot_comma%
echo %branch_str% >> %file_output%

set commit_var=static const char git_commit[] =

for /f %%i in ('git rev-parse HEAD') do set git_commit=%%i
set commit_str=%commit_var% %brace%%git_commit%%brace%%dot_comma%
echo %commit_str% >> %file_output%

set date_var=static const char git_date[] =

for /f %%i in ('git log -1 "--pretty=%%cd" "--date=format:%%Y/%%m/%%dT%%H:%%M:%%S"') do set git_date=%%i
set date_str=%date_var% %brace%%git_date%%brace%%dot_comma%
echo %date_str% >> %file_output%

(
echo.
echo #endif // __GIT_INFO_H
) >> %file_output%
