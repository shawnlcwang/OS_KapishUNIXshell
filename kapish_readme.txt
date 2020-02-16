/* 
 
 * File: kapish.c
 
 * Name: Li Ce (Shawn) Wang
 
 * ID: V00878878
 
*/

Program Description: 

KAPISH is an interactive UNIX shell that read lines from the terminal and repeatedly prompt (? ) and perform these following actions: 
	- read a line from standard input 
	- analyze line to form an array of tokens 
	- analyze token array to form command [options] [args]
	- execute the command  
The commands include both built-in commands and external executable binary code file in which is executed through preset PATH environment environment variable. 
Furthermore, here are potential misbehaviours that may involved with some commands: 
	- setenv var [value]
	- unsetenv var
	- cd [dir]
	- exit/ctrl-d
	- ctrl-c: After press ctrl-c there is a lag to the program stops due to program clean up (wait for the program to stop do not press ctrl-c twice)
	- executables

References/Collaboration: 
https://brennan.io/2015/01/16/write-a-shell-in-c/
https://github.com/dalmia/Operating-Systems/blob/master/3-Process%20Concept/project%20-%20shell/aman_shell.c
http://www.csl.mtu.edu/cs4411.choi/www/Resource/signal.pdf