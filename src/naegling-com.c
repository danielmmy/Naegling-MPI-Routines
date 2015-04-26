#include "naegling-com.h"


char file_path[MAX_FILE_PATH_SIZE];

void bail(const char *on_what){
	openlog("naegling_error.log",LOG_PID|LOG_CONS,LOG_USER);
	syslog(LOG_INFO,"Error: %s\n",on_what);
	closelog();
}

void naegling_log(const char *message){
        openlog("naegling_naegling.log",LOG_PID|LOG_CONS,LOG_USER);
        syslog(LOG_INFO,"Log: %s\n",message);
        closelog();

}

void listen_for_remote_message(){

        /*Try to start new Cluster*/   
        int ret_status=get_cluster_ip_from_server();
        if(!ret_status)
                naegling_log("New cluster started");
        else
                bail("Error starting new cluster");

        int server_fd;
        struct sockaddr_in server;

        //Create socket
        server_fd = socket(AF_INET , SOCK_STREAM , 0);
        if (server_fd == -1){
                bail("Could not create socket");
        }
        naegling_log("Socket created");

        //Prepare the sockaddr_in structure
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(NAEGLING_CLUSTER_PORT);

        //Bind
        if( bind(server_fd,(struct sockaddr *)&server , sizeof(server)) < 0){
                bail("bind failed. Error");
                fprintf(stderr,"Error bind(2)\n");
                exit(1);
        }
        naegling_log("bind done");

        //Listen
        listen(server_fd , SOMAXCONN);

	//accept loop and fork
        for(;;){
                int session_fd=accept(server_fd,0,0);
                if(session_fd==-1) {
                        if(errno==EINTR)
                                continue;
                        bail("failed to accept connection");
                        exit(1);
                }
                signal(SIGCHLD, SIG_IGN);//Beware! Zombies Ahead!
                pid_t pid=fork();
                if (pid==-1) {
                        bail("failed to create child process");
                        exit(1);
                }else if(pid==0){
                        close(server_fd);
                        connection_handler(session_fd);
                        close(session_fd);
                        _Exit(0);
                }else{
                        close(session_fd);
                }
        }




#if 0
	int retval;
	int len_inet;
	struct sockaddr_in addr_inet;
	struct sockaddr_in addr_clnt;
	int sock;
	char dgram[5120];



	/*creating socket address*/
	memset(&addr_inet,0,sizeof addr_inet);
	addr_inet.sin_family = AF_INET;
	addr_inet.sin_port = htons(NAEGLING_SERVER_CLUSTER_PORT);
	addr_inet.sin_addr.s_addr =INADDR_ANY;
	if ( addr_inet.sin_addr.s_addr == INADDR_NONE){
		bail("bad address.");
		return;
	}
	len_inet = sizeof addr_inet;

	/*creating UDP socket*/
	sock=socket(AF_INET,SOCK_DGRAM,0);
	if (sock==-1){
		bail("socket()");
		return;
	}
	/*bindig address to socket*/
	retval=bind(sock,(struct sockaddr *)&addr_inet,len_inet);
	if (retval==-1){
		bail("bind()");
		return;
	}


	/*Start new Cluster*/	
	int ret_status=get_cluster_ip_from_server();
	if(!ret_status)
		naegling_log("New cluster started");
	else
		bail("Error starting new cluster");

	
	/*wait for message*/ 
	for(;;){
		int function_retval=-1;
		len_inet=sizeof addr_clnt;
		retval=recvfrom(sock,dgram,sizeof dgram,0,(struct sockaddr*)&addr_clnt,&len_inet);
		if(retval<0)
			bail("recvfrom(2)");
		dgram[retval]='\0';
		naegling_log(dgram);
		char **message_fields;
		int count=get_field_count(dgram);
		message_fields=(char **)malloc(count*sizeof(char *));
		int i;
		for(i=0;i<count;i++)
			message_fields[i]=(char *)malloc(256*(sizeof(char)));
		get_message_fields(dgram,message_fields);
		if(message_fields[0]!=NULL){
			int option=-1;
			char* ptr;
			option=strtol(message_fields[0],&ptr,10);
			if(option<=0 && ptr==message_fields[0])
				bail("Invalid message format.");
			else{
				switch(option){
					case START_NEW_CLUSTER:
						function_retval=start_new_cluster(message_fields[1]);
						break;
					case ADD_WORKING_NODE:
						function_retval=add_working_node(message_fields[1],message_fields[2]);
						break;
					case REMOVE_WORKING_NODE:
						function_retval=remove_working_node(message_fields[1]);
						break;
					case GET_CLUSTER_STATUS:
						function_retval=get_cluster_status();
						break;
					case REQUEST_JOB_TRANSFER:
						if(FILE_TRANSFER_AVAILABLE){
                                                        FILE_TRANSFER_AVAILABLE=0;
                                                        function_retval=prepare_job_transfer(message_fields[1],message_fields[2]);
                                                        if(function_retval){
                                                                FILE_TRANSFER_AVAILABLE=1;
                                                        }
                                                }else{
                                                        function_retval=1;
                                                }
                                                break;
					case EXECUTE_JOB:
						function_retval=run_job_script(message_fields[1],message_fields[2]);
						break;
					case DOWNLOAD_JOB_FILE:
						if(FILE_TRANSFER_AVAILABLE){
							FILE_TRANSFER_AVAILABLE=0;
							function_retval=prepare_send_file_to_naegling_server(message_fields[1],message_fields[2]);
							if(function_retval){
                                                                FILE_TRANSFER_AVAILABLE=1;
                                                        }
						}else{
							function_retval=1;
						}
						break;
					default:
						bail("Invalid message format.");
				}
			}
			char *ret_message=(char *)malloc(MESSAGE_MAX_SIZE*(sizeof(char)));
			switch(function_retval){
				case 0:
					strcpy(ret_message,"0#");
					break;
				case 1:
					strcpy(ret_message,"1#");
					break;
				case -1:
					strcpy(ret_message,"-1#");
					break;
				default:
					strcpy(ret_message,"-1#");
			}
			function_retval=-1;
			retval=sendto(sock,ret_message,strlen(ret_message),0,(struct sockaddr*)&addr_clnt,len_inet);
			if(retval<0)
				bail("sendto(2)");
			free(ret_message);
		}else{
			bail("Invalid message format.");
		}
		for(i=0;i<count;i++)
                        free(message_fields[i]);
		free(message_fields);
	}
	close(sock);
#endif
}

void connection_handler(int session_fd){
        int i;
        int function_retval;
        int sock = session_fd;
        int read_size;
        int write_size;
        char *message , client_message[TCP_MESSAGE_MAX_SIZE];

        read_size=read(sock,client_message,TCP_MESSAGE_MAX_SIZE);
        if(read_size<0){
                bail("recv(2)");
                return;
        }
        client_message[read_size] = '\0';
        naegling_log(client_message);
        char **message_fields;
        int count=get_field_count(client_message);
        message_fields=(char **)malloc(count*sizeof(char *));
        for(i=0;i<count;i++)
                message_fields[i]=(char *)malloc(256*(sizeof(char)));
        get_message_fields(client_message,message_fields);
        if(message_fields[0]!=NULL){
                int option=-1;
                char* ptr;
                option=strtol(message_fields[0],&ptr,10);
                if(option<=0 && ptr==message_fields[0])
                        bail("Invalid message format.");
                else{
                        switch(option){
				case START_NEW_CLUSTER:
                                	function_retval=start_new_cluster(message_fields[1]);
                                        break;
				case ADD_WORKING_NODE:
                                	function_retval=add_working_node(message_fields[1],message_fields[2]);
                                        break;
				case REMOVE_WORKING_NODE:
                                	function_retval=remove_working_node(message_fields[1]);
                                        break;
				case GET_CLUSTER_STATUS:
                                	function_retval=get_cluster_status();
                                        break;
				case REQUEST_JOB_TRANSFER:
					function_retval=prepare_job_transfer(message_fields[1],message_fields[2],sock);
                                        break;
				case EXECUTE_JOB:
                                	function_retval=run_job_script(message_fields[1],message_fields[2]);
                                        break;
				case DOWNLOAD_JOB_FILE:
					shutdown(sock,SHUT_RD);
                                	function_retval=send_cluster_file(message_fields[1],sock);
                                        break;
                               default:
                                        bail("Invalid message format.");
                        }
                }

        }else{
                bail("Invalid message format.");
        }
        for(i=0;i<count;i++)
                free(message_fields[i]);
        free(message_fields);
        close(sock);
}


int get_field_count(char *message){
	int count=0;
	while(*message){
		if(strchr(MESSAGE_DELIMITER,*message))
			++count;
		++message;
	}
	return ++count;
}


void get_message_fields(char *message,char **message_fields){
	int i;
	char *str;
	str=strtok(message,MESSAGE_DELIMITER);
	i=0;
	while(str){
		strcpy(message_fields[i++],str);
		str=strtok(NULL,MESSAGE_DELIMITER);
	}
}



int get_cluster_status(){
        return CLUSTER_STATUS;
}



char* send_message_to_server(const char *message){
        int retval;
	int z;
        int len_inet;
        struct sockaddr_in addr_server;
        int addr_size;
        int sock;
        char server_message[TCP_MESSAGE_MAX_SIZE];



        /*creating socket address*/
        memset(&addr_server,0,sizeof addr_server);
        addr_server.sin_family = AF_INET;
        addr_server.sin_port = htons(NAEGLING_SERVER_PORT);
        addr_server.sin_addr.s_addr =inet_addr(SERVER_IP_ADDRESS);
        if ( addr_server.sin_addr.s_addr == INADDR_NONE){
                bail("bad address.");
                return NULL;
        }
        len_inet = sizeof addr_server;

        /*creating TCP socket*/
        sock=socket(PF_INET,SOCK_STREAM,0);
        if (sock==-1){
                bail("socket(2)");
                return NULL;
        }

	/*connect to server*/
	z=connect(sock,(struct sockaddr *)&addr_server,len_inet);
	if(z==-1){
		close(sock);
		bail("connect(2)");
		return NULL;
	}

        /*sending message*/
        retval=write(sock,message,strlen(message));
        if(retval<0){
		close(sock);
                bail("sending message to server.");
                return NULL;
        }

        /*wait for answer*/
        retval=read(sock,server_message,TCP_MESSAGE_MAX_SIZE);
        if(retval<0){
		close(sock);
                bail("Error receveing answer from server.");
                return NULL;
        }
        server_message[retval]='\0';
	char *ret_message=(char*)malloc(sizeof(char)*(retval+1));
	strcpy(ret_message,server_message);
	close(sock);
        return ret_message;

}


int get_cluster_ip_from_server(){
	int retval=-1;
        char *message=(char *)malloc(sizeof(char)*TCP_MESSAGE_MAX_SIZE);
	char *mac=(char *)malloc(sizeof(char)*MAX_MAC_SIZE);

	FILE *fp=fopen(ETH1_ADDRESS_PATH,"r");
	if(fp){
		if((fgets(mac,MAX_MAC_SIZE,fp))!=NULL){
	        	sprintf(message,"%d%s%s",REQUEST_CLUSTER_IP,MESSAGE_DELIMITER,mac);
		        char *ret_message=send_message_to_server(message);
			if(ret_message!=NULL){
				start_new_cluster(ret_message);
				free(ret_message);
				retval=0;
			}else{
				bail("Error acquiring cluster ip.");
			}
		}
		fclose(fp);
	}

	free(mac);
	free(message);
        
	return retval;
}





int prepare_job_transfer(const char *job_name, const char *file_name, const int sock){
        int retval=-1;
        char ret_message[256];

        /*
         * Paths string setup
         */
        char *job_directory_path=(char *)malloc(sizeof(char)*MAX_PATH_SIZE);


        strcpy(job_directory_path,NAEGLING_JOBS_PATH);
        strcat(job_directory_path,"/");
        strcat(job_directory_path,job_name);
        strcpy(file_path,job_directory_path);
        strcat(file_path,"/");
        strcat(file_path,file_name);
        /*
         * End of paths string setup
         */


        /*
         * Check if master node's jobs directory exists, if not create
         */
        struct stat status;
        retval=stat(job_directory_path,&status);
        if(retval==-1){
                naegling_log("Creating naegling job directory...");
                retval=mkdir(job_directory_path,S_IRWXU|S_IRWXG|S_IROTH);
		retval=chown(job_directory_path,NAEGLING_UID,NAEGLING_GID);
                if(retval==-1){
                        bail("Error creating naegling job directory.");
                }
        }else{
                naegling_log("Naegling job directory exists.");
        }
        /*
         * End of directory creation
         */

        if(!retval){
		char *log_message=(char*)malloc(sizeof(char)*256);	
		char *buff=(char *)malloc(sizeof(char)*TCP_MESSAGE_MAX_SIZE);//buffer
		int block_sz;

                remove(file_path);
		FILE *fp = fopen(file_path, "ab");		
                if(fp == NULL){
                        bail("File Cannot be opened file on cluster.");	/*
			 *Inform Server that it is ok to send file 
			 */
			strcpy(ret_message,"-1#");
			block_sz=write(sock,ret_message,strlen(ret_message));	
                }else{
			naegling_log("file opened");

			/*
			 *Inform Server that it is ok to send file 
			 */
			strcpy(ret_message,"0#");
			block_sz=write(sock,ret_message,strlen(ret_message));
			shutdown(sock,SHUT_WR);	

                        bzero(buff, TCP_MESSAGE_MAX_SIZE);			
                        while((block_sz = recv(sock, buff,TCP_MESSAGE_MAX_SIZE,0)) > 0){
                            int write_sz = fwrite(buff, sizeof(char), block_sz, fp);
                                if(write_sz < block_sz){
                                        bail("File write failed.");
                                }
                                bzero(buff, TCP_MESSAGE_MAX_SIZE);
				sprintf(log_message,"Bytes written: %d",block_sz);
				naegling_log(log_message);
                        }
                        if(block_sz < 0){
                                if (errno == EAGAIN){
                                        naegling_log("recv() timed out.");
                                }else{
                                        bail("recv() failed.");
                                        return;
                                }
                        }
                        naegling_log("Ok received from client!");
                        fflush(fp);
                        fclose(fp);
                }
		close(sock);
		return 0;


		
		
        }

        free(job_directory_path);
	return retval;
}


#if 0
void *job_file_transfer(void* args){

        /* Defining Variables */
        int sock;
        int nsock;
        int num;
        int sin_size;
        struct sockaddr_in addr_local; /* client addr */
        struct sockaddr_in addr_remote; /* server addr */
        char buff[TCP_MESSAGE_MAX_SIZE]; // Receiver buffer

        /* Get the Socket file descriptor */
        if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
                bail("[Image-Server] ERROR: Failed to obtain Socket Descriptor.");
                return;
        }else
                naegling_log("[Image-Server] Obtaining socket descriptor successfully.");

        /* Fill the client socket address struct */
        addr_local.sin_family = AF_INET; // Protocol Family
        addr_local.sin_port = htons(NAEGLING_CLUSTER_FILE_TRANSFER_PORT); // Port number
        addr_local.sin_addr.s_addr = INADDR_ANY; // AutoFill local address
        bzero(&(addr_local.sin_zero), 8); // Flush the rest of struct

        /* Bind a special Port */
        if( bind(sock, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 ){
                bail("[Image-Server] ERROR: Failed to bind Port.");
                return;
        }else
                naegling_log("[Image-Server] Binded tcp port sucessfully.");

        /* Listen remote connect/calling */
        if(listen(sock,SOMAXCONN) == -1){
                bail("[Image-Server] ERROR: Failed to listen Port.");
                return;
        }
        else
                naegling_log("[Image-Server] Listening the port successfully.");

        int success = 0;
        int block_sz=TCP_MESSAGE_MAX_SIZE;
        while(success == 0 && block_sz==TCP_MESSAGE_MAX_SIZE){
                sin_size = sizeof(struct sockaddr_in);

                /* Wait a connection, and obtain a new socket file despriptor for single connection */
                if ((nsock = accept(sock, (struct sockaddr *)&addr_remote, &sin_size)) == -1){
                    bail("[Image-Server] ERROR: Obtaining new Socket Despcritor.");
                        return;
                }else
                        naegling_log("[Image-Server] Server has got connected.");

                /*Receive File from Client */
                FILE *fp = fopen(file_path, "ab");
                if(fp == NULL)
                        naegling_log("[Image-Server] File Cannot be opened file on server.");
                else{
                        bzero(buff, TCP_MESSAGE_MAX_SIZE);
                        block_sz = 0;
                        while((block_sz = recv(nsock, buff,TCP_MESSAGE_MAX_SIZE,MSG_WAITALL)) > 0){
                            int write_sz = fwrite(buff, sizeof(char), block_sz, fp);
                                if(write_sz < block_sz){
                                        bail("[Image-Server] File write failed on server.");
                                }
                                bzero(buff, TCP_MESSAGE_MAX_SIZE);
                                if (block_sz == 0 || block_sz != 512){
                                        break;
                                }
                        }
                        if(block_sz < 0){
                                if (errno == EAGAIN){
                                        naegling_log("[Image-Server] recv() timed out.");
                                }else{
                                        bail("[Image-Server] recv() failed.");
                                        return;
                                }
                        }
                        naegling_log("[Image-Server] Ok received from client!");
                        fflush(fp);
                        fclose(fp);
                }

            close(nsock);
            naegling_log("[Image-Server] Connection with Client closed. Server will wait now...");
        }
        close(sock);
	//uid_t uid=NAEGLING_UID;
	//gid_t gid=NAEGLING_GID;
	int retval=chown(file_path,NAEGLING_UID,NAEGLING_GID);
	if(retval==-1){
		bail("job_file_transfer:Error changing file ownership");
	}
        FILE_TRANSFER_AVAILABLE=1;
}

#endif

int run_job_script(const char *job_name,const char *script_name){
	char *log_message=(char *)malloc(sizeof(char)*256);
		int retval;
		/*change working directory*/
		char *job_path=(char *)malloc(sizeof(char)*MAX_PATH_SIZE);
		sprintf(job_path,"%s/%s",NAEGLING_JOBS_PATH,job_name);		
	        if ((chdir(job_path)) < 0) {
        	        bail("Error changing working directory.");
               		fprintf(stderr,"Could not start naegling daemon.\n");
			free(job_path);
                	_Exit(1);
	        }


		/*allow script to be executed*/
		char *script_path=(char *)malloc(sizeof(char)*MAX_PATH_SIZE);
		sprintf(script_path,"%s/%s/%s",NAEGLING_JOBS_PATH,job_name,script_name);
		chmod(script_path,S_IRWXU|S_IRWXG);

		/*change process group id*/	
		retval=setgid(NAEGLING_GID);
		if(retval){
			
                        sprintf(log_message,"Error %d setgid seting job process GID",retval);
			bail(log_message);
			strerror_r(errno,log_message,256);
			bail(log_message);
                        _Exit(1);
                }


		/*change process ownership*/
		retval=setuid(NAEGLING_UID);
		if(retval){
			bail("Error seting job process UID");
			_Exit(1);
		}

		/*copy machinefile*/
		char *command=(char *)malloc(sizeof(char)*MAX_COMMAND_SIZE);
		sprintf(command,"cp %s %s/",MACHINEFILE_FILE_PATH,job_path);
		retval=system(command);
#if 0
 		if(retval==-1){
			bail("run_job_script: system(2), cp");
			strerror_r(errno,log_message,256);
			bail(log_message);
			exit(-1);
		}
#endif			

		/*execute script*/
		retval=system(script_path);
                if(retval==-1){
                        bail("run_job_script: system(2), executing script");
			strerror_r(errno,log_message,256);
			bail(log_message);
                        free(job_path);
                        free(script_path);
                        free(command);
                        exit(-1);
                }

		

		/*free dinamic allocated memory*/
		free(job_path);
                free(script_path);
		free(command);
		exit(0);
}


int send_cluster_file(const char *path,int server_sock){

	int z;
	struct stat st;
        stat(path, &st);
        size_t size = st.st_size;
        char *buff=(char*)malloc(sizeof(char)*TCP_MESSAGE_MAX_SIZE); // File buffer
        size_t block_sz;


	FILE *fp=fopen(path,"rb");
	naegling_log(path);
        if(fp && size>0){
		/*inform the file will be send*/
		sprintf(buff,"0#\n");
		z=write(server_sock,buff,size);
		bzero(buff,TCP_MESSAGE_MAX_SIZE);
		/*send file*/
                z=fread(buff,1,size,fp);
                z=write(server_sock,buff,size);
                naegling_log("job_file_transfer: writing on socket");
                if(z==-1){
                        bail("job_file_transfer: write(2)");
                        return;                                              
                }
        }else{
		/*inform the file will not be send*/
                sprintf(buff,"-1#\n");
                z=write(server_sock,buff,size);
		bail("Error Opening file");
	}
	free(buff);

}

#if 0
int prepare_send_file_to_naegling_server(const char *job_name,const char *file_name){
        int retval=-1;
        char *file_path=(char *)malloc(sizeof(char)*MAX_PATH_SIZE);
	sprintf(file_path,"%s/%s/%s",NAEGLING_JOBS_PATH,job_name,file_name);
	struct stat st;
        if(stat(file_path,&st)==0){
		retval=0;
       		pthread_t tid;
	        pthread_create(&tid,NULL,send_file_to_naegling_server,(void*)file_path);

        }else{
		char *error_message=(char *)malloc(sizeof(char)*256);
		sprintf(error_message,"Could not locate file: %s",file_path);
		bail(error_message);
		free(error_message);
	}

#if 0
        int retval=-1;
        


        /*
         * Paths string setup
         */
        char *job_directory_path=(char *)malloc(sizeof(char)*MAX_PATH_SIZE);


        strcpy(job_directory_path,NAEGLING_JOBS_PATH);
        strcat(job_directory_path,"/");
        strcat(job_directory_path,master_domain);
        strcpy(file_path,job_directory_path);
        strcat(file_path,"/");
        strcat(file_path,file_name);
        /*
         * End of paths string setup
         */


        /*
         * Check if master node's jobs directory exists, if not create
         */
        struct stat status;
        retval=stat(job_directory_path,&status);
        if(retval==-1){
                naegling_log("Creating naegling master node directory...");
                retval=mkdir(job_directory_path,S_IRWXU|S_IRWXG|S_IROTH);
                if(retval==-1){
                        bail("Error creating naegling master node directory.");
                }
        }else{
                naegling_log("Naegling jobs directory exists.");
        }
        /*
         * End of directory creation
         */

        if(!retval){
                remove(file_path);
                pthread_t tid;
                pthread_create(&tid,NULL,file_transfer,file_path);
        }

        free(job_directory_path);
#endif
        return retval;

}

void *send_file_to_naegling_server(void *arg){
        char *file_path=(char *)arg;
        struct sockaddr_in addr_server;
        int len_inet;
        int sock;
        int nsock;
        int num;
        int sin_size;
        struct sockaddr_in addr_remote; /* server addr */

	struct stat st;
	stat(file_path, &st);
	size_t size = st.st_size;
        char buff[size]; // File buffer
	size_t block_sz;
	FILE *fp=fopen(file_path,"rb");
	if(fp && size>0){
		fread(buff,1,size,fp);
                sock=socket(PF_INET,SOCK_STREAM,0);
                if(sock==-1){
                	bail("job_file_transfer: Error acquiring socket");
                        return;
                }
                memset(&addr_server,0,sizeof addr_server);
                addr_server.sin_family=AF_INET;
                addr_server.sin_port=htons(9293);
                addr_server.sin_addr.s_addr=inet_addr(SERVER_IP_ADDRESS);
                if(addr_server.sin_addr.s_addr==INADDR_NONE){
                	bail("job_file_transfer: Bad address.");
                        return;
                }
                len_inet=sizeof(addr_server);

		/*wait for server to listen*/
		system("sleep 2");
                
	        int z=connect(sock,(struct sockaddr *)&addr_server,len_inet);
                if(z==-1){
                	bail("job_file_transfer: Error on connect(2)");
                        return;
                }
                z=write(sock,buff,size);
		naegling_log("job_file_transfer: writing on socket");
                if(z==-1){
                	bail("job_file_transfer: write(2)");
                        return;                                              
                }
                close(sock);

                        //bzero(buff, TCP_MESSAGE_MAX_SIZE);
                        //if (block_sz == 0 || block_sz != 512){
                        //        break;
                        //}
	}
        naegling_log("job_file_transfer: Ok received from client!");
        FILE_TRANSFER_AVAILABLE=1;
}
#endif



