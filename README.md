# MyTasklist
@ author - Sergio Yerbes

<br/><br/>

## Usage:

MYTASKLIST [/S system /U username /P password]
         [/SVC | /V]
         
<br/><br/>         

## Description: 

Tasklist customized implementation with a similar functionality than the original Windows command. It retrieves process data from the system, locally or remotely.

<br/><br/>

## Parameter List:

/S : Remote system to connect to, corresponding to the system name. It queries DNS server for routing purposes Optional parameter defined always as the first parameter.

/U : Sets username for remote connections. Forced usage if /S flag is active.

/P : Sets password for remote connections. Also forced usage if /S flag is active.

/V : Verbose mode. It includes additional details of the processes. Optional Parameter.

/SVC : Retrieves the list of services for each processes. Idle process is ignored. Optional Parameter.

<br/><br/>

## Examples:

MyTasklist /S system /U user /P password /V

MyTasklist /V
