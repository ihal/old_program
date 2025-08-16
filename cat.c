/* */
/********************************************/

/*      cat.c catenate and print            */

/********************************************/

/* ver. YYMMDD Name    Description                    */
/* ==== ====== ======= ============================== */
/* 0.1  950731 Hal     for Hobby                      */
/* 0.2  950803 Hal     サイバ−の仕様に合わせる。     */
/* 0.3  950908 Hal     インテルヘクサ表示機能追加     */
/* 0.4  951113 Hal     OS-9用,行端処理 CR -> CR+LF    */
/* 0.4a 951209 Hal     CR CR連続処理時の修正          */
/* */

#include <stdio.h>
#include <string.h>

#define ADDR 2
#define MSDOS

enum boolean {NO,YES};

void		(*filecopy)(
FILE *ifp,	/* 入力ストリーム */
FILE *ofp	/* 出力ストリーム */
);
void		v_filecopy();
void		t_filecopy();
void		ei_filecopy();
void		os9_filecopy();
void		help_msg(void);
void		exit(int i);


void	main(							       /* */
int	argc,							       /* */
char	*argv[]							       /* */
)
 {
	int	i;
	int		j;
	enum boolean	osw_v,osw_h,osw_i,osw_o;
	char		*prog = argv[0];
	char		c;
	FILE		*fp;


	osw_h = osw_v = osw_i= osw_o= NO;
	filecopy = t_filecopy;

/******************************************/

/* Not argument */
	if (argc == 1) {
		(*filecopy)(stdin, stdout);
		exit(0);
	}

/******************************************/

/* Option check loop */
	for (i = 1;i<argc;++i) {
/* One charcter option */
		if (*argv[i]=='-' && *(argv[i]+1)!= '-' &&
		 *(argv[i]+1)!='\0') {
			j = 0;
			while ( (c = *(argv[i]+(++j))) != '\0') {
				switch (c) {
					case 'v':
						osw_v = YES;
						break;
					case 'h':
						osw_h = YES;
						break;
					case 'i':
						osw_i = YES;
						break;
					case 'o':
						osw_o = YES;
						break;
					default:
						fprintf(stderr,
						"%s: illegal option -%c\n"
						,prog,c);
						exit(1);
						break;
				}
			}
/* charcters option */
		} else if (*(argv[i]+1)=='-') {
			if (strcmp((argv[i]+2),"help")==0) {
				osw_h = YES;
				continue;
			}
			if (strcmp((argv[i]+2),"version")==0) {
				fprintf(stderr,"Hal textutils 0.1\n");
				exit(0);
			}
			if (strcmp((argv[i]+2),"show-non-printing")==0) {
				osw_v = YES;
				continue;
			}
			fprintf(stderr,"%s: unrecognized option '%s'\n",
			prog,argv[i]);
			exit(1);
		}
	}

/*************************************************/

	if (osw_h==YES) help_msg();
	if (osw_v==YES) filecopy = v_filecopy;
	if (osw_i==YES) filecopy = ei_filecopy;
	if (osw_o==YES) filecopy = os9_filecopy;

/*************************************************/

/* Parameter check loop and print file.*/

	j = 0;
	for (i = 1;i<argc;++i) {
		if (*argv[i]=='-' && *(argv[i]+1)!='\0') continue;
		j = 1;
		if (*argv[i]=='-') {
			(*filecopy)(stdin,stdout);
		} else {
			if ((fp = fopen(argv[i],"rb")) == NULL) {
				fprintf(stderr,"%s: %s:No such file.\n",
				prog,argv[i]);
				exit(1);
			}
			(*filecopy)(fp,stdout);
			fclose(fp);
		}
	}
	if (j==0) (*filecopy)(stdin,stdout);
}


/* show non printing */
void	v_filecopy(						       /* */
FILE	*ifp,							       /*in*/
FILE	*ofp							       /*out*/
)
 {
	int	c;
	char	s[3];

	s[0] = '^';
	s[1] = s[2] = s[3] = '\0';

	while ((c = getc(ifp))!=EOF) {
		if ( ((char)c & 0xe0)==0 && (char)c!='\x0a') {
			s[1] = (char)(c+0x40);
			fputs(&s[0],ofp);
		} else {
			putc(c,ofp);
		}
	}
}

/* text printing */
void	t_filecopy(						       /* */
FILE	*ifp,							       /*in*/
FILE	*ofp							       /*out*/
)
 {
	int	c,c1;

	c1=NULL;
	while ((c = getc(ifp))!=EOF) {
		if (c == '\x1a') {
			break;
		}
		if (c != '\x0d') {
			if (c1 == '\x0d' && c != '\x0a') {
				putc(c1,ofp);
				putc(c,ofp);
			} else {
				putc(c,ofp);
			}
		}
		c1=c;
	}
}


/* CR --> CR LF convert */
void	os9_filecopy(
FILE	*ifp,
FILE	*ofp
)
 {
 	char	c,c1;
	enum boolean	cr;

	cr=NO;
	while (((c = getc(ifp))!=EOF)) {
		switch (c) {
			case '\x0d':
				if (cr == YES && c != '\x0a') {
					putc('\x0a',ofp);
				} else {
					cr=YES;
				}
				break;
			case '\x0a':
				putc(c,ofp);
				cr=NO;
				break;
			default:
				if (cr == YES && c != '\x0a') {
					putc('\x0a',ofp);
					putc(c,ofp);
				} else {
					putc(c,ofp);
				}
				cr=NO;
				break;
		}
	}
}

/* extend intel hex printing */
void	ei_filecopy(						       /* */
FILE	*ifp,							       /*in*/
FILE	*ofp							       /*out*/
)
 {
	unsigned long int	adr;
	char	c;
	char	s[3];
	int	i,col;

	int hc2d(char *);
	
	s[3]=NULL;
	col=0;
	i=0;
	while ((c = getc(ifp))!=EOF) {
		switch (c) {
			case ':':
				col=1;
				continue;
			case '\n':
				col=0;
				i=0;
				continue;
			default:
				if (col != 0) col++;
			}
		if (col == 2) {
			s[0]=c;
			continue;
		}
		if (col == 3) {
			s[1]=c;
			s[2]=NULL;
			i=hc2d(s);
			i=i*2;
			if (i==0) {
#ifdef MSDOS
				putc('\x1a',ofp);
#endif
				break;
			}
			continue;
		}
		if (col >= 3 && col <= 11) {
			continue;
		}
		if (col >= 12 && (i--) > 0) {
			if (col % 2 == 0) {
				s[0]=c;
				continue;
			} else {
				s[1]=c;
				s[2]=NULL;
				c=(char)hc2d(s);
#ifdef MSDOS
				if (c != '\x0d') putc(c,ofp);
#endif
#ifndef MSDOS
				putc(c,ofp);
#endif
				continue;
			}
		}
	}
}

/* ASCII CHR. --> Hex Conv. */
int hc2d(adr)
char *adr;
{
	unsigned int	power;
	int	ret;
	int	i;
	int	j;

	power=1;
	ret=0;
	
	for (i=ADDR-1;i>=0;i--,power=power*16) {
		if (adr[i]>='0' && adr[i]<='9') j=(int)(adr[i] & 0x0f);
		if (adr[i]>='a' && adr[i]<='f') j=(int)(adr[i] & 0x0f)+0x09;
		ret=ret+j*(int)power;
	}
	return((int)ret);
}



void	help_msg(void)
{
	char	**s;
	char	*help_msg[] = {
		"Usage : cat [OPTION] [FILE]...",
		"Option: -h, --help                 display help and exit.",
		"        -v, --show-non-printing    use ^ notation.",
		"        -i                         Extended Intel hex format out.",
		"        -o                         CR(\\x0d) --> CR LF(\\x0d,\\x0a) conv.",
		"            --version              output version and exit",
		"With no FILE, or when FILE is -, read standard input.",
		0
	};

	s = help_msg;
	for (;*s!=0;++s) {
		fputs(*s  ,stderr);
		fputs("\n",stderr);
	}
	exit(0);
}

