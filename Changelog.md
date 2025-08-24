The list below contains a list of changes made to Analog over and above those of the original 6.0 release from the Analog web site.

# 6.0.18
14/08/2024 	

- Merged changes by @figueredo-tec-br to replace PCRE with PCRE2, all thanks to @figueredo-tec-br
- Modified @figueredo-tec-br changes to minimally statically link PCRE2-10.44 on the main branch
- Updated Makefile with configuration and compilation of PCRE2
- Removed legacy PCRE from source repository
- Fixed Windows 11 and Server 2022 grouping misidentification
- Updated Android versions list
- Updated iPadOS versions list
- Updated iPhoneOS versions list
- Updated macOS versions list
- Added target=”top” to the hyperlinks in the footer to prevent them erroring when clicked in ReportMagic’s frameset output

# 6.0.17
20/06/2021 	

- Merged BSD support additions by laffer1, adding: DragonFly, MidnightBSD, MirBSD and NetBSD to the OS detection
- Pre-emptive Windows 11/Windows Server 2022 support
- Added macOS 11 support
- Added Android 11 and 12 support
- Expanded current iPad/iPod/iPhone (iOS) OS support
- Added pre-emptive support for macOS 12, 13, 14, 15 and iOS up to version 19

# 6.0.16
27/07/2019 	

- New config file parameter: **ANONYMIZERURL**
- Allows OUTPUT HTML/XHTML to hyperlink via an anonymization service, to prevent exposure of the public stats URL to linked sites. This has no impact on XML or COMPUTER (Report Magic) output.
- View: [Creating a Link Anonymiser Service for Analog CE’s ANONYMIZERURL setting](https://www.c-amie.co.uk/technical/creating-a-link-anonymiser-service-for-analog-ces-anonymizerurl-setting/)
- New config file parameter: **LINKNOFOLLOW**
- This setting is enabled by default. Appends rel=”nofollow” to external report links. Reduces the impact of a compromised Analog CE stats page being used to generate SEO spam.
- **Operating System Report:** Mac OS X branding replaced with macOS
- **Operating System Report:** Added macOS code names
- **Operating System Report:** Added pre-emptive detection for iPad iOS 14 and 15
- **Operating System Report:** Added pre-emptive detection for iPhone iOS 14 and 15
- Fixed legacy default URL value for LOGOURL config file setting
- Updated Sample config file is now bundled with the binary releases

# 6.0.15
28/04/2019 	

- **Operating System Report:** Windows Phone 8.0, 8.1 and 10.0 were all bundling into Windows Phone Unknown detection due to a missed user agent change
- Updated images folder in binary package
- Corrected lowercase A in Analog at the report footer

# 6.0.14
14/04/2019 	

- **Operating System Report:** Applied revised logic for Android 9 (pie) detection
- **Operating System Report:** Applied pie detection changes to all legacy Android versions in case of any missed legacy identification issues
- **Operating System Report:** Added Windows Server 2019 to the Windows 10/Windows Server 2016 readout as there is no way to identify between them from the User Agent
- Changed c-amie.co.uk URL’s to https://
- Changed the check for updated link to the changelog page
- Changed the application name to Analog CE

# 6.0.13
03/05/2018 	

- Debian Bug Report [#568693](http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=568593): Altered layout of the HTML/XHTML page title
- Debian Bug Report [#553491](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=553491): Modified XHTML header and validated outputusing W3C validator
- Debian Bug Report [#864738](https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=864738): Removed dead footer link to analog.cx
- **Operating System Report:** Added OS X 10.11 detection
- **Operating System Report:** Added OS X 10.12 detection
- **Operating System Report:** Added OS X 10.13 detection
- **Operating System Report:** Added OS X 10.14 detection
- **Operating System Report:** Added OS X 10.15 detection
- **Operating System Report:** Added OS X 10.16 detection
- **Operating System Report:** Added OS X 10.17 detection
- **Operating System Report:** Changed iPhone, iPad and iPod version labels to be iOS instead of the device
- **Operating System Report:** Added AppleTV detection
- **Operating System Report:** Added Android 6.0, 7.0, 8.0 and 9.0/9.1 detection support
- **Operating System Report:** Added Android code names to the output for Android 1.1 through 8.1
- **Operating System Report:** Subsectioned the Linux detection, with detection support for Fedora, Debian, Mint, Ubuntu, Kubuntu, CentOS, Gentoo, Lindows, Mandrake (if supported by the UA)
- **Operating System Report:** Added OpenBSD detection
- **Operating System Report:** Added FreeBSD detection
- **Operating System Report:** Added Chrome OS detection
- **Operating System Report:** Added Chromecast detection
- **Operating System Report:** Added Sony Playstation 4 detection
- **Operating System Report:** Added Nintendo WiiU, 3DS and Switch detection

# 6.0.12
28/06/2015 	

- **BUG FIX:** Updated the C:Amie RSS Feed Link and Website Link to reflect the new URL
- **Browser Summary:** Added Microsoft Edge detection
- **Operating System Report:** Added Windows 10 / Windows Server 2016 detection
- **Operating System Report:** Added Windows Phone 10 detection
- **Operating System Report:** Added Android 5.0, 5.1 detection
- **Operating System Report:** Added iPhone 8.0, 9.0 , iPad 8.0, 9.0 , iPod 8.0, 9.0 detection
- **Operating System Report:** Added OS X 10.10 detection

# 6.0.11
07/10/2013 	

- **Browser Summary:** Altered MSIE detection algorithm to factor in the MSHTML (Trident) version being used. This will mean that MSIE is reported based upon the renderer version and not the user agent as defined by the compatibility mode – as suggested by Matt Brown
- **Operating System Report:** Added Windows 8.1 / Windows Server 2012 R2 detection
- **Operating System Report:** Added Windows Phone 8.1 detection
- **Operating System Report:** Added Android 4.2, 4.3, 4.4 detection
- **Operating System Report:** Added iPhone 7.0, iPad 7.0, iPod 7.0 detection

# 6.0.10
08/10/2012 	

- **Branch Merge:** Integrated secondary Apple iOS UA syntax heuristic as noted by [Ian Neath](http://memory.psych.mun.ca/) in a test release into the main branch
- **Operating System Report:** Improved iPhone 5.0 detection heuristic
- **Operating System Report:** Improved iPad 5.0 detection heuristic
- **Operating System Report:** Improved iPod 5.0 detection heuristic
- **Operating System Report:** Preemptively added iPhone 6.0, iPad 6.0, iPod 6.0 detection
- Default CSS file now included in the binary download
- Updated graphics files included in the binary download
- An OS X GCC compiled binary is now available

# 6.0.9
17/08/2012 	

- **BUG FIX:** Fixed crashing issue caused by Windows Phone 7 Search app which presents an improper user agent
- **BUG FIX:** Fixed sporadic crashing issue caused by the mobile device parser
- **Operating System Report:** Added Windows Phone 6.5 detection
- **Operating System Report:** Added user agent ‘Windows Phone OS 7.1’ to Windows Phone OS 7.5 (Mango) detection
- **Operating System Report:** Added Windows Phone OS 8.0 detection
- **Operating System Report:** Added Android 4.1 (Jelly Bean) detection
- **Operating System Report:** Added Windows 8.0 / Windows Server 2012 detection
- **Operating System Report:** Created Operating System Category group for Apple OS X. 10.0 – 10.9 can now be individually identified

# 6.0.8
18/07/2011 	

- **Operating System Report:** Added Windows Phone OS 7.5 (Mango) detection
- **Operating System Report:** Added Android 2.3 (Gingerbread) detection
- **Operating System Report:** Added Android 3.0 (Honeycomb) detection
- **Operating System Report:** Added Android 3.1 (Honeycomb) detection
- **Operating System Report:** Added Android 3.2 (Honeycomb) detection
- **Operating System Report:** Added Android 4.0 (Ice Cream Sandwich) detection
- **Operating System Report:** Added iOS 5.0 (iPhone/iPad) detection
- Added update check link to the Analog C:Amie Edition RSS feed to the report output

# 6.0.7
15/11/2010 	

- **Operating System Report:** Added Windows Phone OS 7.0 detection
- **Browser Summary:** Added IEMobile (Windows Phone) Support to the Browsers List

# 6.0.6
04/09/2010 	

- **Operating System Report:** Created Operating System Category group for iOS (Apple)
- **Operating System Report:** Improved iPhone detection toinclude segmented major OS versions
- **Operating System Report:** Added Android 1.6 detection
- **Operating System Report:** Added iPad detection with segmented OS versions
- **Operating System Report:** Added iPod detection with segmented OS versions
- A Linux GCC compiled binary is now available

# 6.0.5
03/09/2010 	

- **Browser Summary:** Added Google Chrome Support to the Browsers List
- **Operating System Report:** Created Operating System Category group for Apple
- **Operating System Report:** Separated “Macintosh” and “OS X” entries into separate items
- **Operating System Report:** Added iPhone detection
- **Operating System Report:** Added Android detection
- **Operating System Report:** Changed “Windows 7” detection to “Windows 7/Server 2008 R2” to reflect the common user agent

# 6.0.4
04/04/2009 	

- **BUG FIX:** The Analog 6.0.0 and Analog C:Amie Edition 6.0.3 use a DTD for XML output that was designed for use against Analog 5.9 beta 2 and was incompatible with the version 6.0 XML output. Several bugs existed in the DTD that prevented XML output use.
- **Settings:** Added new analog.cfg settings option. XMLDTD “http://…/file.dtd”. This bug and setting only effects XML output. The hard coded link to the analog 5.9 beta 2 DTD remains the default option, however by specifying the XMLDTD option you can specify a source to your own DTD and override the original DTD/DTD bugs. For example:
XMLDTD “http://www.c-amie.co.uk/qlink/?id=67”
- **Operating System Report:** Fixed Windows 7.0 Detection issues for OS Release Candidate
- **Operating System Report:** Combined Windows Server 2008 and Vista OS detection

# 6.0.3
06/10/2007 	

- Added hyperlink back to both analog.cx and www.c-amie.co.uk on HTML, XHTML and Computer (ReportMagic) output
- **Operating System Report:** Added specific statements for NT 4.0
- **Operating System Report:** Added Windows 7.0 detection (Blackcomb/Vienna)
- **Operating System Report:** Corrected error in Windows Server 2008 OS detection

# 6.0.2
02/10/2007 	

- Fixed the icon transparency corruption in the release binary
- **Operating System Report:** Added Windows Server 2008 to OS detection list

# 6.0.1
02/10/2007 	

- Initial C:Amie Edition release
- Added new high-resolution icon
- Changed version numbers
- Changed Edition to Analog C:Amie Edition
- **Operating System Report:** Added Windows Vista to OS detection
- **Operating System Report:** Changed ‘Windows 3.1’ detection to include Windows 3.10, 3.11 and NT 3.51 which use the same agent
- **Browser Summary:** Added NetFront to Browser detection

# 6.0
29/06/2005 	

- Initial release sourced from www.analog.cx



