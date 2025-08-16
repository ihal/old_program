/*
********************************************

        hd.c hextal dump

********************************************

ver. YYMMDD Name    Description
==== ====== ======= ========================
0.1  950802 Hal     for Hobby
0.1a 950803 Hal  
0.2  950917 Hal     Extend Intel Hex format タイオウ。
0.2a 950918 Hal  
*/

#include <stdio.h>
#include <string.h>
#define VER "Hal textutils 0.1\n"

enum boolean {NO,YES};

void main(argc,argv)
int	argc;
char	*argv[];
{
	int		i;
	int		j;
	enum boolean	osw_x,osw_d,osw_k,osw_h,osw_f,osw_i;
	char		*prog=argv[0];
	char		c;
#define ADDR 8
	static char	addr10[ADDR];
	static char	addr16[ADDR];
	unsigned long int	address;
	FILE		*fp;

	void		(*filecopy)(FILE *,FILE *,unsigned long int );
	void		h_filecopy();
	void		ei_filecopy();
	void		push(char *,char );
	void		help_msg(void );
	void		exit(int );
	int		tolower(int );
	unsigned long int	hc2d(char *);
	unsigned long int	dc2d(char *);

	osw_x=osw_k=osw_h=osw_f=osw_i=NO;
	filecopy=h_filecopy;
	address=0;
	for (i=0;i<ADDR;i++) {
		addr16[i]='0';
		addr10[i]='0';
	}

/******************************************/

/* Not argument */
	if(argc==1) {
		(*filecopy)(stdin, stdout,address);
		exit(0);
	}

/******************************************/

/* Option check loop */
	for (i=1;i<argc;++i) {
/* '+' charcter option */
		if (*argv[i]=='+') {
			j=0;
			while( (c=*(argv[i]+(++j))) != '\0') {
				if ('0'<=c && '9'>=c) {
					osw_d=YES;
					push(addr10,c);
					push(addr16,c);
					continue;
				}
				if ('a'<=c && 'f'>=c) {
					push(addr16,c);
					continue;
				}
				if ('A'<=c && 'F'>=c) {
					c=(char)(tolower((int)c));
					push(addr16,c);
					continue;
				}
				switch(c) {
					case 'x':
						osw_x=YES;
						break;
					case 'k':
						osw_k=YES;
						break;
				}
			}
		}

/* One charcter option */
		if (*argv[i]=='-' && *(argv[i]+1)!='-' && *(argv[i]+1)!='\0') {
			j=0;
			while( (c=*(argv[i]+(++j))) != '\0') {
				switch(c) {
					case 'h':
						osw_h=YES;
						break;
					case 'f':
						osw_f=YES;
						break;
					case 'i':
						osw_i=YES;
						break;
					default:
						fprintf(stderr,"%s: illegal option -%c\n",prog,c);
						exit(1);
				}
			}
/* charcters option */
		} else if (*(argv[i]+1)=='-') {
			if (strcmp((argv[i]+2),"help")==0) {
				osw_h=YES;
				continue;
			}
			if (strcmp((argv[i]+2),"version")==0) {
				fprintf(stderr,VER);
				exit(0);
			}
			fprintf(stderr,"%s: unrecognized option '%s'\n",prog,argv[i]);
			exit(1);
		}
	}

/******************************************/

	if (osw_d==YES)		address=dc2d(addr10);
	if (osw_x==YES) 	address=hc2d(addr16);

	if (osw_k==YES) {
		if (osw_d==YES && osw_x==NO) address=address*1000;
		if (osw_x==YES) address=address<<0x0c;
	}

	if (osw_h==YES) help_msg();

	if (osw_i==YES) filecopy=ei_filecopy;


/******************************************/

/* Parameter check loop and print file.*/

	j=0;
	for (i=1;i<argc;++i) {
		if(*argv[i]=='-' && *(argv[i]+1)!='\0') continue;
		if(*argv[i]=='+' && *(argv[i]+1)!='\0') continue;
		j=1;
		if (*argv[i]=='-') {
			if (osw_f==YES) printf("%s\n",argv[i]);
			(*filecopy)(stdin,stdout,address);
		} else {
			if ((fp=fopen(argv[i],"rb")) == NULL) {
				fprintf(stderr,"%s: %s:No such file.\n",prog,argv[i]);
				exit(1);
			}
			if (osw_f==YES) printf("%s\n",argv[i]);
			(*filecopy)(fp,stdout,address);
			fclose(fp);
		}
		
	}
	if(j==0) {
		if (osw_f==YES) printf("%s\n",argv[i]);
		(*filecopy)(stdin,stdout,address);
	}
}



/* hex dump */
void h_filecopy(ifp,ofp,offset)
FILE *ifp;
FILE *ofp;
unsigned long int offset;
{
	int	c;
	int	s[16];
	int	i;
	unsigned long int	adr;

	i=0;
	adr=0;
	while(adr<offset) {
		if (getc(ifp)==EOF) return;
		adr++;
	}


	while((c=getc(ifp))!=EOF) {
		s[i++]=c & 0x0ff;
		if (i>=16) {
			fprintf(ofp,"%08lx ",adr);
			for (i=0;i<=15;i++) fprintf(ofp,"%02x ",s[i]);
			for (i=0;i<=15;i++) {
				if (s[i]>'~' || s[i]<' ') s[i]='.';
				fprintf(ofp,"%c",s[i]);
			}
			adr=adr+0x10;
			putc('\n',ofp);
			i=0;
		}
	}
	if (i>0) {
		c=i-1;
		fprintf(ofp,"%08lx ",adr);
		for (i=0;i<=c;i++) fprintf(ofp,"%02x ",s[i]);
		for (;i<=15;i++)   fprintf(ofp,"   ");
		for (i=0;i<=c;i++) {
			if (s[i]>'~' || s[i]<' ') s[i]='.';
			fprintf(ofp,"%c",s[i]);
		}
		for (;i<=15;i++) fprintf(ofp," ");
		putc('\n',ofp);
	}
	putc('\n',ofp);
}

/* Extend intel hex dump */
void ei_filecopy(ifp,ofp,offset)
FILE *ifp;
FILE *ofp;
unsigned long int offset;
{
	int	c;
	int	s[16];
	int	i;
	int	j;
	unsigned long int	adr;

	i=0;
	adr=0;
	while(adr<offset) {
		if (getc(ifp)==EOF) return;
		adr++;
	}


	while((c=getc(ifp))!=EOF) {
		s[i++]=c & 0x0ff;
		if (i>=16) {
			fprintf(ofp,":10%06lx00",adr);
			j=0x010 + (adr & 0x0ff) + (adr >> 8) & 0x0ff + (adr >> 16) & 0x0ff;
			for (i=0;i<=15;i++) {
				fprintf(ofp,"%02x",s[i]);
				j=j+s[i];
			}

			j=j & 0x0ff;
			j=0x100 - j;
			j=j & 0x0ff;
			fprintf(ofp,"%02x\n",j);
			adr=adr+0x10;
			i=0;
		}
	}
	if (i>0) {
		c=i-1;
		j=c & 0x0ff;
		fprintf(ofp,":%02x%06lx00",j,adr);
		j=c + (adr & 0x0ff) + (adr >> 8) & 0x0ff + (adr >> 16) & 0x0ff;
		for (i=0;i<=c;i++) {
			fprintf(ofp,"%02x",s[i]);
			j=j+s[i];
		}
		j=j & 0x0ff;
		j=0x100 - j;
		j=j & 0x0ff;
		fprintf(ofp,"%02x\n",j);
	}
	fprintf(ofp,":00000001ff\n",j);
}










void	push(adr,c)
char	*adr;
char	c;
{
	register int	i;

	for (i=1;i<ADDR;i++) {
		adr[i-1]=adr[i];
	}
	adr[ADDR-1]=c;
}

unsigned long int hc2d(adr)
char *adr;
{
	unsigned long int	ret,power;
	int	i;
	int	j;

	power=1;
	ret=0;
	
	for (i=ADDR-1;i>=0;i--,power=power*16) {
		if (adr[i]>='0' && adr[i]<='9') j=(int)(adr[i] & 0x0f);
		if (adr[i]>='a' && adr[i]<='f') j=(int)(adr[i] & 0x0f)+0x09;
		ret=ret+(unsigned long)j*power;
	}
	return(ret);
}

unsigned long int dc2d(adr)
char *adr;
{
	unsigned long int	ret,power;
	int	i;
	int	j;

	power=1;
	ret=0;
	
	for (i=ADDR-1;i>=0;i--,power=power*10) {
		if (adr[i]>='0' && adr[i]<='9') j=(int)(adr[i] & 0x0f);
		ret=ret+(unsigned long)j*power;
	}
	return(ret);
}

	



void help_msg(void)
{
	void	exit(int );
	char	**s;
	char	*help_msg[] = {
		"Usage : hd [OPTION] [FILE]...",
		"Option: -h, --help                 Display this help and exit.",
		"        -f                         Output with filename.",
		"        -i                         Output Extend Intel Hex format.",
		"        +[x]offset[k]              offset.",
		"            --version              output version information and exit",
		"With no FILE, or when FILE is -, read standard input.",
		0
	};
	s=help_msg;
	for(;*s!=0;++s) {
		fputs(*s  ,stderr);
		fputs("\n",stderr);
	}
	exit(0);
}
