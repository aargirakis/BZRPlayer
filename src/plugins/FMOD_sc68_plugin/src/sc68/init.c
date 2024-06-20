/*
 *	TN_SC68.tnplug
 *	(c) Fredrik Wikstrom
 */

#define LIBNAME "TN_SC68.tnplug"

#include "TN_SC68.tnplug_rev.h"
#include "tn_sc68.h"

const char
#ifdef __GNUC__
__attribute__((used))
#endif
verstag[] = VERSTAG;

int _start () {
	return 100;
}

struct ExecIFace *IExec;
struct DOSIFace *IDOS;
struct UtilityIFace *IUtility;
struct Interface *INewlib;
struct Library *DOSBase;
struct Library *UtilityBase;
struct Library *NewlibBase;

static uint32 libObtain (struct Interface *Self);
static uint32 libRelease (struct Interface *Self);
static struct TNPlugBase *libOpen (struct Interface *Self, uint32 version);
static BPTR libClose (struct Interface *Self);
static BPTR libExpunge (struct Interface *Self);

static CONST_APTR lib_manager_vectors[] = {
	(APTR)libObtain,
	(APTR)libRelease,
	NULL,
	NULL,
	(APTR)libOpen,
	(APTR)libClose,
	(APTR)libExpunge,
	NULL,
	(APTR)-1,
};

static const struct TagItem lib_managerTags[] = {
	{ MIT_Name,			(uint32)"__library"			},
	{ MIT_VectorTable,	(uint32)lib_manager_vectors	},
	{ MIT_Version,		1							},
	{ TAG_END }
};

static CONST_APTR lib_main_vectors[] = {
	(APTR)libObtain,
	(APTR)libRelease,
	NULL,
	NULL,
	(APTR)TNPlug_AnnouncePlayer,
	(APTR)TNPlug_InitPlayer,
	(APTR)TNPlug_OpenPlayer,
	(APTR)TNPlug_ClosePlayer,
	(APTR)TNPlug_DecodeFramePlayer,
	(APTR)TNPlug_ExitPlayer,
	(APTR)TNPlug_TestPlayer,
	(APTR)TNPlug_SeekPlayer,
	(APTR)TNPlug_DoNotify,
	(APTR)-1
};

static const struct TagItem lib_mainTags[] = {
	{ MIT_Name,			(uint32)"main"				},
	{ MIT_VectorTable,	(uint32)lib_main_vectors	},
	{ MIT_Version,		1							},
	{ TAG_END }
};

static CONST_APTR libInterfaces[] = {
	lib_managerTags,
	lib_mainTags,
	NULL
};

static struct TNPlugBase *libInit (struct TNPlugBase *tnpb, BPTR seglist,
	struct ExecIFace *ISys);

static const struct TagItem libCreateTags[] = {
	{ CLT_DataSize,		(uint32)sizeof(struct TNPlugBase)	},
	{ CLT_InitFunc,		(uint32)libInit						},
	{ CLT_Interfaces,	(uint32)libInterfaces				},
	{ TAG_END }
};

const struct Resident lib_res
#ifdef __GNUC__
__attribute__((used))
#endif
=
{
    RTC_MATCHWORD,
    (struct Resident *)&lib_res,
    (APTR)(&lib_res + 1),
    RTF_NATIVE|RTF_AUTOINIT, /* Add RTF_COLDSTART if you want to be resident */
    VERSION,
    NT_LIBRARY, /* Make this NT_DEVICE if needed */
    0, /* PRI, usually not needed unless you're resident */
    LIBNAME,
    VSTRING,
    (APTR)libCreateTags
};

static int OpenLibs (struct TNPlugBase *tnpb);
static void CloseLibs (struct TNPlugBase *tnpb);

static struct TNPlugBase *libInit (struct TNPlugBase *tnpb, BPTR seglist,
	struct ExecIFace *ISys)
{
	tnpb->tnpb_LibNode.lib_Node.ln_Type = NT_LIBRARY;
	tnpb->tnpb_LibNode.lib_Node.ln_Pri  = 0;
	tnpb->tnpb_LibNode.lib_Node.ln_Name = LIBNAME;
	tnpb->tnpb_LibNode.lib_Flags        = LIBF_SUMUSED|LIBF_CHANGED;
	tnpb->tnpb_LibNode.lib_Version      = VERSION;
	tnpb->tnpb_LibNode.lib_Revision     = REVISION;
	tnpb->tnpb_LibNode.lib_IdString     = VSTRING;
	tnpb->tnpb_SegList = seglist;
	IExec = ISys;

	if (OpenLibs(tnpb)) {
		return tnpb;
	}
	CloseLibs(tnpb);

	return NULL;
}

static uint32 libObtain (struct Interface *Self) {
	return ++Self->Data.RefCount;
}

static uint32 libRelease (struct Interface *Self) {
	return --Self->Data.RefCount;
}

static struct TNPlugBase *libOpen (struct Interface *Self, uint32 version) {
	struct TNPlugBase *tnpb = (struct TNPlugBase *)Self->Data.LibBase;

	tnpb->tnpb_LibNode.lib_OpenCnt++;
	tnpb->tnpb_LibNode.lib_Flags &= ~LIBF_DELEXP;

	return tnpb;
}

static BPTR libExpunge (struct Interface *Self);

static BPTR libClose (struct Interface *Self) {
	struct TNPlugBase *tnpb = (struct TNPlugBase *)Self->Data.LibBase;

	tnpb->tnpb_LibNode.lib_OpenCnt--;

	if (tnpb->tnpb_LibNode.lib_OpenCnt) {
		return 0;
	}

	if (tnpb->tnpb_LibNode.lib_Flags & LIBF_DELEXP) {
		return libExpunge(Self);
	} else {
		return 0;
	}
}

static BPTR libExpunge (struct Interface *Self) {
	struct TNPlugBase *tnpb = (struct TNPlugBase *)Self->Data.LibBase;
	BPTR result = 0;

	if (tnpb->tnpb_LibNode.lib_OpenCnt == 0) {
		IExec->Remove(&tnpb->tnpb_LibNode.lib_Node);
		result = tnpb->tnpb_SegList;

		CloseLibs(tnpb);

		IExec->Remove(&tnpb->tnpb_LibNode.lib_Node);
		IExec->DeleteLibrary(&tnpb->tnpb_LibNode);
	} else {
		tnpb->tnpb_LibNode.lib_Flags |= LIBF_DELEXP;
	}

	return result;
}

static int OpenLibs (struct TNPlugBase *tnpb) {
	DOSBase = IExec->OpenLibrary("dos.library", 52);
	IDOS = (struct DOSIFace *)IExec->GetInterface(DOSBase, "main", 1, NULL);
	if (!IDOS) return FALSE;
	UtilityBase = IExec->OpenLibrary("utility.library", 52);
	IUtility = (struct UtilityIFace *)IExec->GetInterface(UtilityBase, "main", 1, NULL);
	if (!IUtility) return FALSE;
	NewlibBase = IExec->OpenLibrary("newlib.library", 52);
	INewlib = IExec->GetInterface(NewlibBase, "main", 1, NULL);
	if (!INewlib) return FALSE;
	return TRUE;
}

static void CloseLibs (struct TNPlugBase *tnpb) {
	IExec->DropInterface(INewlib);
	IExec->CloseLibrary(NewlibBase);
	IExec->DropInterface((struct Interface *)IUtility);
	IExec->CloseLibrary(UtilityBase);
	IExec->DropInterface((struct Interface *)IDOS);
	IExec->CloseLibrary(DOSBase);
}
