#ifndef __NAEGLING_COM_H
#define __NAEGLING_COM_H

#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include "naegling-main.h"
#define MESSAGE_MAX_SIZE 1024


enum{
        START_NODE=1,
        STOP_NODE,
        CREATE_MASTER_NODE,
        CREATE_SLAVE_NODE,
        NODE_STATUS,
        TEMPLATE_STATUS,
        START_MASTER_VIRTUAL_NODE,
        STOP_MASTER_VIRTUAL_NODE,
        START_NEW_CLUSTER,
        ADD_WORKING_NODE,
        REMOVE_WORKING_NODE,
	GET_CLUSTER_STATUS,
	REQUEST_CLUSTER_IP,
        REQUEST_TEMPLATE_TRANSFER,
        REQUEST_JOB_TRANSFER,
	EXECUTE_JOB,
	DOWNLOAD_JOB_FILE
};


/*
 * Function to log error using syslog. ident->naegling_error.log
 * Parameters:
 * 	1-on_what: string with motive from failure. 
 */
void bail(const char *on_what);



/*
 * Function to log using syslog. ident->naegling_naegling.log
 * Parameters:
 *      1-message: string with the log message. 
 */
void naegling_log(const char *message);




/*
 * Function to create a virtual machine using libvirt and hardcoded xml file on a remote host.
 * Parameters:
 *	1-ip: Remote host ip.
 *      2-hypervisor: URI of the hipervisor(driver:///system).           
 *      3-vdisk_path: Path to the virtual image.
 *      4-name: Virtual machine domain name.
 *      5-memory: Ram memory size.
 *      6-cpu: CPU cores quantity.
 */
void listen_for_remote_message();



/*
 * Function to deal with multiple connection with pthreads
 * PARAMETERS:
 *      1-session_fd: client socket descriptor(integer)
 */
void connection_handler(int session_fd);



/*
 * Counts field number from message
 * PARAMETERS:
 *	1-message
 * RETURNS
 *	count
 */
int get_field_count(char *message);


/*
 * Split the message in fiels based on the MESSAGE_DELIMITER
 * PARAMETERS:
 *	1-message.
 *	2-message_fields: bidimensional array to store the separeted fields.
 */
void get_message_fields(char *message,char **message_fields);

/*
 * Returns the cluster status
 * RETURNS:
 *	-1 error
 *	0 cluster ready
 *	1 cluster not ready
 */
int get_cluster_status();



/*
 * Sends message to naegling server
 * PARAMETERS:
 *	1-message: Message to be send.
 * RETURNS:
 *	NULL error.
 *	char * with return message on success.
 */
char * send_message_to_server(const char *message);


/*
 * Get ip to be used by the cluster
 * RETURNS 
 *      -1 error
 *      0 success.
 */
int get_cluster_ip_from_server();



int prepare_job_transfer(const char *job_name, const char *file_name,const int sock);
//void *job_file_transfer(void* args);


/*
 * Runs job
 * PARAMETERS:
 *	1-job_name
 *	2-script_name
 * RETURNS:
 *	-1: error
 *	0: success
 */
int run_job_script(const char *job_name,const char *script_name);



int send_cluster_file(const char *path, int server_sock);

/*
int prepare_send_file_to_naegling_server(const char *job_name, const char *file_name);
void *send_file_to_naegling_server(void *arg);
*/
#endif
