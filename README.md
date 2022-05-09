core Flight System (cFS) Health and Safety Application (HS) 
===========================================================

Open Source Release Readme
==========================

HS Release 2.4.0

Date: 
8/27/2021

Introduction
-------------
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

  Developer's guide information can be generated using Doxygen: 
  doxygen hs_doxygen_config.txt

  This software is licensed under the Apache 2.0 license.  
 
 
Software Included
------------------
  Health and Safety application (HS) 2.4.0
  
 
Software Required
------------------

 HS v2.4.0 requires cFS Caelum, which can be be obtained 
 at https://github.com/nasa/cfs
  
About cFS
-----------
  The cFS is a platform and project independent reusable software framework and
  set of reusable applications developed by NASA Goddard Space Flight Center.
  This framework is used as the basis for the flight software for satellite data
  systems and instruments, but can be used on other embedded systems.  More
  information on the cFS can be found at http://cfs.gsfc.nasa.gov
                      
EOF                       
