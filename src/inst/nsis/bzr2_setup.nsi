#TODO tighten vars scope
#TODO add UAC Publisher (verisign)
#TODO add comments
#TODO try to do not hardcode addSectionContent macro inside addFiletypeAssociationsComponentContent (but add only to those really checked
#TODO add "isPortable" checkbox/flag that when checked don't touch the registry but add ".portable" file to ${DIR_SETTINGS_USER} with version string
#if the invoked installer.exe dir have ".portable" & bzrplayer.exe files then pre-set its checkbox by default (and put "wasPortable" flag to true)
#in "isPortable" mode $INSTDIR will pre-set by default to the same of the installer.exe if "wasPortable" is true
#add a command line param for forcing "isPortable" true

!system "./filetype_associations_component_util_generator.sh" = 0

SetCompressor /FINAL lzma
CRCCheck force

!define NAME_SHORT "BZR2"
!define NAME_UNVERSIONED "BZR Player"
!define NAME "${NAME_UNVERSIONED} 2"
!define NAME_VERSIONED "${NAME_UNVERSIONED} ${VERSION}"
!define CAPTION "${NAME_VERSIONED} Setup"
!define SETUP_FILENAME "bzr-player-${VERSION}-win64.exe"
!define LICENSE_FILE "../../../LICENSE"
!define ICON_FILE "../../app/resources/icon.ico"
!define EXE_FILENAME "BZRPlayer.exe"
!define URL_MAIN "https://bzrplayer.blazer.nu"
!define DESCRIPTION "Audio player supporting a wide array of multi-platform exotic file formats"
!define DIR_SETTINGS_USER "$INSTDIR\user"

var i
var letter
var mruListContent

!include MUI2.nsh
!include nsArray.nsh
!include nsProcess.nsh
!include Registry.nsh
!include WinVer.nsh
!include WordFunc.nsh
!include include\FiletypeAssociationsComponentUtil.nsh
!include include\FiletypeAssociationsUtil.nsh

!macro DetectRunningBzr2
  loop:
  ${nsProcess::FindProcess} "${EXE_FILENAME}" $0
  Pop $0

  ${If} $0 == 603
    goto notRunning
  ${EndIf}

  ${If} $0 == 0
    MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "${NAME_UNVERSIONED} is running. Please close it first" /SD IDCANCEL IDRETRY loop
    abort
  ${EndIf}

  MessageBox MB_OK|MB_ICONEXCLAMATION "Unable to detect if ${NAME_UNVERSIONED} is running. Make sure it is closed before proceeding" /SD IDOK

  notRunning:
!macroend

#TODO avoid associating multiple times the same extension but from different filetypes
!macro addSectionContent ext
  !insertmacro AssociateExtensionMacro "${ext}"
!macroend

!macro createDirSettingsUserWithWritePermissions
  CreateDirectory "${DIR_SETTINGS_USER}"

  setDirPermissions:
  AccessControl::GrantOnFile "${DIR_SETTINGS_USER}" "(BU)" "GenericRead + GenericWrite"
  Pop $0
  ${If} $0 == error
    Pop $0
    DetailPrint "AccessControl error: $0"
    MessageBox MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION "Unable to set permissions for directory$\r$\n${DIR_SETTINGS_USER}:\
    $\r$\n'$0'$\r$\n" /SD IDABORT IDRETRY setDirPermissions IDIGNORE afterSetDirPermissions
    quit
  ${EndIf}

  afterSetDirPermissions:
!macroend

RequestExecutionLevel admin
OutFile "${SETUP_FILENAME}"
Name "${NAME_VERSIONED}"
Caption "${CAPTION}"
InstallDir "$PROGRAMFILES64\${NAME}"
InstallDirRegKey HKLM "Software\${NAME}" "InstallDir"
ShowInstDetails show
ShowUninstDetails show

!define MUI_BGCOLOR "SYSCLR:Window"
!define MUI_TEXTCOLOR "SYSCLR:WindowText"
!define MUI_ICON "${ICON_FILE}"
!define MUI_UNICON "${ICON_FILE}"
!define MUI_ABORTWARNING
!define MUI_ABORTWARNING_CANCEL_DEFAULT
!define MUI_UNABORTWARNING
!define MUI_UNABORTWARNING_CANCEL_DEFAULT

#TODO
#!define MUI_WELCOMEFINISHPAGE_BITMAP "banner.bmp"

#TODO
#!define MUI_UNWELCOMEFINISHPAGE_BITMAP "banner.bmp"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${LICENSE_FILE}"
!define MUI_PAGE_CUSTOMFUNCTION_PRE componentsPre
!define MUI_COMPONENTSPAGE_NODESC
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_TEXT "Run ${NAME}"
!define MUI_FINISHPAGE_RUN_CHECKED
!define MUI_FINISHPAGE_LINK "${URL_MAIN}"
!define MUI_FINISHPAGE_LINK_LOCATION "${URL_MAIN}"
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE saveRunButtonCheckState
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_INSTFILES
!define MUI_UNFINISHPAGE_NOAUTOCLOSE
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section "!${NAME} (required)"
  SectionIn RO

  !insertmacro DetectRunningBzr2

  Delete $INSTDIR\*.*
  Push "$INSTDIR"
  Push "user"
  Call RmDirsButOne

  SetOutPath "$INSTDIR"
  File /r "bin/*"

  !insertmacro createDirSettingsUserWithWritePermissions

  WriteUninstaller "$INSTDIR\uninstall.exe"

  WriteRegStr HKLM "Software\${NAME}" "ApplicationName" "${NAME}"
  WriteRegStr HKLM "Software\${NAME}" "ApplicationDescription" "${DESCRIPTION}"
  WriteRegStr HKLM "Software\${NAME}" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "DisplayIcon" "$INSTDIR\${EXE_FILENAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "DisplayName" "${NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "UninstallString" "$INSTDIR\uninstall.exe"

  # Add to "recommended programs" for the following extensions
  WriteRegStr HKCR "Applications\${EXE_FILENAME}" "" ""
  WriteRegStr HKCR "Applications\${EXE_FILENAME}" "FriendlyAppName" "${NAME}"
  WriteRegStr HKCR "Applications\${EXE_FILENAME}\shell\Open" "" "Play with ${NAME}"
  WriteRegStr HKCR "Applications\${EXE_FILENAME}\shell\Open\command" "" '"$INSTDIR\${EXE_FILENAME}" "%1"'

  #TODO avoid registering multiple times the same extension but from different filetypes
  ${ForEachIn} extensions $R0 $R1
    !insertmacro RegisterAudioExtensionMacro "$R1"
  ${Next}

  # Add to "App Paths" to run bzr2 from ShellExecute/ShellExecuteEx/run dialog without giving a full path
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\${EXE_FILENAME}" "" "$INSTDIR\${EXE_FILENAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\${EXE_FILENAME}" "Path" "$INSTDIR"

  # Windows default programs Registration
  ${If} ${AtLeastWinVista}
    WriteRegStr HKLM "Software\RegisteredApplications" "${NAME_SHORT}" "Software\Clients\Media\${NAME_SHORT}\Capabilities"
    WriteRegStr HKLM "Software\Clients\Media\${NAME_SHORT}\Capabilities" "ApplicationName" "${NAME}"
    WriteRegStr HKLM "Software\Clients\Media\${NAME_SHORT}\Capabilities" "ApplicationDescription" "${NAME_SHORT} - ${DESCRIPTION}"
    WriteRegStr HKLM "Software\Clients\Media\${NAME_SHORT}" "" "${NAME}"
  ${EndIf}
SectionEnd

Section "Start Menu Shortcut"
  CreateDirectory "$SMPROGRAMS\${NAME}"
  CreateShortCut "$SMPROGRAMS\${NAME}\${NAME}.lnk" "$INSTDIR\${EXE_FILENAME}" "" "$INSTDIR\${EXE_FILENAME}" "" "" "" "${DESCRIPTION}"
  CreateShortCut "$SMPROGRAMS\${NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe"
SectionEnd

Section "Desktop Shortcut"
  CreateShortCut "$DESKTOP\${NAME}.lnk" "$INSTDIR\${EXE_FILENAME}"
SectionEnd

SectionGroup "File type associations"
  !insertmacro addFiletypeAssociationsComponentContent
SectionGroupEnd

Section ""
  # unAssociate unchecked extensions
  ${ForEachIn} extensions $R0 $R1
    !insertmacro SectionFlagIsSet $R0 ${SF_SELECTED} +1 extensionUnchecked
  ${Continue}

  extensionUnchecked:
  !insertmacro UnAssociateExtensionMacro "$R1"
  ${Next}

#TODO association removal of not still supported extensions:
#so if a next bzr2 version drop support for an extension, and this was previosuly associated to bzr2, the installer will remove the association:
#remove any BZR.ext from registry (loop on them) that is not included in "extensions" array (they come from old versions)
#TODO (maybe?) unRegister stuff from registry (for all extensions) if its checkbox has not been checked

SectionEnd

Section "Context Menus" secCtxMenus
  ${ForEachIn} extensions $R0 $R1
    ${If} ${SectionIsSelected} $R0
      !insertmacro AddContextMenuExtMacro "$R1"
    ${EndIf}
  ${Next}
  !insertmacro AddContextMenuDirMacro
SectionEnd

Section ""
  ${If} ${SectionIsSelected} ${secCtxMenus}
  ${Else}
    ${ForEachIn} extensions $R0 $R1
      !insertmacro DeleteContextMenuExtMacro "$R1"
    ${Next}
  ${EndIf}
SectionEnd

Section /o "Delete Preferences"
  RMDir /r "${DIR_SETTINGS_USER}"
  !insertmacro createDirSettingsUserWithWritePermissions
SectionEnd

Function componentsPre
  nsArray::Clear extensions
  StrCpy $i 0
  ${ForEach} $i ${firstFiletypeAssociationsSection} ${lastFiletypeAssociationsSection} + 1
    !insertmacro SectionFlagIsSet $i ${SF_SECGRP} +1 isNotSectionGroup
    ${Continue}

    isNotSectionGroup:
    !insertmacro SectionFlagIsSet $i ${SF_SECGRPEND} +1 isNotSectionGroupEnd
    ${Continue}

    isNotSectionGroupEnd:
    SectionGetText "$i" $R0
    nsArray::Set extensions /key=$i $R0
  ${Next}

  # pre-uncheck extensions registered but not associated (if bzr2 it is already installed)
  ClearErrors
  EnumRegKey $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" 0
  IfErrors done 0

  ${ForEachIn} extensions $R0 $R1
    ${If} ${AtLeastWinVista}
      StrCpy $letter ""
      StrCpy $mruListContent ""
      ${registry::Open} "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\$R1\OpenWithList" "K=0" $R2
      StrCmp $R4 0 close loop

      loop:
      ${registry::Find} $R2 $R3 $R4 $R5 $R6
      StrCmp $R3 "" close 0
      StrCmp $R4 "MRUList" 0 noMRUList
      StrCpy $mruListContent $R5
      goto loop

      noMRUList:
      StrCmp $R5 "${EXE_FILENAME}" 0 loop
      StrCpy $letter $R4
      goto loop

      close:
      ${registry::Close} $R2

      StrCpy $R2 "$mruListContent" 1

      ${If} "$R2" != ""
      ${AndIf} $R2 != "$letter"
        goto deselect
      ${Else}
        goto next
      ${EndIf}
    ${Else}
      ReadRegStr $0 HKCR "${NAME_SHORT}$R1" ""
      StrCmp $0 "" next
      ReadRegStr $0 HKCR "$R1" ""
      StrCmp $0 "${NAME_SHORT}$R1" next
    ${EndIf}

   deselect:
   !insertmacro ClearSectionFlag $R0 ${SF_SELECTED}

   next:
  ${Next}

  done:
FunctionEnd

Function .onSelChange
  var /global triggeredSectionNum
  StrCpy $triggeredSectionNum $0

  ${If} $triggeredSectionNum < "${firstFiletypeAssociationsSection}"
  ${OrIf} $triggeredSectionNum > "${lastFiletypeAssociationsSection}"
    goto onSelChangeEnd
  ${EndIf}

  var /global triggeredSectionText
  SectionGetText $triggeredSectionNum $triggeredSectionText
  nsArray::Clear extensionsToToggle

  !insertmacro SectionFlagIsSet $triggeredSectionNum ${SF_SECGRP} isTriggeredSectionGroup isNotTriggeredSectionGroup
  isNotTriggeredSectionGroup:
  nsArray::Set extensionsToToggle /key=$triggeredSectionNum $triggeredSectionText
  goto propagateToggles

  isTriggeredSectionGroup:
  StrCpy $i $triggeredSectionNum
  IntOp $i $i + 1

  ${ForEach} $i $i ${lastFiletypeAssociationsSection} + 1
    !insertmacro SectionFlagIsSet $i ${SF_SECGRPEND} +1 isNotSectionGroupEnd1
    ${Break}

    isNotSectionGroupEnd1:
      SectionGetText "$i" $R0
      nsArray::Set extensionsToToggle /key=$i $R0
  ${Next}

  propagateToggles:
  StrCpy $i 0

  ${ForEach} $i ${firstFiletypeAssociationsSection} ${lastFiletypeAssociationsSection} + 1
    !insertmacro SectionFlagIsSet $i ${SF_SECGRP} +1 isNotSectionGroup
    ${Continue}

    isNotSectionGroup:
    !insertmacro SectionFlagIsSet $i ${SF_SECGRPEND} +1 isNotSectionGroupEnd2
    ${Continue}

    isNotSectionGroupEnd2:
    ${ForEachIn} extensionsToToggle $R0 $R1
      SectionGetText "$i" $R2

      ${If} "$R2" == "$R1"
      ${AndIf} "$i" != "$R0"
        !insertmacro SectionFlagIsSet $R0 ${SF_SELECTED} +1 extensionsToToggleIsNotSet
        !insertmacro SelectSection $i
        goto extensionTogglingEnd

        extensionsToToggleIsNotSet:
        !insertmacro UnSelectSection $i

        extensionTogglingEnd:
      ${EndIf}
    ${Next}
  ${Next}

  onSelChangeEnd:
FunctionEnd

Function RmDirsButOne
  Exch $R0
  Exch
  Exch $R1
  Push $R2
  Push $R3

  ClearErrors
  FindFirst $R3 $R2 "$R1\*.*"
  IfErrors Exit

  Top:
  StrCmp $R2 "." Next
  StrCmp $R2 ".." Next
  StrCmp $R2 $R0 Next
  IfFileExists "$R1\$R2\*.*" 0 Next
  RmDir /r "$R1\$R2"

  Next:
  ClearErrors
  FindNext $R3 $R2
  IfErrors Exit
  Goto Top

  Exit:
  FindClose $R3

  Pop $R3
  Pop $R2
  Pop $R1
  Pop $R0
FunctionEnd

Function saveRunButtonCheckState
  SendMessage $mui.FinishPage.Run ${BM_GETCHECK} 0 0 $mui.FinishPage.ReturnValue

  ${if} $mui.FinishPage.ReturnValue = ${BST_CHECKED}
    var /global isRunButtonChecked
    StrCpy $isRunButtonChecked 1
  ${EndIf}
FunctionEnd

Function .onGUIEnd
  ${if} $isRunButtonChecked == 1
    Exec '"$WINDIR\explorer.exe" "$INSTDIR\${EXE_FILENAME}"'

    # ensures bzr2 opening on foreground
    loop:
    ${nsProcess::FindProcess} "${EXE_FILENAME}" $0
    Pop $0
    IntCmp $0 603 loop
  ${EndIf}

  ${nsProcess::Unload}
  ${registry::Unload}
  delete "$PLUGINSDIR"
FunctionEnd

Section "un.${NAME}"
  SectionIn RO
  !insertmacro DetectRunningBzr2

  Delete "$INSTDIR\*.*"
  RMDir /r "$INSTDIR\data"
  RMDir /r "$INSTDIR\platforms"
  RMDir /r "$INSTDIR\tls"
  RMDir "$INSTDIR"
  RMDir /r "$SMPROGRAMS\${NAME}"
  Delete "$DESKTOP\${NAME}.lnk"

  !insertmacro UnRegisterExtensionsAndCleanupGarbageMacro
  !insertmacro DeleteContextMenuDirMacro

  DeleteRegKey HKCR "Applications\${EXE_FILENAME}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\${EXE_FILENAME}"
  DeleteRegKey HKLM "Software\Clients\Media\${NAME_SHORT}"
  DeleteRegValue HKLM "Software\RegisteredApplications" "${NAME_SHORT}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}"
  DeleteRegKey HKLM "Software\${NAME}"
SectionEnd

Section /o un.Preferences
  RMDir /r "${DIR_SETTINGS_USER}"
  RMDir "$INSTDIR"
SectionEnd

Function un.onGUIEnd
  ${nsProcess::Unload}
  ${registry::Unload}
  delete "$PLUGINSDIR"
FunctionEnd
