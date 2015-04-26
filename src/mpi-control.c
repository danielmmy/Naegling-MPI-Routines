#include "mpi-control.h"



int start_new_cluster(const char *ip){
	CLUSTER_STATUS=1;

	char *network=(char *)malloc(sizeof(char)*MAX_IP_SIZE);
	/*calculate network*/
	int count=0;
	int i=0;
	do{
		if(ip[i]=='.')
			++count;
		network[i]=ip[i];
		++i;
	}while(count<3);
	network[i]='0';
	network[i+1]='\0';
	strcpy(CLUSTER_NETWORK_ADDRESS,network);
	CLUSTER_NETWORK_ADDRESS[i]='\0';

	int retval=reconfigure_network(ip,network);
	if(!retval){
		retval=reconfigure_dhcp(network);
		if(!retval){
			retval=reconfigure_hosts(HOSTS_FILE_PATH,ip);
			if(!retval){
				retval=reconfigure_machinefile();
				if(!retval){
					retval=reconfigure_hosts(WORK_NODES_HOSTS_FILE_PATH,ip);
					if(!retval){
						retval=reconfigure_exports();
						if(!retval){
							retval=reconfigure_pxelinux_cfg_default(ip);
						}
					}
				}
			}
		}
	}
	
	free(network);
	CLUSTER_STATUS=retval;
	
	return retval;
}

int reconfigure_network(const char * ip,const char *network){
	int retval=-1;
//	char *network=(char *)malloc(sizeof(char)*MAX_IP_SIZE);
//	int count=0;
//	int i=0;
//	do{
//		if(ip[i]=='.')
//			++count;
//		network[i]=ip[i];
//		++i;
//	}while(count<3);
//	network[i]='0';
//	network[++i]='\0';


	retval=system("service network-interface stop INTERFACE=eth1");
	FILE *fp=fopen(NETWORK_FILE_PATH,"w");
	if(fp&&retval!=-1){
		fprintf(fp,
			"auto lo\n"
			"iface lo inet loopback\n\n"
			"auto eth0\n"
			"iface eth0 inet dhcp\n\n"
			"auto eth1\n"
			"iface eth1 inet static\n"
			"	address %s\n"
			"	netmask 255.255.255.0\n"
			"	network %s\n"
			,ip,network);
		fclose(fp);
		retval=system("service network-interface start INTERFACE=eth1");
		if(retval!=-1){
			retval=0;
		}else{
			bail("Error starting network interface");
		}
		
	}else{
		bail("Error reconfiguring network.");
	}
	

	return retval;
}

int reconfigure_dhcp(const char *network){
	int retval=-1;
	retval=system("service isc-dhcp-server stop");
	if(retval==-1)	
                bail("Error stoping dhcp.");
	FILE *fp=fopen(DHCPD_FILE_PATH,"w");
        if(fp){
                fprintf(fp,
                        "default-lease-time 600;\n"
			"max-lease-time 7200;\n\n"
			"subnet %s netmask 255.255.255.0{\n"
			"%s\n"
			"}"
                        ,network,CONF_FILES_MARKER);
                fclose(fp);
        	retval=system("service isc-dhcp-server start");
		if(retval!=-1){
			retval=0;
		}else{
			bail("Error starting dhcpd.");
		}
        }else{
        }

	return retval;
}

int reconfigure_hosts(const char *path,const char *ip){
	int retval=-1;
	char *hostname=(char *)malloc(sizeof(char)*MAX_HOSTNAME_SIZE);
	char *command=(char *)malloc(sizeof(char)*MAX_COMMAND_SIZE);
	retval=gethostname(hostname,MAX_HOSTNAME_SIZE-1);
	if(!retval){
		sprintf(command,"echo \"127.0.0.1\tlocalhost\" > %s;echo -n %s >> %s;echo \"\t%s\" >> %s ",path,ip,path,hostname,path);
		retval=system(command);
		if(retval!=-1){
			retval=0;
		}else{
			bail("Error reconfiguring hosts file");
			retval=-1;
		}
	}else{
		retval=-1;
		bail("Error using gethostname(2).");
	}

	free(hostname);
	free(command);
	return retval;
}

int reconfigure_machinefile(){
	int retval=-1;
	char *hostname=(char *)malloc(sizeof(char)*MAX_HOSTNAME_SIZE);
        char *command=(char *)malloc(sizeof(char)*MAX_COMMAND_SIZE);
	retval=gethostname(hostname,MAX_HOSTNAME_SIZE-1);
	if(!retval){
	        sprintf(command,"echo %s > %s",hostname,MACHINEFILE_FILE_PATH);
       		retval=system(command);
	        if(retval!=-1){
        	        retval=0;
        	}else{
			retval=-1;
                	bail("Error reconfiguring machine file");
        	}
	}else{
		retval=-1;
                bail("Error using gethostname(2).");
	}

	free(hostname);
        free(command);
        return retval;

}

int reconfigure_exports(){
	int retval=-1;
	char *command=(char *)malloc(sizeof(char)*MAX_COMMAND_SIZE);
	sprintf(command,"echo  \"/export %s0/24(rw,fsid=0,insecure,no_subtree_check,async)\\n"
			"/export/mpich2 %s0/24(rw,nohide,insecure,no_subtree_check,async)\\n"
			"/export/naegling %s0/24(rw,nohide,insecure,no_subtree_check,async)\\n"
			"/home/work_nodes %s0/24(rw,no_root_squash,no_subtree_check,async)\\n"
			"\" > %s",CLUSTER_NETWORK_ADDRESS,CLUSTER_NETWORK_ADDRESS,CLUSTER_NETWORK_ADDRESS,CLUSTER_NETWORK_ADDRESS,EXPORTS_FILE_PATH);
	retval=system(command);
	if(retval!=-1){
		retval=system("service nfs-kernel-server reload");
		if(retval!=-1)
			retval=0;
	}
	return retval;
}

int reconfigure_pxelinux_cfg_default(const char *ip){
	int retval=-1;
	char *command=(char *)malloc(sizeof(char)*MAX_COMMAND_SIZE);
	sprintf(command,"sed -i 's/nfsroot=[^nfsroot=]*:/nfsroot=%s:/g' %s",ip,PXELINUX_CFG_DEFAULT_FILE_PATH);
	retval=system(command);
	if(retval!=-1){
		retval=0;
	}
	return retval;
}


int add_working_node(const char *hostname, const char *mac){
        int retval=-1;

	char *ip=insert_into_dhcp(hostname,mac);
	if(ip!=NULL){
		retval=insert_into_machinefile(hostname);
		if(1){
			retval=(insert_into_hosts(HOSTS_FILE_PATH,hostname,ip));
			if(1){
				retval=(insert_into_hosts(WORK_NODES_HOSTS_FILE_PATH,hostname,ip));
			}else{
				bail("add_working_node: changing hosts");
			}
		}else{
			bail("add_working_node: changing machinefile");
		}
		free(ip);
	}
        return retval;
}


char * insert_into_dhcp(const char *hostname,const char *mac){
	int retval=-1;
	char *command=(char *)malloc(sizeof(char)*MAX_COMMAND_SIZE);
	char *ip=(char *)malloc(sizeof(char)*MAX_IP_SIZE);
	naegling_log(CLUSTER_NETWORK_ADDRESS);
	sprintf(ip,"%s%d",CLUSTER_NETWORK_ADDRESS,CURRENT_IP);
	naegling_log(ip);
	sprintf(command,"sed -i 's/%s/\\thost %s{\\n"
						"\\t\\thardware ethernet %s;\\n"
						"\\t\\tfixed-address %s;\\n"
						"\\t\\toption host-name \"%s\";\\n"
						"\\t\\tfilename \"pxelinux.0\";\\n"
						"\\t}\\n\\n"
						"%s/' %s",CONF_FILES_MARKER,hostname,mac,ip,hostname,CONF_FILES_MARKER,DHCPD_FILE_PATH);
	retval=system("service isc-dhcp-server stop");
	retval=system(command);
	if(retval!=-1)
		++CURRENT_IP;
	else{
		bail("Editing dhcpd.conf");
	}
	retval=system("service isc-dhcp-server start");
	free(command);
	return ip;
}

int insert_into_machinefile(const char *hostname){
        int retval=-1;
        char *command=(char *)malloc(sizeof(char)*MAX_COMMAND_SIZE);
        sprintf(command,"echo %s >> %s",hostname,MACHINEFILE_FILE_PATH);
        retval=system(command);
        if(retval!=-1){
                retval=0;
        }else{
                retval=-1;
        	bail("Error reconfiguring machine file");
        }

        free(command);
        return retval;

}




int insert_into_hosts(const char *path,const char *hostname, const char *ip){
        int retval=-1;
        char *command=(char *)malloc(sizeof(char)*MAX_COMMAND_SIZE);
        sprintf(command,"echo \"%s\t%s\" >> %s ",ip,hostname,path);
        retval=system(command);
        if(retval!=-1){
                retval=0;
        }else{
                bail("Error reconfiguring hosts file");
                retval=-1;
        }

        free(command);
        return retval;
}


int remove_working_node(const char *hostname){
	int retval=-1;
	retval=remove_from_dhcp(hostname);
	if(!retval){
		retval=remove_from_machinefile(hostname);
		if(!retval){
			retval=remove_from_hosts(HOSTS_FILE_PATH,hostname);
			if(!retval){
				retval=remove_from_hosts(WORK_NODES_HOSTS_FILE_PATH,hostname);
			}
		}
	}
	return retval;
}

int remove_from_dhcp(const char *hostname){
	int retval=-1;
        char *command=(char *)malloc(sizeof(char)*MAX_COMMAND_SIZE);
        sprintf(command,"sed -i '/host %s{/,+6d' %s",hostname,DHCPD_FILE_PATH);
        retval=system(command);
        if(retval!=-1){
                retval=system("service isc-dhcp-server reload");
                if(retval!=-1)
                        retval=0;
                else
                        retval=-1;
        }else{
                retval=-1;
                bail("Error reconfiguring DHCP file");
        }

        free(command);

	return retval;
}

int remove_from_machinefile(const char *hostname){
	int retval=-1;
        char *command=(char *)malloc(sizeof(char)*MAX_COMMAND_SIZE);
        sprintf(command,"sed -i '/%s/d' %s",hostname,MACHINEFILE_FILE_PATH);
        retval=system(command);
        if(retval!=-1){
		retval=0;
        }else{
                retval=-1;
                bail("Error reconfiguring machinefile file");
        }

        free(command);

        return retval;
}

int remove_from_hosts(const char *path,const char *hostname){
	int retval=-1;
        char *command=(char *)malloc(sizeof(char)*MAX_COMMAND_SIZE);
        sprintf(command,"sed -i '/%s/d' %s",hostname,path);
        retval=system(command);
        if(retval!=-1){
                retval=0;
        }else{
                retval=-1;
                bail("Error reconfiguring hosts file");
        }

        free(command);

        return retval;
}
