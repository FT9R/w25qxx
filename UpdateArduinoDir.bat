del ".\Examples\ArduinoNano\w25qxx.*"
del ".\Examples\ArduinoNano\w25qxx_Demo.*"

xcopy ".\w25qxx\w25qxx.*" ".\Examples\ArduinoNano"
xcopy ".\w25qxx\w25qxx_Demo.*" ".\Examples\ArduinoNano"

rename ".\Examples\ArduinoNano\*.c" "*.cpp"