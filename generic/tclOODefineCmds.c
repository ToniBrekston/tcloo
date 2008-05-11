/*
 * tclOODefineCmds.c --
 *
 *	This file contains the implementation of the ::oo::define command,
 *	part of the object-system core (NB: not Tcl_Obj, but ::oo).
 *
 * Copyright (c) 2006 by Donal K. Fellows
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: tclOODefineCmds.c,v 1.10 2008/05/11 10:02:28 dkf Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "tclInt.h"
#include "tclOOInt.h"

/*
 * ----------------------------------------------------------------------
 *
 * BumpGlobalEpoch --
 *	Utility that ensures that call chains that are invalid will get thrown
 *	away at an appropriate time. Note that exactly which epoch gets
 *	advanced will depend on exactly what the class is tangled up in; in
 *	the worst case, the simplest option is to advance the global epoch,
 *	causing *everything* to be thrown away on next usage.
 *
 * ----------------------------------------------------------------------
 */

static inline void
BumpGlobalEpoch(
    Tcl_Interp *interp,
    Class *classPtr)
{
    if (classPtr != NULL
	    && classPtr->subclasses.num == 0
	    && classPtr->instances.num == 0
	    && classPtr->mixinSubs.num == 0) {
	/*
	 * If a class has no subclasses or instances, and is not mixed into
	 * anything, a change to its structure does not require us to
	 * invalidate any call chains. Note that we still bump our object's
	 * epoch if it has any mixins; the relation between a class and its
	 * representative object is special. But it won't hurt.
	 */

	if (classPtr->thisPtr->mixins.num > 0) {
	    classPtr->thisPtr->epoch++;
	}
	return;
    }

    /*
     * Either there's no class (?!) or we're reconfiguring something that is
     * in use. Force regeneration of call chains.
     */

    TclOOGetFoundation(interp)->epoch++;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOOObjectSetFilters --
 *	Install a list of filter method names into an object.
 *
 * ----------------------------------------------------------------------
 */

void
TclOOObjectSetFilters(
    Object *oPtr,
    int numFilters,
    Tcl_Obj *const *filters)
{
    int i;

    if (oPtr->filters.num) {
	Tcl_Obj *filterObj;

	FOREACH(filterObj, oPtr->filters) {
	    Tcl_DecrRefCount(filterObj);
	}
    }

    if (numFilters == 0) {
	/*
	 * No list of filters was supplied, so we're deleting filters.
	 */

	ckfree((char *) oPtr->filters.list);
	oPtr->filters.list = NULL;
	oPtr->filters.num = 0;
    } else {
	/*
	 * We've got a list of filters, so we're creating filters.
	 */

	Tcl_Obj **filtersList;
	int size = sizeof(Tcl_Obj *) * numFilters;	/* should be size_t */

	if (oPtr->filters.num == 0) {
	    filtersList = (Tcl_Obj **) ckalloc(size);
	} else {
	    filtersList = (Tcl_Obj **)
		    ckrealloc((char *) oPtr->filters.list, size);
	}
	for (i=0 ; i<numFilters ; i++) {
	    filtersList[i] = filters[i];
	    Tcl_IncrRefCount(filters[i]);
	}
	oPtr->filters.list = filtersList;
	oPtr->filters.num = numFilters;
    }
    oPtr->epoch++;		/* Only this object can be affected. */
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOOClassSetFilters --
 *	Install a list of filter method names into a class.
 *
 * ----------------------------------------------------------------------
 */

void
TclOOClassSetFilters(
    Tcl_Interp *interp,
    Class *classPtr,
    int numFilters,
    Tcl_Obj *const *filters)
{
    int i;

    if (classPtr->filters.num) {
	Tcl_Obj *filterObj;

	FOREACH(filterObj, classPtr->filters) {
	    Tcl_DecrRefCount(filterObj);
	}
    }

    if (numFilters == 0) {
	/*
	 * No list of filters was supplied, so we're deleting filters.
	 */

	ckfree((char *) classPtr->filters.list);
	classPtr->filters.list = NULL;
	classPtr->filters.num = 0;
    } else {
	/*
	 * We've got a list of filters, so we're creating filters.
	 */

	Tcl_Obj **filtersList;
	int size = sizeof(Tcl_Obj *) * numFilters;	/* should be size_t */

	if (classPtr->filters.num == 0) {
	    filtersList = (Tcl_Obj **) ckalloc(size);
	} else {
	    filtersList = (Tcl_Obj **)
		    ckrealloc((char *) classPtr->filters.list, size);
	}
	for (i=0 ; i<numFilters ; i++) {
	    filtersList[i] = filters[i];
	    Tcl_IncrRefCount(filters[i]);
	}
	classPtr->filters.list = filtersList;
	classPtr->filters.num = numFilters;
    }

    /*
     * There may be many objects affected, so bump the global epoch.
     */

    BumpGlobalEpoch(interp, classPtr);
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOOObjectSetMixins --
 *	Install a list of mixin classes into an object.
 *
 * ----------------------------------------------------------------------
 */

void
TclOOObjectSetMixins(
    Object *oPtr,
    int numMixins,
    Class *const *mixins)
{
    Class *mixinPtr;
    int i;

    if (numMixins == 0) {
	if (oPtr->mixins.num != 0) {
	    FOREACH(mixinPtr, oPtr->mixins) {
		TclOORemoveFromInstances(oPtr, mixinPtr);
	    }
	    ckfree((char *) oPtr->mixins.list);
	    oPtr->mixins.num = 0;
	}
    } else {
	if (oPtr->mixins.num != 0) {
	    FOREACH(mixinPtr, oPtr->mixins) {
		if (mixinPtr != oPtr->selfCls) {
		    TclOORemoveFromInstances(oPtr, mixinPtr);
		}
	    }
	    oPtr->mixins.list = (Class **)
		    ckrealloc((char *) oPtr->mixins.list,
		    sizeof(Class *) * numMixins);
	} else {
	    oPtr->mixins.list = (Class **)
		    ckalloc(sizeof(Class *) * numMixins);
	}
	oPtr->mixins.num = numMixins;
	memcpy(oPtr->mixins.list, mixins, sizeof(Class *) * numMixins);
	FOREACH(mixinPtr, oPtr->mixins) {
	    if (mixinPtr != oPtr->selfCls) {
		TclOOAddToInstances(oPtr, mixinPtr);
	    }
	}
    }
    oPtr->epoch++;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOOClassSetMixins --
 *	Install a list of mixin classes into a class.
 *
 * ----------------------------------------------------------------------
 */

void
TclOOClassSetMixins(
    Tcl_Interp *interp,
    Class *classPtr,
    int numMixins,
    Class *const *mixins)
{
    Class *mixinPtr;
    int i;

    if (numMixins == 0) {
	if (classPtr->mixins.num != 0) {
	    FOREACH(mixinPtr, classPtr->mixins) {
		TclOORemoveFromMixinSubs(classPtr, mixinPtr);
	    }
	    ckfree((char *) classPtr->mixins.list);
	    classPtr->mixins.num = 0;
	}
    } else {
	if (classPtr->mixins.num != 0) {
	    FOREACH(mixinPtr, classPtr->mixins) {
		TclOORemoveFromMixinSubs(classPtr, mixinPtr);
	    }
	    classPtr->mixins.list = (Class **)
		    ckrealloc((char *) classPtr->mixins.list,
		    sizeof(Class *) * numMixins);
	} else {
	    classPtr->mixins.list = (Class **)
		    ckalloc(sizeof(Class *) * numMixins);
	}
	classPtr->mixins.num = numMixins;
	memcpy(classPtr->mixins.list, mixins, sizeof(Class *) * numMixins);
	FOREACH(mixinPtr, classPtr->mixins) {
	    TclOOAddToMixinSubs(classPtr, mixinPtr);
	}
    }
    BumpGlobalEpoch(interp, classPtr);
}

/*
 * ----------------------------------------------------------------------
 *
 * RenameDeleteMethod --
 *	Core of the code to rename and delete methods.
 *
 * ----------------------------------------------------------------------
 */

static int
RenameDeleteMethod(
    Tcl_Interp *interp,
    Object *oPtr,
    int useClass,
    Tcl_Obj *const fromPtr,
    Tcl_Obj *const toPtr)
{
    Tcl_HashEntry *hPtr, *newHPtr;
    Method *mPtr;
    int isNew;

    if (!useClass) {
	if (!oPtr->methodsPtr) {
	noSuchMethod:
	    Tcl_AppendResult(interp, "method ", TclGetString(fromPtr),
		    " does not exist", NULL);
	    return TCL_ERROR;
	}
	hPtr = Tcl_FindHashEntry(oPtr->methodsPtr, (char *) fromPtr);
	if (hPtr == NULL) {
	    goto noSuchMethod;
	}
	if (toPtr) {
	    newHPtr = Tcl_CreateHashEntry(oPtr->methodsPtr, (char *) toPtr,
		    &isNew);
	    if (hPtr == newHPtr) {
	    renameToSelf:
		Tcl_AppendResult(interp, "cannot rename method to itself",
			NULL);
		return TCL_ERROR;
	    } else if (!isNew) {
	    renameToExisting:
		Tcl_AppendResult(interp, "method called ",
			TclGetString(toPtr), " already exists", NULL);
		return TCL_ERROR;
	    }
	}
    } else {
	hPtr = Tcl_FindHashEntry(&oPtr->classPtr->classMethods,
		(char *) fromPtr);
	if (hPtr == NULL) {
	    goto noSuchMethod;
	}
	if (toPtr) {
	    newHPtr = Tcl_CreateHashEntry(&oPtr->classPtr->classMethods,
		    (char *) toPtr, &isNew);
	    if (hPtr == newHPtr) {
		goto renameToSelf;
	    } else if (!isNew) {
		goto renameToExisting;
	    }
	}
    }

    /*
     * Complete the splicing by changing the method's name.
     */

    mPtr = Tcl_GetHashValue(hPtr);
    if (toPtr) {
	Tcl_IncrRefCount(toPtr);
	Tcl_DecrRefCount(mPtr->namePtr);
	mPtr->namePtr = toPtr;
	Tcl_SetHashValue(newHPtr, mPtr);
    } else {
	TclOODeleteMethod(mPtr);
    }
    Tcl_DeleteHashEntry(hPtr);
    return TCL_OK;
}

/*
 * ----------------------------------------------------------------------
 *
 * InitDefineContext --
 *	Does the magic incantations necessary to push the special stack frame
 *	used when processing object definitions. It is up to the caller to
 *	dispose of the frame (with TclPopStackFrame) when finished.
 *
 * ----------------------------------------------------------------------
 */

static inline int
InitDefineContext(
    Tcl_Interp *interp,
    Tcl_Namespace *namespacePtr,
    Object *oPtr,
    int objc,
    Tcl_Obj *const objv[])
{
    CallFrame *framePtr, **framePtrPtr = &framePtr;
    int result;

    /* framePtrPtr is needed to satisfy GCC 3.3's strict aliasing rules */

    result = TclPushStackFrame(interp, (Tcl_CallFrame **) framePtrPtr,
	    namespacePtr, FRAME_IS_OO_DEFINE);
    if (result != TCL_OK) {
	return TCL_ERROR;
    }
    framePtr->clientData = oPtr;
    framePtr->objc = objc;
    framePtr->objv = objv;	/* Reference counts do not need to be
				 * incremented here. */
    return TCL_OK;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOOGetDefineCmdContext --
 *	Extracts the magic token from the current stack frame, or returns NULL
 *	(and leaves an error message) otherwise.
 *
 * ----------------------------------------------------------------------
 */

Tcl_Object
TclOOGetDefineCmdContext(
    Tcl_Interp *interp)
{
    Interp *iPtr = (Interp *) interp;

    if ((iPtr->framePtr == NULL)
	    || (iPtr->framePtr->isProcCallFrame != FRAME_IS_OO_DEFINE)) {
	Tcl_AppendResult(interp, "this command may only be called from within"
		" the context of an ::oo::define or ::oo::objdefine command",
		NULL);
	return NULL;
    }
    return (Tcl_Object) iPtr->framePtr->clientData;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOODefineObjCmd --
 *	Implementation of the "oo::define" command. Works by effectively doing
 *	the same as 'namespace eval', but with extra magic applied so that the
 *	object to be modified is known to the commands in the target
 *	namespace. Also does ensemble-like tricks with dispatch so that error
 *	messages are clearer.
 *
 * ----------------------------------------------------------------------
 */

int
TclOODefineObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Foundation *fPtr = TclOOGetFoundation(interp);
    int result;
    Object *oPtr;

    if (objc < 3) {
	Tcl_WrongNumArgs(interp, 1, objv, "className arg ?arg ...?");
	return TCL_ERROR;
    }

    oPtr = (Object *) Tcl_GetObjectFromObj(interp, objv[1]);
    if (oPtr == NULL) {
	return TCL_ERROR;
    }
    if (oPtr->classPtr == NULL) {
	Tcl_AppendResult(interp, TclGetString(objv[1]),
		" does not refer to a class", NULL);
	return TCL_ERROR;
    }

    /*
     * Make the oo::define namespace the current namespace and evaluate the
     * command(s).
     */

    if (InitDefineContext(interp, fPtr->defineNs, oPtr, objc,objv) != TCL_OK){
	return TCL_ERROR;
    }

    Tcl_Preserve(oPtr);
    if (objc == 3) {
	result = TclEvalObjEx(interp, objv[2], 0,
		((Interp *)interp)->cmdFramePtr, 2);

	if (result == TCL_ERROR) {
	    int length;
	    const char *objName = Tcl_GetStringFromObj(objv[1], &length);
	    int limit = 60;
	    int overflow = (length > limit);

	    Tcl_AppendObjToErrorInfo(interp, Tcl_ObjPrintf(
		    "\n    (in definition script for object \"%.*s%s\" line %d)",
		    (overflow ? limit : length), objName,
		    (overflow ? "..." : ""), interp->errorLine));
	}
    } else {
	Tcl_Obj *objPtr, *obj2Ptr, **objs;
	Interp *iPtr = (Interp *) interp;
	Tcl_Command cmd;
	int dummy;

	/*
	 * More than one argument: fire them through the ensemble processing
	 * engine so that everything appears to be good and proper in error
	 * messages. Note that we cannot just concatenate and send through
	 * Tcl_EvalObjEx, as that doesn't do ensemble processing, and we
	 * cannot go through Tcl_EvalObjv without the extra work to pre-find
	 * the command, as that finds command names in the wrong namespace at
	 * the moment. Ugly!
	 */

	if (iPtr->ensembleRewrite.sourceObjs == NULL) {
	    iPtr->ensembleRewrite.sourceObjs = objv;
	    iPtr->ensembleRewrite.numRemovedObjs = 3;
	    iPtr->ensembleRewrite.numInsertedObjs = 1;
	} else {
	    int ni = iPtr->ensembleRewrite.numInsertedObjs;
	    if (ni < 3) {
		iPtr->ensembleRewrite.numRemovedObjs += 3 - ni;
	    } else {
		iPtr->ensembleRewrite.numInsertedObjs -= 2;
	    }
	}

	/*
	 * Build the list of arguments using a Tcl_Obj as a workspace. See
	 * comments above for why these contortions are necessary.
	 */

	objPtr = Tcl_NewObj();
	obj2Ptr = Tcl_NewObj();
	cmd = Tcl_FindCommand(interp, TclGetString(objv[2]), fPtr->defineNs,
		TCL_NAMESPACE_ONLY);
	if (cmd == NULL) {
	    /* punt this case! */
	    Tcl_AppendObjToObj(obj2Ptr, objv[2]);
	} else {
	    Tcl_GetCommandFullName(interp, cmd, obj2Ptr);
	}
	Tcl_ListObjAppendElement(NULL, objPtr, obj2Ptr);
	Tcl_ListObjReplace(NULL, objPtr, 1, 0, objc-3, objv+3);
	Tcl_ListObjGetElements(NULL, objPtr, &dummy, &objs);

	result = Tcl_EvalObjv(interp, objc-2, objs, TCL_EVAL_INVOKE);
	Tcl_DecrRefCount(objPtr);
    }
    Tcl_Release(oPtr);

    /*
     * Restore the previous "current" namespace.
     */

    TclPopStackFrame(interp);
    return result;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOOObjDefObjCmd --
 *	Implementation of the "oo::objdefine" command. Works by effectively
 *	doing the same as 'namespace eval', but with extra magic applied so
 *	that the object to be modified is known to the commands in the target
 *	namespace. Also does ensemble-like tricks with dispatch so that error
 *	messages are clearer.
 *
 * ----------------------------------------------------------------------
 */

int
TclOOObjDefObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Foundation *fPtr = TclOOGetFoundation(interp);
    int result;
    Object *oPtr;

    if (objc < 3) {
	Tcl_WrongNumArgs(interp, 1, objv, "objectName arg ?arg ...?");
	return TCL_ERROR;
    }

    oPtr = (Object *) Tcl_GetObjectFromObj(interp, objv[1]);
    if (oPtr == NULL) {
	return TCL_ERROR;
    }

    /*
     * Make the oo::objdefine namespace the current namespace and evaluate the
     * command(s).
     */

    if (InitDefineContext(interp, fPtr->objdefNs, oPtr, objc,objv) != TCL_OK){
	return TCL_ERROR;
    }

    Tcl_Preserve(oPtr);
    if (objc == 3) {
	result = TclEvalObjEx(interp, objv[2], 0,
		((Interp *)interp)->cmdFramePtr, 2);

	if (result == TCL_ERROR) {
	    int length;
	    const char *objName = Tcl_GetStringFromObj(objv[1], &length);
	    int limit = 60;
	    int overflow = (length > limit);

	    Tcl_AppendObjToErrorInfo(interp, Tcl_ObjPrintf(
		    "\n    (in definition script for object \"%.*s%s\" line %d)",
		    (overflow ? limit : length), objName,
		    (overflow ? "..." : ""), interp->errorLine));
	}
    } else {
	Tcl_Obj *objPtr, *obj2Ptr, **objs;
	Interp *iPtr = (Interp *) interp;
	Tcl_Command cmd;
	int dummy;

	/*
	 * More than one argument: fire them through the ensemble processing
	 * engine so that everything appears to be good and proper in error
	 * messages. Note that we cannot just concatenate and send through
	 * Tcl_EvalObjEx, as that doesn't do ensemble processing, and we
	 * cannot go through Tcl_EvalObjv without the extra work to pre-find
	 * the command, as that finds command names in the wrong namespace at
	 * the moment. Ugly!
	 */

	if (iPtr->ensembleRewrite.sourceObjs == NULL) {
	    iPtr->ensembleRewrite.sourceObjs = objv;
	    iPtr->ensembleRewrite.numRemovedObjs = 3;
	    iPtr->ensembleRewrite.numInsertedObjs = 1;
	} else {
	    int ni = iPtr->ensembleRewrite.numInsertedObjs;
	    if (ni < 3) {
		iPtr->ensembleRewrite.numRemovedObjs += 3 - ni;
	    } else {
		iPtr->ensembleRewrite.numInsertedObjs -= 2;
	    }
	}

	/*
	 * Build the list of arguments using a Tcl_Obj as a workspace. See
	 * comments above for why these contortions are necessary.
	 */

	objPtr = Tcl_NewObj();
	obj2Ptr = Tcl_NewObj();
	cmd = Tcl_FindCommand(interp, TclGetString(objv[2]), fPtr->objdefNs,
		TCL_NAMESPACE_ONLY);
	if (cmd == NULL) {
	    /* punt this case! */
	    Tcl_AppendObjToObj(obj2Ptr, objv[2]);
	} else {
	    Tcl_GetCommandFullName(interp, cmd, obj2Ptr);
	}
	Tcl_ListObjAppendElement(NULL, objPtr, obj2Ptr);
	Tcl_ListObjReplace(NULL, objPtr, 1, 0, objc-3, objv+3);
	Tcl_ListObjGetElements(NULL, objPtr, &dummy, &objs);

	result = Tcl_EvalObjv(interp, objc-2, objs, TCL_EVAL_INVOKE);
	Tcl_DecrRefCount(objPtr);
    }
    Tcl_Release(oPtr);

    /*
     * Restore the previous "current" namespace.
     */

    TclPopStackFrame(interp);
    return result;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOODefineSelfObjCmd --
 *	Implementation of the "self" subcommand of the "oo::define" command.
 *	Works by effectively doing the same as 'namespace eval', but with
 *	extra magic applied so that the object to be modified is known to the
 *	commands in the target namespace. Also does ensemble-like tricks with
 *	dispatch so that error messages are clearer.
 *
 * ----------------------------------------------------------------------
 */

int
TclOODefineSelfObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Foundation *fPtr = TclOOGetFoundation(interp);
    int result;
    Object *oPtr;

    if (objc < 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "arg ?arg ...?");
	return TCL_ERROR;
    }

    oPtr = (Object *) TclOOGetDefineCmdContext(interp);
    if (oPtr == NULL) {
	return TCL_ERROR;
    }

    /*
     * Make the oo::objdefine namespace the current namespace and evaluate the
     * command(s).
     */

    if (InitDefineContext(interp, fPtr->objdefNs, oPtr, objc,objv) != TCL_OK){
	return TCL_ERROR;
    }

    Tcl_Preserve(oPtr);
    if (objc == 2) {
	result = TclEvalObjEx(interp, objv[1], 0,
		((Interp *)interp)->cmdFramePtr, 2);

	if (result == TCL_ERROR) {
	    int length;
	    const char *objName = Tcl_GetStringFromObj(
		    TclOOObjectName(interp, oPtr), &length);
	    int limit = 60;
	    int overflow = (length > limit);

	    Tcl_AppendObjToErrorInfo(interp, Tcl_ObjPrintf(
		    "\n    (in definition script for object \"%.*s%s\" line %d)",
		    (overflow ? limit : length), objName,
		    (overflow ? "..." : ""), interp->errorLine));
	}
    } else {
	Tcl_Obj *objPtr, *obj2Ptr, **objs;
	Interp *iPtr = (Interp *) interp;
	Tcl_Command cmd;
	int dummy;

	/*
	 * More than one argument: fire them through the ensemble processing
	 * engine so that everything appears to be good and proper in error
	 * messages. Note that we cannot just concatenate and send through
	 * Tcl_EvalObjEx, as that doesn't do ensemble processing, and we
	 * cannot go through Tcl_EvalObjv without the extra work to pre-find
	 * the command, as that finds command names in the wrong namespace at
	 * the moment. Ugly!
	 */

	if (iPtr->ensembleRewrite.sourceObjs == NULL) {
	    iPtr->ensembleRewrite.sourceObjs = objv;
	    iPtr->ensembleRewrite.numRemovedObjs = 2;
	    iPtr->ensembleRewrite.numInsertedObjs = 1;
	} else {
	    int ni = iPtr->ensembleRewrite.numInsertedObjs;
	    if (ni < 2) {
		iPtr->ensembleRewrite.numRemovedObjs += 2 - ni;
	    } else {
		iPtr->ensembleRewrite.numInsertedObjs -= 1;
	    }
	}

	/*
	 * Build the list of arguments using a Tcl_Obj as a workspace. See
	 * comments above for why these contortions are necessary.
	 */

	objPtr = Tcl_NewObj();
	obj2Ptr = Tcl_NewObj();
	cmd = Tcl_FindCommand(interp, TclGetString(objv[1]), fPtr->objdefNs,
		TCL_NAMESPACE_ONLY);
	if (cmd == NULL) {
	    /* punt this case! */
	    Tcl_AppendObjToObj(obj2Ptr, objv[1]);
	} else {
	    Tcl_GetCommandFullName(interp, cmd, obj2Ptr);
	}
	Tcl_ListObjAppendElement(NULL, objPtr, obj2Ptr);
	Tcl_ListObjReplace(NULL, objPtr, 1, 0, objc-2, objv+2);
	Tcl_ListObjGetElements(NULL, objPtr, &dummy, &objs);

	result = Tcl_EvalObjv(interp, objc-1, objs, TCL_EVAL_INVOKE);
	Tcl_DecrRefCount(objPtr);
    }
    Tcl_Release(oPtr);

    /*
     * Restore the previous "current" namespace.
     */

    TclPopStackFrame(interp);
    return result;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOODefineClassObjCmd --
 *	Implementation of the "class" subcommand of the "oo::objdefine"
 *	command.
 *
 * ----------------------------------------------------------------------
 */

int
TclOODefineClassObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Object *oPtr, *o2Ptr;
    Foundation *fPtr = TclOOGetFoundation(interp);

    /*
     * Parse the context to get the object to operate on.
     */

    oPtr = (Object *) TclOOGetDefineCmdContext(interp);
    if (oPtr == NULL) {
	return TCL_ERROR;
    }
    if (oPtr == fPtr->objectCls->thisPtr) {
	Tcl_AppendResult(interp,
		"may not modify the class of the root object", NULL);
	return TCL_ERROR;
    }
    if (oPtr == fPtr->classCls->thisPtr) {
	Tcl_AppendResult(interp,
		"may not modify the class of the class of classes", NULL);
	return TCL_ERROR;
    }

    /*
     * Parse the argument to get the class to set the object's class to.
     */

    if (objc != 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "className");
	return TCL_ERROR;
    }
    o2Ptr = (Object *) Tcl_GetObjectFromObj(interp, objv[1]);
    if (o2Ptr == NULL) {
	return TCL_ERROR;
    }
    if (o2Ptr->classPtr == NULL) {
	Tcl_AppendResult(interp, "the class of an object must be a class",
		NULL);
	return TCL_ERROR;
    }

    /*
     * Apply semantic checks. In particular, classes and non-classes are not
     * interchangable (too complicated to do the conversion!) so we must
     * produce an error if any attempt is made to swap from one to the other.
     */

    if ((oPtr->classPtr == NULL) == TclOOIsReachable(fPtr->classCls,
	    o2Ptr->classPtr)) {
	Tcl_AppendResult(interp, "may not change a ",
		(oPtr->classPtr==NULL ? "non-" : ""), "class object into a ",
		(oPtr->classPtr==NULL ? "" : "non-"), "class object", NULL);
	return TCL_ERROR;
    }

    /*
     * Set the object's class.
     */

    if (oPtr->selfCls != o2Ptr->classPtr) {
	TclOORemoveFromInstances(oPtr, oPtr->selfCls);
	oPtr->selfCls = o2Ptr->classPtr;
	TclOOAddToInstances(oPtr, oPtr->selfCls);
	if (oPtr->classPtr != NULL) {
	    BumpGlobalEpoch(interp, oPtr->classPtr);
	} else {
	    oPtr->epoch++;
	}
    }
    return TCL_OK;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOODefineConstructorObjCmd --
 *	Implementation of the "constructor" subcommand of the "oo::define"
 *	command.
 *
 * ----------------------------------------------------------------------
 */

int
TclOODefineConstructorObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Object *oPtr;
    Class *clsPtr;
    Tcl_Method method;
    int bodyLength;

    if (objc != 3) {
	Tcl_WrongNumArgs(interp, 1, objv, "arguments body");
	return TCL_ERROR;
    }

    /*
     * Extract and validate the context, which is the class that we wish to
     * modify.
     */

    oPtr = (Object *) TclOOGetDefineCmdContext(interp);
    if (oPtr == NULL) {
	return TCL_ERROR;
    }
    clsPtr = oPtr->classPtr;

    Tcl_GetStringFromObj(objv[2], &bodyLength);
    if (bodyLength > 0) {
	/*
	 * Create the method structure.
	 */

	method = (Tcl_Method) TclOONewProcMethod(interp, clsPtr,
		PUBLIC_METHOD, NULL, objv[1], objv[2], NULL);
	if (method == NULL) {
	    return TCL_ERROR;
	}
    } else {
	/*
	 * Delete the constructor method record and set the field in the
	 * class record to NULL.
	 */

	method = NULL;
    }

    /*
     * Place the method structure in the class record. Note that we might not
     * immediately delete the constructor as this might be being done during
     * execution of the constructor itself.
     */

    Tcl_ClassSetConstructor((Tcl_Class) clsPtr, method);
    return TCL_OK;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOODefineDeleteMethodObjCmd --
 *	Implementation of the "deletemethod" subcommand of the "oo::define"
 *	and "oo::objdefine" commands.
 *
 * ----------------------------------------------------------------------
 */

int
TclOODefineDeleteMethodObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    int isInstanceDeleteMethod = (clientData != NULL);
    Object *oPtr;
    int i;

    if (objc < 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "name ?name ...?");
	return TCL_ERROR;
    }

    oPtr = (Object *) TclOOGetDefineCmdContext(interp);
    if (oPtr == NULL) {
	return TCL_ERROR;
    }
    if (!isInstanceDeleteMethod && !oPtr->classPtr) {
	Tcl_AppendResult(interp, "attempt to misuse API", NULL);
	return TCL_ERROR;
    }

    for (i=1 ; i<objc ; i++) {
	/*
	 * Delete the method structure from the appropriate hash table.
	 */

	if (RenameDeleteMethod(interp, oPtr, !isInstanceDeleteMethod,
		objv[i], NULL) != TCL_OK) {
	    return TCL_ERROR;
	}
    }

    if (isInstanceDeleteMethod) {
	oPtr->epoch++;
    } else {
	BumpGlobalEpoch(interp, oPtr->classPtr);
    }
    return TCL_OK;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOODefineDestructorObjCmd --
 *	Implementation of the "destructor" subcommand of the "oo::define"
 *	command.
 *
 * ----------------------------------------------------------------------
 */

int
TclOODefineDestructorObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Object *oPtr;
    Class *clsPtr;
    Tcl_Method method;
    int bodyLength;

    if (objc != 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "body");
	return TCL_ERROR;
    }

    oPtr = (Object *) TclOOGetDefineCmdContext(interp);
    if (oPtr == NULL) {
	return TCL_ERROR;
    }
    clsPtr = oPtr->classPtr;

    Tcl_GetStringFromObj(objv[1], &bodyLength);
    if (bodyLength > 0) {
	/*
	 * Create the method structure.
	 */

	method = (Tcl_Method) TclOONewProcMethod(interp, clsPtr,
		PUBLIC_METHOD, NULL, NULL, objv[1], NULL);
	if (method == NULL) {
	    return TCL_ERROR;
	}
    } else {
	/*
	 * Delete the destructor method record and set the field in the class
	 * record to NULL.
	 */

	method = NULL;
    }

    /*
     * Place the method structure in the class record. Note that we might not
     * immediately delete the destructor as this might be being done during
     * execution of the destructor itself. Also note that setting a
     * destructor during a destructor is fairly dumb anyway.
     */

    Tcl_ClassSetDestructor((Tcl_Class) clsPtr, method);
    return TCL_OK;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOODefineExportObjCmd --
 *	Implementation of the "export" subcommand of the "oo::define" and
 *	"oo::objdefine" commands.
 *
 * ----------------------------------------------------------------------
 */

int
TclOODefineExportObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    int isInstanceExport = (clientData != NULL);
    Object *oPtr;
    Method *mPtr;
    Tcl_HashEntry *hPtr;
    Class *clsPtr;
    int i, isNew;

    if (objc < 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "name ?name ...?");
	return TCL_ERROR;
    }

    oPtr = (Object *) TclOOGetDefineCmdContext(interp);
    if (oPtr == NULL) {
	return TCL_ERROR;
    }
    clsPtr = oPtr->classPtr;
    if (!isInstanceExport && !clsPtr) {
	Tcl_AppendResult(interp, "attempt to misuse API", NULL);
	return TCL_ERROR;
    }

    for (i=1 ; i<objc ; i++) {
	/*
	 * Exporting is done by adding the PUBLIC_METHOD flag to the method
	 * record. If there is no such method in this object or class (i.e.
	 * the method comes from something inherited from or that we're an
	 * instance of) then we put in a blank record with that flag; such
	 * records are skipped over by the call chain engine *except* for
	 * their flags member.
	 */

	if (isInstanceExport) {
	    if (!oPtr->methodsPtr) {
		oPtr->methodsPtr = (Tcl_HashTable *)
			ckalloc(sizeof(Tcl_HashTable));
		Tcl_InitObjHashTable(oPtr->methodsPtr);
	    }
	    hPtr = Tcl_CreateHashEntry(oPtr->methodsPtr, (char *) objv[i],
		    &isNew);
	} else {
	    hPtr = Tcl_CreateHashEntry(&clsPtr->classMethods, (char*) objv[i],
		    &isNew);
	}

	if (isNew) {
	    mPtr = (Method *) ckalloc(sizeof(Method));
	    memset(mPtr, 0, sizeof(Method));
	    Tcl_SetHashValue(hPtr, mPtr);
	} else {
	    mPtr = Tcl_GetHashValue(hPtr);
	}
	mPtr->flags |= PUBLIC_METHOD;
    }
    if (isInstanceExport) {
	oPtr->epoch++;
    } else {
	BumpGlobalEpoch(interp, clsPtr);
    }
    return TCL_OK;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOODefineFilterObjCmd --
 *	Implementation of the "filter" subcommand of the "oo::define" and
 *	"oo::objdefine" commands.
 *
 * ----------------------------------------------------------------------
 */

int
TclOODefineFilterObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    int isInstanceFilter = (clientData != NULL);
    Object *oPtr = (Object *) TclOOGetDefineCmdContext(interp);

    if (oPtr == NULL) {
	return TCL_ERROR;
    }
    if (!isInstanceFilter && !oPtr->classPtr) {
	Tcl_AppendResult(interp, "attempt to misuse API", NULL);
	return TCL_ERROR;
    }

    if (!isInstanceFilter) {
	TclOOClassSetFilters(interp, oPtr->classPtr, objc-1, objv+1);
    } else {
	TclOOObjectSetFilters(oPtr, objc-1, objv+1);
    }
    return TCL_OK;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOODefineForwardObjCmd --
 *	Implementation of the "forward" subcommand of the "oo::define" and
 *	"oo::objdefine" commands.
 *
 * ----------------------------------------------------------------------
 */

int
TclOODefineForwardObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    int isInstanceForward = (clientData != NULL);
    Object *oPtr;
    Method *mPtr;
    int isPublic;
    Tcl_Obj *prefixObj;

    if (objc < 3) {
	Tcl_WrongNumArgs(interp, 1, objv, "name cmdName ?arg ...?");
	return TCL_ERROR;
    }

    oPtr = (Object *) TclOOGetDefineCmdContext(interp);
    if (oPtr == NULL) {
	return TCL_ERROR;
    }
    if (!isInstanceForward && !oPtr->classPtr) {
	Tcl_AppendResult(interp, "attempt to misuse API", NULL);
	return TCL_ERROR;
    }
    isPublic = Tcl_StringMatch(TclGetString(objv[1]), "[a-z]*")
	    ? PUBLIC_METHOD : 0;

    /*
     * Create the method structure.
     */

    prefixObj = Tcl_NewListObj(objc-2, objv+2);
    if (isInstanceForward) {
	mPtr = TclOONewForwardInstanceMethod(interp, oPtr, isPublic, objv[1],
		prefixObj);
    } else {
	mPtr = TclOONewForwardMethod(interp, oPtr->classPtr, isPublic,
		objv[1], prefixObj);
    }
    if (mPtr == NULL) {
	Tcl_DecrRefCount(prefixObj);
	return TCL_ERROR;
    }
    return TCL_OK;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOODefineMethodObjCmd --
 *	Implementation of the "method" subcommand of the "oo::define" and
 *	"oo::objdefine" commands.
 *
 * ----------------------------------------------------------------------
 */

int
TclOODefineMethodObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    int isInstanceMethod = (clientData != NULL);
    Object *oPtr;
    int isPublic;

    if (objc != 4) {
	Tcl_WrongNumArgs(interp, 1, objv, "name args body");
	return TCL_ERROR;
    }

    oPtr = (Object *) TclOOGetDefineCmdContext(interp);
    if (oPtr == NULL) {
	return TCL_ERROR;
    }
    if (!isInstanceMethod && !oPtr->classPtr) {
	Tcl_AppendResult(interp, "attempt to misuse API", NULL);
	return TCL_ERROR;
    }
    isPublic = Tcl_StringMatch(TclGetString(objv[1]), "[a-z]*")
	    ? PUBLIC_METHOD : 0;

    /*
     * Create the method by using the right back-end API.
     */

    if (isInstanceMethod) {
	if (TclOONewProcInstanceMethod(interp, oPtr, isPublic, objv[1],
		objv[2], objv[3], NULL) == NULL) {
	    return TCL_ERROR;
	}
    } else {
	if (TclOONewProcMethod(interp, oPtr->classPtr, isPublic, objv[1],
		objv[2], objv[3], NULL) == NULL) {
	    return TCL_ERROR;
	}
    }
    return TCL_OK;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOODefineMixinObjCmd --
 *	Implementation of the "mixin" subcommand of the "oo::define" and
 *	"oo::objdefine" commands.
 *
 * ----------------------------------------------------------------------
 */

int
TclOODefineMixinObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    const int objc,
    Tcl_Obj *const *objv)
{
    int isInstanceMixin = (clientData != NULL);
    Object *oPtr = (Object *) TclOOGetDefineCmdContext(interp);
    Class **mixins;
    int i;

    if (oPtr == NULL) {
	return TCL_ERROR;
    }
    if (!isInstanceMixin && !oPtr->classPtr) {
	Tcl_AppendResult(interp, "attempt to misuse API", NULL);
	return TCL_ERROR;
    }
    mixins = TclStackAlloc(interp, sizeof(Class *) * (objc-1));

    for (i=1 ; i<objc ; i++) {
	Object *o2Ptr;

	o2Ptr = (Object *) Tcl_GetObjectFromObj(interp, objv[i]);
	if (o2Ptr == NULL) {
	    goto freeAndError;
	}
	if (o2Ptr->classPtr == NULL) {
	    Tcl_AppendResult(interp, "may only mix in classes; \"",
		    TclGetString(objv[i]), "\" is not a class", NULL);
	    goto freeAndError;
	}
	if (!isInstanceMixin &&
		TclOOIsReachable(oPtr->classPtr,o2Ptr->classPtr)){
	    Tcl_AppendResult(interp, "may not mix a class into itself", NULL);
	    goto freeAndError;
	}
	mixins[i-1] = o2Ptr->classPtr;
    }

    if (isInstanceMixin) {
	TclOOObjectSetMixins(oPtr, objc-1, mixins);
    } else {
	TclOOClassSetMixins(interp, oPtr->classPtr, objc-1, mixins);
    }

    TclStackFree(interp, mixins);
    return TCL_OK;

  freeAndError:
    TclStackFree(interp, mixins);
    return TCL_ERROR;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOODefineRenameMethodObjCmd --
 *	Implementation of the "renamemethod" subcommand of the "oo::define"
 *	and "oo::objdefine" commands.
 *
 * ----------------------------------------------------------------------
 */

int
TclOODefineRenameMethodObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    int isInstanceRenameMethod = (clientData != NULL);
    Object *oPtr;

    if (objc != 3) {
	Tcl_WrongNumArgs(interp, 1, objv, "oldName newName");
	return TCL_ERROR;
    }

    oPtr = (Object *) TclOOGetDefineCmdContext(interp);
    if (oPtr == NULL) {
	return TCL_ERROR;
    }
    if (!isInstanceRenameMethod && !oPtr->classPtr) {
	Tcl_AppendResult(interp, "attempt to misuse API", NULL);
	return TCL_ERROR;
    }

    /*
     * Delete the method entry from the appropriate hash table, and transfer
     * the thing it points to to its new entry. To do this, we first need to
     * get the entries from the appropriate hash tables (this can generate a
     * range of errors...)
     */

    if (RenameDeleteMethod(interp, oPtr, !isInstanceRenameMethod,
	    objv[1], objv[2]) != TCL_OK) {
	return TCL_ERROR;
    }

    if (isInstanceRenameMethod) {
	oPtr->epoch++;
    } else {
	BumpGlobalEpoch(interp, oPtr->classPtr);
    }
    return TCL_OK;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOODefineSuperclassObjCmd --
 *	Implementation of the "superclass" subcommand of the "oo::define"
 *	command.
 *
 * ----------------------------------------------------------------------
 */

int
TclOODefineSuperclassObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    Object *oPtr, *o2Ptr;
    Foundation *fPtr = TclOOGetFoundation(interp);
    Class **superclasses, *superPtr;
    int i, j;

    if (objc < 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "className ?className ...?");
	return TCL_ERROR;
    }

    /*
     * Get the class to operate on.
     */

    oPtr = (Object *) TclOOGetDefineCmdContext(interp);
    if (oPtr == NULL) {
	return TCL_ERROR;
    }
    if (oPtr->classPtr == NULL) {
	Tcl_AppendResult(interp, "only classes may have superclasses defined",
		NULL);
	return TCL_ERROR;
    }
    if (oPtr == fPtr->objectCls->thisPtr) {
	Tcl_AppendResult(interp,
		"may not modify the superclass of the root object", NULL);
	return TCL_ERROR;
    }

    /*
     * Allocate some working space.
     */

    superclasses = (Class **) ckalloc(sizeof(Class *) * (objc-1));

    /*
     * Parse the arguments to get the class to use as superclasses.
     */

    for (i=0 ; i<objc-1 ; i++) {
	o2Ptr = (Object *) Tcl_GetObjectFromObj(interp, objv[i+1]);
	if (o2Ptr == NULL) {
	    goto failedAfterAlloc;
	}
	if (o2Ptr->classPtr == NULL) {
	    Tcl_AppendResult(interp, "only a class can be a superclass",NULL);
	    goto failedAfterAlloc;
	}
	for (j=0 ; j<i ; j++) {
	    if (superclasses[j] == o2Ptr->classPtr) {
		Tcl_AppendResult(interp,
			"class should only be a direct superclass once",NULL);
		goto failedAfterAlloc;
	    }
	}
	if (TclOOIsReachable(oPtr->classPtr, o2Ptr->classPtr)) {
	    Tcl_AppendResult(interp,
		    "attempt to form circular dependency graph", NULL);
	failedAfterAlloc:
	    ckfree((char *) superclasses);
	    return TCL_ERROR;
	}
	superclasses[i] = o2Ptr->classPtr;
    }

    /*
     * Install the list of superclasses into the class. Note that this also
     * involves splicing the class out of the superclasses' subclass list that
     * it used to be a member of and splicing it into the new superclasses'
     * subclass list.
     */

    if (oPtr->classPtr->superclasses.num != 0) {
	FOREACH(superPtr, oPtr->classPtr->superclasses) {
	    TclOORemoveFromSubclasses(oPtr->classPtr, superPtr);
	}
	ckfree((char *) oPtr->classPtr->superclasses.list);
    }
    oPtr->classPtr->superclasses.list = superclasses;
    oPtr->classPtr->superclasses.num = objc-1;
    FOREACH(superPtr, oPtr->classPtr->superclasses) {
	TclOOAddToSubclasses(oPtr->classPtr, superPtr);
    }
    BumpGlobalEpoch(interp, oPtr->classPtr);

    return TCL_OK;
}

/*
 * ----------------------------------------------------------------------
 *
 * TclOODefineUnexportObjCmd --
 *	Implementation of the "unexport" subcommand of the "oo::define" and
 *	"oo::objdefine" commands.
 *
 * ----------------------------------------------------------------------
 */

int
TclOODefineUnexportObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const *objv)
{
    int isInstanceUnexport = (clientData != NULL);
    Object *oPtr;
    Method *mPtr;
    Tcl_HashEntry *hPtr;
    Class *clsPtr;
    int i, isNew;

    if (objc < 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "name ?name ...?");
	return TCL_ERROR;
    }

    oPtr = (Object *) TclOOGetDefineCmdContext(interp);
    if (oPtr == NULL) {
	return TCL_ERROR;
    }
    clsPtr = oPtr->classPtr;
    if (!isInstanceUnexport && !clsPtr) {
	Tcl_AppendResult(interp, "attempt to misuse API", NULL);
	return TCL_ERROR;
    }

    for (i=1 ; i<objc ; i++) {
	/*
	 * Unexporting is done by removing the PUBLIC_METHOD flag from the
	 * method record. If there is no such method in this object or class
	 * (i.e. the method comes from something inherited from or that we're
	 * an instance of) then we put in a blank record without that flag;
	 * such records are skipped over by the call chain engine *except* for
	 * their flags member.
	 */

	if (isInstanceUnexport) {
	    if (!oPtr->methodsPtr) {
		oPtr->methodsPtr = (Tcl_HashTable *)
			ckalloc(sizeof(Tcl_HashTable));
		Tcl_InitObjHashTable(oPtr->methodsPtr);
	    }
	    hPtr = Tcl_CreateHashEntry(oPtr->methodsPtr, (char *) objv[i],
		    &isNew);
	} else {
	    hPtr = Tcl_CreateHashEntry(&clsPtr->classMethods, (char*) objv[i],
		    &isNew);
	}

	if (isNew) {
	    mPtr = (Method *) ckalloc(sizeof(Method));
	    memset(mPtr, 0, sizeof(Method));
	    Tcl_SetHashValue(hPtr, mPtr);
	} else {
	    mPtr = Tcl_GetHashValue(hPtr);
	}
	mPtr->flags &= ~PUBLIC_METHOD;
    }
    if (isInstanceUnexport) {
	oPtr->epoch++;
    } else {
	BumpGlobalEpoch(interp, clsPtr);
    }
    return TCL_OK;
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
