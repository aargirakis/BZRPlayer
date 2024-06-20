#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG
#define dbug(args) ((struct ExecIFace *)(*(struct ExecBase **)4)->MainInterface)->DebugPrintF args
#else
#define dbug(args)
#endif

#endif
