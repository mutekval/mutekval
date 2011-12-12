#include<sys/types.h>
#include<sys/socket.h>
#include<sys/ioctl.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdio.h>

#include″tftpdfunc.h″

#define PACKSIZE BLOCKSIZE+4

struct bf{
  int counter;   /*数据缓冲区的长度或是标志*/
  char buf[PACKSIZE];   /*数据缓冲区*/
}buffers[2];

/*bf.counter的标志*/
#define BF_ALLOC-3    /*bf.buf已被分配，但尚未填入数据*/
#define BF_FREE  -2    /*bf.buf尚未被分配*/

static int next; /*下一个可用缓冲区*/
static int current;   /*当前正在使用的缓冲区*/

/*用于转换文件传输格式的两个变量*/
int newline=0;
int prevchar=-1;   /*前一个字符*/

void read_block(FILE*file,int convert);
int write_block(FILE*file,int convert);
struct tftphdr*rwrite_init(int);

struct tftphdr *write_init(void){return rwrite_init(0);}
struct tftphdr *read_init(void){return rwrite_init(1);}

struct tftpdr*
rwrite_init(int x)
{
  newline=0;
  prevchar=-1;
  buffers[0].counter=BF_ALLOC;   /*分配第一块缓冲区*/
  current=0;
  buffers[1].counter=BF_FREE;
  next=x;
  return (struct tftphdr*)buffers[0].buf;
}

int my_read(FILE*file,struct tftphdr **dpp,
	    int convert/*convert值为1，需转换成netascii码*/)
{
  struct bf *b;
  
  buffers[current].counter=BF_FREE;
  current=!current;
  
  b=&buffers[current];
  if(b->counter==BF_FREE)   /*缓冲区空*/
    read_block(file.convert);
  
  dpp=(struct tftphdr)b->buf;
  return b->counter;
}

void read_block(FILE*file,int convert   /*值为1， 需转换成ascii码*/)
{
  int i;
  char *p;
  int c;
  struct bf *b;
  struct tftphdr *dp;
  
  b=&buffers[next];
  if(b->counter !=BF_FREE)
    return;
	next=!next;
	
	dp=(struct tftpdr*)b->buf;
	
	if(convert ==0){    /*直接以二进制码读出，不需转换，直接读出*/
	  b->counter=read(fileno(file),dp->th_data,BLOCKSIZE);
	  return;
	}
	/*需转换成ascii码，逐个读出字符并处理行尾的换行符*/
	p=dp->th_data;
	for(i=0;i<BLOCKSIZE;i++){
	  if (newline){
	    if(prevchar==′\n′)
	      c=′\n′;
	    else  c=′\0′;
	    newline=0;
	  }
	  else{
	    c=getc(file);
	    if(c==EOF)break;
	    if(c==′\n′||c==′\r′){
	      prevchar=c;
	      c=′\r′;
	      newline=1;
	    }
	  }
	  *p++=c;
	}
	b->counter=(int)(p-dp->th_data);
}


int my_write(FILE*file,struct tftphdr** dpp,int ct, int convert)
{
  buffers[current].counter=ct;    /*数据块的长度*/
  current=!current;
  if(buffers[current].counter!=BF_FREE)/*有数据*/
    write_block(file,convert);   /*写入数据，刷新缓冲区*/
  buffers[current].counter=BF_ALLOC; /*标志该缓冲区已分配*/
  *dpp=(struct tftphdr)buffers[current].buf;
  return ct;
}

int write_block(FILE*file,int convert)
{
  char*buf;
  int count;
  int ct;
  char*p;
  int c;  /*当前读出的字符*/
  struct bf*b;
  struct tftphdr*dp;

  b=&buffers[next];
  if(b->counte <-1)      /*数据块长度*/
  b->counter=BF_FREE;
  dp=(struct tftphdr*)b->buf;
  next=!next;
  140	buf=dp->th_data;
  
  if(count<0)
    return-1;
  if(convert==0)/*不需转换*/
    return= write(fileno(file);buf, count);
  
  p=buf;
  ct=count;
  while(ct--)
  c=*p++;
  if(prevchar==′\r′)
    if(c==′\n′)   /*回车后是换行*/
      fseek(file ,-1,1);
    else
      if(c==′\0′)   /*回车后是空字符*/
	goto   skipit;   /*空字符不写入文件*/
  putc(c,file);
  skipit;
  prevchar=c;

  return count;
}

int synch(int f/*套接字*/)
{
  int i,j=0;
  char rbuf[PACKSIZE];
  struct sockaddr_in from;
  size_t fromlen;
  
while(1){
  (void) ioctl(f,FIONREAD,&i);
  if(i){
    j++;
    fromlen=sizeof from;
    (void)recvfrom(f,rbuf,sizeof(rbuf),0,
		   (struct sockaddr*)&from,&fromlen);
  }else{
    return(j);
  }
}
