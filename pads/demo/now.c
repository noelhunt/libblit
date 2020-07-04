#include <sys/types.h>
#include <time.h>
#include <pads.h>

class Date : public PadRcv {
	Pad	*pad;
	void	date();
public:
		Date();
};

Date::Date(){
	Menu m;
	pad = new Pad( (PadRcv*) this );
	pad->banner( "Current Date" );
	pad->name( "date" );
	m.first( "date", (Action) &Date::date );
	pad->menu( m );
	pad->makecurrent();
}

void Date::date(){
	long t;
	time(&t);
	pad->insert( 1, "%s", ctime(&t) );
}

int main(int argc, char *argv[]){
	const char *error = PadsInit(PADSTERM);
	if( error ){
		fprintf( stderr, "%s", error );
		exit(1);
	}
	new Date;
	PadsServe();
	return 0;
}
