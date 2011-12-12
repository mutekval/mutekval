#ifndef _TFTPDFUNC_H
#define   TFTPDFUNC_H1
int my_read(FILE*file, struct tftphdr**dpp,int convert);
int my_write(FILE*file, struct tftphdr**dpp, int ct, int convert);
void read_block(FILE*file, int convert);
int write_block(FILE*file, int convert);
int synch(int);
struct tftphdr*read_init(void);
struct tftphdr*write_init(void);

#endif
