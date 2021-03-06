/*
 * This file is (mostly) automatically generated from tclOO.decls.
 */

/* !BEGIN!: Do not edit below this line. */

/*
 * Exported function declarations:
 */

#ifndef TclOOGetDefineCmdContext_TCL_DECLARED
#define TclOOGetDefineCmdContext_TCL_DECLARED
/* 0 */
EXTERN Tcl_Object	TclOOGetDefineCmdContext(Tcl_Interp *interp);
#endif
#ifndef TclOOMakeProcInstanceMethod_TCL_DECLARED
#define TclOOMakeProcInstanceMethod_TCL_DECLARED
/* 1 */
EXTERN Tcl_Method	TclOOMakeProcInstanceMethod(Tcl_Interp *interp,
				Object *oPtr, int flags, Tcl_Obj *nameObj,
				Tcl_Obj *argsObj, Tcl_Obj *bodyObj,
				const Tcl_MethodType *typePtr,
				ClientData clientData, Proc **procPtrPtr);
#endif
#ifndef TclOOMakeProcMethod_TCL_DECLARED
#define TclOOMakeProcMethod_TCL_DECLARED
/* 2 */
EXTERN Tcl_Method	TclOOMakeProcMethod(Tcl_Interp *interp,
				Class *clsPtr, int flags, Tcl_Obj *nameObj,
				const char *namePtr, Tcl_Obj *argsObj,
				Tcl_Obj *bodyObj,
				const Tcl_MethodType *typePtr,
				ClientData clientData, Proc **procPtrPtr);
#endif
#ifndef TclOONewProcInstanceMethod_TCL_DECLARED
#define TclOONewProcInstanceMethod_TCL_DECLARED
/* 3 */
EXTERN Method *		TclOONewProcInstanceMethod(Tcl_Interp *interp,
				Object *oPtr, int flags, Tcl_Obj *nameObj,
				Tcl_Obj *argsObj, Tcl_Obj *bodyObj,
				ProcedureMethod **pmPtrPtr);
#endif
#ifndef TclOONewProcMethod_TCL_DECLARED
#define TclOONewProcMethod_TCL_DECLARED
/* 4 */
EXTERN Method *		TclOONewProcMethod(Tcl_Interp *interp, Class *clsPtr,
				int flags, Tcl_Obj *nameObj,
				Tcl_Obj *argsObj, Tcl_Obj *bodyObj,
				ProcedureMethod **pmPtrPtr);
#endif
#ifndef TclOOObjectCmdCore_TCL_DECLARED
#define TclOOObjectCmdCore_TCL_DECLARED
/* 5 */
EXTERN int		TclOOObjectCmdCore(Object *oPtr, Tcl_Interp *interp,
				int objc, Tcl_Obj *const *objv,
				int publicOnly, Class *startCls);
#endif
#ifndef TclOOIsReachable_TCL_DECLARED
#define TclOOIsReachable_TCL_DECLARED
/* 6 */
EXTERN int		TclOOIsReachable(Class *targetPtr, Class *startPtr);
#endif
#ifndef TclOONewForwardMethod_TCL_DECLARED
#define TclOONewForwardMethod_TCL_DECLARED
/* 7 */
EXTERN Method *		TclOONewForwardMethod(Tcl_Interp *interp,
				Class *clsPtr, int isPublic,
				Tcl_Obj *nameObj, Tcl_Obj *prefixObj);
#endif
#ifndef TclOONewForwardInstanceMethod_TCL_DECLARED
#define TclOONewForwardInstanceMethod_TCL_DECLARED
/* 8 */
EXTERN Method *		TclOONewForwardInstanceMethod(Tcl_Interp *interp,
				Object *oPtr, int isPublic, Tcl_Obj *nameObj,
				Tcl_Obj *prefixObj);
#endif
#ifndef TclOONewProcInstanceMethodEx_TCL_DECLARED
#define TclOONewProcInstanceMethodEx_TCL_DECLARED
/* 9 */
EXTERN Tcl_Method	TclOONewProcInstanceMethodEx(Tcl_Interp *interp,
				Tcl_Object oPtr,
				TclOO_PreCallProc *preCallPtr,
				TclOO_PostCallProc *postCallPtr,
				ProcErrorProc errProc, ClientData clientData,
				Tcl_Obj *nameObj, Tcl_Obj *argsObj,
				Tcl_Obj *bodyObj, int flags,
				void **internalTokenPtr);
#endif
#ifndef TclOONewProcMethodEx_TCL_DECLARED
#define TclOONewProcMethodEx_TCL_DECLARED
/* 10 */
EXTERN Tcl_Method	TclOONewProcMethodEx(Tcl_Interp *interp,
				Tcl_Class clsPtr,
				TclOO_PreCallProc *preCallPtr,
				TclOO_PostCallProc *postCallPtr,
				ProcErrorProc errProc, ClientData clientData,
				Tcl_Obj *nameObj, Tcl_Obj *argsObj,
				Tcl_Obj *bodyObj, int flags,
				void **internalTokenPtr);
#endif
#ifndef TclOOInvokeObject_TCL_DECLARED
#define TclOOInvokeObject_TCL_DECLARED
/* 11 */
EXTERN int		TclOOInvokeObject(Tcl_Interp *interp,
				Tcl_Object object, Tcl_Class startCls,
				int publicPrivate, int objc,
				Tcl_Obj *const *objv);
#endif
#ifndef TclOOObjectSetFilters_TCL_DECLARED
#define TclOOObjectSetFilters_TCL_DECLARED
/* 12 */
EXTERN void		TclOOObjectSetFilters(Object *oPtr, int numFilters,
				Tcl_Obj *const *filters);
#endif
#ifndef TclOOClassSetFilters_TCL_DECLARED
#define TclOOClassSetFilters_TCL_DECLARED
/* 13 */
EXTERN void		TclOOClassSetFilters(Tcl_Interp *interp,
				Class *classPtr, int numFilters,
				Tcl_Obj *const *filters);
#endif
#ifndef TclOOObjectSetMixins_TCL_DECLARED
#define TclOOObjectSetMixins_TCL_DECLARED
/* 14 */
EXTERN void		TclOOObjectSetMixins(Object *oPtr, int numMixins,
				Class *const *mixins);
#endif
#ifndef TclOOClassSetMixins_TCL_DECLARED
#define TclOOClassSetMixins_TCL_DECLARED
/* 15 */
EXTERN void		TclOOClassSetMixins(Tcl_Interp *interp,
				Class *classPtr, int numMixins,
				Class *const *mixins);
#endif

typedef struct TclOOIntStubs {
    int magic;
    const struct TclOOIntStubHooks *hooks;

    Tcl_Object (*tclOOGetDefineCmdContext) (Tcl_Interp *interp); /* 0 */
    Tcl_Method (*tclOOMakeProcInstanceMethod) (Tcl_Interp *interp, Object *oPtr, int flags, Tcl_Obj *nameObj, Tcl_Obj *argsObj, Tcl_Obj *bodyObj, const Tcl_MethodType *typePtr, ClientData clientData, Proc **procPtrPtr); /* 1 */
    Tcl_Method (*tclOOMakeProcMethod) (Tcl_Interp *interp, Class *clsPtr, int flags, Tcl_Obj *nameObj, const char *namePtr, Tcl_Obj *argsObj, Tcl_Obj *bodyObj, const Tcl_MethodType *typePtr, ClientData clientData, Proc **procPtrPtr); /* 2 */
    Method * (*tclOONewProcInstanceMethod) (Tcl_Interp *interp, Object *oPtr, int flags, Tcl_Obj *nameObj, Tcl_Obj *argsObj, Tcl_Obj *bodyObj, ProcedureMethod **pmPtrPtr); /* 3 */
    Method * (*tclOONewProcMethod) (Tcl_Interp *interp, Class *clsPtr, int flags, Tcl_Obj *nameObj, Tcl_Obj *argsObj, Tcl_Obj *bodyObj, ProcedureMethod **pmPtrPtr); /* 4 */
    int (*tclOOObjectCmdCore) (Object *oPtr, Tcl_Interp *interp, int objc, Tcl_Obj *const *objv, int publicOnly, Class *startCls); /* 5 */
    int (*tclOOIsReachable) (Class *targetPtr, Class *startPtr); /* 6 */
    Method * (*tclOONewForwardMethod) (Tcl_Interp *interp, Class *clsPtr, int isPublic, Tcl_Obj *nameObj, Tcl_Obj *prefixObj); /* 7 */
    Method * (*tclOONewForwardInstanceMethod) (Tcl_Interp *interp, Object *oPtr, int isPublic, Tcl_Obj *nameObj, Tcl_Obj *prefixObj); /* 8 */
    Tcl_Method (*tclOONewProcInstanceMethodEx) (Tcl_Interp *interp, Tcl_Object oPtr, TclOO_PreCallProc *preCallPtr, TclOO_PostCallProc *postCallPtr, ProcErrorProc errProc, ClientData clientData, Tcl_Obj *nameObj, Tcl_Obj *argsObj, Tcl_Obj *bodyObj, int flags, void **internalTokenPtr); /* 9 */
    Tcl_Method (*tclOONewProcMethodEx) (Tcl_Interp *interp, Tcl_Class clsPtr, TclOO_PreCallProc *preCallPtr, TclOO_PostCallProc *postCallPtr, ProcErrorProc errProc, ClientData clientData, Tcl_Obj *nameObj, Tcl_Obj *argsObj, Tcl_Obj *bodyObj, int flags, void **internalTokenPtr); /* 10 */
    int (*tclOOInvokeObject) (Tcl_Interp *interp, Tcl_Object object, Tcl_Class startCls, int publicPrivate, int objc, Tcl_Obj *const *objv); /* 11 */
    void (*tclOOObjectSetFilters) (Object *oPtr, int numFilters, Tcl_Obj *const *filters); /* 12 */
    void (*tclOOClassSetFilters) (Tcl_Interp *interp, Class *classPtr, int numFilters, Tcl_Obj *const *filters); /* 13 */
    void (*tclOOObjectSetMixins) (Object *oPtr, int numMixins, Class *const *mixins); /* 14 */
    void (*tclOOClassSetMixins) (Tcl_Interp *interp, Class *classPtr, int numMixins, Class *const *mixins); /* 15 */
} TclOOIntStubs;

#if defined(USE_TCLOO_STUBS) && !defined(USE_TCLOO_STUB_PROCS)
extern const TclOOIntStubs *tclOOIntStubsPtr;
#endif /* defined(USE_TCLOO_STUBS) && !defined(USE_TCLOO_STUB_PROCS) */

#if defined(USE_TCLOO_STUBS) && !defined(USE_TCLOO_STUB_PROCS)

/*
 * Inline function declarations:
 */

#ifndef TclOOGetDefineCmdContext
#define TclOOGetDefineCmdContext \
	(tclOOIntStubsPtr->tclOOGetDefineCmdContext) /* 0 */
#endif
#ifndef TclOOMakeProcInstanceMethod
#define TclOOMakeProcInstanceMethod \
	(tclOOIntStubsPtr->tclOOMakeProcInstanceMethod) /* 1 */
#endif
#ifndef TclOOMakeProcMethod
#define TclOOMakeProcMethod \
	(tclOOIntStubsPtr->tclOOMakeProcMethod) /* 2 */
#endif
#ifndef TclOONewProcInstanceMethod
#define TclOONewProcInstanceMethod \
	(tclOOIntStubsPtr->tclOONewProcInstanceMethod) /* 3 */
#endif
#ifndef TclOONewProcMethod
#define TclOONewProcMethod \
	(tclOOIntStubsPtr->tclOONewProcMethod) /* 4 */
#endif
#ifndef TclOOObjectCmdCore
#define TclOOObjectCmdCore \
	(tclOOIntStubsPtr->tclOOObjectCmdCore) /* 5 */
#endif
#ifndef TclOOIsReachable
#define TclOOIsReachable \
	(tclOOIntStubsPtr->tclOOIsReachable) /* 6 */
#endif
#ifndef TclOONewForwardMethod
#define TclOONewForwardMethod \
	(tclOOIntStubsPtr->tclOONewForwardMethod) /* 7 */
#endif
#ifndef TclOONewForwardInstanceMethod
#define TclOONewForwardInstanceMethod \
	(tclOOIntStubsPtr->tclOONewForwardInstanceMethod) /* 8 */
#endif
#ifndef TclOONewProcInstanceMethodEx
#define TclOONewProcInstanceMethodEx \
	(tclOOIntStubsPtr->tclOONewProcInstanceMethodEx) /* 9 */
#endif
#ifndef TclOONewProcMethodEx
#define TclOONewProcMethodEx \
	(tclOOIntStubsPtr->tclOONewProcMethodEx) /* 10 */
#endif
#ifndef TclOOInvokeObject
#define TclOOInvokeObject \
	(tclOOIntStubsPtr->tclOOInvokeObject) /* 11 */
#endif
#ifndef TclOOObjectSetFilters
#define TclOOObjectSetFilters \
	(tclOOIntStubsPtr->tclOOObjectSetFilters) /* 12 */
#endif
#ifndef TclOOClassSetFilters
#define TclOOClassSetFilters \
	(tclOOIntStubsPtr->tclOOClassSetFilters) /* 13 */
#endif
#ifndef TclOOObjectSetMixins
#define TclOOObjectSetMixins \
	(tclOOIntStubsPtr->tclOOObjectSetMixins) /* 14 */
#endif
#ifndef TclOOClassSetMixins
#define TclOOClassSetMixins \
	(tclOOIntStubsPtr->tclOOClassSetMixins) /* 15 */
#endif

#endif /* defined(USE_TCLOO_STUBS) && !defined(USE_TCLOO_STUB_PROCS) */

/* !END!: Do not edit above this line. */
