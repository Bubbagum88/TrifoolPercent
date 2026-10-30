// TrifoolPercent -- merged ocarina song-recognition engine.
//
// Vanilla OoT has two parallel paths through its ocarina code:
//
//   1. The "button-value" path, used for every teachable song. The game stores
//      only each note's button index (0x2 D4, 0x5 F4, 0x9 A4, 0xB B4, 0xE D5)
//      and uses sNoteValueIndexMap / sOcarinaNoteValues to collapse chromatic
//      input back onto those five slots before matching sOcarinaSongNotes.
//      Half-step bends are discarded.
//
//   2. The "chromatic" path, used only for the Scarecrow memory game and the
//      scarecrow's-song playback. It stores the full semitone value (0x00..0x11,
//      an 18-note range covering flats and sharps around each button) and
//      compares them directly, so a D-flat sounds different from a D.
//
// Triforce% merged the two by gutting the collapse step in path 1 so every
// song recognition and every playback runs on the chromatic representation
// used by path 2. That is what unlocks the eight-note "Long of Time" variant
// of Song of Time, which contains notes (0x0, 0x4, 0x7) that have no
// button-value representation. Upstream the merge is a set of MIPS patches
// to the vanilla ROM (statics/ocarina.c Statics_OcarinaCodePatches +
// scripts/statics/ocarina_patches.inc: removing sNoteValueIndexMap,
// sOcarinaNoteValues, the 0xC0 semitone wipe, Audio_OcaMapNoteValue, and
// forcing the "always like Pierre's song" multi-screen playback).
//
// SoH already ships with chromatic input available (OnOcarinaNote delivers
// a note + bend to listeners) and the vanilla recognition path still runs
// against sOcarinaSongNotes. Rather than patching the engine's internals,
// this module runs a parallel chromatic matcher: it listens to every note
// Link plays via the GameInteractor OnOcarinaNote hook, keeps a sliding
// window of the last few chromatic values, and fires a match callback when
// the window ends in one of our chromatic song patterns -- Long of Time
// first, with room for Overture of Sages and others as the beats land.
//
// This runs alongside vanilla recognition, so the Song of Time still works
// unchanged (Nabooru does not retire it); Long of Time is a strict superset
// that matches on its own longer, chromatic sequence. Later beats can gate
// which songs are armed via Trifool flags.
//
// Adapted from Triforce% statics/ocarina.c (DetectableSong/CanonSong,
// LongOfTime_Detectable, OvertureOfSages_Detectable, Statics_OcarinaCodePatches)
// (c) 2019-2022 The Triforce% Team, used under upstream/TRIFORCE-PERCENT-LICENSE.txt.

#include <array>
#include <cstdint>
#include <cstring>

#include <libultraship/bridge/consolevariablebridge.h>

#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/ShipInit.hpp"
#include "soh/SohGui/SohMenu.h"
#include "soh/SohGui/SohGui.hpp"
#include "soh/SohGui/UIWidgets.hpp"

extern "C" {
#include "TrifoolPercent.h"
#include "macros.h"
#include "functions.h"
#include "variables.h"
extern PlayState* gPlayState;
}

#define CVAR_SONG_ENGINE_VALUE CVarGetInteger(CVAR_TRIFOOL_SONG_ENGINE, 0)
#define CVAR_LONG_OF_TIME_VALUE CVarGetInteger(CVAR_TRIFOOL_LONG_OF_TIME, 0)

// -----------------------------------------------------------------------------
// Chromatic note encoding.
// -----------------------------------------------------------------------------
//
// Semitone layout 0x00..0x11 covers two octaves of D-centred ocarina range:
//
//   0x00  C4    0x06  F#4   0x0C  C5
//   0x01  C#4   0x07  G4    0x0D  C#5
//   0x02  D4    0x08  G#4   0x0E  D5    <- C-up
//   0x03  D#4   0x09  A4    0x0F  D#5
//   0x04  E4    0x0A  A#4   0x10  E5
//   0x05  F4    0x0B  B4    0x11  F5
//        ^C-down     ^A         ^C-right        ^C-left (0x0B)
//
// Vanilla OoT's five button values land at indices 2, 5, 9, 0xB, 0xE. SoH's
// GameInteractor OnOcarinaNote hook delivers (note, modulator, bend) where
// `note` is the OcarinaNoteIdx enum (OCARINA_NOTE_A..OCARINA_NOTE_C_UP, 0..4)
// and `bend` is the applied semitone offset (chromatic ocarina feature). We
// collapse both into a single chromatic value here.
//
// base_semitone[note] layout:   A  Cdn Crt Clf Cup
//                               9   5  11  11?  14   -- see note below.
//
// Vanilla OoT 1.0 maps the five ocarina buttons as:
//   OCARINA_NOTE_A      (A button)  -> A4   (0x09)
//   OCARINA_NOTE_C_DOWN              -> F4   (0x05)
//   OCARINA_NOTE_C_RIGHT             -> B4   (0x0B)
//   OCARINA_NOTE_C_LEFT              -> A4   -- duplicate pitch; this is a
//                                               known Nintendo quirk, the C-left
//                                               button plays the same A4 the
//                                               A button does. OoT songs rely
//                                               on this: Minuet of Forest's
//                                               (0x02, 0x0E, 0x0B, 0x09, ...)
//                                               hits the C-left value via the
//                                               A-button index.
//   OCARINA_NOTE_C_UP                -> D5   (0x0E)
//
// Triforce% treats C-left as its own position because its chromatic rebuild
// gives each physical button a distinct pitch slot; vanilla OoT would never
// hit the C-left slot through sOcarinaSongNotes without a note bend. For now
// we use the vanilla pitch mapping (which keeps vanilla songs' button indices
// stable) and leave 0x02 D4 reachable via OCARINA_NOTE_A + bend=-7, same way
// Nabooru's Long of Time teach walks through it in upstream Triforce%.

static constexpr uint8_t kBaseSemitone[5] = {
    /* OCARINA_NOTE_A      */ 0x09,
    /* OCARINA_NOTE_C_DOWN */ 0x05,
    /* OCARINA_NOTE_C_RIGHT*/ 0x0B,
    /* OCARINA_NOTE_C_LEFT */ 0x02, // see note above; this is upstream Triforce%'s choice
    /* OCARINA_NOTE_C_UP   */ 0x0E,
};

static constexpr uint8_t kMaxSemitone = 0x11;
static constexpr uint8_t kNoNote = 0xFF;

// Longest Trifool-detectable sequence (Long of Time is 8). Over-provision to
// keep the matcher stable if a longer song lands later.
static constexpr size_t kHistoryDepth = 12;

static std::array<uint8_t, kHistoryDepth> sNoteHistory{};
static size_t sNoteCount = 0; // how many notes have ever landed; indexes mod kHistoryDepth.

// -----------------------------------------------------------------------------
// Chromatic detectable songs.
// -----------------------------------------------------------------------------
//
// Mirrors Triforce%'s DetectableSong { num_notes, s8 notes[8] }. We carry
// the song identity as an enum so the match callback can dispatch without
// pointer compares, and a Trifool-local id (ptf) that other beats can key
// their behavior to without reusing OoT's OCARINA_SONG_* enum.

enum TrifoolSongId : uint8_t {
    TRIFOOL_SONG_NONE = 0,
    TRIFOOL_SONG_LONG_OF_TIME,
    TRIFOOL_SONG_OVERTURE_OF_SAGES, // placeholder for Beat 3; not matched until armed.
    TRIFOOL_SONG_COUNT,
};

struct TrifoolDetectableSong {
    TrifoolSongId id;
    uint8_t numNotes;
    uint8_t notes[8];
    const char* debugName;
    bool (*isArmed)(); // returns true when this song may currently match.
};

static bool AlwaysArmed() { return true; }
static bool LongOfTimeArmed() { return CVAR_LONG_OF_TIME_VALUE != 0; }
static bool OvertureOfSagesArmed() { return false; } // Beat 3 will wire this.

// Long of Time: upstream Triforce% statics/ocarina.c LongOfTime_Detectable.
// Note order mirrors the melody Nabooru teaches: C-left low (0x02) ... G4 (0x07)
// ... D5 variants. The engine matches when these eight chromatic values close
// out the note history.
static constexpr TrifoolDetectableSong kDetectableSongs[] = {
    {
        TRIFOOL_SONG_LONG_OF_TIME,
        8,
        { 0x7, 0x5, 0x7, 0x9, 0x2, 0x0, 0x4, 0x2 },
        "Long of Time",
        LongOfTimeArmed,
    },
    {
        TRIFOOL_SONG_OVERTURE_OF_SAGES,
        6,
        { 0xE, 0x5, 0x2, 0x9, 0xB, 0xE, 0x00, 0x00 },
        "Overture of Sages",
        OvertureOfSagesArmed,
    },
};

static_assert(sizeof(kDetectableSongs) / sizeof(kDetectableSongs[0]) + 1 == TRIFOOL_SONG_COUNT,
              "song table out of sync with TrifoolSongId");

// -----------------------------------------------------------------------------
// Match state exposed to other TrifoolPercent beats.
// -----------------------------------------------------------------------------
//
// SoH's own OnOcarinaSongAction hook fires only for vanilla songs the engine
// recognised. For the chromatic songs the vanilla path never sees, we poll
// here. A beat consumes the match by calling TrifoolSongEngine_ConsumeMatch().

static TrifoolSongId sLastMatchedSong = TRIFOOL_SONG_NONE;

extern "C" {

uint8_t TrifoolSongEngine_PeekLastMatchedSong() {
    return static_cast<uint8_t>(sLastMatchedSong);
}

uint8_t TrifoolSongEngine_ConsumeMatch() {
    uint8_t id = static_cast<uint8_t>(sLastMatchedSong);
    sLastMatchedSong = TRIFOOL_SONG_NONE;
    return id;
}

void TrifoolSongEngine_ResetHistory() {
    sNoteHistory.fill(kNoNote);
    sNoteCount = 0;
    sLastMatchedSong = TRIFOOL_SONG_NONE;
}

} // extern "C"

// -----------------------------------------------------------------------------
// Hook plumbing.
// -----------------------------------------------------------------------------

static uint8_t EncodeChromatic(uint8_t note, int8_t bend) {
    if (note >= 5) {
        return kNoNote;
    }
    int32_t v = static_cast<int32_t>(kBaseSemitone[note]) + static_cast<int32_t>(bend);
    if (v < 0) v = 0;
    if (v > kMaxSemitone) v = kMaxSemitone;
    return static_cast<uint8_t>(v);
}

static uint8_t HistoryAt(size_t ago) {
    // ago=0 is the most recent note; ago=1 the note before it, etc.
    if (ago >= kHistoryDepth || ago >= sNoteCount) {
        return kNoNote;
    }
    size_t idx = (sNoteCount - 1 - ago) % kHistoryDepth;
    return sNoteHistory[idx];
}

static TrifoolSongId MatchAgainstTable() {
    for (const auto& song : kDetectableSongs) {
        if (!song.isArmed()) continue;
        if (song.numNotes == 0 || song.numNotes > kHistoryDepth) continue;
        if (sNoteCount < song.numNotes) continue;
        bool ok = true;
        // Compare the last numNotes entries of history (oldest first in the song table)
        // against the song's notes[] left-to-right.
        for (size_t i = 0; i < song.numNotes; ++i) {
            uint8_t have = HistoryAt(song.numNotes - 1 - i);
            if (have != song.notes[i]) { ok = false; break; }
        }
        if (ok) {
            return song.id;
        }
    }
    return TRIFOOL_SONG_NONE;
}

static void OnOcarinaNoteReceived(uint8_t note, float modulator, int8_t bend) {
    if (!CVAR_SONG_ENGINE_VALUE) {
        return;
    }
    uint8_t chromatic = EncodeChromatic(note, bend);
    if (chromatic == kNoNote) {
        return;
    }
    sNoteHistory[sNoteCount % kHistoryDepth] = chromatic;
    sNoteCount++;

    TrifoolSongId match = MatchAgainstTable();
    if (match != TRIFOOL_SONG_NONE) {
        sLastMatchedSong = match;
        // Clear the window so a long song's trailing notes don't also match a
        // shorter overlapping song on the next note. Upstream's engine does the
        // equivalent by resetting sOcarinaPlayingPosition at song end.
        sNoteHistory.fill(kNoNote);
        sNoteCount = 0;
    }
}

static void OnOcarinaActionSettled() {
    // Vanilla engine finished (recognised a song, cancelled, or ran out). Reset
    // the chromatic window so our matcher starts clean for the next performance.
    sNoteHistory.fill(kNoNote);
    sNoteCount = 0;
    // Leave sLastMatchedSong alone -- it is cleared by the beat that consumes
    // the match, or by TrifoolSongEngine_ResetHistory().
}

// -----------------------------------------------------------------------------
// Registration.
// -----------------------------------------------------------------------------

static void RegisterTrifoolSongEngine() {
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnOcarinaNote>(OnOcarinaNoteReceived);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnOcarinaSongAction>(OnOcarinaActionSettled);
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnLoadGame>(
        [](int32_t fileNum) { TrifoolSongEngine_ResetHistory(); });
    // Avoid carrying a half-played window across scene boundaries (e.g. song pause,
    // area transition). OnSceneSpawnActors is the per-scene entry hook used elsewhere
    // in this module (see Beat1KeyOfCourage.cpp).
    GameInteractor::Instance->RegisterGameHook<GameInteractor::OnSceneSpawnActors>(
        []() { TrifoolSongEngine_ResetHistory(); });
}

static RegisterShipInitFunc songEngineInitFunc(RegisterTrifoolSongEngine,
                                               { CVAR_TRIFOOL_SONG_ENGINE,
                                                 CVAR_TRIFOOL_LONG_OF_TIME });

// -----------------------------------------------------------------------------
// Menu widgets.
// -----------------------------------------------------------------------------

namespace SohGui {
extern std::shared_ptr<SohMenu> mSohMenu;
}

#include "menu/TrifoolMenu.h"

void TrifoolMenu::RegisterSongEngineWidget() {
    WidgetPath path = { "TrifoolPercent", "Beats & Progression", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Chromatic song-recognition engine", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_SONG_ENGINE)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Runs TrifoolPercent's merged chromatic song matcher alongside vanilla "
            "song recognition. The vanilla matcher still handles the six Sheik songs "
            "and the three warp cues; this one adds songs that use half-steps or more "
            "than six notes, starting with Long of Time. Toggle individual songs below."));
}

void TrifoolMenu::RegisterLongOfTimeCutsceneWidget() {
    WidgetPath path = { "TrifoolPercent", "Cutscenes", SECTION_COLUMN_1 };
    SohGui::mSohMenu->AddWidget(path, "Long of Time (eight-note Song of Time)", WIDGET_CVAR_CHECKBOX)
        .CVar(CVAR_TRIFOOL_LONG_OF_TIME)
        .Options(UIWidgets::CheckboxOptions().DefaultValue(false).Tooltip(
            "Arms the Long of Time (0x7 0x5 0x7 0x9 0x2 0x0 0x4 0x2) chromatic pattern "
            "in the Trifool matcher. Playing it leaves sLastMatchedSong = LONG_OF_TIME "
            "for the next Trifool beat to consume (time travel without Master Sword "
            "lands in a later beat, matching upstream Nabooru's teach). Requires "
            "the engine toggle above."));
}
