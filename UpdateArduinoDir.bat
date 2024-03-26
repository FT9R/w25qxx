del ".\Examples\ArduinoNano\w25qxx.*"
del ".\Examples\ArduinoNano\w25qxx_Demo.*"

xcopy ".\w25qxx\w25qxx.h" ".\Examples\ArduinoNano"
xcopy ".\w25qxx\w25qxx.c" ".\Examples\ArduinoNano"
xcopy ".\w25qxx\w25qxx_Demo.h" ".\Examples\ArduinoNano"
xcopy ".\w25qxx\w25qxx_Demo.c" ".\Examples\ArduinoNano"

rename ".\Examples\ArduinoNano\w25qxx.c" "w25qxx.cpp"
rename ".\Examples\ArduinoNano\w25qxx_Demo.c" "w25qxx_Demo.cpp"