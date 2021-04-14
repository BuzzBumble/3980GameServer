#include "gameserver.h"
#define PORT 5000
#define MAXLINE 1024
#define PTCL_VERSION 1

static int max(int x, int y) {
	if (x > y) {
		return x;
	} else {
		return y;
	}
}

int main() {
	GameServer *gs = malloc(sizeof(GameServer));

	GameServerInit(gs, PTCL_VERSION, PORT);
	GameServerLoop(gs);
	
	free(gs);
	return 0;
}
