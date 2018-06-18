/*
 * /*
AUTHOR: 			Liam Stannard
DATA:   			09/05/18
DESCRIPTION:		This is a source file that implements some minix system calls which do the following: select an array of process ids from a specified index in the PM's process table,
get the name of a process (identified by its pid),
get information about the children of a given process (identified by its pid)
ACHIEVED: 9.25/10
*/
 
#include <stdio.h>
#include "pm.h"             // for glo.h:  mp, call_nr, who_p etc.
#include "mproc.h"          // for proc table
#include <string.h>         // for strlen or strnlen

#define INVALID_ARG -22     /* gets converted to errno and -1 return value
                             * return instead of -1 when detect invalid 
                             * argument. Do not set errno in system calls.
                             * See: See: /usr/src/lib/libc/sys-minix/syscall.c
                             */

int do_getpids() {
	
	//message data used by the system call
	int idx = m_in.m1_i1;					// starting index position in table
	int n = m_in.m1_i2; 					// max number of pids to populate pids array
	int flags_mask = m_in.m1_i3;			// mask to use for selecting pids
	pid_t *homeArr = (pid_t*)m_in.m1_p1;	// array to store pids
	
	//local variables
	pid_t pids[n+1];
	int i =0;
	int arrlen  = n+1;
	int flagsSwitchMatch = 0;
	
	//checks all variables passed by the message are valid
	if (n <= 0 || idx < 0 || idx >= NR_PROCS  ||homeArr==NULL ||flags_mask <0) 
		return INVALID_ARG;
		
		if (n > NR_PROCS)
			n = NR_PROCS;
	//sets the flag switch mode using an int as a boolean 0 = false 1 = true;
	if((flags_mask & MP_FLAGS_SWITCH) ==0)							
		flagsSwitchMatch = 0;
	else
		flagsSwitchMatch =1;
	
	//runs through up to a max of n times or until idx is >= NR_PROCS
        while (n > 0 && idx < NR_PROCS) {
        pid_t pid = mproc[idx].mp_pid;
		//if not null
        if (pid) {
			//if flagsSwitchMatch is 0 and flags mask and the current process flags have a match
			if(flagsSwitchMatch ==0 && (mproc[idx].mp_flags & flags_mask) > 0){
				//add pid to the array of matched processes
				pids[i] = pid;		
				n--;
				i++;
			}
			//if flagsSwitchMatch is 1 and the complement flags mask and the current process flags have a match
			if (flagsSwitchMatch ==1 && (mproc[idx].mp_flags &~ flags_mask)>0){
				//add pid to the array of matched processes
				pids[i] = pid;
				n--;
				i++;
			}	
        }
        idx++; // move to next idx, until found n non-zero pids
        }
		//fill the rest of the array with 0s
	while (i<arrlen) {
		pids[i] = 0;
		i++;
    }
		//copy the array to who has called the systemcalls address space
	int j = sys_vircopy( SELF,(vir_bytes)pids,  who_e, (vir_bytes) homeArr, (sizeof(pid_t)*(arrlen))); 
    if( j<0)
		return j;
	else
	return idx == NR_PROCS ? 0 : idx;// return the next index in table
}

int do_getprocname() {
    //message data used by the system call
	pid_t pid = m_in.m1_i1;					// process id to find in the table
	char name[PROC_NAME_LEN];				// array to store process name
	char* homeArr =(char*) m_in.m1_p1;		//destination array for data to be copied back to
	int idx =0;								//index of process table

	//checks all variables passed by the message are valid
	if (pid <0||m_in.m1_p1==NULL ) 
		return INVALID_ARG;

	//
    while (idx < NR_PROCS) {
		if (pid == mproc[idx].mp_pid && strlen(mproc[idx].mp_name)<= PROC_NAME_LEN) {
			strcpy(name, mproc[idx].mp_name);
			break;
		}
		idx++; // move to next idx, until found n non-zero pids
        }
	///if a name hasnt been found
	if(!name){
		//assign a blank string to the array
		strcpy(name,"");
	}	
	//copy back to other address space
    int j = sys_vircopy( SELF,(vir_bytes)name,  who_e, (vir_bytes) homeArr, (sizeof(char)*(PROC_NAME_LEN))); 
	if( j<0)
		return j;
	else
    return 0;
}

int do_getchildinf() {
	//message data used by the system call
	pid_t ppid = m_in.m1_i1; 									//parent process id
	int nchildren = m_in.m1_i2; 								//number of children to find
	int flags_mask = m_in.m1_i3;								//a flags mask used to select processes
	struct procinf cpinf[sizeof(struct procinf)*nchildren];		//local array used to store child process of ppid
	
	
	//local variables
	int idx =0;													//index of provess table
	int flagsSwitchMatch = 0;									//boolean value can be 0 = false or 1 = true for if flag switch matches flag_mask
	int childrenAdded = 0;										//number of children added
	int i =0;													//index of cpinf	
	struct procinf* homeArr =(struct procinf*) m_in.m1_p1;		//destination array for data to be copied back to
	//checks all variables passed by the message are valid
	if (nchildren < 0 || ppid < 0  || flags_mask <0 ||homeArr==NULL) 
		return INVALID_ARG;
		
	if (nchildren > NR_PROCS)
        nchildren = NR_PROCS;
	
	if((flags_mask & MP_FLAGS_SWITCH) ==0)
		flagsSwitchMatch = 0;
	else
		flagsSwitchMatch =1;
	
    while (idx < NR_PROCS && nchildren >childrenAdded) {
			//if parent id of process[idx] is equal to ppid then..
        if (ppid == mproc[mproc[idx].mp_parent].mp_pid) {
			if(flagsSwitchMatch ==0 && (mproc[idx].mp_flags & flags_mask) > 0 ){
				//assign process info to cpinf
				cpinf[i].ppid = ppid;
				cpinf[i].pid = mproc[idx].mp_pid;
				cpinf[i].flags =mproc[idx].mp_flags;
				childrenAdded++;
				i++;
			}
			if (flagsSwitchMatch ==1 && (mproc[idx].mp_flags &~ flags_mask)>0){
				//assign process info to cpinf
				cpinf[i].ppid = ppid;
				cpinf[i].pid = mproc[idx].mp_pid;
				cpinf[i].flags =mproc[idx].mp_flags;
				i++;
				childrenAdded++;
			}
        }
        idx++; // move to next idx, until found n non-zero pids
    }
	while (i<nchildren) {
		cpinf[i].ppid = ppid= 0;
		cpinf[i].pid = mproc[idx].mp_pid= 0;
		cpinf[i].flags = 0;
		i++;
    }
	//copy to desination address space
	int j =sys_vircopy( SELF,(vir_bytes)cpinf,  who_e, (vir_bytes) homeArr, (sizeof( struct procinf )*(nchildren)));
	if( j<0)
		return j;
	else
	return childrenAdded;// return the number of children added
}
