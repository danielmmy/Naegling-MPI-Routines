#ifndef __NAEGLING_MAIN_H_
#define __NAEGLING_MAIN_H_

#include <sys/stat.h>
#include <pwd.h>
#include "mpi-control.h"
#include "naegling-com.h"

#define NAEGLING_CLUSTER_PORT 9291
#define NAEGLING_SERVER_PORT 9292
#define UDP_MESSAGE_MAX_SIZE 1024
#define TCP_MESSAGE_MAX_SIZE 5242880
#define MAX_UDP_MESSAGE_WAIT 5
#define MAX_IP_SIZE 16
#define MAX_MAC_SIZE 18
#define MAX_COMMAND_SIZE 1024
#define MAX_HOSTNAME_SIZE 256
#define MAX_PATH_SIZE 256
#define MAX_FILE_PATH_SIZE 512
#define SLEEP_TIME_TO_REQUEST_IP 20
#define NAEGLING_UID 1000
#define NAEGLING_GID 1000

extern const char* NAEGLING_PATH;
extern const char* NAEGLING_TEMPLATE_PATH;
extern const char* MESSAGE_DELIMITER;
extern const char* NETWORK_FILE_PATH;
extern const char* DHCPD_FILE_PATH;
extern const char* HOSTS_FILE_PATH;
extern const char* WORK_NODES_HOSTS_FILE_PATH;
extern const char* MACHINEFILE_FILE_PATH;
extern const char* EXPORTS_FILE_PATH;
extern const char* PXELINUX_CFG_DEFAULT_FILE_PATH;
extern const char* SERVER_IP_ADDRESS;
extern const char* ETH1_ADDRESS_PATH;
extern  char CLUSTER_NETWORK_ADDRESS[MAX_IP_SIZE];
extern const char* CONF_FILES_MARKER;
extern const char *NAEGLING_JOBS_PATH;
extern short int CLUSTER_STATUS;
extern  unsigned short int CURRENT_IP;
extern int FILE_TRANSFER_AVAILABLE;
#endif
