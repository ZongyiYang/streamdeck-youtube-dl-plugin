if not exist "Sources\Vendor" mkdir Sources\Vendor

echo "Downloading youtube-dl:"
powershell -Command "Invoke-WebRequest https://github.com/ytdl-org/youtube-dl/releases/download/2020.11.18/youtube-dl.exe -OutFile Sources\Vendor\youtube-dl.exe"

echo "Downloading ffmpeg:"
powershell -Command "Invoke-WebRequest https://github.com/BtbN/FFmpeg-Builds/releases/download/autobuild-2020-11-17-12-29/ffmpeg-N-99928-g96f1b45b8c-win64-lgpl.zip -OutFile Sources\Vendor\ffmpeg-N-99928-g96f1b45b8c-win64-lgpl.zip"
echo "unzipping:"
powershell -command "Expand-Archive Sources\Vendor\ffmpeg-N-99928-g96f1b45b8c-win64-lgpl.zip Sources\Vendor\ffmpeg"
del /f /q Sources\Vendor\ffmpeg-N-99928-g96f1b45b8c-win64-lgpl.zip

move Sources\Vendor\ffmpeg\ffmpeg-N-99928-g96f1b45b8c-win64-lgpl\bin\ffmpeg.exe Sources\Vendor\ffmpeg.exe
rmdir /s /q Sources\Vendor\ffmpeg

pause