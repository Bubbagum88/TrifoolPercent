include ../../../toolchain/make/main.mk

# Scene 68 == kenjyanoma_scene == Chamber of the Sages.
# This directory patches the vanilla scene in-place so Link lands in
# the beta-populated version through ENTR_CHAMBER_OF_THE_SAGES_0.
SCENEDIR=../../../build-romhack/scene/68
TARGETS=room_0.zmap scene.zscene
SCENEPATCHDEPS=
ROOMPATCHDEPS=

# Build is intentionally disabled while scene_patch.txt / room0_patch.txt
# still have placeholder offsets. Flip MAKESUBDIRS in scenes/Makefile
# (and this `default:` back to the shared sceneroom.mk target) once the
# real offsets are filled in from the vanilla scene binary.
.PHONY: default clean
default:
	@echo "scenes/beta_rooms/SacredRealm: patches are stubs, nothing to build yet"

clean:
	rm -f room_0.zmap scene.zscene

# When ready, drop the stub targets above and uncomment:
# include ../../../toolchain/make/sceneroom.mk
