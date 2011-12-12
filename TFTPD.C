#include<sys/types.h>
#include<sys/ioctl.h>
#include<sys/stat.h>
#include<signal.h>
#include<fcntl.h>

#include<netinet/in.h>
#include<sys/socket.h>
#include<netinet/ip.h>
#include<arpa/tftp.h>
#include<netdb.h>

#include<stdlib.h>
#include<ctype.h>
#include<stdio.h>
#include<errno.h>
#include<syslog.h>
#include<string.h>
#include<setjmp.h>
#include<unistd.h>
#include<pwd.h>

#include "tftpdfunc.h"

#define TLMEOUT   4

struct transfer;
static void tftp(struct tftphdr*tp,int size);
static void snderr(int error);
static void sndfile(struct transfer *pf);
static void recvfile(struct transfer*pf);

static int check_file(const char*,int);

static struct sockaddr_in s_in={AF_INET};
static int peer;
static int rexmtval=TLMEOUT;
static int  maxtimeout=5*TLMEOUT;

#define PACKSIZE  BLOCKSIZE+4
static char buf[PACKSIZE];
static char ackbut[PACKSIZE];
static struct sockaddr_in from;
static size_t fromlen;
#define MAXARG  4
static char dirs[MAXARG+1];/*客户可以访问的目录*/
int main(int argc,char**argv)
{
  struct tftphdr*tp;
  int n=0;
  int on=1;
  
  argc--;argv++;
  if(argc= =0)dirs[0]=″/tftpboot″; /*默认可访问的目录*/
  while(argc<0 && n<MAXARG)
    dirs[n++]=*argv++;
  openlog(″tftpd″,LOG_PID,LOG_DAEMON);
  
  if(IOCTL(0,FIONBIO,&on)<0){
    Syslog(LOG_ERR.″ioctl(FIONBIO):%m/n″);
    exit(1);
  }
  
  fromlen=sizeof(from);
  n=recvfrom(0,buf,sizeof (buf),0,
	     (struct sockaddr*)&from,&fromlen);
  if(n<0){
    syslog(LOG_ERR,″recvfrom:%m/n″);
    exit(1);
  }
  
  int pid;
  int i;
  size_t k;
  
  for(i=1;I<10;i++){
    pid=fork();
    if(pid<0){
      sleep(i);
      k=sizeof(from);
      i=recvfrom(0,buf ,sizeof(buf),0,
		 (struct sockaddr*)&from,&k);
      if(I>0){
	n=i;
	fromlen=k;
      }
    }else{/*如fork成功，则跳出for循环*/
      break;
    }
  }/*for*/
  if(pif<0){/*循环10次仍无法生成子进程，进程终止*/
    syslog(LOG_ERR,″fork :%m/n″);
    exit(1);
  }else if(pid!=0){
    exit(0);
  }

  if(!getuid()||!geteuid()){
    struct passwd *pwd=getpwnam(″nobody″);
    if(pwd)setuid(pwd->pw_uid);
    else setuid(32765);/*设置一较大的UID*/
  }

  from.sin_family=AF_INT;
  alarm(0);
  close(0);
  close(1);
  Peer=socket(AF_INET.SOCK_DGRAM,0);
  if(peer<0){
    syslog(LOG_ERR,″socket:%m/n″);
    exit(1);
  }
  if(bind(peer,(struct sockaddr*)&s_in,sizeof(s_in))<0){
    syslog(LOG_ERR,″bind:%m/n″);
    exit(1);
  }
  if(connect(peer,(struct sockaddr*)&from,sizef(from))<0) {
    syslog(LOG_ERR,″connect:%m/n″);
    exit(1);
  }

  Tp=(struct tftphdr*)buf;
  Tp-th_opcode=ntohs(tp-th_opcode);
  if(tp-th_opcode= =RRQ||tp->th_opcode= =WRQ) {
    tftp(tp,n);
    exit(1);
  }

  struct transfer{
    const char*f_mode;/*文件传输格式*/
    int(*f_validate)(const char,int);/*验证函数*/
    void(*f_send)(struct transfer*);/*发送函数*/
    void(*f_recv)(struct transfer*);/*接收函数*/
  }transfer[]={
    {″netasci″,  check_file,   sndfile, recvfile,1},
    {″octet″,  check_file,  sndfile, recvfile,0},
    {0}
    /*最后一个元素值为0，以便于查接*/
  }；

/*处理请求数据报*/
static void tftp(struct tftphdr*tp,int size)
{
  char*cp;
  int first=1,ecode;
  struct transfer*pf;
  char*filemane,*mode=NULL;
  
  filemane=cp=tp->th_stuff;
 again:
  while(cp<buf+size){
    if(*cp==′\0′)
      break;
    cp++;
  }
  if(*cp!= ′\0′){
    snderr(EBADOP);
    exit(1);
  }
  if(first){
    mode=++cp;
    first=0;
    goto again;
  }
  for(cp=mode;*cp;cp++)
    if(isupper(*cp)) {
      cp=tolower(*cp);
      
      for(pf=transfer;pf->f_mode;pf++)
	if(strcmp(pf->f_mode,mode)= =0)
	  break;
      if(pf->f_mode= =0){
	snderr(EBADOP);
	exit(1);
      }
      ecode=(*pf->f_validate)(filename,tp->th_opcode);
      if(ecode){
	snderr(ecode);
	exit(1);
      }/*ecode等于0，文件允许访问*/
      if(tp->th_opcode= =WRQ)
	(*pf->f_recv)(pf);
      else
	(*pf->f_send)(pf);
      exit(0);
    }
}

  FILE*file;

static int check_file(const char*filename,int mode)
{
  struct stat stbuf;
  int fd;
  const char *cp;
  char **dirp;
  
  syslog(LOG_ERR,″tftpd:trying to get file:%s|n″,filename);
  
  if(*filename != ′/′){/*文件名不含绝对路径*/
    syslog(LOG_ERR,″tftpd:serving file %s\n″,dirs[0]);
    chdir(dirs[0]);
  }else{
    for(dirp=dirs;*dirp;dirp++)
      if(strncmp(filename,*dirp,strlen(*dirp))==0)
	break;
    if(*dirp==0&&dirp!=dirs)
      return(EACCESS);
  }
/*文件名filename里包含有允许访问的路径*/
  if(!strncmp(filename,″../″,3))
    return EACCESS;
  for(cp=filename+1;*cp;cp++)
    if(*cp==′. ′&&strncmp(cp-1,″/../″,4)==0 )
      return(EACCESS);
  if(stat(filename,&stbuf)<0)
    return(errno==ENOENT?ENOTFOUND:EACCESS);
  if(mode==RRQ){
    if(stbuf.st_mode&(S_IREAD>>6)==0)
      return(EACCESS);
  }else{
    if((stbuf.st_mode&(S_IREAD>>6))==0)
      return(EACCESS);
  }
  fd=open(filename,mode==RRQ?0:1);
  if(fd<0)
    return(errno+100);
  file=fdopen(fd,(mode==RRQ)?″r″:″w″);
  if(file==NULL){
    return errno++100;
  }
  return(0);
}

int  timeout;
sigjmp_buf timeoutbuf;

static void timer(int signum)
{
  (void)signum;

  timeout += rexmtval;
  if(timeout>=naxtimeout)
    exit(1);
  siglongjmp(timeoutbuf,1);/*跳到断点处执行*/
}

/*用于发送文件的函数*/
static void sndfile(struct transfer*pf)
{
  struct tftphdr*dp;
  struct tftphdr*ap;   /*ACK数据报报头*/
  int block=1;
  int size, n;
  
  signal(SIGALRM,timer); /*设置定时器*/
  dp=read_init();  /*取得一块缓冲区用于存放读出的文件块*/
  ap=(struct tftphdr*)ackbuf;
  do{
    size=my_read(file,*dp,pf->f_convert);/*读文件块到缓冲区里*/
    if(size<0){
      snderr(errno+100);
      goto abort;
    }
    dp->th_opcode=htons((u_short)DATA);
    dp->th_block=htons((u_short)block);
    timeout=-;
    (void)sigsetjmp(timeoutbuf,1); /*设置断点*/
    
  send_data:
    if(send(peer,dp,size+4,0)!=size+4){
      syslog(LOG_ERR,″tftpd:write:%m\n″);
      goto abort;
    }
    read_block(file,pf->f_convert);
    for(;;){
      alarm(rexmtval);   /*启动定时器*/
      n=recv(peer,ackbuf,sizeof(ackbuf),0);
      alarm(0); /*收到数据报，关闭定时器*/
      if(n<0){
	syslog(LOG_ERR,″tftpd:read:%m\n″);
	goto abort;
      }
      ap->th_opcode=ntohs(u_short)ap->th_opcode;
      ap->th_block=ntohs((u_short)ap->th_bolck);
      
      if(ap->th_opcode==ERROR)
	goto abort;
    
      if(ap->th_opcode==ACK){
	if(ap->th_block==block){
	  break;
	}
	/*同步*/
	(void)synch(peer);
	if(ap->th_block==(block-1)){
	  goto send_data; /*返回重发*/
	}
      }
      
    }/*for*/
    block++;
  }while (size==BLOCKSIZE);
  abort;
  (void )fclose(file);
}

Static void justquit(int signum)
{
  (void)signum;
  eixt(0);
}

/*接收文件*/
static void recvfile(struct transfer *pf)
{
  struct tftphdr *dp;
  struct tftphdr ap;  /*ACK缓冲区*/
  int block=0;
  int n, size;
  
  signal(SIGALRM,timer);
  dp=write_init();  /*取得一块缓冲区用于存放接收到的文件块*/
  ap=(struct tftphdr *)adkbuf;

  do{
    timeout=0;
    ap->th_opcode=htons(u_short)ACK;
    ap->th_block=htons((u_short)block);
    block++;
    (void)sigsetjmp(timeoutbuf,1);
    send_ack;
    if(send(peer,ackbuf,4,0)!=4){
      syslog(LOG_ERR,″tftpd:write:%m\n″);
      goto abort;
    }

    write_block(file,pf->f_convert);

    for(;;){
      alarm(rexmtval);
      n=recv(peer,dp,PACKSIZE,0); /*接收文件块*/
      alarm(0);
      if(n<0)
	syslog(LOG_ERR,″tftpd:read:%m\n″);
      goto abort;
    }
    dp->th_opcode=ntohs(u_shot)dp->th_opcode;
    dp->th_block=ntohs(u_short)dp->th_block;
    if(dp->th_opcode==ERROR)
      goto abort;
    if(dp->th_opcode==DATA){
      if(dp->th_block==block){
	break;  /*normal*/
      }
      /*同步*/
      (void)synch(peer);
      if(dp->th_block==(block-1){
	goto send_ack;   /*重发*/
      }
	 size =my+write(file,&dp,n-4,pf->f_convert);
	 if(size!=(n-4))
	 if(size<0)
	 snderr(errno+100);
	 else snderr(ENOSPACE);
	 goto abort;
	 }
    }while(size==BLOCKSIZE);
    write_block(file,pf->f_convert);
    (void)fclose(file);   /*关闭文件*/
    
    ap->th_opcode=htons(u_short)ACK);
  ap->th_block=htons(u_short)(block);
  (void)send(peer,ackbuf,4,0);
  /*发送最后一个文件块的ACK*/
  signal(SIGALRM,justquit);   /*设置定时器*/
  alarm(rexmtval);
  n=recv(peer,buf ,sizeof(buf),0);  /*超时将退出*/
  alarm(0);
  if(n>=4 &&dp->th_block){
    /*客户端重发了最后一文件块，南非要发ACK*/
    （void）send(peer,ackbuf,4,0);
  }
  abort;
  return;
}

struct errmsg{
  int e_code;
  const char*e_msg;
}errmsgs[]={
  {EUNDEF,   ″Undefined error code″},
  {ENOTFOUND,   ″File not found″},
  {EACCESS,  ″Access violation″},
  {ENOSPACE ,  ″Disk full or allocation exceeded″},
  {EBADOP,   ″Illegal TFTP operation″},
  {EBADID,   ″Unknown transfer ID″},
  {EEXISTS,   ″File already exists″},
  {ENOUSER,  ″No such user″},
  {-1,  0},
};

/*此函数用于发送一个ERROR数据报*/
static void snderr(int error)
{
  struct tftphdr*tp;
  int length;
  struct errmsg *pe;
  
  tp=(struct tftphdr*)buf;
  tp->th_opcode=htons(u_short)ERROR;
  tp->th_code=htons(u_short)error;
  for(pe=errmsgs;pe->e_code>=0;pe++)
    if(pe->e_code==error)/*该错误号已定义*/
      break;
  if(pe->e_code<0){
    pe->e_msg=strerror(error-100);
    tp->th_code=EUNDEF; /*置错误号为"UNDEF"*/
  }
  strcpy(tp->th_msg,pe-e_msg);
  length=strlen(pe->e_msg);
  tp->th_msg[length]= ′\0′;
  length +=5;
  if(send(peer,buf,length,0)!=length)
    syslog(LOG_ERR,″snderr:%m\n″);
}
