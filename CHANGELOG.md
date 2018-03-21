## Next Release

BREAKING CHANGES: 
FEATURES: 
IMPROVEMENTS: 
BUG FIXES:

## v3.35
 
FEATURES:
 * add table users_data
 * add table revoked_tokens 
 * add jwt support
 * add web_pipe_count command for track_* pipes
 
## v3.34
 
FEATURES:
 * video api 1.01
 * add table users_data 
 
## v3.33
 
FEATURES:
 * video api 1.0

BUG FIXES:
 * fixed bug with large mysql packets

## v3.32
 
FEATURES:
 * now we can work with may FreeSwitch servers

IMPROVEMENTS:
 * add logs output to tcpServer
 * remove limits to maximum lines in answer (remove MAX_LINES_IN_ANSWER constant from code)

## v3.31

BUG FIXES:
 
 * errors in history strigs and db connections
 
## v3.30

BUG FIXES:
 
 * errors when restart mysql server
 * fix bug #36 CometServerApi.js:824 Invalid pipe name subscription web_chat_pipe chatControl
 * fix bug #35 _cometServerApi.prototype.send_msg("pipe_log\n"+pipe_name+"\n"+marker+"\n"); 
 * fix bug #34 tabSignal.connect is not a function 
 
## v3.26

BUG FIXES:
 
 * Command systemctl stop comet.service does not work on Ubuntu 16 ( #17 )

IMPROVEMENTS:
 
* Added pidfile option in section `main`.`pidfile`

* Added queryes like `set paramName paramValue`
* Added queryes like `set sectionName paramName paramValue`

## v3.25

IMPROVEMENTS:

* Added error messages for comet.ini parser

## v3.24

IMPROVEMENTS:

* Allowed adding spaces after section name in .ini file
 
BUG FIXES:

 * Was rename parameter `benchmark` to `statistics`

## v3.22

IMPROVEMENTS:

* Added functions for aggregating statistics of using server
* Added settings to .ini file for controll content-type header
 
## v3.21

IMPROVEMENTS:

* Added caching in memory for sending static files
* Added cache-control headers for get requests
 
## v3.2

BUG FIXES:
 * Js Api version 3.28 (Fix bug in auth callbacks)
 * Fix bug in auth callbacks
 * complete api function getTrackPipeUsers
 

## v3.1
 
FEATURES: 

 * Added function to getting users in track_pipes from js api
 * Added deleting api query for pipes history #5 
 * Added CODE_OF_CONDUCT.md
 * Added CONTRIBUTING.md
 * Added CHANGELOG.md
 * Added server version to ws answer