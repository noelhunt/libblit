#include <pads.h>

int main(int argc, char *argv[]){
	PadsInit(PADSTERM);
	PadsServe();
	return 0;
}
