@echo off
@rem $Id$

call gssetgs.bat
%GSC% -q -sDEVICE#djet500 -r300 -dNOPAUSE -sPROGNAME=gsdj500 -- gslp.ps %1 %2 %3 %4 %5 %6 %7 %8 %9
