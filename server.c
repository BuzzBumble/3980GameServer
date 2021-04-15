#include "gameserver.h"
#define PORT 5000
#define MAXLINE 1024
#define PTCL_VERSION 1

int main() {
	GameServer *gs = malloc(sizeof(GameServer));

	if (GameServerInit(gs, PTCL_VERSION, PORT) == -1) {
		puts("Could not initialize game server");
		free(gs);
		exit(0);
	}
	GameServerLoop(gs);
	
	free(gs);
	return 0;
}
