Unicode true

Name "Wolfenstein 2026"
OutFile "${OUTPUT_FILE}"
RequestExecutionLevel user
SilentInstall silent
AutoCloseWindow true
CRCCheck on
SetCompressor /SOLID lzma
Icon "${ICON_FILE}"

VIProductVersion "1.0.0.0"
VIAddVersionKey "ProductName" "Wolfenstein 2026"
VIAddVersionKey "FileDescription" "Wolfenstein 2026"
VIAddVersionKey "FileVersion" "1.0.0"

Section
    SetOutPath "$LOCALAPPDATA\Wolfenstein 2026"
    File /r "${SOURCE_DIR}/*"
    ExecWait '"$OUTDIR\wolfenstein2026.exe"'
SectionEnd
