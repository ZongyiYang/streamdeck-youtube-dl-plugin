set pluginName=youtube-dl-plugin

taskkill /IM StreamDeck.exe /F
timeout /t 1

del Release\com.elgato.%pluginName%.streamDeckPlugin
xcopy /y Sources\Windows\Release\%pluginName%.exe Sources\com.elgato.%pluginName%.sdPlugin\
DistributionTool.exe -b -i Sources\com.elgato.%pluginName%.sdPlugin -o Release

rmdir /s /q "C:\Users\%USERNAME%\AppData\Roaming\Elgato\StreamDeck\Plugins\com.elgato.%pluginName%.sdPlugin"
start "" "Release/com.elgato.%pluginName%.streamDeckPlugin"

pause