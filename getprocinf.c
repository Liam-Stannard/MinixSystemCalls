/*
AUTHOR: 			Liam Stannard
DATA:   			09/05/18
DESCRIPTION:		This is a program that implements some minix libary functions which are used to produce system calls to do the following: select an array of process ids from a specified index in the PM's process table,
get the name of a process (identified by its pid),
get information about the children of a given process (identified by its pid)
 */
#include <lib.h>      // provides _syscall and message
#include <unistd.h>   // provides function prototype
#include <errno.h>

int getpids(int idx, int n, int flags_mask, pid_t *pids) {
		//checks all variables passed by the message are valid
	if (n <= 0 || idx < 0 || idx >= NR_PROCS || pids==NULL  || flags_mask <0){
		errno = EINVAL;
		return -1;
	}
	//if n is bigger than the size of the process table make it the size of the process table
	if (n > NR_PROCS)
        n = NR_PROCS;

	//sets the message to be used in the system call
	message m;
    m.m1_i1 = idx;
	m.m1_i2 = n;
	m.m1_i3 = flags_mask;
    m.m1_p1 = (char*) pids;
	return _syscall(PM_PROC_NR, GETPIDS, &m);
}

int getprocname(pid_t pid, char *name) {
	//checks all variables passed are valid
	if(pid<0 || name ==NULL ){
		errno = EINVAL;
		return -1;
}
	//sets the message to be used in the system call
	message m;
    m.m1_i1 = (int) pid;
    m.m1_p1 = name;
    return _syscall(PM_PROC_NR, GETPROCNAME, &m);
	
}
        
int getchildinf(pid_t ppid, int nchildren, int flags_mask, 
    struct procinf *cpinf) {
		//checks all variables passed by the message are valid
	if (nchildren < 0 || ppid < 0 ||cpinf ==NULL || flags_mask <0) {
		errno = EINVAL;
		return -1;
	}
	//if n is bigger than the size of the process table make it the size of the process table
		if (nchildren > NR_PROCS)
        nchildren = NR_PROCS;
	//sets the message to be used in the system call
    message m;
    m.m1_i1 = ppid;
	m.m1_i2 = nchildren;
	m.m1_i3 = flags_mask;
    m.m1_p1 = (char*) cpinf;
	return _syscall(PM_PROC_NR, GETCHILDINF, &m);
}
