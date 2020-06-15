#include <pads.h>

#define	MAXLIN 512

class Story : public PadRcv {
	char	file[MAXLIN];
	FILE	*fp;
	long	lines;
	Pad	*pad;
	void	linereq(long,Attrib);
public:
	void	open();
		Story(char *f)	{ strncpy( file, f, strlen(f)-1 ); }
};

class News  : public PadRcv {
	Pad	*pad;
public:
		News();
};

int main(){
	const char *error = PadsInit(PADSTERM);
	if( chdir( "/export/home/noel/news" ) ) exit(1);
	if( error ){
		fprintf( stderr, "%s", error );
		exit(1);
	}
	new News();
	PadsServe();
	return 0;
}

News::News(){
	Story *s;
	FILE *fp, *popen(char*,char*);
	Menu m( "open", (Action) &Story::open );
	char file[MAXLIN];
	long uniq = 0;
	pad = new Pad( (PadRcv *) this );
	pad->options(TRUNCATE|SORTED); 
	pad->banner( "News:" );
	pad->name( "News" );
	pad->makecurrent();	
	if( !(fp = Popen("ls", "r")) ){
		pad->insert( 1, "can't ls" );
		return;
	}
	while( fgets(file, MAXLIN, fp) ){
		s = new Story( file );
		pad->insert( ++uniq, (Attrib) 0, (PadRcv*) s, m, "%s", file );
	}
	pclose(fp);
}

void Story::open(){
	char buf[256];

	if( !pad ){
		pad = new Pad( (PadRcv*) this );
		pad->banner( "%s:", file );
		pad->name( file );
		if( !(fp = fopen( file, "r" )) ){
			pad->insert( 1, "cannot open file" );
			return;
		}
		lines = 0;
		while( fgets( buf, 256, fp ) ) ++lines;
		pad->lines(lines);
	}
	pad->makecurrent();
}

void Story::linereq( long i, Attrib a ){
	char buf[256];
	long n;

	fseek( fp, 0, 0 );
	for( n = i; n > 0; --n )
		fgets( buf, 256, fp );
	pad->insert( i, a, "%s ", buf );
}
