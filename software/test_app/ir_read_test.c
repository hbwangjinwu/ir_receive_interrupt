#include <stdio.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <unistd.h>  
  
  
int main(int argc ,char *argv[])  
{  
    int fd;  
    unsigned char ir_val;  
      
    fd = open("/dev/irm",O_RDWR);  
    if (fd < 0)  
    {  
        printf("open file error\n");  
    }  
    while(1)  
    {  
        read(fd,&ir_val,1);  
        printf("ir_val = 0x%x\n",ir_val);  
          
    }  
    return 0;  
}  

