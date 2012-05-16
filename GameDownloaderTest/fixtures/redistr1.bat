if NOT "%1" == "" (
	echo %1 %2 %3 %4 %5 %6 %7 %8 > %0.txt 
)

if "%1" == "" (
	echo "NO ARGS" > %0.txt
)