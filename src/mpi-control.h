#ifndef __MPI_CONTROL_H_
#define __MPI_CONTROL_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "naegling-main.h"


/*
 * Function to reconfigure all config files necessary to the cluster
 * PARAMETERS:
 *	1-ip: eth1 IP address to be set
 * RETURNS:
 *	-1 on error.
 *	0 if successful.
 */
int start_new_cluster(const char *ip);

/*
 * Function to reconfigure eth1 interface
 * PARAMETERS:
 *	1-ip: New IP address to be set.
 *	2-network
 * RETURNS:
 *	-1 on error.
 *	0 if successful.
 */
int reconfigure_network(const char *ip,const char *network);

/*
 * Function to reconfigure dhcp3-server
 * PARAMETERS:
 *      1-network: dhcp network.
 * RETURNS:
 *      -1 on error.
 *      0 if successful.
 */
int reconfigure_dhcpd(const char *network);



/*
 * Function to reconfigure hosts file
 * PARAMETERS:
 *	1-path: Path to conf file.
 *      2-ip: New IP address to be set.
 * RETURNS:
 *      -1 on error.
 *      0 if successful.
 */
int reconfigure_hosts(const char *path,const char *ip);


/*
 * Function to reconfigure machinefile file
 * RETURNS:
 *      -1 on error.
 *      0 if successful.
 */
int reconfigure_machinefile();


/*
 * Function to reconfigure nfs exports file
 * RETURNS:
 *      -1 on error.
 *      0 if successful.
 */
int reconfigure_exports();



/*
 * Function to reconfigure pxelinux.cfg default file
 * PARAMETERS:
 *      1-ip: New IP address to be set.
 * RETURNS:
 *      -1 on error.
 *      0 if successful.
 */
int reconfigure_pxelinux_cfg_default(const char *ip);


/*
 * Adds a new diskless working node to the cluster.
 * PARAMETERS:
 *      1-hostname: Working node hostname name
 *      2-mac: Working node MAC address.
 * RETURNS:
 *      -1 on error.
 *      0 if successful.
 */
int add_working_node(const char *hostname, const char *mac);



/*
 * Adds a new diskless working node to the dhcp. Used by add_working_node.
 * PARAMETERS:
 *      1-hostname: Working node hostname name
 *      2-mac: Working node MAC address.
 * RETURNS:
 *      NULL on error.
 *      IP address if successful.
 */
char * insert_into_dhcp(const char *hostname,const char *mac);


/*
 * Adds a new diskless working node to the mpi machinefile. Used by add_working_node.
 * PARAMETERS:
 *      1-hostname: Working node hostname name
 * RETURNS:
 *      -1 on error.
 *      0 on success.
 */
int insert_into_machinefile(const char *hostname);


/*
 * Adds a new diskless working node to the hosts file. Used by add_working_node.
 * PARAMETERS:
 *	1-path: Path to conf file.
 *      2-hostname: Working node hostname name
 *	3-ip: Working node IP address.
 * RETURNS:
 *      -1 on error.
 *      0 on success.
 */
int insert_into_hosts(const char *path, const char *hostname, const char *ip);


/*
 * Removes a diskless working node from the cluster.
 * PARAMETERS:
 *      1-hostname: Working node hostname name
 * RETURNS:
 *      -1 on error.
 *      0 if successful.
 */
int remove_working_node(const char *hostname);


/*
 * Removes a diskless working node from the dhcp. Used by remove_working_node.
 * PARAMETERS:
 *      1-hostname: Working node hostname name
 * RETURNS:
 *      -1 error.
 *      0 success.
 */
int remove_from_dhcp(const char *hostname);



/*
 * Removes a diskless working node from the mpi machinefile. Used by remove_working_node.
 * PARAMETERS:
 *      1-hostname: Working node hostname name
 * RETURNS:
 *      -1 on error.
 *      0 on success.
 */
int remove_from_machinefile(const char *hostname);


/*
 * Removes a diskless working node from the hosts file. Used by remove_working_node.
 * PARAMETERS:
 *      1-path: Path to conf file.
 *      2-hostname: Working node hostname name.
 * RETURNS:
 *      -1 on error.
 *      0 on success.
 */
int remove_from_hosts(const char *path,const char *hostname);
#endif
