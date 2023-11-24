


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


int debug_msg( const char *format, ... )
{	
    //char tmpbuf[4096];
    unsigned int send_len;
    char * tmpbuf = malloc(8192);
    if( tmpbuf == NULL ){
        return -1;
    }
    va_list vArgs;
    va_start(vArgs, format);				 
    vsnprintf( (char *)&tmpbuf[0], 8192, (char const *)format, vArgs );			   
    va_end(vArgs);																										
    send_len = strlen(&tmpbuf[0]);
	//__android_log_write( ANDROID_LOG_DEBUG, LOG_TAG, tmpbuf );  
	printf("%s", tmpbuf);
    if( tmpbuf != NULL ){
        free(tmpbuf);       
    }
    return 0;
}


u8 g_debug_buff[256];
int g_debug_buff_index;
#define macdbg_prser Ser_Printf
                                                        
//#define Ser_WrStr do{ LOGI("%s", buffer); }while(0)
int g_printf_switch = 0x01;
int Ser_Printf (const char *format, ...)
{   
    unsigned char buffer[80 + 1];
    va_list  vArgs;
    if( g_printf_switch == 0x00 ){
        return 1;
    }
    va_start(vArgs, format);
    vsnprintf((char *)buffer, sizeof(buffer), (char const *)format, vArgs);
    va_end(vArgs);
    //Ser_WrStr;
    strcpy( &g_debug_buff[g_debug_buff_index], buffer );
    g_debug_buff_index = g_debug_buff_index + strlen(buffer);
    return 0;
}


int macdbg_dmphex(const char* buff, int len)
{
    int retval = 0; 
    int x, y, tot, lineoff;
    const char* curr;
    lineoff = 0;
    curr = buff;
    tot = 0;
               
    for( x = 0; len > x+16; ){                      
         macdbg_prser("0x%08x:  ", lineoff);           
         for( y = 0; y < 16; y++ ){
              macdbg_prser("%02x ", (unsigned char)*(curr + y));
         }
         macdbg_prser("  ");
         for( y = 0; y < 16; y++ ){
              char c;
              c = *(curr + y);
              if( c > 31 && c < 127 ){
                  macdbg_prser("%c", c);
              }else{
                  macdbg_prser("%c", '.');
              }
              tot++;
         }
         curr += 16;
         x += 16;
         lineoff+=16;
	    //syslog( LOG_INFO, "%s", g_debug_buff );

		debug_msg(  "%s\n", g_debug_buff );
         memset( &g_debug_buff[0x00], 0x00, sizeof(g_debug_buff) );
         g_debug_buff_index = 0x00;
    }
                  
    //do last line
    //Ser_Printf("tot %d.\r\n", tot );
    //Ser_Printf("len %d.\r\n", len );
    if( tot < len ){
        curr = (buff + tot);
        macdbg_prser("0x%08x:  ", lineoff);
        for( y = 0; y < (len - tot); y++ ){
             macdbg_prser("%02x ", (unsigned char)*(curr + y));
        }
        //padding with spaces
        //Ser_Printf("(len - tot) %d.\r\n", (len - tot) );
        if( (len - tot) < 16 ){
            for( y = 0; y < (32 - ((len - tot)*2)); y++ ){
                 macdbg_prser(" ");
            }
        }
        for( y = 0; y < 16-(len - tot); y++ ){
             macdbg_prser(" ");
        }
        macdbg_prser("  "); 
	   //Ser_Printf("(len - tot) %d.\r\n", (len - tot) );
        for( y = 0; y < (len - tot); y++ ){
            char c;
            c = *(curr + y);
            if( c >31 && c < 127 ){
                macdbg_prser("%c", c);
            }else{
                macdbg_prser("%c", '.');
            }
        }
    }
   // syslog( LOG_INFO, "%s", g_debug_buff );

   debug_msg( "%s\n", g_debug_buff );

   
    memset( &g_debug_buff[0x00], 0x00, sizeof(g_debug_buff) );
    g_debug_buff_index = 0x00;
    return retval;
}






