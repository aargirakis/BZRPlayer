!include StrFunc.nsh
!include WordFunc.nsh

${StrCase}

Function RegisterAudioExtension
  ${WordReplace} "$R0" "." "" "+1" $0
  ${StrCase} $0 $0 "U"
  WriteRegStr HKCR "${NAME_SHORT}$R0" "" "$0 File (${NAME_SHORT})"
  WriteRegStr HKCR "${NAME_SHORT}$R0\shell" "" "Open"
  WriteRegStr HKCR "${NAME_SHORT}$R0\shell\Open" "" "Play"
  WriteRegStr HKCR "${NAME_SHORT}$R0\shell\Open" "MultiSelectModel" "Player"
  WriteRegStr HKCR "${NAME_SHORT}$R0\shell\Open\command" "" '"$INSTDIR\${EXE_FILENAME}" "%1"'
  WriteRegStr HKCR "${NAME_SHORT}$R0\DefaultIcon" "" '"$INSTDIR\${EXE_FILENAME}",0'
  WriteRegStr HKCR "Applications\${EXE_FILENAME}\SupportedTypes" "$R0" ""

  ${If} ${AtLeastWinVista}
    WriteRegStr HKLM "Software\Clients\Media\${NAME_SHORT}\Capabilities\FileAssociations" "$R0" "${NAME_SHORT}$R0"
  ${EndIf}
FunctionEnd

Function AssociateExtension
  ReadRegStr $1 HKCR "$R0" ""
  StrCmp $1 "" NoBackup
  StrCmp $1 "${NAME_SHORT}$R0" NoBackup
  WriteRegStr HKCR "$R0" "${NAME_SHORT}.backup" $1

  NoBackup:
  WriteRegStr HKCR "$R0" "" "${NAME_SHORT}$R0"

  ${If} ${AtLeastWinVista}
    DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\$R0\UserChoice"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\$R0\OpenWithList" "a" "${EXE_FILENAME}"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\$R0\OpenWithList" "MRUList" "a"
    WriteRegBin HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\$R0\OpenWithProgids" "${NAME_SHORT}$R0" ""
  ${EndIf}
FunctionEnd

!macro unAssociateExtensionWinVistaPlusMacro $R0
  StrCpy $letter ""
  StrCpy $mruListContent ""
  ${registry::Open} "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\$R0\OpenWithList" "K=0" $R1
  StrCmp $R1 0 close loop

  loop:
  ${registry::Find} $R1 $R2 $R3 $R4 $R5
  StrCmp $R2 "" close 0
  StrCmp $R3 "MRUList" 0 noMRUList
  StrCpy $mruListContent $R4
  goto loop

  noMRUList:
  StrCmp $R4 "${EXE_FILENAME}" 0 loop
  StrCpy $letter $R3
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\$R0\OpenWithList" "$R3"
  goto loop

  close:
  ${registry::Close} "$R1"
  ${registry::Read} "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\$R0\OpenWithProgids" "${NAME_SHORT}$R0" $R1 $R2
  StrCmp $R2 "" notFound 0
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\$R0\OpenWithProgids" "${NAME_SHORT}$R0"

  notFound:
  ReadRegStr $R1 HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\$R0\UserChoice" "ProgId"

  StrCpy $R2 "$mruListContent" 1
  ${If} $R1 == ${NAME_SHORT}$R0
  ${OrIf} "$R2" != ""
  ${AndIf} $R2 == "$letter"
    DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\$R0\UserChoice"
  ${EndIf}
!macroend

Function unAssociateExtension
  ReadRegStr $R1 HKCR "$R0" ""
  StrCmp $R1 "${NAME_SHORT}$R0" 0 noOwn
  ReadRegStr $R1 HKCR "$R0" "${NAME_SHORT}.backup"
  StrCmp $R1 "" 0 restore
  DeleteRegValue HKCR "$R0" ""

  restore:
  WriteRegStr HKCR "$R0" "" $R1
  DeleteRegValue HKCR "$R0" "${NAME_SHORT}.backup"

  noOwn:
  ${If} ${AtLeastWinVista}
    !insertmacro unAssociateExtensionWinVistaPlusMacro "$R0"
  ${EndIf}
FunctionEnd

#TODO needed?
#Function un.RegisterExtension
#  ReadRegStr $R1 HKCR "$R0" ""
#  StrCmp $R1 "${NAME_SHORT}$R0" 0 noOwn
#  ReadRegStr $R1 HKCR "$R0" "${NAME_SHORT}.backup"
#  StrCmp $R1 "" 0 restore
#  DeleteRegKey HKCR "$R0"
#  goto noOwn
#
#  restore:
#  WriteRegStr HKCR "$R0" "" $R1
#  DeleteRegValue HKCR "$R0" "${NAME_SHORT}.backup"
#
#  noOwn:
#  DeleteRegKey HKCR "${NAME_SHORT}$R0"
#  DeleteRegKey HKLM "Software\Clients\Media\${NAME_SHORT}\Capabilities\FileAssociations\${NAME_SHORT}$R0"
#FunctionEnd

# unassociate & unregister any extension (even those no longer supported); this will also remove their context menus entries
Function un.RegisterExtensionsAndCleanupGarbage
  StrCpy $0 0

  loop1:
  EnumRegKey $R0 HKCR "" $0
  StrCmp $R0 "" done1
  IntOp $0 $0 + 1
  StrCpy $R1 $R0 1
  StrCmp $R1 "." 0 loop1
  ReadRegStr $R1 HKCR "$R0" ""
  StrCmp $R1 "${NAME_SHORT}$R0" 0 noOwn
  ReadRegStr $R1 HKCR "$R0" "${NAME_SHORT}.backup"
  StrCmp $R1 "" 0 restore
  DeleteRegValue HKCR "$R0" ""

  restore:
  WriteRegStr HKCR "$R0" "" $R1
  DeleteRegValue HKCR "$R0" "${NAME_SHORT}.backup"

  noOwn:
  DeleteRegKey HKCR "${NAME_SHORT}$R0"
  DeleteRegKey HKLM "Software\Clients\Media\${NAME_SHORT}\Capabilities\FileAssociations\${NAME_SHORT}$R0"
  goto loop1

  done1:
  ${If} ${AtLeastWinVista}
    StrCpy $0 0

    loop2:
    EnumRegKey $R0 HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts" $0
    StrCmp $R0 "" done2
    IntOp $0 $0 + 1
    StrCpy $R1 $R0 1
    StrCmp $R1 "." 0 loop2
    !insertmacro unAssociateExtensionWinVistaPlusMacro "$R0"
    goto loop2

    done2:
  ${EndIf}
FunctionEnd

!macro RegisterAudioExtensionMacro EXT
  Push $R0
  StrCpy $R0 "${EXT}"
  Call RegisterAudioExtension
  Pop $R0
!macroend

!macro AssociateExtensionMacro EXT
  Push $R0
  StrCpy $R0 "${EXT}"
  Call AssociateExtension
  Pop $R0
!macroend

!macro UnAssociateExtensionMacro EXT
  Push $R0
  StrCpy $R0 "${EXT}"
  Call unAssociateExtension
  Pop $R0
!macroend

#!macro UnRegisterExtensionMacro EXT
#  Push $R0
#  StrCpy $R0 "${EXT}"
#  Call un.RegisterExtension
#  Pop $R0
#!macroend

!macro UnRegisterExtensionsAndCleanupGarbageMacro
  Call un.RegisterExtensionsAndCleanupGarbage
!macroend

!macro AddContextMenuEntryMacro ENTRY
  WriteRegStr HKCR ${ENTRY}\shell\PlayWith${NAME_SHORT} "" "Play with ${NAME}"
  WriteRegStr HKCR ${ENTRY}\shell\PlayWith${NAME_SHORT} "Icon" '"$INSTDIR\${EXE_FILENAME}",0'
  WriteRegStr HKCR ${ENTRY}\shell\PlayWith${NAME_SHORT} "MultiSelectModel" "Player"
  WriteRegStr HKCR ${ENTRY}\shell\PlayWith${NAME_SHORT}\command "" '"$INSTDIR\${EXE_FILENAME}" "%1"'

  #TODO need a specific bzr2 flag to trigger for this (eg --enqueue)
  #WriteRegStr HKCR ${ENTRY}\shell\AddToPlaylist${NAME_SHORT} "" "Add to ${NAME}'s Playlist"
  #WriteRegStr HKCR ${ENTRY}\shell\AddToPlaylist${NAME_SHORT} "Icon" '"$INSTDIR\${EXE_FILENAME}",0'
  #WriteRegStr HKCR ${ENTRY}\shell\AddToPlaylist${NAME_SHORT} "MultiSelectModel" "Player"
  #WriteRegStr HKCR ${ENTRY}\shell\AddToPlaylist${NAME_SHORT}\command "" '"$INSTDIR\${EXE_FILENAME}" "%1"'
!macroend

!macro AddContextMenuExtMacro EXT
  !insertmacro AddContextMenuEntryMacro "${NAME_SHORT}${EXT}"
!macroend

!macro AddContextMenuDirMacro
  !insertmacro AddContextMenuEntryMacro "Directory"
!macroend

!macro DeleteContextMenuEntryMacro ENTRY
  DeleteRegKey HKCR ${ENTRY}\shell\PlayWith${NAME_SHORT}
  DeleteRegKey HKCR ${ENTRY}\shell\AddToPlaylist${NAME_SHORT}
!macroend

!macro DeleteContextMenuExtMacro EXT
  !insertmacro DeleteContextMenuEntryMacro "${NAME_SHORT}${EXT}"
!macroend

!macro DeleteContextMenuDirMacro
  !insertmacro DeleteContextMenuEntryMacro "Directory"
!macroend
