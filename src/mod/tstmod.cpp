#include "fpsgame.h"
#include "tstmod.h"
#include <cstdarg>

namespace tst {

	using namespace server;

	int startmillis = 5000, numtraitors = 0, totaltraitors = 2;
	bool roundstarted = false;

	void startround() {
		roundstarted = true;

		int playercount = 0;

		// count active players
		loopv(clients) {
			if (clients[i]->state.state == CS_ALIVE) playercount++;
		}

		if (! playercount) return;

		numtraitors = min(playercount, totaltraitors);

		msgf(-1, "The traitor has been chosen");

		for (int t = 0; t < numtraitors; t++) {
			int traitor = rnd(playercount - t);

			conoutf("traitor chosen: %d\n", traitor);

			loopv(clients) {
				if (clients[i]->state.state != CS_ALIVE || clients[i]->tst.traitor) traitor++;
				else if (i == traitor) maketraitor(clients[i]);
			}
		}

		loopv(clients) if (clients[i]->state.state == CS_ALIVE && ! clients[i]->tst.traitor) msgf(clients[i], "You are \fs\f1innocent\fr! Kill the traitor");
	}

	void maketraitor(clientinfo *ci) {
		ci->tst.traitor = true;
		
		msgf(ci, "You are the \fs\f3traitor! Kill everyone\fr without being noticed.");
	}

	// 0 green: player talk
	// 1 blue: "echo" command
	// 2 yellow: gameplay messages 
	// 3 red: important errors
	// 4 gray
	// 5 magenta
	// 6 orange
	// 7 white

	char *traitornames(char *str) {

		*str = 0;

		vector<clientinfo *> traitors;

		loopv(clients) if (clients[i]->tst.traitor) traitors.add(clients[i]);

		int length = traitors.length();

		if (! length) return str;

		for (int i = 0; i < length; i++) {
			char buffer[32];
			sprintf(buffer, "\"\fs\f%d%s\fr\"", traitors[i]->state.state == CS_ALIVE ? 3 : 4, traitors[i]->name);
			concatstring(str, buffer, MAXSTRLEN);

			if (length - i > 2) concatstring(str, ", ", MAXSTRLEN);
			else if (length - i > 1) concatstring(str, " and ", MAXSTRLEN);
		}

		return str;
	}

	void msgf(int cn, const char *fmt...) {
		va_list args;
		va_start(args, fmt);

		string buffer;

		vsprintf(buffer, fmt, args);

		sendf(cn, 1, "ris", N_SERVMSG, buffer);
	}

	void msgf(clientinfo *ci, const char *fmt...) {
		va_list args;
		va_start(args, fmt);

		string buffer;

		vsprintf(buffer, fmt, args);

		sendf(ci->clientnum, 1, "ris", N_SERVMSG, buffer);
	}

	void toggleautospec(clientinfo *actor, clientinfo *spectator) {
		autospec(actor, spectator, ! spectator->tst.autospec);
	}

	void autospec(clientinfo *actor, clientinfo *spectator, bool state) {
		spectator->tst.autospec = state;

		msgf(spectator, "Autospectating is now \fs%s\fr", state ? "\f0ENABLED" : "\f3DISABLED");

		if (actor != spectator) {
			msgf(actor, "Autospectating for \"\fs\f1%s\fr\" is now \fs%s\fr", state ? "\f0ENABLED" : "\f3DISABLED");
		}
	}

}
