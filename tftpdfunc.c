#include<sys/types.h>
#include<sys/socket.h>
#include<sys/ioctl.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdio.h>

#include��tftpdfunc.h��

#define PACKSIZE BLOCKSIZE+4

struct bf{
  int counter;   /*���ݻ������ĳ��Ȼ��Ǳ�־*/
  char buf[PACKSIZE];   /*���ݻ�����*/
}buffers[2];

/*bf.counter�ı�־*/
#define BF_ALLOC-3    /*bf.buf�ѱ����䣬����δ��������*/
#define BF_FREE  -2    /*bf.buf��δ������*/

static int next; /*��һ�����û�����*/
static int current;   /*��ǰ����ʹ�õĻ�����*/

/*����ת���ļ������ʽ����������*/
int newline=0;
int prevchar=-1;   /*ǰһ���ַ�*/

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
  buffers[0].counter=BF_ALLOC;   /*�����һ�黺����*/
  current=0;
  buffers[1].counter=BF_FREE;
  next=x;
  return (struct tftphdr*)buffers[0].buf;
}

int my_read(FILE*file,struct tftphdr **dpp,
	    int convert/*convertֵΪ1����ת����netascii��*/)
{
  struct bf *b;
  
  buffers[current].counter=BF_FREE;
  current=!current;
  
  b=&buffers[current];
  if(b->counter==BF_FREE)   /*��������*/
    read_block(file.convert);
  
  dpp=(struct tftphdr)b->buf;
  return b->counter;
}

void read_block(FILE*file,int convert   /*ֵΪ1�� ��ת����ascii��*/)
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
	
	if(convert ==0){    /*ֱ���Զ����������������ת����ֱ�Ӷ���*/
	  b->counter=read(fileno(file),dp->th_data,BLOCKSIZE);
	  return;
	}
	/*��ת����ascii�룬��������ַ���������β�Ļ��з�*/
	p=dp->th_data;
	for(i=0;i<BLOCKSIZE;i++){
	  if (newline){
	    if(prevchar==��\n��)
	      c=��\n��;
	    else  c=��\0��;
	    newline=0;
	  }
	  else{
	    c=getc(file);
	    if(c==EOF)break;
	    if(c==��\n��||c==��\r��){
	      prevchar=c;
	      c=��\r��;
	      newline=1;
	    }
	  }
	  *p++=c;
	}
	b->counter=(int)(p-dp->th_data);
}


int my_write(FILE*file,struct tftphdr** dpp,int ct, int convert)
{
  buffers[current].counter=ct;    /*���ݿ�ĳ���*/
  current=!current;
  if(buffers[current].counter!=BF_FREE)/*������*/
    write_block(file,convert);   /*д�����ݣ�ˢ�»�����*/
  buffers[current].counter=BF_ALLOC; /*��־�û������ѷ���*/
  *dpp=(struct tftphdr)buffers[current].buf;
  return ct;
}

int write_block(FILE*file,int convert)
{
  char*buf;
  int count;
  int ct;
  char*p;
  int c;  /*��ǰ�������ַ�*/
  struct bf*b;
  struct tftphdr*dp;

  b=&buffers[next];
  if(b->counte <-1)      /*���ݿ鳤��*/
  b->counter=BF_FREE;
  dp=(struct tftphdr*)b->buf;
  next=!next;
  140	buf=dp->th_data;
  
  if(count<0)
    return-1;
  if(convert==0)/*����ת��*/
    return= write(fileno(file);buf, count);
  
  p=buf;
  ct=count;
  while(ct--)
  c=*p++;
  if(prevchar==��\r��)
    if(c==��\n��)   /*�س����ǻ���*/
      fseek(file ,-1,1);
    else
      if(c==��\0��)   /*�س����ǿ��ַ�*/
	goto   skipit;   /*���ַ���д���ļ�*/
  putc(c,file);
  skipit;
  prevchar=c;

  return count;
}

int synch(int f/*�׽���*/)
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
