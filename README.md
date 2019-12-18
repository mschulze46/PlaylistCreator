# Playlist Creator
Playlist Creator is the classical one-trick pony: Creates playlists - no more, no less.
Originally developed by oddgravity, now released to the general public.

Be aware that the codebase is rather old and uses C++ with MFC (Microsoft Foundation Classes). As for the IDE Visual Studio 2008 was used.

So if somebody wants to pick up development I would recommend to do a rewrite in a modern language. My choice would have been C# with WPF but I never found the spare time to do this.

Cheers
Michael

## System Environment
* IDE: Microsoft Visual Studio 2008
* Language: C++ with MFC (Microsoft Foundation Classes)
* Installer: NSIS

## Getting Started
The following build environment was installed on a Windows 7 machine:
1. Install Visual Studio 2008 incl. Service Pack 1
  * [Microsoft Visual Studio 2008 Service Pack 1 (Installer)](https://www.microsoft.com/en-US/download/details.aspx?id=10986)
  * [Microsoft Visual Studio 2008 Service Pack 1 (iso)](https://www.microsoft.com/en-US/download/details.aspx?id=13276)
2. Install .NET Framework 4 Full
  * [Microsoft .NET Framework 4 (Standalone Installer)](https://www.microsoft.com/en-US/download/details.aspx?id=17718)
3. Install Microsoft Windows SDK for Windows 7
  * [Microsoft Windows SDK for Windows 7 and .NET Framework 4](https://www.microsoft.com/en-us/download/details.aspx?id=8279)
4. Configure Windows SDK in Visual Studio 2008
  * Open Visual Studio 2008
  * Goto: Tools > Options... > Projects and Solutions > VC++ Directories
  * Include files: Add Platform SDK include directory at first position
    ```
    C:\Program Files\Microsoft SDKs\Windows\v7.1\Include
    ```
    ![VS2008-WinSDK-1](https://github.com/mschulze46/PlaylistCreator/blob/master/docs/GettingStarted-VS2008-WinSDK-1.png "GettingStarted-VS2008-WinSDK-1")
  * Library files: Add Platform SDK library directory at first position
    ```
    C:\Program Files\Microsoft SDKs\Windows\v7.1\Lib
    ```
    ![VS2008-WinSDK-2](https://github.com/mschulze46/PlaylistCreator/blob/master/docs/GettingStarted-VS2008-WinSDK-2.png "GettingStarted-VS2008-WinSDK-2")

## License
Playlist Creator is MIT-licensed.
