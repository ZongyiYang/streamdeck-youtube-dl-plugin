if not exist "Sources\Vendor" mkdir Sources\Vendor

echo "Downloading yt-dlp:"
powershell -Command "Invoke-WebRequest https://github.com/yt-dlp/yt-dlp/releases/download/2022.05.18/yt-dlp.exe -OutFile Sources\Vendor\yt-dlp.exe"

echo "Downloading ffmpeg:"
powershell -Command "Invoke-WebRequest https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-master-latest-win64-lgpl.zip -OutFile Sources\Vendor\ffmpeg.zip"
echo "unzipping:"
powershell -command "Expand-Archive Sources\Vendor\ffmpeg.zip Sources\Vendor\ffmpeg"
del /f /q Sources\Vendor\ffmpeg.zip

move Sources\Vendor\ffmpeg\ffmpeg-master-latest-win64-lgpl\bin\ffmpeg.exe Sources\Vendor\ffmpeg.exe
rmdir /s /q Sources\Vendor\ffmpeg

pause