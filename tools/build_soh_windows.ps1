# Windows counterpart to tools/build_soh.sh. Copies the TrifoolPercent module into the
# Shipwright checkout, applies engine patches, and builds Ship of Harkinian with MSVC.
# Run tools/setup.sh (from Git Bash or WSL) first to clone third_party/*.
#
# Output: third_party/Shipwright/build/soh/Release/soh.exe
$ErrorActionPreference = "Stop"

$Root = Resolve-Path (Join-Path $PSScriptRoot "..")
$Soh  = Join-Path $Root "third_party/Shipwright"

# Reset the Shipwright checkout to the pinned commit before each build so engine patches
# re-apply cleanly and previous module edits don't leak. setup.sh handles the clone +
# checkout; here we just undo any previous build edits in tracked files.
git -C "$Soh" reset --hard HEAD | Out-Null

$ModDst = Join-Path $Soh "soh/soh/Enhancements/TrifoolPercent"
if (Test-Path $ModDst) { Remove-Item -Recurse -Force $ModDst }
Copy-Item -Recurse (Join-Path $Root "soh/Enhancements/TrifoolPercent") $ModDst

# Apply engine patches (if any). A rejected hunk means SHIPWRIGHT_COMMIT in tools/setup.sh
# has drifted past the patch's anchor -- re-anchor the patch by hand and commit the fix.
$PatchDir = Join-Path $Root "patches/shipwright"
if (Test-Path $PatchDir) {
    Get-ChildItem -Path $PatchDir -Filter *.patch | ForEach-Object {
        Write-Host "Applying $($_.Name) to Shipwright..."
        git -C "$Soh" apply --whitespace=nowarn $_.FullName
        if ($LASTEXITCODE -ne 0) {
            Write-Error "FAILED to apply $($_.Name). Does SHIPWRIGHT_COMMIT in tools/setup.sh need to be bumped, or the patch re-anchored?"
            exit 1
        }
    }
}

cmake -S "$Soh" -B "$Soh/build" -G "Visual Studio 17 2022" -A x64
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
cmake --build "$Soh/build" --config Release -j
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Built $Soh/build/soh/Release/soh.exe. Put build/TrifoolPercent.o2r in its mods folder."
