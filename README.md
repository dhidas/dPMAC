# dPMAC

This is a tool and terminal application for PMAC2 Turbo boards writted in c++ using sockets.

There are tools to download, backup, restore, grab the gather buffer, and change IP addresses.  Usage is partially described below, per executable.

## To compile:
make

executables land in bin/

## Executables

### dpterm
Open a terminal application to a pmac:

dpterm [IP address]
```
>> .help
Commands:
  .help                              - print help
  .quit                              - quit
  .download [file]                   - Download file to pmac
  .logging  [file]                   - logging (without [file] is to stop, with will log to file
  .gather   [file]                   - Upload gather buffer from pmac to file
  .backup   [file]                   - Upload backup CFG from pmac to file
  .ivars    [file] [start] [stop]    - dump I variables to file (start stop optional integers)
  .pvars    [file] [start] [stop]    - dump P variables to file (start stop optional integers)
  .qvars    [file] [start] [stop]    - dump Q variables to file (start stop optional integers)
  .mvars    [file] [start] [stop]    - dump M variables to file (start stop optional integers)
  .mdefs    [file] [start] [stop]    - dump M variable definitions to file (start stop optional integers)
  .cat      [file] [start] [stop]    - print file from line start to stop
  .ip       [addr]                   - get ip, or set if [addr] is given
  .watch    [cmds]                   - watch vars
```
  
### dpbackup
Make a backup file from what is currently on the PMAC
```  
dpbackup [IP address] [OutFile]
```

### dpdownload
Download a file to PMAC.  Does not issue a save
```
dpdownload [IP address] [InFile]
```

### dprestore
Download file to PMAC from factory reset.  Issues the following commands in order:
$$$***
save
$$$
[the download]
save
$$$
```
dprestore [IP address] [InFile]
```

### dpip
Change the IP address on a device (make sure to hold the switch on PMAC)
```
dpip [IP address] [new IP address]
```

### dpgather
Download the gather buffer in hex format from PMAC
```
dpgather [IP address] [OutFile]
```
