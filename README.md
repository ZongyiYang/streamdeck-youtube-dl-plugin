# Description

**NOTE: Youtube-DL has been replaced with the drop-in replacement [yt-dlp](https://github.com/yt-dlp/yt-dlp). Therefore, as of release 1.1, the internal exe bundled with this plugin is now yt-dlp instead of youtube-dl.**

This is a Windows plugin for the Elgato Stream Deck that executes youtube-dl (now yt-dlp) on a URL link copied to the clipboard once the button is pressed. It also has the additional feature of downloading media content from reddit URLs.

This plugin differs from a SuperMacro solution in the following ways:
-	This plugin has no dependancies. It is a lightweight solution for only this specific task.
-	By controlling the yt-dlp.exe process, this plugin can keep track of multiple downloads, log the reason for any failures, and gives the option to kill hanging download tasks.
-	This plugin allows for a easy way to update yt-dlp without the need for extra macro keys.
-	By holding the button, this plugin opens the output folder to allow easy access to recent downloads.
-	This plugin also downloads images from reddit if given a reddit URL.

**Note: it is suggested that the 'Update' button in 'Advanced Settings' is pressed to `Update yt-dlp` before use since the default one bundled with this application is likely outdated. Youtube/Twitch/etc often changes, and yt-dlp will require an update in order to handle whatever changed in the API.**

![](example.png)

# Use cases:
-	Archive videos or images from Reddit posts, Youtube, Twitch clips, etc.
-	Multiple buttons with different output folders allows for easy organization of downloads.

# Basic Settings

![](basicsettings.png)

`Label`

Label you can set to keep track of the button's functionality. It will be displayed on the Stream Deck button.

`Download Settings`

Settings for the button's functionality. One or more options can be selected and the button will attempt them all when the button is pressed, using the most recently copied content in your clipboard as the url:

Download Video: Attempt the download of a mp4 or audio-less mp4 from the url.

Download Audio: Attempt the download of a mp3 file.

Reddit Download: Attempt the download of an image from a reddit post.

Multiple download settings can be enabled. For example, enabling both Reddit Download and Download Video will have the button attempt to do both.

The "Reddit Download" option is special in that it does not log a failure if Video or Audio downloads were selected and downloaded successfully. It will only trigger a failure if it is the only download method selected, or if every selected download method has failed.

**Developer Note: Ideally yt-dlp should be able to download images by itself, removing the need for this extra option, but it is currently not a feature in the application so it is implemented separately in this plugin.**

`Output Folder`

The output folder location for where the downloaded content will be saved. Holding the button for this plugin down will open this folder.

# Advanced Settings

![](advancedsettings.png)

Max Downloads: Allows the user to limit the number of downloads as a safeguard against downloading from a playlist link with many videos.

yt-dlp Path: Allows the user to set a custom path to yt-dlp.exe. This plugin unpacks it's own yt-dlp.exe directly from the plugin, but if the user chooses to use their own build they can place the file path here.

Custom Command: Allows the user to supply a custom yt-dlp command. The plugin will invoke this command as `<yt-dlp path> <your command> <url>` sequentially with any other download options selected in the Basic Settings. This allows the user to create custom youtube-dl commands for their prefered quality or resolution or playlist settings.

Command Preview: Gives the user a preview of all the calls to yt-dlp invoked by this button.

Update: Calls yt-dlp --update

Kill Tasks: Gives the user the option to kill any hanging download tasks. Kill Button Tasks kills only tasks launched by this button, and Kill All Tasks will kill pending tasks launched by all buttons.

# Error Logging

If the download could not be completed, a short error message is displayed on the button iteself. More detailed logs are available at:

`<Your AppData Folder>\Roaming\Elgato\StreamDeck\logs\com.elgato.youtube-dl-plugin0.log`

Causes of errors include:
- Bad clipboard data. The button will indicate a failure if the clipboard content is not a url.
- Bad output folder or missing yt-dlp.exe
- yt-dlp has failed for it's own reasons

# Installation

Download the file `com.elgato.streamdeck-youtube-dl-plugin.streamDeckPlugin` from the Release folder.

Executing this file will install the plugin to your Stream Deck.

After placing this plugin on your Stream Deck, it is required that the `Update` button in the `Advanced Settings` menu of this button is pressed to update yt-dlp. The older version of yt-dlp bundeled with this plugin is likely outdated and is only there to allow for a easy first time update.

It is recommended that this `Update` button is pressed if downloads from Youtube/Twitch/etc stop working (check with the [yt-dlp Github](https://github.com/yt-dlp/yt-dlp).

# Developer Tools

The script `setup.bat` downloads ffmpeg and yt-dlp exes from their repos and places them in `Sources\Vendor`.

The script `reinstall.bat` repackages a plugin (that must be already built and located in `Sources\Windows\Release`) using the Elgato provided `DistributionTool.exe` into a plugin file usable by the Stream Deck device. It also shuts down the Stream Deck application, removes any instance of the previously installed plugin, reinstalls it, and restarts the Stream Deck application.

# Support the Developer

If you like the plugin and want to give a small donation, you can use the links below. Thanks!

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=95B98SPYFY3LS&currency_code=USD)

[![](Resources/DonateQR.png)](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=95B98SPYFY3LS&currency_code=USD)

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/6ee92c7c736a4f13875c3d3707c426f2)](https://app.codacy.com/gh/ZongyiYang/streamdeck-youtube-dl-plugin?utm_source=github.com&utm_medium=referral&utm_content=ZongyiYang/streamdeck-youtube-dl-plugin&utm_campaign=Badge_Grade_Settings)