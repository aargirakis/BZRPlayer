GetWB0bject equ -30 
PutWBObject equ -36 
GetIcon equ -42
Putlcon equ -48
FreeFreeList equ -54
FreeWBObject equ -60
AllocWBObject equ -66
AddFreeList equ -72
GetDiskObject equ -78			* struct DiskObject *GetDiskObject(char *);  D0 = (A0);
PutDiskObject equ -84
FreeDiskObject equ -90			* void FreeDiskObject(struct DiskObject *); (A0)
FindToolType equ -96			* char *FindToolType(char **list, char *key) where "list" e.g. ["FILETYPE=text"]; D0 = (A0, A1)
MatchToolValue equ -102
BumpRevision equ -108

ICONENTRIES equ 18				* includes the 4 standard Library functions (open, close,..)