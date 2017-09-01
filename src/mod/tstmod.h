#ifndef __TSTMOD_H__
#define __TSTMOD_H__
#define TSTMOD 1

namespace tst {

	using namespace server;

	extern int startmillis, totaltraitors, numtraitors;
	extern bool roundstarted;

	struct tstinfo {
		bool traitor = false, autospec = false;
	};

	void startround();
	void msgf(int cn, const char *fmt...);
	void msgf(clientinfo *ci, const char *fmt...);
	void maketraitor(server::clientinfo *ci);
	char *traitornames(char *str);

	void toggleautospec(clientinfo *actor, clientinfo *spectator);
	void autospec(clientinfo *actor, clientinfo *spectator, bool state);
}


#endif

#ifdef SERVMODE // for server.cpp

void unspectate(clientinfo *ci);

struct tstservmode : servmode {

	void update() {
		if (! tst::roundstarted && gamemillis >= tst::startmillis) {
			tst::startround();
		}
	}

	void gotospec(clientinfo *ci) {
		ci->state.state = CS_SPECTATOR;
        ci->state.timeplayed += lastmillis - ci->state.lasttimeplayed;
        if(!ci->local && (!ci->privilege || ci->warned)) aiman::removeai(ci);
        sendf(-1, 1, "ri3", N_SPECTATOR, ci->clientnum, 1);
	}

	void moved(clientinfo *ci, const vec &oldpos, bool oldclip, const vec &newpos, bool newclip) {
		if (oldpos != newpos) conoutf("%s just moved", ci->name);
	}

	void died(clientinfo *victim, clientinfo *actor) {

		this->gotospec(victim);

		if (victim->tst.traitor) {
			tst::numtraitors--;

			if (actor) {
				tst::msgf(-1, "The traitor \"\fs\f3%s\fr\" has been killed by \"\fs\f1%s\fr\".", victim->name, actor->name);
			}

			else {
				tst::msgf(-1, "The traitor \"\fs\f3%s\fr\" killed himself.", victim->name);
			}

			if (tst::numtraitors <= 0) {
				server::startintermission();
			}
		}

		else if (actor && ! actor->tst.traitor) {
			suicide(actor);
			tst::msgf(actor, "You killed someone \fs\f1innocent\fr!");
		}

		// count players
		int innocentcount = 0;

		loopv(clients) {
			if (clients[i]->state.state == CS_ALIVE && ! clients[i]->tst.traitor) innocentcount++;
		}

		if (innocentcount == 0) {
			// only traitors left
			server::startintermission();
			return;
		}
	}

	bool canspawn(clientinfo *ci, bool connecting = false) {
		return ! tst::roundstarted;
	}

	void leavegame(clientinfo *ci) {
		if (ci->tst.traitor) {
			tst::numtraitors--;

			tst::msgf(-1, "The traitor \"\fs\f3%s\fr\" left the game.", ci->name);
		
			if (tst::numtraitors <= 0) {
				server::startintermission();
			}
		}
	}

	// void entergame(clientinfo *ci) {
	// 	if (clients.length())
	// }

	void intermission() {

		int innocentcount = 0;

		loopv(clients) {
			if (clients[i]->state.state == CS_ALIVE && ! clients[i]->tst.traitor) innocentcount++;
		}

		string temp;
		
		if (innocentcount == 0) {
			tst::msgf(-1, tst::totaltraitors ? "The traitors %s won." : "The traitor %s won.", tst::traitornames(temp));
		}

		else if (tst::totaltraitors > 1) {
			tst::msgf(-1, "The traitors %s failed to kill all the innocents.", tst::traitornames(temp));
		}
	}

	void cleanup() {
		loopv(clients) {
			clients[i]->tst.traitor = false;
		}

		tst::roundstarted = false;
		tst::numtraitors = 0;

		loopv(clients) {
			if (clients[i]->tst.autospec == false) unspectate(clients[i]);
		}
	}

};

#endif