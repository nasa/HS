# core Flight System (cFS) Health and Safety Application (HS) 

## Introduction

The Health and Safety application (HS) is a core Flight System (cFS) 
application that is a plug in to the Core Flight Executive (cFE) component 
of the cFS.  
  
The HS application provides functionality for Application Monitoring, 
Event Monitoring, Hardware Watchdog Servicing, Execution Counter Reporting
(optional), and CPU Aliveness Indication (via UART). 

The HS application is written in C and depends on the cFS Operating System 
Abstraction Layer (OSAL) and cFE components.  
There is additional HS application-specific configuration information
contained in the application user's guide.

User's guide information can be generated using Doxygen (from top mission directory):
```
  make prep
  make -C build/docs/hs-usersguide hs-usersguide
```

## Software Required

cFS Framework (cFE, OSAL, PSP)

A demonstration bundle of the Core Flight System including the cFE, OSAL, and PSP can be obtained at https://github.com/nasa/cfs

For information about a mission ready cFS bundle, see: https://github.com/nasa/cFS#cfs-gov-mission-ready-version

## Known issues

See all [open issues](https://github.com/nasa/HS/issues) and closed to milestones later than this version.

## Getting Help

For best results, submit issues:questions or issues:help wanted requests at <https://github.com/nasa/cFS>.

Official cFS page: <http://cfs.gsfc.nasa.gov>
