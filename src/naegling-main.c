#include "naegling-main.h"

const char* NAEGLING_PATH={"/opt/naegling"};
const char* NAEGLING_TEMPLATE_PATH={"/opt/naegling/templates"};
const char* MESSAGE_DELIMITER={"#"};
const char* NETWORK_FILE_PATH={"/etc/network/interfaces"};
const char* DHCPD_FILE_PATH={"/etc/dhcp/dhcpd.conf"};
const char* HOSTS_FILE_PATH={"/etc/hosts"};
const char* WORK_NODES_HOSTS_FILE_PATH={"/home/work_nodes/etc/hosts"};
const char* MACHINEFILE_FILE_PATH={"/home/naegling/machinefile"};
const char* PXELINUX_CFG_DEFAULT_FILE_PATH={"/var/lib/tftpboot/pxelinux.cfg/default"};
const char* EXPORTS_FILE_PATH={"/etc/exports"};
const char* SERVER_IP_ADDRESS={"192.168.125.1"};
const char* ETH1_ADDRESS_PATH={"/sys/class/net/eth1/address"};
const char* CONF_FILES_MARKER={"##@$!##"};
const char *NAEGLING_JOBS_PATH={"/home/naegling/jobs"};
char CLUSTER_NETWORK_ADDRESS[MAX_IP_SIZE];
short int CLUSTER_STATUS=1;
unsigned short int CURRENT_IP=1;
int FILE_TRANSFER_AVAILABLE=1;

int main (int argc, char *argv[]){
	/*
	 * Initialize CLUSTER_NETWORK_ADDRESS as a string to
	 * avoid seg fault
	 */
	CLUSTER_NETWORK_ADDRESS[0]='\0';


	/*
	 * Check if naegling directory exists, if not create
	 */
	int retval=0;
	struct stat status;
	retval=stat(NAEGLING_PATH,&status);
	if(retval==-1){
		naegling_log("Creating naegling directory...");
		retval=mkdir(NAEGLING_PATH,S_IRWXU|S_IRWXG|S_IROTH);
		if(retval==-1){
			bail("Error creating naegling directory.");
			fprintf(stderr,"Could not start naegling daemon.\n");
			return -1;
		}
	}else{
		naegling_log("Naegling directory exists.");
	}

	retval=stat(NAEGLING_TEMPLATE_PATH,&status);
	if(retval==-1){
		naegling_log("Creating naegling templates directory...");
                retval=mkdir(NAEGLING_TEMPLATE_PATH,S_IRWXU|S_IRWXG|S_IROTH);
                if(retval==-1){
                        bail("Error creating naegling template directory.");
			fprintf(stderr,"Could not start naegling daemon.\n");
                        return -1;
                }
	}else{
		naegling_log("Naegling template directory exists.");
	}


	/*
	 * End of directory creation
	 */




        /*
         * Check if naegling jobs directory exists, if not create
         */
        retval=stat(NAEGLING_JOBS_PATH,&status);
        if(retval==-1){
                naegling_log("Creating naegling JOBS directory...");
                retval=mkdir(NAEGLING_JOBS_PATH,S_IRWXU|S_IRWXG|S_IROTH);
		retval=chown(NAEGLING_JOBS_PATH,NAEGLING_UID,NAEGLING_GID);
                if(retval==-1){
                        bail("Error creating naegling jobs directory.");
                        fprintf(stderr,"Could not start naegling daemon.\n");
                        return -1;
                }
        }else{
                naegling_log("Naegling jobs directory exists.");
        }
        /*
         * End of directory creation
         */





	/*
	 * Creating the daemon
	 */
	 pid_t pid, sid;
        
        /* Fork off the parent process */
        pid = fork();
        if (pid < 0) {
		bail("Error forking process.");
		fprintf(stderr,"Could not start naegling daemon.\n");
                return -1;
        }
        /* If we got a good PID, then
         * we can exit the parent process.
	 * The parent will wait and try to request an ip 
	 * before exiting.
	 */
        if (pid > 0) {
		naegling_log("Process forked successfully.");
		
//		printf("Waiting for cluster ip...\n");		
//		sleep(SLEEP_TIME_TO_REQUEST_IP);
//		int retval=get_cluster_ip_from_server();		
//		if(!retval)
//			printf("IP acquired.\nNew cluster started.\n");
//		else
//			fprintf(stderr,"Error starting new cluster...");

                return 0;
        }

        /* Change the file mode mask */
        umask(0);

	/* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0) {
                bail("Error creating child process SID.");
		fprintf(stderr,"Could not start naegling daemon.\n");
                return -1;
        }

	/* Change the current working directory */
        if ((chdir("/")) < 0) {
                bail("Error changing working directory.");
		fprintf(stderr,"Could not start naegling daemon.\n");
		return -1;
        }


	fprintf(stdout,"Naegling daemon started successfully.\n");

	/* Close out the standard file descriptors */
        //close(STDIN_FILENO); FIXME commented for debug uncomment after
        //close(STDOUT_FILENO);
        //close(STDERR_FILENO);
	/*
	 * End of daemon creation
	 */

	listen_for_remote_message();
	return 0;
}
