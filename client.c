/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "htfsk_daemon"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <pthread.h>
#include <limits.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <inttypes.h>
#include <sched.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/cdefs.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/ioctl.h> 
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/poll.h>

#include <asm/types.h>

#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/route.h>
#include <linux/input.h>

//#include <cutils/sockets.h>
//#include <cutils/properties.h>
//#include <cutils/android_reboot.h>
//#include <cutils/klog.h>

//#include <utils/Log.h>
//#include <android/log.h>
#include <syslog.h>
//#include <log/log.h>

#include <netdb.h>
//#include <net/if.h> 
#include <net/if_arp.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

#include "client.h"
#include "debug.h"
#include "file_op.h"
#include "util.h"
#include "version.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "string.h"
#include <malloc.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h> 
#include <sys/time.h>
#include <netdb.h>
//#include <net/if.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <errno.h> 
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <syslog.h>
#include <pthread.h>
#include <time.h>
//#include "typedef.h"
//#include "com_socket.h"



//#include "uart.h"




fd_set reader_fds;
pthread_mutex_t g_mutex_AnswerIPC;
pthread_t gMain_thread;
extern volatile unsigned char g_dog_using[10];
extern volatile unsigned char g_dog_registered[10];
extern volatile unsigned int g_bind_port[10];
static void sigusr1_opt(int arg);
extern void init_ttyS(int fd);



static void *socket_java_execthread(void *arg);

static void sigusr1_opt(int arg)
{
    //printf( "in sigusr1_opt\r\n" );
}

static void sigusr2_opt(int arg)
{
    syslog( LOG_INFO, "in sigusr2_opt\r\n" );
}




void g_init( void )
{
    //memset( (void *)g_dog_using, 0x00, sizeof(g_dog_using) );
    //memset( (void *)g_dog_registered, 0x00, sizeof(g_dog_registered) );
    //memset( (void *)g_bind_port, 0x00, sizeof(g_bind_port) );
}





void dump_time_version( void )  
{
    printf( "**************************************\n" );
	printf( "***   compile_date = %s   ***\n", __DATE__ );
    printf( "***   compile_time = %s      ***\n", __TIME__ );
	printf( "***   software version = %s     ***\n", "V2.02" );
	printf( "**************************************\n\n" );	
}




#define MAXINTERFACES   16 
#define BACKLOG 4
#define MAX_CONNECTED_NO 10

struct sockaddr_in server_sockaddr,client_sockaddr;
struct sockaddr_in tmp_sockaddr;
int client_fd = -1;         
int i=0x00;

//建立服务器端口
int tcpserver_starup(void)
{
	int sockfd;

	/*创建socket连接*/
	if((sockfd = socket(AF_INET,SOCK_STREAM,0))== -1){
		perror("socket");
		sockfd = -1;
		goto exit;
	}		
	//fcntl(sockfd,F_SETFL,O_NONBLOCK);
	/*enable address reuse */ 
	int on;
	int ret;
	on = 1; 
	ret = setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	
	/*设置sockaddr_in 结构体中相关参数*/
	server_sockaddr.sin_family=AF_INET;
	server_sockaddr.sin_port = htons(7086);
	server_sockaddr.sin_addr.s_addr=INADDR_ANY;
	bzero(&(server_sockaddr.sin_zero),8);


	/*绑定函数bind*/
	if( bind(sockfd,(struct sockaddr *)&server_sockaddr,sizeof(struct sockaddr))== -1 ){
		perror("bind");
		sockfd = -1;	
		goto exit;		
	}
                                                  
	/*调用listen函数*/
	if( listen(sockfd, BACKLOG) == -1 ){                                
		perror("listen");                                     	
		sockfd = -1;                                   	
		goto exit;                                      		
	}                                               
                                                                                                          	
exit:
	return sockfd;
}





int tcpconnect_poll(int sockfd,fd_set *pcur_fds )
{
	int sin_size;
	int newc_fd;
	int Threaderr;
	int ret;
	pthread_t socket_web_thread_id;
	struct sockaddr_in client_sockaddr;
	int * my_args_array;

	my_args_array = malloc(32);
	if( my_args_array == NULL ){
        syslog( LOG_INFO, "mem is overed.\n" );
		return;
	}
	//printf("newc_fd01 %d \r\n",123);
	
	sin_size = sizeof(struct sockaddr);
	newc_fd  = accept( sockfd, (struct sockaddr *)&client_sockaddr, &sin_size );
    //printf("newc_fd0 %d \r\n",newc_fd);
	//printf("sin_port = %d\n", client_sockaddr.sin_port );
	//printf("%x\n", client_sockaddr.sin_family );

	
	
	if( newc_fd < 0 ){				
        perror("accept");
		return -1;
	}else{
		fcntl( newc_fd, F_SETFL, O_NONBLOCK );
		//client_fd = newc_fd;
		//FD_SET(client_fd, pcur_fds);	
	}                                                                                                                     
	//printf("in shell testsss567.\n");

	my_args_array[0] = newc_fd;
	my_args_array[1] = client_sockaddr.sin_port;
	my_args_array[2] = (int)client_sockaddr.sin_addr.s_addr;
	
	Threaderr = pthread_create( &socket_web_thread_id, NULL, socket_java_execthread, &my_args_array[0] ); 
    ret = pthread_detach(socket_web_thread_id);
    if( Threaderr != 0 ){
		printf(  "pthread_create error.\n");
		//return -1;
	}  
}



#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>


 
int main()
{
    int client_socket;
    struct sockaddr_in server_addr;
    char* server_ip = "10.11.102.232";
    char* server_port = "7086";
    char buf[1024] = { 'p', 'w', 'd', '\n' };    
                   
    client_socket = socket(PF_INET,SOCK_STREAM,0);
 
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(atoi(server_port));
 
    if( connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1 ){ 
        printf("connect error\n");
		return 1;
	}
	
    int send_strlen,recvlen;
    //send_strlen = send(client_socket, buf, strlen(buf), 0);

	send_strlen = send(client_socket, "./fuckdir.sh vendor/ \n", 128, 0);
	printf( "send cmd = %s.\n", buf );
	//macdbg_dmphex( (const char *)buf, strlen(buf) );

	memset( buf, 0x00, sizeof(buf) );
    recvlen = 0;
    while( 1 ){  
      recvlen += recv( client_socket, &buf[recvlen], sizeof(buf)-recvlen, 0 );
	  //printf( "recvlenxxx234 = %d\n", recvlen );
	  if( recvlen == 8 && strcmp(buf, "ACKOKAY\n")==0x00 ){
          break;
	  }
    }     
    close(client_socket);

	printf( "rev ack = %s.\n", buf );
    return 0;
}






int mainxxxxx(void)
{	
    int fd,server_sockfd;
    fd_set temp_fds;
    int ret,i,j;
    u8 dev_name[64];
            
	dump_time_version();
     
    signal(SIGUSR1,sigusr1_opt);
    signal(SIGPIPE,SIG_IGN);		
    //忽略SIGPIPE
    signal(SIGUSR2,sigusr2_opt);
    FD_ZERO(&reader_fds);
                                                              
    syslog( LOG_INFO, "start com_socket\n" );
    server_sockfd = tcpserver_starup();
	printf( "server_sockfd = %d.\n", server_sockfd );
	
    if( server_sockfd > 0 ){
        //syslog( LOG_INFO, "server sockfd = %d.\r\n", server_sockfd );
        FD_SET(server_sockfd, &reader_fds); 
    }						  
    //init the mutex
    ret = pthread_mutex_init( &g_mutex_AnswerIPC, NULL );
    if( ret != 0 ){
        perror("pthread_mutex_init");
        syslog( LOG_INFO, "pthread_mutex_init" ); 
        exit(1);
    }		
       
    gMain_thread = pthread_self();
    while( 1 ){
	  temp_fds = reader_fds;
	  ret = select( server_sockfd+1, &temp_fds,(fd_set *)0, (fd_set *)0, NULL );
	  printf( "select ret = %d.\n", ret );
	  if( ret < 1 ){
		  perror("select567");
		  //exit(1);
		  continue;
	  }
	  for( fd = 0; fd < server_sockfd+1; fd++ ){
		   if( FD_ISSET(fd, &temp_fds) ){		
			   if( fd == server_sockfd ){					
				   tcpconnect_poll(fd, &reader_fds);					
			   }
		   }
	  }
	}
	close(server_sockfd);
	exit(1);		
}








#define BUFLEN 20480
#define RESULT_MAX_BUFF_SIZE  4096

#define t_assert(x) { \
	if(!(x))  {err = -__LINE__;goto error;} \
}

static volatile int keepRunning = 1;
static int test_running = 0x00;
static int g_status[4] = {-1,-1,-1,-1};




int exec_cmd_and_get_result( const char *cmd_str, char *buffer ) 
{     
    int cnt;
    FILE *pf;
    pf = popen( cmd_str, "r" );    
    cnt = fread( buffer, 1, RESULT_MAX_BUFF_SIZE-6, pf );   
    buffer[cnt-1] = '\0';       

	//debug_msg( "fread cnt = %d\n", cnt );
	//debug_msg( "strlen(buffer) = %d\n", strlen(buffer) );
	//debug_msg( "buffer = %s\n", buffer );
	//macdbg_dmphex(buffer, cnt);
    
    pclose(pf);
    return 0;
}




int do_create_thread(func_pointer thread_func, void *arg)
{   
    int Threaderr, ret;
    pthread_attr_t attr;
    pthread_t tmp_thread_id;
    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    Threaderr = pthread_create( &tmp_thread_id, NULL, thread_func, arg );
    ret = pthread_detach( tmp_thread_id );
    if( Threaderr != 0 ){
        debug_msg("pthread_create error.\n");
    }
    return tmp_thread_id;
    //当线程为joinable时,使用pthread_join来获取线程返回值,并释放资源
    //当线程为非joinable时,也可在线程中调用 pthread_detach(pthread_self())来分离自己
}



//恢复出厂设置,删除apk
static int restore_factory_setting_rm_apk()
{    
    return 0;
}              


//安装最新的apk
int restore_factory_setting()
{                                        
    system("pm install "RELEASE_APK_BACKUP_PATH);
    return 0;               
}        




//安装出厂时自带的apk
int restore_factory_old_setting()
{    
    return 0x00;                       
}        


//钩子函数
static int plug_func( char *cmd_buff, int current_fd )
{                 
    int ret;
    int arg=0x00;
    char log[200];
    func_pointer test_func = NULL;
    int status;
    char * ret_p;

    char temp_buff[256];
    int len;
    int index;
    int i;


	ret = 0x00;
	len = strlen(cmd_buff);
	if( len > 256 ){
         goto quit_error;
    }

    memset( &temp_buff[0], 0x00, sizeof(temp_buff) );
    memcpy( &temp_buff[0], &cmd_buff[0], len );
    for( index=0x00; index<len; index++ ){
         if( temp_buff[index] == ' ' ){
             continue;
         }else{
             break;
         }  
    }
    memset( &cmd_buff[0], 0x00, sizeof(temp_buff) );
    memcpy( &cmd_buff[0], &temp_buff[index], len-index );
	
    if( strncmp( cmd_buff, "plug_func_", 10 ) == 0x00 ){
        if( strncmp( cmd_buff, "plug_func_0", 11 ) == 0x00 ){
            //0号功能,测试用途
            debug_msg( "plug_func cmd.0 = %s", cmd_buff ); 
            if( test_running == 0x00 ){
                test_running =  0x01;			
                //test_func = main_key;
                //do_create_thread( test_func, (void *)&arg );
            }
            sprintf( log, "run result = 0x%x.\n", 0x55 );
            ret = send( current_fd, log, strlen(log), 0 );
            close(current_fd);
        }else if( strncmp( cmd_buff, "plug_func_1", 11 ) == 0x00 ){
            //1号功能,进入boot recovery模式
            debug_msg( "plug_func cmd.1 = %s", cmd_buff ); 
            sprintf( log, "run result ok.\n" );
            ret = send( current_fd, log, strlen(log), 0 );
            close(current_fd);
                   			
            //op_misc_file();
            //android_reboot(ANDROID_RB_RESTART2, 0, "recovery");
            while(1){ 
              pause(); 
            }  
            //never reached
        }else if( strncmp( cmd_buff, "plug_func_2", 11 ) == 0x00 ){
            //2号功能,打开adb进程                                      
            debug_msg( "plug_func cmd.2 = %s", cmd_buff );                           
            //property_set("sys.adbd_debug.on", "1");
            //setprop sys.adbd_debug.on 1
            sprintf( log, "run result ok.\n" );
            ret = send( current_fd, log, strlen(log), 0 );
            close(current_fd);
        }else if( strncmp( cmd_buff, "plug_func_3", 11 ) == 0x00 ){
            //3号功能,恢复出厂设置                                     
            debug_msg( "plug_func cmd.3 = %s", cmd_buff );                           
            sprintf( log, "run result ok.\n" );
            ret = send( current_fd, log, strlen(log), 0 );
            close(current_fd);
            restore_factory_setting();
        }else if( strncmp( cmd_buff, "plug_func_4", 11 ) == 0x00 ){
            //4号功能,获取上次命令的执行结果        
            #ifdef HTFSK_DBG
            debug_msg( "plug_func cmd.4 = %s", cmd_buff );
            #endif
            sprintf( log, "run result = %x %x %x %x.\n", g_status[0], g_status[1], g_status[2], g_status[3] );
            
            ret = send( current_fd, log, strlen(log), 0 );
            close(current_fd);
        }else if( strncmp( cmd_buff, "plug_func_5", 11 ) == 0x00 ){
            //5号功能,系统交互命令                               
            debug_msg( "plug_func cmd.5 = %s", cmd_buff );                           
            //sprintf( log, "run result = %x %x %x %x.\n", g_status[0], g_status[1], g_status[2], g_status[3] );
            //prepare_version_file();
            //ret = parse_version_from_file(VERSION_FILE_PATH, log);
            if( ret == 0x00 ){
                ret = send( current_fd, log, strlen(log), 0 );
                close( current_fd );
            }else{
                debug_msg( "parse error" ); 
                sprintf( log, "run result error.\n" ); 
                ret = send( current_fd, log, strlen(log), 0 );
                close( current_fd );
            }
        }else if( strncmp( cmd_buff, "plug_func_6", 11 ) == 0x00 ){
            //6号功能,恢复出厂设置 老的                                    
            debug_msg( "plug_func cmd.6 = %s", cmd_buff );                           
            sprintf( log, "run result ok.\n" );
            ret = send( current_fd, log, strlen(log), 0 );
            close(current_fd);
            restore_factory_old_setting();
        }else if( strncmp( cmd_buff, "plug_func_7", 11 ) == 0x00 ){
            //7号功能,清除安装标志                                  
            system( "setprop sys.install_apk.status 0");
            debug_msg( "plug_func cmd.7 = %s", cmd_buff );                           
            sprintf( log, "run result ok.\n" );
            ret = send( current_fd, log, strlen(log), 0 );
            close(current_fd);	
        }else if( strncmp( cmd_buff, "plug_func_8", 11 ) == 0x00 ){
            //8号功能,apk机器版本号保存                                
            debug_msg( "plug_func cmd.8 = %s", cmd_buff );   
            //do_apk_version_save();
            //sprintf( log, "run result ok.\n" );
            //ret = send( current_fd, log, strlen(log), 0 );
            close(current_fd);
        }else{
            goto quit_error;
        }
        return 0;
    }else{                         
        return 1;                             
    }   
    quit_error:
    debug_msg( "parse error" ); 
    sprintf( log, "run result error.\n" ); 
    ret = send( current_fd, log, strlen(log), 0 );
    close( current_fd );
    return 0;
}



static void init_g_status( void )
{
    int i;
    for( i=0x00; i<4; i++ ){
         g_status[i] = -1;
    }
}


int is_file_exist(const char *file_path)
{
    if( file_path == NULL ){
        return -1; 
	}
	if( access(file_path, F_OK) == 0 ){
        return 0; 
	}else{
        return -2; 
	}
}


static int is_dir_exist(const char *dir_path)
{                 
    DIR *dp;        
    dp = NULL;
    if( dir_path == NULL ){
        return -1; 
    }   
    if( (dp = opendir(dir_path)) == NULL ){
        return -2; 
    }else{
        closedir(dp);
        return 0; 
    }
}



//exec 9<>/dev/tcp/10.11.102.232/7086 && echo -e "88888" >&9 && exec 9>&-


//exec 9<>/dev/tcp/10.11.102.232/7086 && echo -e "9999999999999" >&9

static void *socket_java_execthread( void *arg )   
{     
    char log[4096]; 
    int status;
    int connect_number = 6;
    int fdListen = -1, new_fd = -1;
    int ret;
    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof (peeraddr);
    int numbytes ;
    char buff[4096];
    char * ret_p;
                                           	
    new_fd = *(int *)arg;                                                                                                                                               
    while( 1 ){
      //printf( "new_fd-0x = %d.", new_fd );                                           	  
      memset( buff, 0, sizeof(buff) );
      if( (numbytes = recv(new_fd,buff,sizeof(buff),0)) <= 0 ){
          printf( "error...%d", numbytes);
          //perror("recv");
          close(new_fd);
          break;
      }
	  
	  printf("numbytes = %d\n", numbytes);
	  //printf("new_fd = %d\n", new_fd);
	  //printf("buff = %s\n", buff);
	   macdbg_dmphex(buff,10);
	   
	   
	   
	 
      if( strncmp( buff, "closesocket", 11 ) == 0x00 ){
          close(new_fd);
          break;
      }

      #ifdef HTFSK_DBG
      //debug_msg( "strlen(buff) = %d.", strlen(buff) );
      debug_msg( "cmd = %s", buff );
      #endif
      init_g_status();
              
      
      status = system( buff );
      g_status[0] = status;	  
      #ifdef HTFSK_DBG
	  if( status ){
          debug_msg( "run system() return status = 0x%x.", status );
	  }
      #endif
	  
	  close(new_fd);
          break;
	   

	  
     
               
	  //ret = send( new_fd, log, strlen(log), 0 );
    }
    //debug_msg( "exit socket thread.\n");
    return 0;
}

//kill -9 $(busybox pidof htfsk)
//cmd = pm install -r /storage/sdcard0/xdn/xxxx.apk


U16 get_proc_pid( char *proc_name )
{      
    int pid;
    char buffer [RESULT_MAX_BUFF_SIZE];	  
    char cmd_str[RESULT_MAX_BUFF_SIZE];                                                 
	memset( cmd_str, 0x00, sizeof(cmd_str) );
	strcpy( cmd_str, "pidof " ); 
	strcat( cmd_str, proc_name );            
	//debug_msg( "cmd_str = %s\n", cmd_str );
    exec_cmd_and_get_result(cmd_str, buffer);
	pid = atoi(buffer);
    return pid;
}

static int monitor_pid = 0x00;
				  
void get_current_time_str( char* time_buff, int buff_len )   
{
    time_t current_time;
    struct tm *p;   
    time( &current_time );    
    p = gmtime( &current_time );  
    strftime( time_buff, buff_len, "%Y-%m-%d_%H-%M-%S", p );  
    debug_msg( "current_time: %s\n", time_buff );  
}

static void do_get_system_log( void )
{
    char time_buff[100];
	char cmd_str1[256];  
	char cmd_str2[256];  
	
	memset( time_buff, 0x00, sizeof(time_buff) );
	get_current_time_str( time_buff, sizeof(time_buff) );
	
	system("mkdir -p /data/system_log/");                                          
	memset( cmd_str1, 0x00, sizeof(cmd_str1) );
	strcpy( cmd_str1, "dmesg > /data/system_log/linux_" ); 
	strcat( cmd_str1, time_buff );   
	strcat( cmd_str1, ".txt" ); 
    system( cmd_str1 );

	memset( cmd_str2, 0x00, sizeof(cmd_str2) );
	strcpy( cmd_str2, "logcat -d > /data/system_log/android_" ); 
	strcat( cmd_str2, time_buff );   
	strcat( cmd_str2, ".txt" ); 
    system( cmd_str2 ); 
}            
            //__unused                             
void *monitor_system_log( void *arg  )                                                              
{                                                       
    int debug_cnt,system_server_pid;
    debug_cnt = 0x00; 
    while( 1 ){    
      sleep(1);
      //debug_msg("in main %d", debug_cnt );
	  //pidof system_server 
	  system_server_pid = get_proc_pid("system_server");
	  //debug_msg("system_server_pid %d", system_server_pid );
	  if( system_server_pid > 0 ){
          if( monitor_pid == 0x00 ){
              monitor_pid = system_server_pid;
		  }else{
              if( monitor_pid != system_server_pid ){    
                  debug_msg("system_server oops start!!!" );  
				  do_get_system_log();
				  monitor_pid = 0x00;
				  debug_msg("system_server oops end!!!" );  
			  } 
		  }     
	  }
      debug_cnt++;
    }	
}           


static void tv_log_monitor( void )
{             
    int ret;
    int arg = 0;
    int first_run;                                           	
    func_pointer test_func = NULL;
    test_func = monitor_system_log;
    do_create_thread( test_func, (void *)&arg );        
    first_run = 0x00;          
}        

                                            
void *monitor_video_node( void *arg  )                                                              
{                                                       
	int kill_flag = 0;
    while( 1 )
	{    
      if( is_file_exist("/dev/video0")!=0x00 && is_file_exist("/dev/video1")==0x00 && kill_flag == 0){
		system( "killall mediaserver" ); 
		kill_flag = 1;
        debug_msg( "mediaserver restart!!!" );  
		sleep(4);
		system( "killall com.ctg.itrdc.clouddesk" ); 
	  }
      sleep(4);
	  if( is_file_exist("/dev/video0") == 0 &&  kill_flag == 1)
	  {
		  kill_flag = 0;
          debug_msg( "/dev/video0 recover" );  
	  }
    }	
}           




static void camera_video_driver_node_monitor( void )
{             
    int arg = 0;
    func_pointer test_func = NULL;
    test_func = monitor_video_node;
    do_create_thread( test_func, (void *)&arg );        
}    


int clear_old_files( void )
{      
    int i,index,len,current_start,cnt;
    char buffer [RESULT_MAX_BUFF_SIZE];	  
    char cmd_str[RESULT_MAX_BUFF_SIZE];    
	char name[128];   
	//strcpy( cmd_str, "busybox ls -t /data/system_log/ | sed -n \'6,$p\' |xargs rm -rf" ); 

	strcpy( cmd_str, "busybox ls -t /data/system_log/ | sed -n '4,$p'" ); 
	debug_msg( "cmd_str = %s\n", cmd_str );
    exec_cmd_and_get_result(cmd_str, buffer);
	macdbg_dmphex(buffer, 100);
             
    debug_msg( "buffer len = %d\n", strlen(buffer) );
	len = strlen(buffer);
    index = 0x00;
    while(1){ 
      current_start = index;    
      cnt   = 0x00;       
      memset( name, 0x00, sizeof(name) ); 
      while(1){
        if( buffer[index] != '\n' && buffer[index] != '\0' ){
            index++;
			cnt++;
	    }else{
            break;
		}
	  }
      if( cnt ){
          memcpy( name, &buffer[current_start], cnt );      
		  debug_msg( "name = %s\n", name );
		  index++;
		  memset( cmd_str, 0x00, sizeof(cmd_str) );
	      strcpy( cmd_str, "rm -f /data/system_log/" ); 
	      strcat( cmd_str, name );   
          system( cmd_str );
      }else{
          break; 
      }
    }    
    return 0;
}

                                    
void *monitor_log_file_del( void *arg  )                                                              
{                                                       
    int debug_cnt; 
    while( 1 ){    
      sleep(3600*12);
	  clear_old_files();
    }	
}           


static void log_files_delete( void )
{             
    int ret;
    int arg = 0;
    int first_run;                                           	
    func_pointer test_func = NULL;
    test_func = monitor_log_file_del;
    do_create_thread( test_func, (void *)&arg );        
    first_run = 0x00;          
}        
                                
int mainxxxx( int argc , char **argv  ) 
{
    int ret;
    char log[200]; 
    int connect_number = 6;
    int fdListen = -1, new_fd = -1;
    struct sockaddr_un peeraddr;
    socklen_t socklen = sizeof(peeraddr);
          
	debug_msg( "start htfsk, DAEMON_VERSION = %s.", DAEMON_VERSION ); 

	//clear_old_files();
    //log_files_delete();                                            
    //tv_log_monitor();
    //camera_video_driver_node_monitor();


  
    //获取init.rc中配置的名为"htfsk"的socket
    //fdListen = android_get_control_socket( SOCKET_NAME );
    if( fdListen < 0 ){
        sprintf(log,"Failed to get socket '" SOCKET_NAME "' errno:%d", errno);
        debug_msg( "listen %s.", log ); 
        exit(-1);
    }
    ret = listen(fdListen, connect_number);    
    if( ret < 0 ) {
        debug_msg("listen");
        exit(-1);
    }
    while( 1 ){
      new_fd = accept(fdListen, (struct sockaddr *) &peeraddr, &socklen);
      #ifdef HTFSK_DBG
      debug_msg( "new thread-->accept_fd %d.", new_fd );
      #endif
      if( new_fd < 0 ){
          debug_msg( "accept error,errno = %d", errno );
          continue;
      }
      do_create_thread( socket_java_execthread, (void *)&new_fd );
    }   
    debug_msg("exiting");
    return 0;
}






