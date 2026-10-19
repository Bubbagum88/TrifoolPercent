// TrifoolPercent progression beat flags.
//
// Session-scoped flags for the Triforce% story beats that gate later cutscenes:
// first Nabooru meeting (Thieves' Hideout), Long of Time learned (Hideout, second
// visit), Running Man race won, and Chamber of Sages visited. Each flag mirrors the
// shape of TrifoolBeat_HasSagesCharm / TrifoolBeat_GrantSagesCharm in
// Beat2_0SagesCharm.cpp -- a single bool, with a getter and a setter exposed to C so
// actor / scene glue can key off it.
//
// The flags live in-memory only; a future slice will persist the ones that need to
// survive a save (`eventChkInf` free bits, same route Setup.cpp uses). Scoping them
// here now unblocks the Sacred Realm and Staff Roll chain scaffolds without waiting on
// the save-slot decisions.

extern "C" {
#include "TrifoolPercent.h"
}

static bool sHasMetNabooru = false;
static bool sHasLearnedLongOfTime = false;
static bool sHasBeatenRunningMan = false;
static bool sHasVisitedChamberOfSages = false;

extern "C" bool TrifoolBeat_HasMetNabooru(void) { return sHasMetNabooru; }
extern "C" void TrifoolBeat_GrantMetNabooru(void) { sHasMetNabooru = true; }

extern "C" bool TrifoolBeat_HasLearnedLongOfTime(void) { return sHasLearnedLongOfTime; }
extern "C" void TrifoolBeat_GrantLearnedLongOfTime(void) { sHasLearnedLongOfTime = true; }

extern "C" bool TrifoolBeat_HasBeatenRunningMan(void) { return sHasBeatenRunningMan; }
extern "C" void TrifoolBeat_GrantBeatenRunningMan(void) { sHasBeatenRunningMan = true; }

extern "C" bool TrifoolBeat_HasVisitedChamberOfSages(void) { return sHasVisitedChamberOfSages; }
extern "C" void TrifoolBeat_GrantVisitedChamberOfSages(void) { sHasVisitedChamberOfSages = true; }
