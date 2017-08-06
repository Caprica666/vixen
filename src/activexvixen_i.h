

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Sun Dec 19 03:58:44 2010
 */
/* Compiler settings for ..\..\src\activex\vixen.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __activexvixen_i_h__
#define __activexvixen_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IVixenObj_FWD_DEFINED__
#define __IVixenObj_FWD_DEFINED__
typedef interface IVixenObj IVixenObj;
#endif 	/* __IVixenObj_FWD_DEFINED__ */


#ifndef __IVixenGroup_FWD_DEFINED__
#define __IVixenGroup_FWD_DEFINED__
typedef interface IVixenGroup IVixenGroup;
#endif 	/* __IVixenGroup_FWD_DEFINED__ */


#ifndef __IVixenModel_FWD_DEFINED__
#define __IVixenModel_FWD_DEFINED__
typedef interface IVixenModel IVixenModel;
#endif 	/* __IVixenModel_FWD_DEFINED__ */


#ifndef __IVixenImage_FWD_DEFINED__
#define __IVixenImage_FWD_DEFINED__
typedef interface IVixenImage IVixenImage;
#endif 	/* __IVixenImage_FWD_DEFINED__ */


#ifndef __IVixenMaterial_FWD_DEFINED__
#define __IVixenMaterial_FWD_DEFINED__
typedef interface IVixenMaterial IVixenMaterial;
#endif 	/* __IVixenMaterial_FWD_DEFINED__ */


#ifndef __IVixenAppearance_FWD_DEFINED__
#define __IVixenAppearance_FWD_DEFINED__
typedef interface IVixenAppearance IVixenAppearance;
#endif 	/* __IVixenAppearance_FWD_DEFINED__ */


#ifndef __IVixenEngine_FWD_DEFINED__
#define __IVixenEngine_FWD_DEFINED__
typedef interface IVixenEngine IVixenEngine;
#endif 	/* __IVixenEngine_FWD_DEFINED__ */


#ifndef __IVixenCamera_FWD_DEFINED__
#define __IVixenCamera_FWD_DEFINED__
typedef interface IVixenCamera IVixenCamera;
#endif 	/* __IVixenCamera_FWD_DEFINED__ */


#ifndef __IVixenScriptor_FWD_DEFINED__
#define __IVixenScriptor_FWD_DEFINED__
typedef interface IVixenScriptor IVixenScriptor;
#endif 	/* __IVixenScriptor_FWD_DEFINED__ */


#ifndef __IVixenTrackBall_FWD_DEFINED__
#define __IVixenTrackBall_FWD_DEFINED__
typedef interface IVixenTrackBall IVixenTrackBall;
#endif 	/* __IVixenTrackBall_FWD_DEFINED__ */


#ifndef __IVixenRayPicker_FWD_DEFINED__
#define __IVixenRayPicker_FWD_DEFINED__
typedef interface IVixenRayPicker IVixenRayPicker;
#endif 	/* __IVixenRayPicker_FWD_DEFINED__ */


#ifndef __IVixenNamePicker_FWD_DEFINED__
#define __IVixenNamePicker_FWD_DEFINED__
typedef interface IVixenNamePicker IVixenNamePicker;
#endif 	/* __IVixenNamePicker_FWD_DEFINED__ */


#ifndef __IVixenArcBall_FWD_DEFINED__
#define __IVixenArcBall_FWD_DEFINED__
typedef interface IVixenArcBall IVixenArcBall;
#endif 	/* __IVixenArcBall_FWD_DEFINED__ */


#ifndef __IVixenNavigator_FWD_DEFINED__
#define __IVixenNavigator_FWD_DEFINED__
typedef interface IVixenNavigator IVixenNavigator;
#endif 	/* __IVixenNavigator_FWD_DEFINED__ */


#ifndef __IVixenScene_FWD_DEFINED__
#define __IVixenScene_FWD_DEFINED__
typedef interface IVixenScene IVixenScene;
#endif 	/* __IVixenScene_FWD_DEFINED__ */


#ifndef __WorldEvents_FWD_DEFINED__
#define __WorldEvents_FWD_DEFINED__
typedef interface WorldEvents WorldEvents;
#endif 	/* __WorldEvents_FWD_DEFINED__ */


#ifndef __IVixenWorld_FWD_DEFINED__
#define __IVixenWorld_FWD_DEFINED__
typedef interface IVixenWorld IVixenWorld;
#endif 	/* __IVixenWorld_FWD_DEFINED__ */


#ifndef __VixenWorld_FWD_DEFINED__
#define __VixenWorld_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenWorld VixenWorld;
#else
typedef struct VixenWorld VixenWorld;
#endif /* __cplusplus */

#endif 	/* __VixenWorld_FWD_DEFINED__ */


#ifndef __WorldProp_FWD_DEFINED__
#define __WorldProp_FWD_DEFINED__

#ifdef __cplusplus
typedef class WorldProp WorldProp;
#else
typedef struct WorldProp WorldProp;
#endif /* __cplusplus */

#endif 	/* __WorldProp_FWD_DEFINED__ */


#ifndef __VixenObj_FWD_DEFINED__
#define __VixenObj_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenObj VixenObj;
#else
typedef struct VixenObj VixenObj;
#endif /* __cplusplus */

#endif 	/* __VixenObj_FWD_DEFINED__ */


#ifndef __VixenGroup_FWD_DEFINED__
#define __VixenGroup_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenGroup VixenGroup;
#else
typedef struct VixenGroup VixenGroup;
#endif /* __cplusplus */

#endif 	/* __VixenGroup_FWD_DEFINED__ */


#ifndef __VixenImage_FWD_DEFINED__
#define __VixenImage_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenImage VixenImage;
#else
typedef struct VixenImage VixenImage;
#endif /* __cplusplus */

#endif 	/* __VixenImage_FWD_DEFINED__ */


#ifndef __VixenMaterial_FWD_DEFINED__
#define __VixenMaterial_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenMaterial VixenMaterial;
#else
typedef struct VixenMaterial VixenMaterial;
#endif /* __cplusplus */

#endif 	/* __VixenMaterial_FWD_DEFINED__ */


#ifndef __VixenAppearance_FWD_DEFINED__
#define __VixenAppearance_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenAppearance VixenAppearance;
#else
typedef struct VixenAppearance VixenAppearance;
#endif /* __cplusplus */

#endif 	/* __VixenAppearance_FWD_DEFINED__ */


#ifndef __VixenModel_FWD_DEFINED__
#define __VixenModel_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenModel VixenModel;
#else
typedef struct VixenModel VixenModel;
#endif /* __cplusplus */

#endif 	/* __VixenModel_FWD_DEFINED__ */


#ifndef __VixenEngine_FWD_DEFINED__
#define __VixenEngine_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenEngine VixenEngine;
#else
typedef struct VixenEngine VixenEngine;
#endif /* __cplusplus */

#endif 	/* __VixenEngine_FWD_DEFINED__ */


#ifndef __VixenCamera_FWD_DEFINED__
#define __VixenCamera_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenCamera VixenCamera;
#else
typedef struct VixenCamera VixenCamera;
#endif /* __cplusplus */

#endif 	/* __VixenCamera_FWD_DEFINED__ */


#ifndef __VixenNavigator_FWD_DEFINED__
#define __VixenNavigator_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenNavigator VixenNavigator;
#else
typedef struct VixenNavigator VixenNavigator;
#endif /* __cplusplus */

#endif 	/* __VixenNavigator_FWD_DEFINED__ */


#ifndef __VixenArcBall_FWD_DEFINED__
#define __VixenArcBall_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenArcBall VixenArcBall;
#else
typedef struct VixenArcBall VixenArcBall;
#endif /* __cplusplus */

#endif 	/* __VixenArcBall_FWD_DEFINED__ */


#ifndef __VixenTrackBall_FWD_DEFINED__
#define __VixenTrackBall_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenTrackBall VixenTrackBall;
#else
typedef struct VixenTrackBall VixenTrackBall;
#endif /* __cplusplus */

#endif 	/* __VixenTrackBall_FWD_DEFINED__ */


#ifndef __VixenScriptor_FWD_DEFINED__
#define __VixenScriptor_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenScriptor VixenScriptor;
#else
typedef struct VixenScriptor VixenScriptor;
#endif /* __cplusplus */

#endif 	/* __VixenScriptor_FWD_DEFINED__ */


#ifndef __VixenRayPicker_FWD_DEFINED__
#define __VixenRayPicker_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenRayPicker VixenRayPicker;
#else
typedef struct VixenRayPicker VixenRayPicker;
#endif /* __cplusplus */

#endif 	/* __VixenRayPicker_FWD_DEFINED__ */


#ifndef __VixenNamePicker_FWD_DEFINED__
#define __VixenNamePicker_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenNamePicker VixenNamePicker;
#else
typedef struct VixenNamePicker VixenNamePicker;
#endif /* __cplusplus */

#endif 	/* __VixenNamePicker_FWD_DEFINED__ */


#ifndef __VixenScene_FWD_DEFINED__
#define __VixenScene_FWD_DEFINED__

#ifdef __cplusplus
typedef class VixenScene VixenScene;
#else
typedef struct VixenScene VixenScene;
#endif /* __cplusplus */

#endif 	/* __VixenScene_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __VixenLib_LIBRARY_DEFINED__
#define __VixenLib_LIBRARY_DEFINED__

/* library VixenLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_VixenLib;

#ifndef __IVixenObj_INTERFACE_DEFINED__
#define __IVixenObj_INTERFACE_DEFINED__

/* interface IVixenObj */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenObj;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C09BFB5D-4294-443F-9AC4-3932835E431E")
    IVixenObj : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VixenObj( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VixenObj( 
            /* [in] */ ULONGLONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Active( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Active( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            BSTR __MIDL__IVixenObj0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenObj0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IDispatch *__MIDL__IVixenObj0002) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ IDispatch **__MIDL__IVixenObj0003) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenObjVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenObj * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenObj * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenObj * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenObj * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenObj * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenObj * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenObj * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VixenObj )( 
            IVixenObj * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VixenObj )( 
            IVixenObj * This,
            /* [in] */ ULONGLONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVixenObj * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVixenObj * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenObj * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Active )( 
            IVixenObj * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Active )( 
            IVixenObj * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenObj * This,
            BSTR __MIDL__IVixenObj0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenObj0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IVixenObj * This,
            /* [in] */ IDispatch *__MIDL__IVixenObj0002);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVixenObj * This,
            /* [retval][out] */ IDispatch **__MIDL__IVixenObj0003);
        
        END_INTERFACE
    } IVixenObjVtbl;

    interface IVixenObj
    {
        CONST_VTBL struct IVixenObjVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenObj_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenObj_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenObj_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenObj_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenObj_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenObj_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenObj_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenObj_get_VixenObj(This,pVal)	\
    ( (This)->lpVtbl -> get_VixenObj(This,pVal) ) 

#define IVixenObj_put_VixenObj(This,newVal)	\
    ( (This)->lpVtbl -> put_VixenObj(This,newVal) ) 

#define IVixenObj_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IVixenObj_put_Name(This,pVal)	\
    ( (This)->lpVtbl -> put_Name(This,pVal) ) 

#define IVixenObj_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenObj_get_Active(This,pVal)	\
    ( (This)->lpVtbl -> get_Active(This,pVal) ) 

#define IVixenObj_put_Active(This,newVal)	\
    ( (This)->lpVtbl -> put_Active(This,newVal) ) 

#define IVixenObj_IsClass(This,__MIDL__IVixenObj0000,__MIDL__IVixenObj0001)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenObj0000,__MIDL__IVixenObj0001) ) 

#define IVixenObj_Copy(This,__MIDL__IVixenObj0002)	\
    ( (This)->lpVtbl -> Copy(This,__MIDL__IVixenObj0002) ) 

#define IVixenObj_Clone(This,__MIDL__IVixenObj0003)	\
    ( (This)->lpVtbl -> Clone(This,__MIDL__IVixenObj0003) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenObj_INTERFACE_DEFINED__ */


#ifndef __IVixenGroup_INTERFACE_DEFINED__
#define __IVixenGroup_INTERFACE_DEFINED__

/* interface IVixenGroup */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenGroup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("34B8C62E-50EC-4E82-A653-0F0D46F87405")
    IVixenGroup : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VixenObj( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VixenObj( 
            /* [in] */ ULONGLONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Active( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Active( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            BSTR __MIDL__IVixenGroup0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenGroup0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IDispatch *__MIDL__IVixenGroup0002) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ IDispatch **__MIDL__IVixenGroup0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_First( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Last( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAt( 
            long __MIDL__IVixenGroup0004,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumChildren( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Append( 
            IDispatch *__MIDL__IVixenGroup0005) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Empty( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Find( 
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsChild( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsParent( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenGroupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenGroup * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenGroup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenGroup * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenGroup * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenGroup * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenGroup * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VixenObj )( 
            IVixenGroup * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VixenObj )( 
            IVixenGroup * This,
            /* [in] */ ULONGLONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVixenGroup * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVixenGroup * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenGroup * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Active )( 
            IVixenGroup * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Active )( 
            IVixenGroup * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenGroup * This,
            BSTR __MIDL__IVixenGroup0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenGroup0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IVixenGroup * This,
            /* [in] */ IDispatch *__MIDL__IVixenGroup0002);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVixenGroup * This,
            /* [retval][out] */ IDispatch **__MIDL__IVixenGroup0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_First )( 
            IVixenGroup * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Last )( 
            IVixenGroup * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAt )( 
            IVixenGroup * This,
            long __MIDL__IVixenGroup0004,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IVixenGroup * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NumChildren )( 
            IVixenGroup * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Append )( 
            IVixenGroup * This,
            IDispatch *__MIDL__IVixenGroup0005);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IVixenGroup * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Empty )( 
            IVixenGroup * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Find )( 
            IVixenGroup * This,
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsChild )( 
            IVixenGroup * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsParent )( 
            IVixenGroup * This,
            /* [retval][out] */ BOOL *pVal);
        
        END_INTERFACE
    } IVixenGroupVtbl;

    interface IVixenGroup
    {
        CONST_VTBL struct IVixenGroupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenGroup_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenGroup_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenGroup_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenGroup_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenGroup_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenGroup_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenGroup_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenGroup_get_VixenObj(This,pVal)	\
    ( (This)->lpVtbl -> get_VixenObj(This,pVal) ) 

#define IVixenGroup_put_VixenObj(This,newVal)	\
    ( (This)->lpVtbl -> put_VixenObj(This,newVal) ) 

#define IVixenGroup_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IVixenGroup_put_Name(This,pVal)	\
    ( (This)->lpVtbl -> put_Name(This,pVal) ) 

#define IVixenGroup_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenGroup_get_Active(This,pVal)	\
    ( (This)->lpVtbl -> get_Active(This,pVal) ) 

#define IVixenGroup_put_Active(This,newVal)	\
    ( (This)->lpVtbl -> put_Active(This,newVal) ) 

#define IVixenGroup_IsClass(This,__MIDL__IVixenGroup0000,__MIDL__IVixenGroup0001)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenGroup0000,__MIDL__IVixenGroup0001) ) 

#define IVixenGroup_Copy(This,__MIDL__IVixenGroup0002)	\
    ( (This)->lpVtbl -> Copy(This,__MIDL__IVixenGroup0002) ) 

#define IVixenGroup_Clone(This,__MIDL__IVixenGroup0003)	\
    ( (This)->lpVtbl -> Clone(This,__MIDL__IVixenGroup0003) ) 

#define IVixenGroup_get_First(This,pVal)	\
    ( (This)->lpVtbl -> get_First(This,pVal) ) 

#define IVixenGroup_get_Last(This,pVal)	\
    ( (This)->lpVtbl -> get_Last(This,pVal) ) 

#define IVixenGroup_GetAt(This,__MIDL__IVixenGroup0004,pVal)	\
    ( (This)->lpVtbl -> GetAt(This,__MIDL__IVixenGroup0004,pVal) ) 

#define IVixenGroup_get_Parent(This,pVal)	\
    ( (This)->lpVtbl -> get_Parent(This,pVal) ) 

#define IVixenGroup_get_NumChildren(This,pVal)	\
    ( (This)->lpVtbl -> get_NumChildren(This,pVal) ) 

#define IVixenGroup_Append(This,__MIDL__IVixenGroup0005)	\
    ( (This)->lpVtbl -> Append(This,__MIDL__IVixenGroup0005) ) 

#define IVixenGroup_Remove(This)	\
    ( (This)->lpVtbl -> Remove(This) ) 

#define IVixenGroup_Empty(This)	\
    ( (This)->lpVtbl -> Empty(This) ) 

#define IVixenGroup_Find(This,s,opts,pVal)	\
    ( (This)->lpVtbl -> Find(This,s,opts,pVal) ) 

#define IVixenGroup_IsChild(This,pVal)	\
    ( (This)->lpVtbl -> IsChild(This,pVal) ) 

#define IVixenGroup_IsParent(This,pVal)	\
    ( (This)->lpVtbl -> IsParent(This,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenGroup_INTERFACE_DEFINED__ */


#ifndef __IVixenModel_INTERFACE_DEFINED__
#define __IVixenModel_INTERFACE_DEFINED__

/* interface IVixenModel */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenModel;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9FB8A877-51BE-4C67-82D8-5D86FBD4B607")
    IVixenModel : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VixenObj( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VixenObj( 
            /* [in] */ ULONGLONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Active( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Active( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            BSTR __MIDL__IVixenModel0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenModel0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IDispatch *__MIDL__IVixenModel0002) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ IDispatch **__MIDL__IVixenModel0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_First( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Last( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAt( 
            long __MIDL__IVixenModel0004,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumChildren( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Append( 
            IDispatch *__MIDL__IVixenModel0005) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Empty( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Find( 
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsChild( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsParent( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Translate( 
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Scale( 
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Rotate( 
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z,
            float angle) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Move( 
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Size( 
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Hints( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Hints( 
            /* [in] */ long newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenModelVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenModel * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenModel * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenModel * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenModel * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenModel * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenModel * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenModel * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VixenObj )( 
            IVixenModel * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VixenObj )( 
            IVixenModel * This,
            /* [in] */ ULONGLONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVixenModel * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVixenModel * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenModel * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Active )( 
            IVixenModel * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Active )( 
            IVixenModel * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenModel * This,
            BSTR __MIDL__IVixenModel0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenModel0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IVixenModel * This,
            /* [in] */ IDispatch *__MIDL__IVixenModel0002);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVixenModel * This,
            /* [retval][out] */ IDispatch **__MIDL__IVixenModel0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_First )( 
            IVixenModel * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Last )( 
            IVixenModel * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAt )( 
            IVixenModel * This,
            long __MIDL__IVixenModel0004,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IVixenModel * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NumChildren )( 
            IVixenModel * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Append )( 
            IVixenModel * This,
            IDispatch *__MIDL__IVixenModel0005);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IVixenModel * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Empty )( 
            IVixenModel * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Find )( 
            IVixenModel * This,
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsChild )( 
            IVixenModel * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsParent )( 
            IVixenModel * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Translate )( 
            IVixenModel * This,
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Scale )( 
            IVixenModel * This,
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Rotate )( 
            IVixenModel * This,
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z,
            float angle);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Move )( 
            IVixenModel * This,
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Size )( 
            IVixenModel * This,
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IVixenModel * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Hints )( 
            IVixenModel * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Hints )( 
            IVixenModel * This,
            /* [in] */ long newVal);
        
        END_INTERFACE
    } IVixenModelVtbl;

    interface IVixenModel
    {
        CONST_VTBL struct IVixenModelVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenModel_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenModel_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenModel_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenModel_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenModel_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenModel_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenModel_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenModel_get_VixenObj(This,pVal)	\
    ( (This)->lpVtbl -> get_VixenObj(This,pVal) ) 

#define IVixenModel_put_VixenObj(This,newVal)	\
    ( (This)->lpVtbl -> put_VixenObj(This,newVal) ) 

#define IVixenModel_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IVixenModel_put_Name(This,pVal)	\
    ( (This)->lpVtbl -> put_Name(This,pVal) ) 

#define IVixenModel_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenModel_get_Active(This,pVal)	\
    ( (This)->lpVtbl -> get_Active(This,pVal) ) 

#define IVixenModel_put_Active(This,newVal)	\
    ( (This)->lpVtbl -> put_Active(This,newVal) ) 

#define IVixenModel_IsClass(This,__MIDL__IVixenModel0000,__MIDL__IVixenModel0001)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenModel0000,__MIDL__IVixenModel0001) ) 

#define IVixenModel_Copy(This,__MIDL__IVixenModel0002)	\
    ( (This)->lpVtbl -> Copy(This,__MIDL__IVixenModel0002) ) 

#define IVixenModel_Clone(This,__MIDL__IVixenModel0003)	\
    ( (This)->lpVtbl -> Clone(This,__MIDL__IVixenModel0003) ) 

#define IVixenModel_get_First(This,pVal)	\
    ( (This)->lpVtbl -> get_First(This,pVal) ) 

#define IVixenModel_get_Last(This,pVal)	\
    ( (This)->lpVtbl -> get_Last(This,pVal) ) 

#define IVixenModel_GetAt(This,__MIDL__IVixenModel0004,pVal)	\
    ( (This)->lpVtbl -> GetAt(This,__MIDL__IVixenModel0004,pVal) ) 

#define IVixenModel_get_Parent(This,pVal)	\
    ( (This)->lpVtbl -> get_Parent(This,pVal) ) 

#define IVixenModel_get_NumChildren(This,pVal)	\
    ( (This)->lpVtbl -> get_NumChildren(This,pVal) ) 

#define IVixenModel_Append(This,__MIDL__IVixenModel0005)	\
    ( (This)->lpVtbl -> Append(This,__MIDL__IVixenModel0005) ) 

#define IVixenModel_Remove(This)	\
    ( (This)->lpVtbl -> Remove(This) ) 

#define IVixenModel_Empty(This)	\
    ( (This)->lpVtbl -> Empty(This) ) 

#define IVixenModel_Find(This,s,opts,pVal)	\
    ( (This)->lpVtbl -> Find(This,s,opts,pVal) ) 

#define IVixenModel_IsChild(This,pVal)	\
    ( (This)->lpVtbl -> IsChild(This,pVal) ) 

#define IVixenModel_IsParent(This,pVal)	\
    ( (This)->lpVtbl -> IsParent(This,pVal) ) 

#define IVixenModel_Translate(This,x,y,z)	\
    ( (This)->lpVtbl -> Translate(This,x,y,z) ) 

#define IVixenModel_Scale(This,x,y,z)	\
    ( (This)->lpVtbl -> Scale(This,x,y,z) ) 

#define IVixenModel_Rotate(This,x,y,z,angle)	\
    ( (This)->lpVtbl -> Rotate(This,x,y,z,angle) ) 

#define IVixenModel_Move(This,x,y,z)	\
    ( (This)->lpVtbl -> Move(This,x,y,z) ) 

#define IVixenModel_Size(This,x,y,z)	\
    ( (This)->lpVtbl -> Size(This,x,y,z) ) 

#define IVixenModel_Reset(This)	\
    ( (This)->lpVtbl -> Reset(This) ) 

#define IVixenModel_get_Hints(This,pVal)	\
    ( (This)->lpVtbl -> get_Hints(This,pVal) ) 

#define IVixenModel_put_Hints(This,newVal)	\
    ( (This)->lpVtbl -> put_Hints(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenModel_INTERFACE_DEFINED__ */


#ifndef __IVixenImage_INTERFACE_DEFINED__
#define __IVixenImage_INTERFACE_DEFINED__

/* interface IVixenImage */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenImage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EC99A915-0CB7-43A4-B9E7-7889CA0FE72D")
    IVixenImage : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VixenObj( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VixenObj( 
            /* [in] */ ULONGLONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Active( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Active( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            BSTR __MIDL__IVixenImage0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenImage0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IDispatch *__MIDL__IVixenImage0002) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ IDispatch **__MIDL__IVixenImage0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Width( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Width( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Height( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Height( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Depth( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Depth( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FileName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FileName( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Format( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Format( 
            /* [in] */ long newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenImageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenImage * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenImage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenImage * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenImage * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenImage * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenImage * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenImage * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VixenObj )( 
            IVixenImage * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VixenObj )( 
            IVixenImage * This,
            /* [in] */ ULONGLONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVixenImage * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVixenImage * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenImage * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Active )( 
            IVixenImage * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Active )( 
            IVixenImage * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenImage * This,
            BSTR __MIDL__IVixenImage0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenImage0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IVixenImage * This,
            /* [in] */ IDispatch *__MIDL__IVixenImage0002);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVixenImage * This,
            /* [retval][out] */ IDispatch **__MIDL__IVixenImage0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Width )( 
            IVixenImage * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Width )( 
            IVixenImage * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Height )( 
            IVixenImage * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Height )( 
            IVixenImage * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Depth )( 
            IVixenImage * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Depth )( 
            IVixenImage * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FileName )( 
            IVixenImage * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FileName )( 
            IVixenImage * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Format )( 
            IVixenImage * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Format )( 
            IVixenImage * This,
            /* [in] */ long newVal);
        
        END_INTERFACE
    } IVixenImageVtbl;

    interface IVixenImage
    {
        CONST_VTBL struct IVixenImageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenImage_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenImage_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenImage_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenImage_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenImage_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenImage_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenImage_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenImage_get_VixenObj(This,pVal)	\
    ( (This)->lpVtbl -> get_VixenObj(This,pVal) ) 

#define IVixenImage_put_VixenObj(This,newVal)	\
    ( (This)->lpVtbl -> put_VixenObj(This,newVal) ) 

#define IVixenImage_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IVixenImage_put_Name(This,pVal)	\
    ( (This)->lpVtbl -> put_Name(This,pVal) ) 

#define IVixenImage_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenImage_get_Active(This,pVal)	\
    ( (This)->lpVtbl -> get_Active(This,pVal) ) 

#define IVixenImage_put_Active(This,newVal)	\
    ( (This)->lpVtbl -> put_Active(This,newVal) ) 

#define IVixenImage_IsClass(This,__MIDL__IVixenImage0000,__MIDL__IVixenImage0001)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenImage0000,__MIDL__IVixenImage0001) ) 

#define IVixenImage_Copy(This,__MIDL__IVixenImage0002)	\
    ( (This)->lpVtbl -> Copy(This,__MIDL__IVixenImage0002) ) 

#define IVixenImage_Clone(This,__MIDL__IVixenImage0003)	\
    ( (This)->lpVtbl -> Clone(This,__MIDL__IVixenImage0003) ) 

#define IVixenImage_get_Width(This,pVal)	\
    ( (This)->lpVtbl -> get_Width(This,pVal) ) 

#define IVixenImage_put_Width(This,newVal)	\
    ( (This)->lpVtbl -> put_Width(This,newVal) ) 

#define IVixenImage_get_Height(This,pVal)	\
    ( (This)->lpVtbl -> get_Height(This,pVal) ) 

#define IVixenImage_put_Height(This,newVal)	\
    ( (This)->lpVtbl -> put_Height(This,newVal) ) 

#define IVixenImage_get_Depth(This,pVal)	\
    ( (This)->lpVtbl -> get_Depth(This,pVal) ) 

#define IVixenImage_put_Depth(This,newVal)	\
    ( (This)->lpVtbl -> put_Depth(This,newVal) ) 

#define IVixenImage_get_FileName(This,pVal)	\
    ( (This)->lpVtbl -> get_FileName(This,pVal) ) 

#define IVixenImage_put_FileName(This,newVal)	\
    ( (This)->lpVtbl -> put_FileName(This,newVal) ) 

#define IVixenImage_get_Format(This,pVal)	\
    ( (This)->lpVtbl -> get_Format(This,pVal) ) 

#define IVixenImage_put_Format(This,newVal)	\
    ( (This)->lpVtbl -> put_Format(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenImage_INTERFACE_DEFINED__ */


#ifndef __IVixenMaterial_INTERFACE_DEFINED__
#define __IVixenMaterial_INTERFACE_DEFINED__

/* interface IVixenMaterial */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenMaterial;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FDCB2CB3-36EF-4CA4-A6FE-CAF579BF2E96")
    IVixenMaterial : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VixenObj( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VixenObj( 
            /* [in] */ ULONGLONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Active( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Active( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            BSTR __MIDL__IVixenMaterial0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenMaterial0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IDispatch *__MIDL__IVixenMaterial0002) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ IDispatch **__MIDL__IVixenMaterial0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Diffuse( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Diffuse( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Specular( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Specular( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Ambient( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Ambient( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Emission( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Emission( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Shine( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Shine( 
            /* [in] */ float newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenMaterialVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenMaterial * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenMaterial * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenMaterial * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenMaterial * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenMaterial * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenMaterial * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenMaterial * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VixenObj )( 
            IVixenMaterial * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VixenObj )( 
            IVixenMaterial * This,
            /* [in] */ ULONGLONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVixenMaterial * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVixenMaterial * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenMaterial * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Active )( 
            IVixenMaterial * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Active )( 
            IVixenMaterial * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenMaterial * This,
            BSTR __MIDL__IVixenMaterial0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenMaterial0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IVixenMaterial * This,
            /* [in] */ IDispatch *__MIDL__IVixenMaterial0002);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVixenMaterial * This,
            /* [retval][out] */ IDispatch **__MIDL__IVixenMaterial0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Diffuse )( 
            IVixenMaterial * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Diffuse )( 
            IVixenMaterial * This,
            /* [in] */ OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Specular )( 
            IVixenMaterial * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Specular )( 
            IVixenMaterial * This,
            /* [in] */ OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Ambient )( 
            IVixenMaterial * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Ambient )( 
            IVixenMaterial * This,
            /* [in] */ OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Emission )( 
            IVixenMaterial * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Emission )( 
            IVixenMaterial * This,
            /* [in] */ OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Shine )( 
            IVixenMaterial * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Shine )( 
            IVixenMaterial * This,
            /* [in] */ float newVal);
        
        END_INTERFACE
    } IVixenMaterialVtbl;

    interface IVixenMaterial
    {
        CONST_VTBL struct IVixenMaterialVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenMaterial_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenMaterial_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenMaterial_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenMaterial_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenMaterial_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenMaterial_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenMaterial_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenMaterial_get_VixenObj(This,pVal)	\
    ( (This)->lpVtbl -> get_VixenObj(This,pVal) ) 

#define IVixenMaterial_put_VixenObj(This,newVal)	\
    ( (This)->lpVtbl -> put_VixenObj(This,newVal) ) 

#define IVixenMaterial_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IVixenMaterial_put_Name(This,pVal)	\
    ( (This)->lpVtbl -> put_Name(This,pVal) ) 

#define IVixenMaterial_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenMaterial_get_Active(This,pVal)	\
    ( (This)->lpVtbl -> get_Active(This,pVal) ) 

#define IVixenMaterial_put_Active(This,newVal)	\
    ( (This)->lpVtbl -> put_Active(This,newVal) ) 

#define IVixenMaterial_IsClass(This,__MIDL__IVixenMaterial0000,__MIDL__IVixenMaterial0001)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenMaterial0000,__MIDL__IVixenMaterial0001) ) 

#define IVixenMaterial_Copy(This,__MIDL__IVixenMaterial0002)	\
    ( (This)->lpVtbl -> Copy(This,__MIDL__IVixenMaterial0002) ) 

#define IVixenMaterial_Clone(This,__MIDL__IVixenMaterial0003)	\
    ( (This)->lpVtbl -> Clone(This,__MIDL__IVixenMaterial0003) ) 

#define IVixenMaterial_get_Diffuse(This,pVal)	\
    ( (This)->lpVtbl -> get_Diffuse(This,pVal) ) 

#define IVixenMaterial_put_Diffuse(This,newVal)	\
    ( (This)->lpVtbl -> put_Diffuse(This,newVal) ) 

#define IVixenMaterial_get_Specular(This,pVal)	\
    ( (This)->lpVtbl -> get_Specular(This,pVal) ) 

#define IVixenMaterial_put_Specular(This,newVal)	\
    ( (This)->lpVtbl -> put_Specular(This,newVal) ) 

#define IVixenMaterial_get_Ambient(This,pVal)	\
    ( (This)->lpVtbl -> get_Ambient(This,pVal) ) 

#define IVixenMaterial_put_Ambient(This,newVal)	\
    ( (This)->lpVtbl -> put_Ambient(This,newVal) ) 

#define IVixenMaterial_get_Emission(This,pVal)	\
    ( (This)->lpVtbl -> get_Emission(This,pVal) ) 

#define IVixenMaterial_put_Emission(This,newVal)	\
    ( (This)->lpVtbl -> put_Emission(This,newVal) ) 

#define IVixenMaterial_get_Shine(This,pVal)	\
    ( (This)->lpVtbl -> get_Shine(This,pVal) ) 

#define IVixenMaterial_put_Shine(This,newVal)	\
    ( (This)->lpVtbl -> put_Shine(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenMaterial_INTERFACE_DEFINED__ */


#ifndef __IVixenAppearance_INTERFACE_DEFINED__
#define __IVixenAppearance_INTERFACE_DEFINED__

/* interface IVixenAppearance */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenAppearance;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("43E25B24-C304-472D-8D1D-51E1C22E27EA")
    IVixenAppearance : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VixenObj( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VixenObj( 
            /* [in] */ ULONGLONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Active( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Active( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            BSTR __MIDL__IVixenAppearance0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenAppearance0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IDispatch *__MIDL__IVixenAppearance0002) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ IDispatch **__MIDL__IVixenAppearance0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Image( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Image( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Material( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Material( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Attr( 
            int index,
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Attr( 
            int index,
            /* [in] */ long newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenAppearanceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenAppearance * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenAppearance * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenAppearance * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenAppearance * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenAppearance * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenAppearance * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenAppearance * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VixenObj )( 
            IVixenAppearance * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VixenObj )( 
            IVixenAppearance * This,
            /* [in] */ ULONGLONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVixenAppearance * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVixenAppearance * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenAppearance * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Active )( 
            IVixenAppearance * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Active )( 
            IVixenAppearance * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenAppearance * This,
            BSTR __MIDL__IVixenAppearance0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenAppearance0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IVixenAppearance * This,
            /* [in] */ IDispatch *__MIDL__IVixenAppearance0002);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVixenAppearance * This,
            /* [retval][out] */ IDispatch **__MIDL__IVixenAppearance0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Image )( 
            IVixenAppearance * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Image )( 
            IVixenAppearance * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Material )( 
            IVixenAppearance * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Material )( 
            IVixenAppearance * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Attr )( 
            IVixenAppearance * This,
            int index,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Attr )( 
            IVixenAppearance * This,
            int index,
            /* [in] */ long newVal);
        
        END_INTERFACE
    } IVixenAppearanceVtbl;

    interface IVixenAppearance
    {
        CONST_VTBL struct IVixenAppearanceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenAppearance_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenAppearance_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenAppearance_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenAppearance_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenAppearance_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenAppearance_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenAppearance_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenAppearance_get_VixenObj(This,pVal)	\
    ( (This)->lpVtbl -> get_VixenObj(This,pVal) ) 

#define IVixenAppearance_put_VixenObj(This,newVal)	\
    ( (This)->lpVtbl -> put_VixenObj(This,newVal) ) 

#define IVixenAppearance_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IVixenAppearance_put_Name(This,pVal)	\
    ( (This)->lpVtbl -> put_Name(This,pVal) ) 

#define IVixenAppearance_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenAppearance_get_Active(This,pVal)	\
    ( (This)->lpVtbl -> get_Active(This,pVal) ) 

#define IVixenAppearance_put_Active(This,newVal)	\
    ( (This)->lpVtbl -> put_Active(This,newVal) ) 

#define IVixenAppearance_IsClass(This,__MIDL__IVixenAppearance0000,__MIDL__IVixenAppearance0001)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenAppearance0000,__MIDL__IVixenAppearance0001) ) 

#define IVixenAppearance_Copy(This,__MIDL__IVixenAppearance0002)	\
    ( (This)->lpVtbl -> Copy(This,__MIDL__IVixenAppearance0002) ) 

#define IVixenAppearance_Clone(This,__MIDL__IVixenAppearance0003)	\
    ( (This)->lpVtbl -> Clone(This,__MIDL__IVixenAppearance0003) ) 

#define IVixenAppearance_get_Image(This,pVal)	\
    ( (This)->lpVtbl -> get_Image(This,pVal) ) 

#define IVixenAppearance_put_Image(This,newVal)	\
    ( (This)->lpVtbl -> put_Image(This,newVal) ) 

#define IVixenAppearance_get_Material(This,pVal)	\
    ( (This)->lpVtbl -> get_Material(This,pVal) ) 

#define IVixenAppearance_put_Material(This,newVal)	\
    ( (This)->lpVtbl -> put_Material(This,newVal) ) 

#define IVixenAppearance_get_Attr(This,index,pVal)	\
    ( (This)->lpVtbl -> get_Attr(This,index,pVal) ) 

#define IVixenAppearance_put_Attr(This,index,newVal)	\
    ( (This)->lpVtbl -> put_Attr(This,index,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenAppearance_INTERFACE_DEFINED__ */


#ifndef __IVixenEngine_INTERFACE_DEFINED__
#define __IVixenEngine_INTERFACE_DEFINED__

/* interface IVixenEngine */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenEngine;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8B41BFE3-D167-4704-92F8-113B317B681B")
    IVixenEngine : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VixenObj( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VixenObj( 
            /* [in] */ ULONGLONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Active( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Active( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            BSTR __MIDL__IVixenEngine0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenEngine0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IDispatch *__MIDL__IVixenEngine0002) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ IDispatch **__MIDL__IVixenEngine0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_First( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Last( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAt( 
            long __MIDL__IVixenEngine0004,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumChildren( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Append( 
            IDispatch *__MIDL__IVixenEngine0005) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Empty( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Find( 
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsChild( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsParent( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Target( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Target( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Duration( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Duration( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimeOfs( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimeOfs( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StartTime( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StartTime( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Speed( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Speed( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Control( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Control( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_State( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsRunning( 
            /* [retval][out] */ BOOL *__MIDL__IVixenEngine0006) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenEngineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenEngine * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenEngine * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenEngine * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenEngine * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenEngine * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenEngine * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VixenObj )( 
            IVixenEngine * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VixenObj )( 
            IVixenEngine * This,
            /* [in] */ ULONGLONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVixenEngine * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVixenEngine * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenEngine * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Active )( 
            IVixenEngine * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Active )( 
            IVixenEngine * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenEngine * This,
            BSTR __MIDL__IVixenEngine0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenEngine0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IVixenEngine * This,
            /* [in] */ IDispatch *__MIDL__IVixenEngine0002);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVixenEngine * This,
            /* [retval][out] */ IDispatch **__MIDL__IVixenEngine0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_First )( 
            IVixenEngine * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Last )( 
            IVixenEngine * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAt )( 
            IVixenEngine * This,
            long __MIDL__IVixenEngine0004,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IVixenEngine * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NumChildren )( 
            IVixenEngine * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Append )( 
            IVixenEngine * This,
            IDispatch *__MIDL__IVixenEngine0005);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IVixenEngine * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Empty )( 
            IVixenEngine * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Find )( 
            IVixenEngine * This,
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsChild )( 
            IVixenEngine * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsParent )( 
            IVixenEngine * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Target )( 
            IVixenEngine * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Target )( 
            IVixenEngine * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            IVixenEngine * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Duration )( 
            IVixenEngine * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimeOfs )( 
            IVixenEngine * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimeOfs )( 
            IVixenEngine * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StartTime )( 
            IVixenEngine * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_StartTime )( 
            IVixenEngine * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Speed )( 
            IVixenEngine * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Speed )( 
            IVixenEngine * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Control )( 
            IVixenEngine * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Control )( 
            IVixenEngine * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IVixenEngine * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Start )( 
            IVixenEngine * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IVixenEngine * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsRunning )( 
            IVixenEngine * This,
            /* [retval][out] */ BOOL *__MIDL__IVixenEngine0006);
        
        END_INTERFACE
    } IVixenEngineVtbl;

    interface IVixenEngine
    {
        CONST_VTBL struct IVixenEngineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenEngine_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenEngine_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenEngine_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenEngine_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenEngine_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenEngine_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenEngine_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenEngine_get_VixenObj(This,pVal)	\
    ( (This)->lpVtbl -> get_VixenObj(This,pVal) ) 

#define IVixenEngine_put_VixenObj(This,newVal)	\
    ( (This)->lpVtbl -> put_VixenObj(This,newVal) ) 

#define IVixenEngine_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IVixenEngine_put_Name(This,pVal)	\
    ( (This)->lpVtbl -> put_Name(This,pVal) ) 

#define IVixenEngine_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenEngine_get_Active(This,pVal)	\
    ( (This)->lpVtbl -> get_Active(This,pVal) ) 

#define IVixenEngine_put_Active(This,newVal)	\
    ( (This)->lpVtbl -> put_Active(This,newVal) ) 

#define IVixenEngine_IsClass(This,__MIDL__IVixenEngine0000,__MIDL__IVixenEngine0001)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenEngine0000,__MIDL__IVixenEngine0001) ) 

#define IVixenEngine_Copy(This,__MIDL__IVixenEngine0002)	\
    ( (This)->lpVtbl -> Copy(This,__MIDL__IVixenEngine0002) ) 

#define IVixenEngine_Clone(This,__MIDL__IVixenEngine0003)	\
    ( (This)->lpVtbl -> Clone(This,__MIDL__IVixenEngine0003) ) 

#define IVixenEngine_get_First(This,pVal)	\
    ( (This)->lpVtbl -> get_First(This,pVal) ) 

#define IVixenEngine_get_Last(This,pVal)	\
    ( (This)->lpVtbl -> get_Last(This,pVal) ) 

#define IVixenEngine_GetAt(This,__MIDL__IVixenEngine0004,pVal)	\
    ( (This)->lpVtbl -> GetAt(This,__MIDL__IVixenEngine0004,pVal) ) 

#define IVixenEngine_get_Parent(This,pVal)	\
    ( (This)->lpVtbl -> get_Parent(This,pVal) ) 

#define IVixenEngine_get_NumChildren(This,pVal)	\
    ( (This)->lpVtbl -> get_NumChildren(This,pVal) ) 

#define IVixenEngine_Append(This,__MIDL__IVixenEngine0005)	\
    ( (This)->lpVtbl -> Append(This,__MIDL__IVixenEngine0005) ) 

#define IVixenEngine_Remove(This)	\
    ( (This)->lpVtbl -> Remove(This) ) 

#define IVixenEngine_Empty(This)	\
    ( (This)->lpVtbl -> Empty(This) ) 

#define IVixenEngine_Find(This,s,opts,pVal)	\
    ( (This)->lpVtbl -> Find(This,s,opts,pVal) ) 

#define IVixenEngine_IsChild(This,pVal)	\
    ( (This)->lpVtbl -> IsChild(This,pVal) ) 

#define IVixenEngine_IsParent(This,pVal)	\
    ( (This)->lpVtbl -> IsParent(This,pVal) ) 

#define IVixenEngine_get_Target(This,pVal)	\
    ( (This)->lpVtbl -> get_Target(This,pVal) ) 

#define IVixenEngine_put_Target(This,newVal)	\
    ( (This)->lpVtbl -> put_Target(This,newVal) ) 

#define IVixenEngine_get_Duration(This,pVal)	\
    ( (This)->lpVtbl -> get_Duration(This,pVal) ) 

#define IVixenEngine_put_Duration(This,newVal)	\
    ( (This)->lpVtbl -> put_Duration(This,newVal) ) 

#define IVixenEngine_get_TimeOfs(This,pVal)	\
    ( (This)->lpVtbl -> get_TimeOfs(This,pVal) ) 

#define IVixenEngine_put_TimeOfs(This,newVal)	\
    ( (This)->lpVtbl -> put_TimeOfs(This,newVal) ) 

#define IVixenEngine_get_StartTime(This,pVal)	\
    ( (This)->lpVtbl -> get_StartTime(This,pVal) ) 

#define IVixenEngine_put_StartTime(This,newVal)	\
    ( (This)->lpVtbl -> put_StartTime(This,newVal) ) 

#define IVixenEngine_get_Speed(This,pVal)	\
    ( (This)->lpVtbl -> get_Speed(This,pVal) ) 

#define IVixenEngine_put_Speed(This,newVal)	\
    ( (This)->lpVtbl -> put_Speed(This,newVal) ) 

#define IVixenEngine_get_Control(This,pVal)	\
    ( (This)->lpVtbl -> get_Control(This,pVal) ) 

#define IVixenEngine_put_Control(This,newVal)	\
    ( (This)->lpVtbl -> put_Control(This,newVal) ) 

#define IVixenEngine_get_State(This,pVal)	\
    ( (This)->lpVtbl -> get_State(This,pVal) ) 

#define IVixenEngine_Start(This)	\
    ( (This)->lpVtbl -> Start(This) ) 

#define IVixenEngine_Stop(This)	\
    ( (This)->lpVtbl -> Stop(This) ) 

#define IVixenEngine_IsRunning(This,__MIDL__IVixenEngine0006)	\
    ( (This)->lpVtbl -> IsRunning(This,__MIDL__IVixenEngine0006) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenEngine_INTERFACE_DEFINED__ */


#ifndef __IVixenCamera_INTERFACE_DEFINED__
#define __IVixenCamera_INTERFACE_DEFINED__

/* interface IVixenCamera */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenCamera;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("39961685-6CC7-4CB6-BA12-96686A92E8E3")
    IVixenCamera : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VixenObj( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VixenObj( 
            /* [in] */ ULONGLONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Active( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Active( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            BSTR __MIDL__IVixenCamera0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenCamera0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IDispatch *__MIDL__IVixenCamera0002) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ IDispatch **__MIDL__IVixenCamera0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_First( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Last( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAt( 
            long __MIDL__IVixenCamera0004,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumChildren( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Append( 
            IDispatch *__MIDL__IVixenCamera0005) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Empty( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Find( 
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsChild( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsParent( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Translate( 
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Scale( 
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Rotate( 
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z,
            float angle) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Move( 
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Size( 
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Hints( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Hints( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Hither( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Hither( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Yon( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Yon( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Aspect( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Aspect( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FieldOfView( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FieldOfView( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ViewVolume( 
            float left,
            float right,
            float top,
            float bottom,
            float hither,
            float yon) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenCameraVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenCamera * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenCamera * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenCamera * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenCamera * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenCamera * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenCamera * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenCamera * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VixenObj )( 
            IVixenCamera * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VixenObj )( 
            IVixenCamera * This,
            /* [in] */ ULONGLONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVixenCamera * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVixenCamera * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenCamera * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Active )( 
            IVixenCamera * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Active )( 
            IVixenCamera * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenCamera * This,
            BSTR __MIDL__IVixenCamera0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenCamera0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IVixenCamera * This,
            /* [in] */ IDispatch *__MIDL__IVixenCamera0002);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVixenCamera * This,
            /* [retval][out] */ IDispatch **__MIDL__IVixenCamera0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_First )( 
            IVixenCamera * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Last )( 
            IVixenCamera * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAt )( 
            IVixenCamera * This,
            long __MIDL__IVixenCamera0004,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IVixenCamera * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NumChildren )( 
            IVixenCamera * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Append )( 
            IVixenCamera * This,
            IDispatch *__MIDL__IVixenCamera0005);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IVixenCamera * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Empty )( 
            IVixenCamera * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Find )( 
            IVixenCamera * This,
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsChild )( 
            IVixenCamera * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsParent )( 
            IVixenCamera * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Translate )( 
            IVixenCamera * This,
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Scale )( 
            IVixenCamera * This,
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Rotate )( 
            IVixenCamera * This,
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z,
            float angle);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Move )( 
            IVixenCamera * This,
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Size )( 
            IVixenCamera * This,
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IVixenCamera * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Hints )( 
            IVixenCamera * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Hints )( 
            IVixenCamera * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Hither )( 
            IVixenCamera * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Hither )( 
            IVixenCamera * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Yon )( 
            IVixenCamera * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Yon )( 
            IVixenCamera * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Aspect )( 
            IVixenCamera * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Aspect )( 
            IVixenCamera * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FieldOfView )( 
            IVixenCamera * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FieldOfView )( 
            IVixenCamera * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ViewVolume )( 
            IVixenCamera * This,
            float left,
            float right,
            float top,
            float bottom,
            float hither,
            float yon);
        
        END_INTERFACE
    } IVixenCameraVtbl;

    interface IVixenCamera
    {
        CONST_VTBL struct IVixenCameraVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenCamera_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenCamera_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenCamera_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenCamera_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenCamera_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenCamera_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenCamera_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenCamera_get_VixenObj(This,pVal)	\
    ( (This)->lpVtbl -> get_VixenObj(This,pVal) ) 

#define IVixenCamera_put_VixenObj(This,newVal)	\
    ( (This)->lpVtbl -> put_VixenObj(This,newVal) ) 

#define IVixenCamera_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IVixenCamera_put_Name(This,pVal)	\
    ( (This)->lpVtbl -> put_Name(This,pVal) ) 

#define IVixenCamera_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenCamera_get_Active(This,pVal)	\
    ( (This)->lpVtbl -> get_Active(This,pVal) ) 

#define IVixenCamera_put_Active(This,newVal)	\
    ( (This)->lpVtbl -> put_Active(This,newVal) ) 

#define IVixenCamera_IsClass(This,__MIDL__IVixenCamera0000,__MIDL__IVixenCamera0001)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenCamera0000,__MIDL__IVixenCamera0001) ) 

#define IVixenCamera_Copy(This,__MIDL__IVixenCamera0002)	\
    ( (This)->lpVtbl -> Copy(This,__MIDL__IVixenCamera0002) ) 

#define IVixenCamera_Clone(This,__MIDL__IVixenCamera0003)	\
    ( (This)->lpVtbl -> Clone(This,__MIDL__IVixenCamera0003) ) 

#define IVixenCamera_get_First(This,pVal)	\
    ( (This)->lpVtbl -> get_First(This,pVal) ) 

#define IVixenCamera_get_Last(This,pVal)	\
    ( (This)->lpVtbl -> get_Last(This,pVal) ) 

#define IVixenCamera_GetAt(This,__MIDL__IVixenCamera0004,pVal)	\
    ( (This)->lpVtbl -> GetAt(This,__MIDL__IVixenCamera0004,pVal) ) 

#define IVixenCamera_get_Parent(This,pVal)	\
    ( (This)->lpVtbl -> get_Parent(This,pVal) ) 

#define IVixenCamera_get_NumChildren(This,pVal)	\
    ( (This)->lpVtbl -> get_NumChildren(This,pVal) ) 

#define IVixenCamera_Append(This,__MIDL__IVixenCamera0005)	\
    ( (This)->lpVtbl -> Append(This,__MIDL__IVixenCamera0005) ) 

#define IVixenCamera_Remove(This)	\
    ( (This)->lpVtbl -> Remove(This) ) 

#define IVixenCamera_Empty(This)	\
    ( (This)->lpVtbl -> Empty(This) ) 

#define IVixenCamera_Find(This,s,opts,pVal)	\
    ( (This)->lpVtbl -> Find(This,s,opts,pVal) ) 

#define IVixenCamera_IsChild(This,pVal)	\
    ( (This)->lpVtbl -> IsChild(This,pVal) ) 

#define IVixenCamera_IsParent(This,pVal)	\
    ( (This)->lpVtbl -> IsParent(This,pVal) ) 

#define IVixenCamera_Translate(This,x,y,z)	\
    ( (This)->lpVtbl -> Translate(This,x,y,z) ) 

#define IVixenCamera_Scale(This,x,y,z)	\
    ( (This)->lpVtbl -> Scale(This,x,y,z) ) 

#define IVixenCamera_Rotate(This,x,y,z,angle)	\
    ( (This)->lpVtbl -> Rotate(This,x,y,z,angle) ) 

#define IVixenCamera_Move(This,x,y,z)	\
    ( (This)->lpVtbl -> Move(This,x,y,z) ) 

#define IVixenCamera_Size(This,x,y,z)	\
    ( (This)->lpVtbl -> Size(This,x,y,z) ) 

#define IVixenCamera_Reset(This)	\
    ( (This)->lpVtbl -> Reset(This) ) 

#define IVixenCamera_get_Hints(This,pVal)	\
    ( (This)->lpVtbl -> get_Hints(This,pVal) ) 

#define IVixenCamera_put_Hints(This,newVal)	\
    ( (This)->lpVtbl -> put_Hints(This,newVal) ) 

#define IVixenCamera_get_Hither(This,pVal)	\
    ( (This)->lpVtbl -> get_Hither(This,pVal) ) 

#define IVixenCamera_put_Hither(This,newVal)	\
    ( (This)->lpVtbl -> put_Hither(This,newVal) ) 

#define IVixenCamera_get_Yon(This,pVal)	\
    ( (This)->lpVtbl -> get_Yon(This,pVal) ) 

#define IVixenCamera_put_Yon(This,newVal)	\
    ( (This)->lpVtbl -> put_Yon(This,newVal) ) 

#define IVixenCamera_get_Aspect(This,pVal)	\
    ( (This)->lpVtbl -> get_Aspect(This,pVal) ) 

#define IVixenCamera_put_Aspect(This,newVal)	\
    ( (This)->lpVtbl -> put_Aspect(This,newVal) ) 

#define IVixenCamera_get_FieldOfView(This,pVal)	\
    ( (This)->lpVtbl -> get_FieldOfView(This,pVal) ) 

#define IVixenCamera_put_FieldOfView(This,newVal)	\
    ( (This)->lpVtbl -> put_FieldOfView(This,newVal) ) 

#define IVixenCamera_ViewVolume(This,left,right,top,bottom,hither,yon)	\
    ( (This)->lpVtbl -> ViewVolume(This,left,right,top,bottom,hither,yon) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenCamera_INTERFACE_DEFINED__ */


#ifndef __IVixenScriptor_INTERFACE_DEFINED__
#define __IVixenScriptor_INTERFACE_DEFINED__

/* interface IVixenScriptor */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenScriptor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c9103b79-15dd-4f8e-80f6-83402307ccc5")
    IVixenScriptor : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VixenObj( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VixenObj( 
            /* [in] */ ULONGLONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Active( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Active( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            BSTR __MIDL__IVixenScriptor0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenScriptor0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IDispatch *__MIDL__IVixenScriptor0002) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ IDispatch **__MIDL__IVixenScriptor0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_First( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Last( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAt( 
            long __MIDL__IVixenScriptor0004,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumChildren( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Append( 
            IDispatch *__MIDL__IVixenScriptor0005) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Empty( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Find( 
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsChild( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsParent( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Target( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Target( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Duration( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Duration( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimeOfs( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimeOfs( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StartTime( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StartTime( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Speed( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Speed( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Control( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Control( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_State( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsRunning( 
            /* [retval][out] */ BOOL *__MIDL__IVixenScriptor0006) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Exec( 
            BSTR __MIDL__IVixenScriptor0007) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Kill( 
            BSTR __MIDL__IVixenScriptor0008) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Begin( 
            BSTR anim,
            BSTR follow,
            BSTR target) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE End( 
            BSTR __MIDL__IVixenScriptor0009) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenScriptorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenScriptor * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenScriptor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenScriptor * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenScriptor * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenScriptor * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenScriptor * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenScriptor * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VixenObj )( 
            IVixenScriptor * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VixenObj )( 
            IVixenScriptor * This,
            /* [in] */ ULONGLONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVixenScriptor * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVixenScriptor * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenScriptor * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Active )( 
            IVixenScriptor * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Active )( 
            IVixenScriptor * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenScriptor * This,
            BSTR __MIDL__IVixenScriptor0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenScriptor0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IVixenScriptor * This,
            /* [in] */ IDispatch *__MIDL__IVixenScriptor0002);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVixenScriptor * This,
            /* [retval][out] */ IDispatch **__MIDL__IVixenScriptor0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_First )( 
            IVixenScriptor * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Last )( 
            IVixenScriptor * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAt )( 
            IVixenScriptor * This,
            long __MIDL__IVixenScriptor0004,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IVixenScriptor * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NumChildren )( 
            IVixenScriptor * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Append )( 
            IVixenScriptor * This,
            IDispatch *__MIDL__IVixenScriptor0005);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IVixenScriptor * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Empty )( 
            IVixenScriptor * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Find )( 
            IVixenScriptor * This,
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsChild )( 
            IVixenScriptor * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsParent )( 
            IVixenScriptor * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Target )( 
            IVixenScriptor * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Target )( 
            IVixenScriptor * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            IVixenScriptor * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Duration )( 
            IVixenScriptor * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimeOfs )( 
            IVixenScriptor * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimeOfs )( 
            IVixenScriptor * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StartTime )( 
            IVixenScriptor * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_StartTime )( 
            IVixenScriptor * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Speed )( 
            IVixenScriptor * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Speed )( 
            IVixenScriptor * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Control )( 
            IVixenScriptor * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Control )( 
            IVixenScriptor * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IVixenScriptor * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Start )( 
            IVixenScriptor * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IVixenScriptor * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsRunning )( 
            IVixenScriptor * This,
            /* [retval][out] */ BOOL *__MIDL__IVixenScriptor0006);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Exec )( 
            IVixenScriptor * This,
            BSTR __MIDL__IVixenScriptor0007);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Kill )( 
            IVixenScriptor * This,
            BSTR __MIDL__IVixenScriptor0008);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Begin )( 
            IVixenScriptor * This,
            BSTR anim,
            BSTR follow,
            BSTR target);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *End )( 
            IVixenScriptor * This,
            BSTR __MIDL__IVixenScriptor0009);
        
        END_INTERFACE
    } IVixenScriptorVtbl;

    interface IVixenScriptor
    {
        CONST_VTBL struct IVixenScriptorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenScriptor_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenScriptor_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenScriptor_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenScriptor_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenScriptor_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenScriptor_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenScriptor_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenScriptor_get_VixenObj(This,pVal)	\
    ( (This)->lpVtbl -> get_VixenObj(This,pVal) ) 

#define IVixenScriptor_put_VixenObj(This,newVal)	\
    ( (This)->lpVtbl -> put_VixenObj(This,newVal) ) 

#define IVixenScriptor_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IVixenScriptor_put_Name(This,pVal)	\
    ( (This)->lpVtbl -> put_Name(This,pVal) ) 

#define IVixenScriptor_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenScriptor_get_Active(This,pVal)	\
    ( (This)->lpVtbl -> get_Active(This,pVal) ) 

#define IVixenScriptor_put_Active(This,newVal)	\
    ( (This)->lpVtbl -> put_Active(This,newVal) ) 

#define IVixenScriptor_IsClass(This,__MIDL__IVixenScriptor0000,__MIDL__IVixenScriptor0001)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenScriptor0000,__MIDL__IVixenScriptor0001) ) 

#define IVixenScriptor_Copy(This,__MIDL__IVixenScriptor0002)	\
    ( (This)->lpVtbl -> Copy(This,__MIDL__IVixenScriptor0002) ) 

#define IVixenScriptor_Clone(This,__MIDL__IVixenScriptor0003)	\
    ( (This)->lpVtbl -> Clone(This,__MIDL__IVixenScriptor0003) ) 

#define IVixenScriptor_get_First(This,pVal)	\
    ( (This)->lpVtbl -> get_First(This,pVal) ) 

#define IVixenScriptor_get_Last(This,pVal)	\
    ( (This)->lpVtbl -> get_Last(This,pVal) ) 

#define IVixenScriptor_GetAt(This,__MIDL__IVixenScriptor0004,pVal)	\
    ( (This)->lpVtbl -> GetAt(This,__MIDL__IVixenScriptor0004,pVal) ) 

#define IVixenScriptor_get_Parent(This,pVal)	\
    ( (This)->lpVtbl -> get_Parent(This,pVal) ) 

#define IVixenScriptor_get_NumChildren(This,pVal)	\
    ( (This)->lpVtbl -> get_NumChildren(This,pVal) ) 

#define IVixenScriptor_Append(This,__MIDL__IVixenScriptor0005)	\
    ( (This)->lpVtbl -> Append(This,__MIDL__IVixenScriptor0005) ) 

#define IVixenScriptor_Remove(This)	\
    ( (This)->lpVtbl -> Remove(This) ) 

#define IVixenScriptor_Empty(This)	\
    ( (This)->lpVtbl -> Empty(This) ) 

#define IVixenScriptor_Find(This,s,opts,pVal)	\
    ( (This)->lpVtbl -> Find(This,s,opts,pVal) ) 

#define IVixenScriptor_IsChild(This,pVal)	\
    ( (This)->lpVtbl -> IsChild(This,pVal) ) 

#define IVixenScriptor_IsParent(This,pVal)	\
    ( (This)->lpVtbl -> IsParent(This,pVal) ) 

#define IVixenScriptor_get_Target(This,pVal)	\
    ( (This)->lpVtbl -> get_Target(This,pVal) ) 

#define IVixenScriptor_put_Target(This,newVal)	\
    ( (This)->lpVtbl -> put_Target(This,newVal) ) 

#define IVixenScriptor_get_Duration(This,pVal)	\
    ( (This)->lpVtbl -> get_Duration(This,pVal) ) 

#define IVixenScriptor_put_Duration(This,newVal)	\
    ( (This)->lpVtbl -> put_Duration(This,newVal) ) 

#define IVixenScriptor_get_TimeOfs(This,pVal)	\
    ( (This)->lpVtbl -> get_TimeOfs(This,pVal) ) 

#define IVixenScriptor_put_TimeOfs(This,newVal)	\
    ( (This)->lpVtbl -> put_TimeOfs(This,newVal) ) 

#define IVixenScriptor_get_StartTime(This,pVal)	\
    ( (This)->lpVtbl -> get_StartTime(This,pVal) ) 

#define IVixenScriptor_put_StartTime(This,newVal)	\
    ( (This)->lpVtbl -> put_StartTime(This,newVal) ) 

#define IVixenScriptor_get_Speed(This,pVal)	\
    ( (This)->lpVtbl -> get_Speed(This,pVal) ) 

#define IVixenScriptor_put_Speed(This,newVal)	\
    ( (This)->lpVtbl -> put_Speed(This,newVal) ) 

#define IVixenScriptor_get_Control(This,pVal)	\
    ( (This)->lpVtbl -> get_Control(This,pVal) ) 

#define IVixenScriptor_put_Control(This,newVal)	\
    ( (This)->lpVtbl -> put_Control(This,newVal) ) 

#define IVixenScriptor_get_State(This,pVal)	\
    ( (This)->lpVtbl -> get_State(This,pVal) ) 

#define IVixenScriptor_Start(This)	\
    ( (This)->lpVtbl -> Start(This) ) 

#define IVixenScriptor_Stop(This)	\
    ( (This)->lpVtbl -> Stop(This) ) 

#define IVixenScriptor_IsRunning(This,__MIDL__IVixenScriptor0006)	\
    ( (This)->lpVtbl -> IsRunning(This,__MIDL__IVixenScriptor0006) ) 

#define IVixenScriptor_Exec(This,__MIDL__IVixenScriptor0007)	\
    ( (This)->lpVtbl -> Exec(This,__MIDL__IVixenScriptor0007) ) 

#define IVixenScriptor_Kill(This,__MIDL__IVixenScriptor0008)	\
    ( (This)->lpVtbl -> Kill(This,__MIDL__IVixenScriptor0008) ) 

#define IVixenScriptor_Begin(This,anim,follow,target)	\
    ( (This)->lpVtbl -> Begin(This,anim,follow,target) ) 

#define IVixenScriptor_End(This,__MIDL__IVixenScriptor0009)	\
    ( (This)->lpVtbl -> End(This,__MIDL__IVixenScriptor0009) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenScriptor_INTERFACE_DEFINED__ */


#ifndef __IVixenTrackBall_INTERFACE_DEFINED__
#define __IVixenTrackBall_INTERFACE_DEFINED__

/* interface IVixenTrackBall */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenTrackBall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D593AA6B-A10D-4602-873B-88FA632D0266")
    IVixenTrackBall : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VixenObj( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VixenObj( 
            /* [in] */ ULONGLONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Active( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Active( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            BSTR __MIDL__IVixenTrackBall0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenTrackBall0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IDispatch *__MIDL__IVixenTrackBall0002) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ IDispatch **__MIDL__IVixenTrackBall0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_First( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Last( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAt( 
            long __MIDL__IVixenTrackBall0004,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumChildren( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Append( 
            IDispatch *__MIDL__IVixenTrackBall0005) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Empty( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Find( 
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsChild( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsParent( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Target( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Target( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Duration( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Duration( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimeOfs( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimeOfs( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StartTime( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StartTime( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Speed( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Speed( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Control( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Control( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_State( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsRunning( 
            /* [retval][out] */ BOOL *__MIDL__IVixenTrackBall0006) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetButtons( 
            long track,
            long zoom) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenTrackBallVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenTrackBall * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenTrackBall * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenTrackBall * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenTrackBall * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenTrackBall * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenTrackBall * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenTrackBall * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VixenObj )( 
            IVixenTrackBall * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VixenObj )( 
            IVixenTrackBall * This,
            /* [in] */ ULONGLONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVixenTrackBall * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVixenTrackBall * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenTrackBall * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Active )( 
            IVixenTrackBall * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Active )( 
            IVixenTrackBall * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenTrackBall * This,
            BSTR __MIDL__IVixenTrackBall0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenTrackBall0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IVixenTrackBall * This,
            /* [in] */ IDispatch *__MIDL__IVixenTrackBall0002);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVixenTrackBall * This,
            /* [retval][out] */ IDispatch **__MIDL__IVixenTrackBall0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_First )( 
            IVixenTrackBall * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Last )( 
            IVixenTrackBall * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAt )( 
            IVixenTrackBall * This,
            long __MIDL__IVixenTrackBall0004,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IVixenTrackBall * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NumChildren )( 
            IVixenTrackBall * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Append )( 
            IVixenTrackBall * This,
            IDispatch *__MIDL__IVixenTrackBall0005);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IVixenTrackBall * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Empty )( 
            IVixenTrackBall * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Find )( 
            IVixenTrackBall * This,
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsChild )( 
            IVixenTrackBall * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsParent )( 
            IVixenTrackBall * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Target )( 
            IVixenTrackBall * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Target )( 
            IVixenTrackBall * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            IVixenTrackBall * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Duration )( 
            IVixenTrackBall * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimeOfs )( 
            IVixenTrackBall * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimeOfs )( 
            IVixenTrackBall * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StartTime )( 
            IVixenTrackBall * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_StartTime )( 
            IVixenTrackBall * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Speed )( 
            IVixenTrackBall * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Speed )( 
            IVixenTrackBall * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Control )( 
            IVixenTrackBall * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Control )( 
            IVixenTrackBall * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IVixenTrackBall * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Start )( 
            IVixenTrackBall * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IVixenTrackBall * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsRunning )( 
            IVixenTrackBall * This,
            /* [retval][out] */ BOOL *__MIDL__IVixenTrackBall0006);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetButtons )( 
            IVixenTrackBall * This,
            long track,
            long zoom);
        
        END_INTERFACE
    } IVixenTrackBallVtbl;

    interface IVixenTrackBall
    {
        CONST_VTBL struct IVixenTrackBallVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenTrackBall_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenTrackBall_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenTrackBall_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenTrackBall_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenTrackBall_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenTrackBall_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenTrackBall_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenTrackBall_get_VixenObj(This,pVal)	\
    ( (This)->lpVtbl -> get_VixenObj(This,pVal) ) 

#define IVixenTrackBall_put_VixenObj(This,newVal)	\
    ( (This)->lpVtbl -> put_VixenObj(This,newVal) ) 

#define IVixenTrackBall_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IVixenTrackBall_put_Name(This,pVal)	\
    ( (This)->lpVtbl -> put_Name(This,pVal) ) 

#define IVixenTrackBall_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenTrackBall_get_Active(This,pVal)	\
    ( (This)->lpVtbl -> get_Active(This,pVal) ) 

#define IVixenTrackBall_put_Active(This,newVal)	\
    ( (This)->lpVtbl -> put_Active(This,newVal) ) 

#define IVixenTrackBall_IsClass(This,__MIDL__IVixenTrackBall0000,__MIDL__IVixenTrackBall0001)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenTrackBall0000,__MIDL__IVixenTrackBall0001) ) 

#define IVixenTrackBall_Copy(This,__MIDL__IVixenTrackBall0002)	\
    ( (This)->lpVtbl -> Copy(This,__MIDL__IVixenTrackBall0002) ) 

#define IVixenTrackBall_Clone(This,__MIDL__IVixenTrackBall0003)	\
    ( (This)->lpVtbl -> Clone(This,__MIDL__IVixenTrackBall0003) ) 

#define IVixenTrackBall_get_First(This,pVal)	\
    ( (This)->lpVtbl -> get_First(This,pVal) ) 

#define IVixenTrackBall_get_Last(This,pVal)	\
    ( (This)->lpVtbl -> get_Last(This,pVal) ) 

#define IVixenTrackBall_GetAt(This,__MIDL__IVixenTrackBall0004,pVal)	\
    ( (This)->lpVtbl -> GetAt(This,__MIDL__IVixenTrackBall0004,pVal) ) 

#define IVixenTrackBall_get_Parent(This,pVal)	\
    ( (This)->lpVtbl -> get_Parent(This,pVal) ) 

#define IVixenTrackBall_get_NumChildren(This,pVal)	\
    ( (This)->lpVtbl -> get_NumChildren(This,pVal) ) 

#define IVixenTrackBall_Append(This,__MIDL__IVixenTrackBall0005)	\
    ( (This)->lpVtbl -> Append(This,__MIDL__IVixenTrackBall0005) ) 

#define IVixenTrackBall_Remove(This)	\
    ( (This)->lpVtbl -> Remove(This) ) 

#define IVixenTrackBall_Empty(This)	\
    ( (This)->lpVtbl -> Empty(This) ) 

#define IVixenTrackBall_Find(This,s,opts,pVal)	\
    ( (This)->lpVtbl -> Find(This,s,opts,pVal) ) 

#define IVixenTrackBall_IsChild(This,pVal)	\
    ( (This)->lpVtbl -> IsChild(This,pVal) ) 

#define IVixenTrackBall_IsParent(This,pVal)	\
    ( (This)->lpVtbl -> IsParent(This,pVal) ) 

#define IVixenTrackBall_get_Target(This,pVal)	\
    ( (This)->lpVtbl -> get_Target(This,pVal) ) 

#define IVixenTrackBall_put_Target(This,newVal)	\
    ( (This)->lpVtbl -> put_Target(This,newVal) ) 

#define IVixenTrackBall_get_Duration(This,pVal)	\
    ( (This)->lpVtbl -> get_Duration(This,pVal) ) 

#define IVixenTrackBall_put_Duration(This,newVal)	\
    ( (This)->lpVtbl -> put_Duration(This,newVal) ) 

#define IVixenTrackBall_get_TimeOfs(This,pVal)	\
    ( (This)->lpVtbl -> get_TimeOfs(This,pVal) ) 

#define IVixenTrackBall_put_TimeOfs(This,newVal)	\
    ( (This)->lpVtbl -> put_TimeOfs(This,newVal) ) 

#define IVixenTrackBall_get_StartTime(This,pVal)	\
    ( (This)->lpVtbl -> get_StartTime(This,pVal) ) 

#define IVixenTrackBall_put_StartTime(This,newVal)	\
    ( (This)->lpVtbl -> put_StartTime(This,newVal) ) 

#define IVixenTrackBall_get_Speed(This,pVal)	\
    ( (This)->lpVtbl -> get_Speed(This,pVal) ) 

#define IVixenTrackBall_put_Speed(This,newVal)	\
    ( (This)->lpVtbl -> put_Speed(This,newVal) ) 

#define IVixenTrackBall_get_Control(This,pVal)	\
    ( (This)->lpVtbl -> get_Control(This,pVal) ) 

#define IVixenTrackBall_put_Control(This,newVal)	\
    ( (This)->lpVtbl -> put_Control(This,newVal) ) 

#define IVixenTrackBall_get_State(This,pVal)	\
    ( (This)->lpVtbl -> get_State(This,pVal) ) 

#define IVixenTrackBall_Start(This)	\
    ( (This)->lpVtbl -> Start(This) ) 

#define IVixenTrackBall_Stop(This)	\
    ( (This)->lpVtbl -> Stop(This) ) 

#define IVixenTrackBall_IsRunning(This,__MIDL__IVixenTrackBall0006)	\
    ( (This)->lpVtbl -> IsRunning(This,__MIDL__IVixenTrackBall0006) ) 

#define IVixenTrackBall_SetButtons(This,track,zoom)	\
    ( (This)->lpVtbl -> SetButtons(This,track,zoom) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenTrackBall_INTERFACE_DEFINED__ */


#ifndef __IVixenRayPicker_INTERFACE_DEFINED__
#define __IVixenRayPicker_INTERFACE_DEFINED__

/* interface IVixenRayPicker */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenRayPicker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("688348A8-C410-499D-AFF5-557DADFB14D7")
    IVixenRayPicker : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VixenObj( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VixenObj( 
            /* [in] */ ULONGLONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Active( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Active( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            BSTR __MIDL__IVixenRayPicker0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenRayPicker0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IDispatch *__MIDL__IVixenRayPicker0002) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ IDispatch **__MIDL__IVixenRayPicker0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_First( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Last( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAt( 
            long __MIDL__IVixenRayPicker0004,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumChildren( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Append( 
            IDispatch *__MIDL__IVixenRayPicker0005) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Empty( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Find( 
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsChild( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsParent( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Target( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Target( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Duration( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Duration( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimeOfs( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimeOfs( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StartTime( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StartTime( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Speed( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Speed( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Control( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Control( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_State( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsRunning( 
            /* [retval][out] */ BOOL *__MIDL__IVixenRayPicker0006) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Options( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Options( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Nearest( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Buttons( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Buttons( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Hilite( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Hilite( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PickShape( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PickShape( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Selected( 
            /* [retval][out] */ SAFEARRAY * *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE UnHiliteAll( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenRayPickerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenRayPicker * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenRayPicker * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenRayPicker * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenRayPicker * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenRayPicker * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenRayPicker * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenRayPicker * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VixenObj )( 
            IVixenRayPicker * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VixenObj )( 
            IVixenRayPicker * This,
            /* [in] */ ULONGLONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVixenRayPicker * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVixenRayPicker * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenRayPicker * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Active )( 
            IVixenRayPicker * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Active )( 
            IVixenRayPicker * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenRayPicker * This,
            BSTR __MIDL__IVixenRayPicker0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenRayPicker0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IVixenRayPicker * This,
            /* [in] */ IDispatch *__MIDL__IVixenRayPicker0002);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVixenRayPicker * This,
            /* [retval][out] */ IDispatch **__MIDL__IVixenRayPicker0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_First )( 
            IVixenRayPicker * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Last )( 
            IVixenRayPicker * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAt )( 
            IVixenRayPicker * This,
            long __MIDL__IVixenRayPicker0004,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IVixenRayPicker * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NumChildren )( 
            IVixenRayPicker * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Append )( 
            IVixenRayPicker * This,
            IDispatch *__MIDL__IVixenRayPicker0005);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IVixenRayPicker * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Empty )( 
            IVixenRayPicker * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Find )( 
            IVixenRayPicker * This,
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsChild )( 
            IVixenRayPicker * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsParent )( 
            IVixenRayPicker * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Target )( 
            IVixenRayPicker * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Target )( 
            IVixenRayPicker * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            IVixenRayPicker * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Duration )( 
            IVixenRayPicker * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimeOfs )( 
            IVixenRayPicker * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimeOfs )( 
            IVixenRayPicker * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StartTime )( 
            IVixenRayPicker * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_StartTime )( 
            IVixenRayPicker * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Speed )( 
            IVixenRayPicker * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Speed )( 
            IVixenRayPicker * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Control )( 
            IVixenRayPicker * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Control )( 
            IVixenRayPicker * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IVixenRayPicker * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Start )( 
            IVixenRayPicker * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IVixenRayPicker * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsRunning )( 
            IVixenRayPicker * This,
            /* [retval][out] */ BOOL *__MIDL__IVixenRayPicker0006);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Options )( 
            IVixenRayPicker * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Options )( 
            IVixenRayPicker * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Nearest )( 
            IVixenRayPicker * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Buttons )( 
            IVixenRayPicker * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Buttons )( 
            IVixenRayPicker * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Hilite )( 
            IVixenRayPicker * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Hilite )( 
            IVixenRayPicker * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PickShape )( 
            IVixenRayPicker * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PickShape )( 
            IVixenRayPicker * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Selected )( 
            IVixenRayPicker * This,
            /* [retval][out] */ SAFEARRAY * *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *UnHiliteAll )( 
            IVixenRayPicker * This);
        
        END_INTERFACE
    } IVixenRayPickerVtbl;

    interface IVixenRayPicker
    {
        CONST_VTBL struct IVixenRayPickerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenRayPicker_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenRayPicker_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenRayPicker_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenRayPicker_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenRayPicker_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenRayPicker_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenRayPicker_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenRayPicker_get_VixenObj(This,pVal)	\
    ( (This)->lpVtbl -> get_VixenObj(This,pVal) ) 

#define IVixenRayPicker_put_VixenObj(This,newVal)	\
    ( (This)->lpVtbl -> put_VixenObj(This,newVal) ) 

#define IVixenRayPicker_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IVixenRayPicker_put_Name(This,pVal)	\
    ( (This)->lpVtbl -> put_Name(This,pVal) ) 

#define IVixenRayPicker_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenRayPicker_get_Active(This,pVal)	\
    ( (This)->lpVtbl -> get_Active(This,pVal) ) 

#define IVixenRayPicker_put_Active(This,newVal)	\
    ( (This)->lpVtbl -> put_Active(This,newVal) ) 

#define IVixenRayPicker_IsClass(This,__MIDL__IVixenRayPicker0000,__MIDL__IVixenRayPicker0001)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenRayPicker0000,__MIDL__IVixenRayPicker0001) ) 

#define IVixenRayPicker_Copy(This,__MIDL__IVixenRayPicker0002)	\
    ( (This)->lpVtbl -> Copy(This,__MIDL__IVixenRayPicker0002) ) 

#define IVixenRayPicker_Clone(This,__MIDL__IVixenRayPicker0003)	\
    ( (This)->lpVtbl -> Clone(This,__MIDL__IVixenRayPicker0003) ) 

#define IVixenRayPicker_get_First(This,pVal)	\
    ( (This)->lpVtbl -> get_First(This,pVal) ) 

#define IVixenRayPicker_get_Last(This,pVal)	\
    ( (This)->lpVtbl -> get_Last(This,pVal) ) 

#define IVixenRayPicker_GetAt(This,__MIDL__IVixenRayPicker0004,pVal)	\
    ( (This)->lpVtbl -> GetAt(This,__MIDL__IVixenRayPicker0004,pVal) ) 

#define IVixenRayPicker_get_Parent(This,pVal)	\
    ( (This)->lpVtbl -> get_Parent(This,pVal) ) 

#define IVixenRayPicker_get_NumChildren(This,pVal)	\
    ( (This)->lpVtbl -> get_NumChildren(This,pVal) ) 

#define IVixenRayPicker_Append(This,__MIDL__IVixenRayPicker0005)	\
    ( (This)->lpVtbl -> Append(This,__MIDL__IVixenRayPicker0005) ) 

#define IVixenRayPicker_Remove(This)	\
    ( (This)->lpVtbl -> Remove(This) ) 

#define IVixenRayPicker_Empty(This)	\
    ( (This)->lpVtbl -> Empty(This) ) 

#define IVixenRayPicker_Find(This,s,opts,pVal)	\
    ( (This)->lpVtbl -> Find(This,s,opts,pVal) ) 

#define IVixenRayPicker_IsChild(This,pVal)	\
    ( (This)->lpVtbl -> IsChild(This,pVal) ) 

#define IVixenRayPicker_IsParent(This,pVal)	\
    ( (This)->lpVtbl -> IsParent(This,pVal) ) 

#define IVixenRayPicker_get_Target(This,pVal)	\
    ( (This)->lpVtbl -> get_Target(This,pVal) ) 

#define IVixenRayPicker_put_Target(This,newVal)	\
    ( (This)->lpVtbl -> put_Target(This,newVal) ) 

#define IVixenRayPicker_get_Duration(This,pVal)	\
    ( (This)->lpVtbl -> get_Duration(This,pVal) ) 

#define IVixenRayPicker_put_Duration(This,newVal)	\
    ( (This)->lpVtbl -> put_Duration(This,newVal) ) 

#define IVixenRayPicker_get_TimeOfs(This,pVal)	\
    ( (This)->lpVtbl -> get_TimeOfs(This,pVal) ) 

#define IVixenRayPicker_put_TimeOfs(This,newVal)	\
    ( (This)->lpVtbl -> put_TimeOfs(This,newVal) ) 

#define IVixenRayPicker_get_StartTime(This,pVal)	\
    ( (This)->lpVtbl -> get_StartTime(This,pVal) ) 

#define IVixenRayPicker_put_StartTime(This,newVal)	\
    ( (This)->lpVtbl -> put_StartTime(This,newVal) ) 

#define IVixenRayPicker_get_Speed(This,pVal)	\
    ( (This)->lpVtbl -> get_Speed(This,pVal) ) 

#define IVixenRayPicker_put_Speed(This,newVal)	\
    ( (This)->lpVtbl -> put_Speed(This,newVal) ) 

#define IVixenRayPicker_get_Control(This,pVal)	\
    ( (This)->lpVtbl -> get_Control(This,pVal) ) 

#define IVixenRayPicker_put_Control(This,newVal)	\
    ( (This)->lpVtbl -> put_Control(This,newVal) ) 

#define IVixenRayPicker_get_State(This,pVal)	\
    ( (This)->lpVtbl -> get_State(This,pVal) ) 

#define IVixenRayPicker_Start(This)	\
    ( (This)->lpVtbl -> Start(This) ) 

#define IVixenRayPicker_Stop(This)	\
    ( (This)->lpVtbl -> Stop(This) ) 

#define IVixenRayPicker_IsRunning(This,__MIDL__IVixenRayPicker0006)	\
    ( (This)->lpVtbl -> IsRunning(This,__MIDL__IVixenRayPicker0006) ) 

#define IVixenRayPicker_get_Options(This,pVal)	\
    ( (This)->lpVtbl -> get_Options(This,pVal) ) 

#define IVixenRayPicker_put_Options(This,newVal)	\
    ( (This)->lpVtbl -> put_Options(This,newVal) ) 

#define IVixenRayPicker_get_Nearest(This,pVal)	\
    ( (This)->lpVtbl -> get_Nearest(This,pVal) ) 

#define IVixenRayPicker_get_Buttons(This,pVal)	\
    ( (This)->lpVtbl -> get_Buttons(This,pVal) ) 

#define IVixenRayPicker_put_Buttons(This,newVal)	\
    ( (This)->lpVtbl -> put_Buttons(This,newVal) ) 

#define IVixenRayPicker_get_Hilite(This,pVal)	\
    ( (This)->lpVtbl -> get_Hilite(This,pVal) ) 

#define IVixenRayPicker_put_Hilite(This,newVal)	\
    ( (This)->lpVtbl -> put_Hilite(This,newVal) ) 

#define IVixenRayPicker_get_PickShape(This,pVal)	\
    ( (This)->lpVtbl -> get_PickShape(This,pVal) ) 

#define IVixenRayPicker_put_PickShape(This,newVal)	\
    ( (This)->lpVtbl -> put_PickShape(This,newVal) ) 

#define IVixenRayPicker_get_Selected(This,pVal)	\
    ( (This)->lpVtbl -> get_Selected(This,pVal) ) 

#define IVixenRayPicker_UnHiliteAll(This)	\
    ( (This)->lpVtbl -> UnHiliteAll(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenRayPicker_INTERFACE_DEFINED__ */


#ifndef __IVixenNamePicker_INTERFACE_DEFINED__
#define __IVixenNamePicker_INTERFACE_DEFINED__

/* interface IVixenNamePicker */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenNamePicker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD6D579F-7ED8-4B2A-9CBB-53272CCAC22F")
    IVixenNamePicker : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VixenObj( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VixenObj( 
            /* [in] */ ULONGLONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Active( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Active( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            BSTR __MIDL__IVixenNamePicker0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenNamePicker0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IDispatch *__MIDL__IVixenNamePicker0002) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ IDispatch **__MIDL__IVixenNamePicker0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_First( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Last( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAt( 
            long __MIDL__IVixenNamePicker0004,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumChildren( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Append( 
            IDispatch *__MIDL__IVixenNamePicker0005) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Empty( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Find( 
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsChild( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsParent( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Target( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Target( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Duration( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Duration( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimeOfs( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimeOfs( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StartTime( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StartTime( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Speed( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Speed( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Control( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Control( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_State( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsRunning( 
            /* [retval][out] */ BOOL *__MIDL__IVixenNamePicker0006) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Options( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Options( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Nearest( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Buttons( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Buttons( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Hilite( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Hilite( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PickShape( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PickShape( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Selected( 
            /* [retval][out] */ SAFEARRAY * *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE UnHiliteAll( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NameFilter( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_NameFilter( 
            /* [in] */ BSTR newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenNamePickerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenNamePicker * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenNamePicker * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenNamePicker * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenNamePicker * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenNamePicker * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenNamePicker * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenNamePicker * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VixenObj )( 
            IVixenNamePicker * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VixenObj )( 
            IVixenNamePicker * This,
            /* [in] */ ULONGLONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVixenNamePicker * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVixenNamePicker * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenNamePicker * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Active )( 
            IVixenNamePicker * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Active )( 
            IVixenNamePicker * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenNamePicker * This,
            BSTR __MIDL__IVixenNamePicker0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenNamePicker0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IVixenNamePicker * This,
            /* [in] */ IDispatch *__MIDL__IVixenNamePicker0002);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVixenNamePicker * This,
            /* [retval][out] */ IDispatch **__MIDL__IVixenNamePicker0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_First )( 
            IVixenNamePicker * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Last )( 
            IVixenNamePicker * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAt )( 
            IVixenNamePicker * This,
            long __MIDL__IVixenNamePicker0004,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IVixenNamePicker * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NumChildren )( 
            IVixenNamePicker * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Append )( 
            IVixenNamePicker * This,
            IDispatch *__MIDL__IVixenNamePicker0005);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IVixenNamePicker * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Empty )( 
            IVixenNamePicker * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Find )( 
            IVixenNamePicker * This,
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsChild )( 
            IVixenNamePicker * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsParent )( 
            IVixenNamePicker * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Target )( 
            IVixenNamePicker * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Target )( 
            IVixenNamePicker * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            IVixenNamePicker * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Duration )( 
            IVixenNamePicker * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimeOfs )( 
            IVixenNamePicker * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimeOfs )( 
            IVixenNamePicker * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StartTime )( 
            IVixenNamePicker * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_StartTime )( 
            IVixenNamePicker * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Speed )( 
            IVixenNamePicker * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Speed )( 
            IVixenNamePicker * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Control )( 
            IVixenNamePicker * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Control )( 
            IVixenNamePicker * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IVixenNamePicker * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Start )( 
            IVixenNamePicker * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IVixenNamePicker * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsRunning )( 
            IVixenNamePicker * This,
            /* [retval][out] */ BOOL *__MIDL__IVixenNamePicker0006);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Options )( 
            IVixenNamePicker * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Options )( 
            IVixenNamePicker * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Nearest )( 
            IVixenNamePicker * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Buttons )( 
            IVixenNamePicker * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Buttons )( 
            IVixenNamePicker * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Hilite )( 
            IVixenNamePicker * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Hilite )( 
            IVixenNamePicker * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PickShape )( 
            IVixenNamePicker * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PickShape )( 
            IVixenNamePicker * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Selected )( 
            IVixenNamePicker * This,
            /* [retval][out] */ SAFEARRAY * *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *UnHiliteAll )( 
            IVixenNamePicker * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NameFilter )( 
            IVixenNamePicker * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_NameFilter )( 
            IVixenNamePicker * This,
            /* [in] */ BSTR newVal);
        
        END_INTERFACE
    } IVixenNamePickerVtbl;

    interface IVixenNamePicker
    {
        CONST_VTBL struct IVixenNamePickerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenNamePicker_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenNamePicker_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenNamePicker_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenNamePicker_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenNamePicker_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenNamePicker_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenNamePicker_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenNamePicker_get_VixenObj(This,pVal)	\
    ( (This)->lpVtbl -> get_VixenObj(This,pVal) ) 

#define IVixenNamePicker_put_VixenObj(This,newVal)	\
    ( (This)->lpVtbl -> put_VixenObj(This,newVal) ) 

#define IVixenNamePicker_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IVixenNamePicker_put_Name(This,pVal)	\
    ( (This)->lpVtbl -> put_Name(This,pVal) ) 

#define IVixenNamePicker_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenNamePicker_get_Active(This,pVal)	\
    ( (This)->lpVtbl -> get_Active(This,pVal) ) 

#define IVixenNamePicker_put_Active(This,newVal)	\
    ( (This)->lpVtbl -> put_Active(This,newVal) ) 

#define IVixenNamePicker_IsClass(This,__MIDL__IVixenNamePicker0000,__MIDL__IVixenNamePicker0001)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenNamePicker0000,__MIDL__IVixenNamePicker0001) ) 

#define IVixenNamePicker_Copy(This,__MIDL__IVixenNamePicker0002)	\
    ( (This)->lpVtbl -> Copy(This,__MIDL__IVixenNamePicker0002) ) 

#define IVixenNamePicker_Clone(This,__MIDL__IVixenNamePicker0003)	\
    ( (This)->lpVtbl -> Clone(This,__MIDL__IVixenNamePicker0003) ) 

#define IVixenNamePicker_get_First(This,pVal)	\
    ( (This)->lpVtbl -> get_First(This,pVal) ) 

#define IVixenNamePicker_get_Last(This,pVal)	\
    ( (This)->lpVtbl -> get_Last(This,pVal) ) 

#define IVixenNamePicker_GetAt(This,__MIDL__IVixenNamePicker0004,pVal)	\
    ( (This)->lpVtbl -> GetAt(This,__MIDL__IVixenNamePicker0004,pVal) ) 

#define IVixenNamePicker_get_Parent(This,pVal)	\
    ( (This)->lpVtbl -> get_Parent(This,pVal) ) 

#define IVixenNamePicker_get_NumChildren(This,pVal)	\
    ( (This)->lpVtbl -> get_NumChildren(This,pVal) ) 

#define IVixenNamePicker_Append(This,__MIDL__IVixenNamePicker0005)	\
    ( (This)->lpVtbl -> Append(This,__MIDL__IVixenNamePicker0005) ) 

#define IVixenNamePicker_Remove(This)	\
    ( (This)->lpVtbl -> Remove(This) ) 

#define IVixenNamePicker_Empty(This)	\
    ( (This)->lpVtbl -> Empty(This) ) 

#define IVixenNamePicker_Find(This,s,opts,pVal)	\
    ( (This)->lpVtbl -> Find(This,s,opts,pVal) ) 

#define IVixenNamePicker_IsChild(This,pVal)	\
    ( (This)->lpVtbl -> IsChild(This,pVal) ) 

#define IVixenNamePicker_IsParent(This,pVal)	\
    ( (This)->lpVtbl -> IsParent(This,pVal) ) 

#define IVixenNamePicker_get_Target(This,pVal)	\
    ( (This)->lpVtbl -> get_Target(This,pVal) ) 

#define IVixenNamePicker_put_Target(This,newVal)	\
    ( (This)->lpVtbl -> put_Target(This,newVal) ) 

#define IVixenNamePicker_get_Duration(This,pVal)	\
    ( (This)->lpVtbl -> get_Duration(This,pVal) ) 

#define IVixenNamePicker_put_Duration(This,newVal)	\
    ( (This)->lpVtbl -> put_Duration(This,newVal) ) 

#define IVixenNamePicker_get_TimeOfs(This,pVal)	\
    ( (This)->lpVtbl -> get_TimeOfs(This,pVal) ) 

#define IVixenNamePicker_put_TimeOfs(This,newVal)	\
    ( (This)->lpVtbl -> put_TimeOfs(This,newVal) ) 

#define IVixenNamePicker_get_StartTime(This,pVal)	\
    ( (This)->lpVtbl -> get_StartTime(This,pVal) ) 

#define IVixenNamePicker_put_StartTime(This,newVal)	\
    ( (This)->lpVtbl -> put_StartTime(This,newVal) ) 

#define IVixenNamePicker_get_Speed(This,pVal)	\
    ( (This)->lpVtbl -> get_Speed(This,pVal) ) 

#define IVixenNamePicker_put_Speed(This,newVal)	\
    ( (This)->lpVtbl -> put_Speed(This,newVal) ) 

#define IVixenNamePicker_get_Control(This,pVal)	\
    ( (This)->lpVtbl -> get_Control(This,pVal) ) 

#define IVixenNamePicker_put_Control(This,newVal)	\
    ( (This)->lpVtbl -> put_Control(This,newVal) ) 

#define IVixenNamePicker_get_State(This,pVal)	\
    ( (This)->lpVtbl -> get_State(This,pVal) ) 

#define IVixenNamePicker_Start(This)	\
    ( (This)->lpVtbl -> Start(This) ) 

#define IVixenNamePicker_Stop(This)	\
    ( (This)->lpVtbl -> Stop(This) ) 

#define IVixenNamePicker_IsRunning(This,__MIDL__IVixenNamePicker0006)	\
    ( (This)->lpVtbl -> IsRunning(This,__MIDL__IVixenNamePicker0006) ) 

#define IVixenNamePicker_get_Options(This,pVal)	\
    ( (This)->lpVtbl -> get_Options(This,pVal) ) 

#define IVixenNamePicker_put_Options(This,newVal)	\
    ( (This)->lpVtbl -> put_Options(This,newVal) ) 

#define IVixenNamePicker_get_Nearest(This,pVal)	\
    ( (This)->lpVtbl -> get_Nearest(This,pVal) ) 

#define IVixenNamePicker_get_Buttons(This,pVal)	\
    ( (This)->lpVtbl -> get_Buttons(This,pVal) ) 

#define IVixenNamePicker_put_Buttons(This,newVal)	\
    ( (This)->lpVtbl -> put_Buttons(This,newVal) ) 

#define IVixenNamePicker_get_Hilite(This,pVal)	\
    ( (This)->lpVtbl -> get_Hilite(This,pVal) ) 

#define IVixenNamePicker_put_Hilite(This,newVal)	\
    ( (This)->lpVtbl -> put_Hilite(This,newVal) ) 

#define IVixenNamePicker_get_PickShape(This,pVal)	\
    ( (This)->lpVtbl -> get_PickShape(This,pVal) ) 

#define IVixenNamePicker_put_PickShape(This,newVal)	\
    ( (This)->lpVtbl -> put_PickShape(This,newVal) ) 

#define IVixenNamePicker_get_Selected(This,pVal)	\
    ( (This)->lpVtbl -> get_Selected(This,pVal) ) 

#define IVixenNamePicker_UnHiliteAll(This)	\
    ( (This)->lpVtbl -> UnHiliteAll(This) ) 

#define IVixenNamePicker_get_NameFilter(This,pVal)	\
    ( (This)->lpVtbl -> get_NameFilter(This,pVal) ) 

#define IVixenNamePicker_put_NameFilter(This,newVal)	\
    ( (This)->lpVtbl -> put_NameFilter(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenNamePicker_INTERFACE_DEFINED__ */


#ifndef __IVixenArcBall_INTERFACE_DEFINED__
#define __IVixenArcBall_INTERFACE_DEFINED__

/* interface IVixenArcBall */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenArcBall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("30183A69-779C-4E7A-B6FF-D257E9BD7714")
    IVixenArcBall : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VixenObj( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VixenObj( 
            /* [in] */ ULONGLONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Active( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Active( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            BSTR __MIDL__IVixenArcBall0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenArcBall0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IDispatch *__MIDL__IVixenArcBall0002) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ IDispatch **__MIDL__IVixenArcBall0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_First( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Last( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAt( 
            long __MIDL__IVixenArcBall0004,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumChildren( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Append( 
            IDispatch *__MIDL__IVixenArcBall0005) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Empty( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Find( 
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsChild( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsParent( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Target( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Target( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Duration( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Duration( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimeOfs( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimeOfs( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StartTime( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StartTime( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Speed( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Speed( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Control( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Control( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_State( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsRunning( 
            /* [retval][out] */ BOOL *__MIDL__IVixenArcBall0006) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Button( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Button( 
            /* [in] */ long newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenArcBallVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenArcBall * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenArcBall * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenArcBall * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenArcBall * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenArcBall * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenArcBall * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenArcBall * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VixenObj )( 
            IVixenArcBall * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VixenObj )( 
            IVixenArcBall * This,
            /* [in] */ ULONGLONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVixenArcBall * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVixenArcBall * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenArcBall * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Active )( 
            IVixenArcBall * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Active )( 
            IVixenArcBall * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenArcBall * This,
            BSTR __MIDL__IVixenArcBall0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenArcBall0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IVixenArcBall * This,
            /* [in] */ IDispatch *__MIDL__IVixenArcBall0002);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVixenArcBall * This,
            /* [retval][out] */ IDispatch **__MIDL__IVixenArcBall0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_First )( 
            IVixenArcBall * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Last )( 
            IVixenArcBall * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAt )( 
            IVixenArcBall * This,
            long __MIDL__IVixenArcBall0004,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IVixenArcBall * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NumChildren )( 
            IVixenArcBall * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Append )( 
            IVixenArcBall * This,
            IDispatch *__MIDL__IVixenArcBall0005);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IVixenArcBall * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Empty )( 
            IVixenArcBall * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Find )( 
            IVixenArcBall * This,
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsChild )( 
            IVixenArcBall * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsParent )( 
            IVixenArcBall * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Target )( 
            IVixenArcBall * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Target )( 
            IVixenArcBall * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            IVixenArcBall * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Duration )( 
            IVixenArcBall * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimeOfs )( 
            IVixenArcBall * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimeOfs )( 
            IVixenArcBall * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StartTime )( 
            IVixenArcBall * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_StartTime )( 
            IVixenArcBall * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Speed )( 
            IVixenArcBall * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Speed )( 
            IVixenArcBall * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Control )( 
            IVixenArcBall * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Control )( 
            IVixenArcBall * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IVixenArcBall * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Start )( 
            IVixenArcBall * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IVixenArcBall * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsRunning )( 
            IVixenArcBall * This,
            /* [retval][out] */ BOOL *__MIDL__IVixenArcBall0006);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Button )( 
            IVixenArcBall * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Button )( 
            IVixenArcBall * This,
            /* [in] */ long newVal);
        
        END_INTERFACE
    } IVixenArcBallVtbl;

    interface IVixenArcBall
    {
        CONST_VTBL struct IVixenArcBallVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenArcBall_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenArcBall_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenArcBall_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenArcBall_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenArcBall_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenArcBall_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenArcBall_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenArcBall_get_VixenObj(This,pVal)	\
    ( (This)->lpVtbl -> get_VixenObj(This,pVal) ) 

#define IVixenArcBall_put_VixenObj(This,newVal)	\
    ( (This)->lpVtbl -> put_VixenObj(This,newVal) ) 

#define IVixenArcBall_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IVixenArcBall_put_Name(This,pVal)	\
    ( (This)->lpVtbl -> put_Name(This,pVal) ) 

#define IVixenArcBall_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenArcBall_get_Active(This,pVal)	\
    ( (This)->lpVtbl -> get_Active(This,pVal) ) 

#define IVixenArcBall_put_Active(This,newVal)	\
    ( (This)->lpVtbl -> put_Active(This,newVal) ) 

#define IVixenArcBall_IsClass(This,__MIDL__IVixenArcBall0000,__MIDL__IVixenArcBall0001)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenArcBall0000,__MIDL__IVixenArcBall0001) ) 

#define IVixenArcBall_Copy(This,__MIDL__IVixenArcBall0002)	\
    ( (This)->lpVtbl -> Copy(This,__MIDL__IVixenArcBall0002) ) 

#define IVixenArcBall_Clone(This,__MIDL__IVixenArcBall0003)	\
    ( (This)->lpVtbl -> Clone(This,__MIDL__IVixenArcBall0003) ) 

#define IVixenArcBall_get_First(This,pVal)	\
    ( (This)->lpVtbl -> get_First(This,pVal) ) 

#define IVixenArcBall_get_Last(This,pVal)	\
    ( (This)->lpVtbl -> get_Last(This,pVal) ) 

#define IVixenArcBall_GetAt(This,__MIDL__IVixenArcBall0004,pVal)	\
    ( (This)->lpVtbl -> GetAt(This,__MIDL__IVixenArcBall0004,pVal) ) 

#define IVixenArcBall_get_Parent(This,pVal)	\
    ( (This)->lpVtbl -> get_Parent(This,pVal) ) 

#define IVixenArcBall_get_NumChildren(This,pVal)	\
    ( (This)->lpVtbl -> get_NumChildren(This,pVal) ) 

#define IVixenArcBall_Append(This,__MIDL__IVixenArcBall0005)	\
    ( (This)->lpVtbl -> Append(This,__MIDL__IVixenArcBall0005) ) 

#define IVixenArcBall_Remove(This)	\
    ( (This)->lpVtbl -> Remove(This) ) 

#define IVixenArcBall_Empty(This)	\
    ( (This)->lpVtbl -> Empty(This) ) 

#define IVixenArcBall_Find(This,s,opts,pVal)	\
    ( (This)->lpVtbl -> Find(This,s,opts,pVal) ) 

#define IVixenArcBall_IsChild(This,pVal)	\
    ( (This)->lpVtbl -> IsChild(This,pVal) ) 

#define IVixenArcBall_IsParent(This,pVal)	\
    ( (This)->lpVtbl -> IsParent(This,pVal) ) 

#define IVixenArcBall_get_Target(This,pVal)	\
    ( (This)->lpVtbl -> get_Target(This,pVal) ) 

#define IVixenArcBall_put_Target(This,newVal)	\
    ( (This)->lpVtbl -> put_Target(This,newVal) ) 

#define IVixenArcBall_get_Duration(This,pVal)	\
    ( (This)->lpVtbl -> get_Duration(This,pVal) ) 

#define IVixenArcBall_put_Duration(This,newVal)	\
    ( (This)->lpVtbl -> put_Duration(This,newVal) ) 

#define IVixenArcBall_get_TimeOfs(This,pVal)	\
    ( (This)->lpVtbl -> get_TimeOfs(This,pVal) ) 

#define IVixenArcBall_put_TimeOfs(This,newVal)	\
    ( (This)->lpVtbl -> put_TimeOfs(This,newVal) ) 

#define IVixenArcBall_get_StartTime(This,pVal)	\
    ( (This)->lpVtbl -> get_StartTime(This,pVal) ) 

#define IVixenArcBall_put_StartTime(This,newVal)	\
    ( (This)->lpVtbl -> put_StartTime(This,newVal) ) 

#define IVixenArcBall_get_Speed(This,pVal)	\
    ( (This)->lpVtbl -> get_Speed(This,pVal) ) 

#define IVixenArcBall_put_Speed(This,newVal)	\
    ( (This)->lpVtbl -> put_Speed(This,newVal) ) 

#define IVixenArcBall_get_Control(This,pVal)	\
    ( (This)->lpVtbl -> get_Control(This,pVal) ) 

#define IVixenArcBall_put_Control(This,newVal)	\
    ( (This)->lpVtbl -> put_Control(This,newVal) ) 

#define IVixenArcBall_get_State(This,pVal)	\
    ( (This)->lpVtbl -> get_State(This,pVal) ) 

#define IVixenArcBall_Start(This)	\
    ( (This)->lpVtbl -> Start(This) ) 

#define IVixenArcBall_Stop(This)	\
    ( (This)->lpVtbl -> Stop(This) ) 

#define IVixenArcBall_IsRunning(This,__MIDL__IVixenArcBall0006)	\
    ( (This)->lpVtbl -> IsRunning(This,__MIDL__IVixenArcBall0006) ) 

#define IVixenArcBall_get_Button(This,pVal)	\
    ( (This)->lpVtbl -> get_Button(This,pVal) ) 

#define IVixenArcBall_put_Button(This,newVal)	\
    ( (This)->lpVtbl -> put_Button(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenArcBall_INTERFACE_DEFINED__ */


#ifndef __IVixenNavigator_INTERFACE_DEFINED__
#define __IVixenNavigator_INTERFACE_DEFINED__

/* interface IVixenNavigator */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenNavigator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("658C1ADB-4CE3-42DF-8005-623F2B90CA0F")
    IVixenNavigator : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VixenObj( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VixenObj( 
            /* [in] */ ULONGLONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Active( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Active( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            BSTR __MIDL__IVixenNavigator0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenNavigator0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IDispatch *__MIDL__IVixenNavigator0002) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ IDispatch **__MIDL__IVixenNavigator0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_First( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Last( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetAt( 
            long __MIDL__IVixenNavigator0004,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Parent( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_NumChildren( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Append( 
            IDispatch *__MIDL__IVixenNavigator0005) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Remove( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Empty( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Find( 
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsChild( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsParent( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Target( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Target( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Duration( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Duration( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimeOfs( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimeOfs( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_StartTime( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_StartTime( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Speed( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Speed( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Control( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Control( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_State( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsRunning( 
            /* [retval][out] */ BOOL *__MIDL__IVixenNavigator0006) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Buttons( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Buttons( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TurnSpeed( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TurnSpeed( 
            /* [in] */ float newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenNavigatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenNavigator * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenNavigator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenNavigator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenNavigator * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenNavigator * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenNavigator * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenNavigator * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VixenObj )( 
            IVixenNavigator * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VixenObj )( 
            IVixenNavigator * This,
            /* [in] */ ULONGLONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVixenNavigator * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVixenNavigator * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenNavigator * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Active )( 
            IVixenNavigator * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Active )( 
            IVixenNavigator * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenNavigator * This,
            BSTR __MIDL__IVixenNavigator0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenNavigator0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IVixenNavigator * This,
            /* [in] */ IDispatch *__MIDL__IVixenNavigator0002);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVixenNavigator * This,
            /* [retval][out] */ IDispatch **__MIDL__IVixenNavigator0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_First )( 
            IVixenNavigator * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Last )( 
            IVixenNavigator * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetAt )( 
            IVixenNavigator * This,
            long __MIDL__IVixenNavigator0004,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Parent )( 
            IVixenNavigator * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_NumChildren )( 
            IVixenNavigator * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Append )( 
            IVixenNavigator * This,
            IDispatch *__MIDL__IVixenNavigator0005);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IVixenNavigator * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Empty )( 
            IVixenNavigator * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Find )( 
            IVixenNavigator * This,
            /* [in] */ BSTR s,
            /* [in] */ long opts,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsChild )( 
            IVixenNavigator * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsParent )( 
            IVixenNavigator * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Target )( 
            IVixenNavigator * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Target )( 
            IVixenNavigator * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Duration )( 
            IVixenNavigator * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Duration )( 
            IVixenNavigator * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimeOfs )( 
            IVixenNavigator * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimeOfs )( 
            IVixenNavigator * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_StartTime )( 
            IVixenNavigator * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_StartTime )( 
            IVixenNavigator * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Speed )( 
            IVixenNavigator * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Speed )( 
            IVixenNavigator * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Control )( 
            IVixenNavigator * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Control )( 
            IVixenNavigator * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IVixenNavigator * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Start )( 
            IVixenNavigator * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IVixenNavigator * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsRunning )( 
            IVixenNavigator * This,
            /* [retval][out] */ BOOL *__MIDL__IVixenNavigator0006);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Buttons )( 
            IVixenNavigator * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Buttons )( 
            IVixenNavigator * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TurnSpeed )( 
            IVixenNavigator * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TurnSpeed )( 
            IVixenNavigator * This,
            /* [in] */ float newVal);
        
        END_INTERFACE
    } IVixenNavigatorVtbl;

    interface IVixenNavigator
    {
        CONST_VTBL struct IVixenNavigatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenNavigator_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenNavigator_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenNavigator_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenNavigator_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenNavigator_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenNavigator_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenNavigator_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenNavigator_get_VixenObj(This,pVal)	\
    ( (This)->lpVtbl -> get_VixenObj(This,pVal) ) 

#define IVixenNavigator_put_VixenObj(This,newVal)	\
    ( (This)->lpVtbl -> put_VixenObj(This,newVal) ) 

#define IVixenNavigator_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IVixenNavigator_put_Name(This,pVal)	\
    ( (This)->lpVtbl -> put_Name(This,pVal) ) 

#define IVixenNavigator_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenNavigator_get_Active(This,pVal)	\
    ( (This)->lpVtbl -> get_Active(This,pVal) ) 

#define IVixenNavigator_put_Active(This,newVal)	\
    ( (This)->lpVtbl -> put_Active(This,newVal) ) 

#define IVixenNavigator_IsClass(This,__MIDL__IVixenNavigator0000,__MIDL__IVixenNavigator0001)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenNavigator0000,__MIDL__IVixenNavigator0001) ) 

#define IVixenNavigator_Copy(This,__MIDL__IVixenNavigator0002)	\
    ( (This)->lpVtbl -> Copy(This,__MIDL__IVixenNavigator0002) ) 

#define IVixenNavigator_Clone(This,__MIDL__IVixenNavigator0003)	\
    ( (This)->lpVtbl -> Clone(This,__MIDL__IVixenNavigator0003) ) 

#define IVixenNavigator_get_First(This,pVal)	\
    ( (This)->lpVtbl -> get_First(This,pVal) ) 

#define IVixenNavigator_get_Last(This,pVal)	\
    ( (This)->lpVtbl -> get_Last(This,pVal) ) 

#define IVixenNavigator_GetAt(This,__MIDL__IVixenNavigator0004,pVal)	\
    ( (This)->lpVtbl -> GetAt(This,__MIDL__IVixenNavigator0004,pVal) ) 

#define IVixenNavigator_get_Parent(This,pVal)	\
    ( (This)->lpVtbl -> get_Parent(This,pVal) ) 

#define IVixenNavigator_get_NumChildren(This,pVal)	\
    ( (This)->lpVtbl -> get_NumChildren(This,pVal) ) 

#define IVixenNavigator_Append(This,__MIDL__IVixenNavigator0005)	\
    ( (This)->lpVtbl -> Append(This,__MIDL__IVixenNavigator0005) ) 

#define IVixenNavigator_Remove(This)	\
    ( (This)->lpVtbl -> Remove(This) ) 

#define IVixenNavigator_Empty(This)	\
    ( (This)->lpVtbl -> Empty(This) ) 

#define IVixenNavigator_Find(This,s,opts,pVal)	\
    ( (This)->lpVtbl -> Find(This,s,opts,pVal) ) 

#define IVixenNavigator_IsChild(This,pVal)	\
    ( (This)->lpVtbl -> IsChild(This,pVal) ) 

#define IVixenNavigator_IsParent(This,pVal)	\
    ( (This)->lpVtbl -> IsParent(This,pVal) ) 

#define IVixenNavigator_get_Target(This,pVal)	\
    ( (This)->lpVtbl -> get_Target(This,pVal) ) 

#define IVixenNavigator_put_Target(This,newVal)	\
    ( (This)->lpVtbl -> put_Target(This,newVal) ) 

#define IVixenNavigator_get_Duration(This,pVal)	\
    ( (This)->lpVtbl -> get_Duration(This,pVal) ) 

#define IVixenNavigator_put_Duration(This,newVal)	\
    ( (This)->lpVtbl -> put_Duration(This,newVal) ) 

#define IVixenNavigator_get_TimeOfs(This,pVal)	\
    ( (This)->lpVtbl -> get_TimeOfs(This,pVal) ) 

#define IVixenNavigator_put_TimeOfs(This,newVal)	\
    ( (This)->lpVtbl -> put_TimeOfs(This,newVal) ) 

#define IVixenNavigator_get_StartTime(This,pVal)	\
    ( (This)->lpVtbl -> get_StartTime(This,pVal) ) 

#define IVixenNavigator_put_StartTime(This,newVal)	\
    ( (This)->lpVtbl -> put_StartTime(This,newVal) ) 

#define IVixenNavigator_get_Speed(This,pVal)	\
    ( (This)->lpVtbl -> get_Speed(This,pVal) ) 

#define IVixenNavigator_put_Speed(This,newVal)	\
    ( (This)->lpVtbl -> put_Speed(This,newVal) ) 

#define IVixenNavigator_get_Control(This,pVal)	\
    ( (This)->lpVtbl -> get_Control(This,pVal) ) 

#define IVixenNavigator_put_Control(This,newVal)	\
    ( (This)->lpVtbl -> put_Control(This,newVal) ) 

#define IVixenNavigator_get_State(This,pVal)	\
    ( (This)->lpVtbl -> get_State(This,pVal) ) 

#define IVixenNavigator_Start(This)	\
    ( (This)->lpVtbl -> Start(This) ) 

#define IVixenNavigator_Stop(This)	\
    ( (This)->lpVtbl -> Stop(This) ) 

#define IVixenNavigator_IsRunning(This,__MIDL__IVixenNavigator0006)	\
    ( (This)->lpVtbl -> IsRunning(This,__MIDL__IVixenNavigator0006) ) 

#define IVixenNavigator_get_Buttons(This,pVal)	\
    ( (This)->lpVtbl -> get_Buttons(This,pVal) ) 

#define IVixenNavigator_put_Buttons(This,newVal)	\
    ( (This)->lpVtbl -> put_Buttons(This,newVal) ) 

#define IVixenNavigator_get_TurnSpeed(This,pVal)	\
    ( (This)->lpVtbl -> get_TurnSpeed(This,pVal) ) 

#define IVixenNavigator_put_TurnSpeed(This,newVal)	\
    ( (This)->lpVtbl -> put_TurnSpeed(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenNavigator_INTERFACE_DEFINED__ */


#ifndef __IVixenScene_INTERFACE_DEFINED__
#define __IVixenScene_INTERFACE_DEFINED__

/* interface IVixenScene */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenScene;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("82810AFA-3164-4715-8F1D-6470F46C34ED")
    IVixenScene : public IDispatch
    {
    public:
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VixenObj( 
            /* [retval][out] */ ULONGLONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VixenObj( 
            /* [in] */ ULONGLONG newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Name( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Name( 
            /* [in] */ BSTR pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Active( 
            /* [retval][out] */ BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Active( 
            /* [in] */ BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            BSTR __MIDL__IVixenScene0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenScene0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Copy( 
            /* [in] */ IDispatch *__MIDL__IVixenScene0002) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [retval][out] */ IDispatch **__MIDL__IVixenScene0003) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_BackColor( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_BackColor( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Ambient( 
            /* [retval][out] */ OLE_COLOR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Ambient( 
            /* [in] */ OLE_COLOR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Options( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Options( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EnableOptions( 
            long __MIDL__IVixenScene0004) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DisableOptions( 
            long __MIDL__IVixenScene0005) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_TimeInc( 
            /* [retval][out] */ float *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_TimeInc( 
            /* [in] */ float newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ShowAll( void) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Priority( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Models( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Models( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Camera( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Camera( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Engines( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Engines( 
            /* [in] */ IDispatch *newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenSceneVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenScene * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenScene * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenScene * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenScene * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenScene * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenScene * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenScene * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_VixenObj )( 
            IVixenScene * This,
            /* [retval][out] */ ULONGLONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_VixenObj )( 
            IVixenScene * This,
            /* [in] */ ULONGLONG newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IVixenScene * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IVixenScene * This,
            /* [in] */ BSTR pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenScene * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Active )( 
            IVixenScene * This,
            /* [retval][out] */ BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Active )( 
            IVixenScene * This,
            /* [in] */ BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenScene * This,
            BSTR __MIDL__IVixenScene0000,
            /* [retval][out] */ BOOL *__MIDL__IVixenScene0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Copy )( 
            IVixenScene * This,
            /* [in] */ IDispatch *__MIDL__IVixenScene0002);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IVixenScene * This,
            /* [retval][out] */ IDispatch **__MIDL__IVixenScene0003);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_BackColor )( 
            IVixenScene * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_BackColor )( 
            IVixenScene * This,
            /* [in] */ OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Ambient )( 
            IVixenScene * This,
            /* [retval][out] */ OLE_COLOR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Ambient )( 
            IVixenScene * This,
            /* [in] */ OLE_COLOR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Options )( 
            IVixenScene * This,
            /* [retval][out] */ long *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Options )( 
            IVixenScene * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EnableOptions )( 
            IVixenScene * This,
            long __MIDL__IVixenScene0004);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DisableOptions )( 
            IVixenScene * This,
            long __MIDL__IVixenScene0005);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_TimeInc )( 
            IVixenScene * This,
            /* [retval][out] */ float *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_TimeInc )( 
            IVixenScene * This,
            /* [in] */ float newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ShowAll )( 
            IVixenScene * This);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Priority )( 
            IVixenScene * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Models )( 
            IVixenScene * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Models )( 
            IVixenScene * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Camera )( 
            IVixenScene * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Camera )( 
            IVixenScene * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Engines )( 
            IVixenScene * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Engines )( 
            IVixenScene * This,
            /* [in] */ IDispatch *newVal);
        
        END_INTERFACE
    } IVixenSceneVtbl;

    interface IVixenScene
    {
        CONST_VTBL struct IVixenSceneVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenScene_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenScene_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenScene_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenScene_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenScene_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenScene_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenScene_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenScene_get_VixenObj(This,pVal)	\
    ( (This)->lpVtbl -> get_VixenObj(This,pVal) ) 

#define IVixenScene_put_VixenObj(This,newVal)	\
    ( (This)->lpVtbl -> put_VixenObj(This,newVal) ) 

#define IVixenScene_get_Name(This,pVal)	\
    ( (This)->lpVtbl -> get_Name(This,pVal) ) 

#define IVixenScene_put_Name(This,pVal)	\
    ( (This)->lpVtbl -> put_Name(This,pVal) ) 

#define IVixenScene_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenScene_get_Active(This,pVal)	\
    ( (This)->lpVtbl -> get_Active(This,pVal) ) 

#define IVixenScene_put_Active(This,newVal)	\
    ( (This)->lpVtbl -> put_Active(This,newVal) ) 

#define IVixenScene_IsClass(This,__MIDL__IVixenScene0000,__MIDL__IVixenScene0001)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenScene0000,__MIDL__IVixenScene0001) ) 

#define IVixenScene_Copy(This,__MIDL__IVixenScene0002)	\
    ( (This)->lpVtbl -> Copy(This,__MIDL__IVixenScene0002) ) 

#define IVixenScene_Clone(This,__MIDL__IVixenScene0003)	\
    ( (This)->lpVtbl -> Clone(This,__MIDL__IVixenScene0003) ) 

#define IVixenScene_get_BackColor(This,pVal)	\
    ( (This)->lpVtbl -> get_BackColor(This,pVal) ) 

#define IVixenScene_put_BackColor(This,newVal)	\
    ( (This)->lpVtbl -> put_BackColor(This,newVal) ) 

#define IVixenScene_get_Ambient(This,pVal)	\
    ( (This)->lpVtbl -> get_Ambient(This,pVal) ) 

#define IVixenScene_put_Ambient(This,newVal)	\
    ( (This)->lpVtbl -> put_Ambient(This,newVal) ) 

#define IVixenScene_get_Options(This,pVal)	\
    ( (This)->lpVtbl -> get_Options(This,pVal) ) 

#define IVixenScene_put_Options(This,newVal)	\
    ( (This)->lpVtbl -> put_Options(This,newVal) ) 

#define IVixenScene_EnableOptions(This,__MIDL__IVixenScene0004)	\
    ( (This)->lpVtbl -> EnableOptions(This,__MIDL__IVixenScene0004) ) 

#define IVixenScene_DisableOptions(This,__MIDL__IVixenScene0005)	\
    ( (This)->lpVtbl -> DisableOptions(This,__MIDL__IVixenScene0005) ) 

#define IVixenScene_get_TimeInc(This,pVal)	\
    ( (This)->lpVtbl -> get_TimeInc(This,pVal) ) 

#define IVixenScene_put_TimeInc(This,newVal)	\
    ( (This)->lpVtbl -> put_TimeInc(This,newVal) ) 

#define IVixenScene_ShowAll(This)	\
    ( (This)->lpVtbl -> ShowAll(This) ) 

#define IVixenScene_put_Priority(This,newVal)	\
    ( (This)->lpVtbl -> put_Priority(This,newVal) ) 

#define IVixenScene_get_Models(This,pVal)	\
    ( (This)->lpVtbl -> get_Models(This,pVal) ) 

#define IVixenScene_put_Models(This,newVal)	\
    ( (This)->lpVtbl -> put_Models(This,newVal) ) 

#define IVixenScene_get_Camera(This,pVal)	\
    ( (This)->lpVtbl -> get_Camera(This,pVal) ) 

#define IVixenScene_put_Camera(This,newVal)	\
    ( (This)->lpVtbl -> put_Camera(This,newVal) ) 

#define IVixenScene_get_Engines(This,pVal)	\
    ( (This)->lpVtbl -> get_Engines(This,pVal) ) 

#define IVixenScene_put_Engines(This,newVal)	\
    ( (This)->lpVtbl -> put_Engines(This,newVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenScene_INTERFACE_DEFINED__ */


#ifndef __WorldEvents_DISPINTERFACE_DEFINED__
#define __WorldEvents_DISPINTERFACE_DEFINED__

/* dispinterface WorldEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID_WorldEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("6db72162-0aa4-4837-ba22-e6173304bee1")
    WorldEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct WorldEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            WorldEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            WorldEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            WorldEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            WorldEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            WorldEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            WorldEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            WorldEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } WorldEventsVtbl;

    interface WorldEvents
    {
        CONST_VTBL struct WorldEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define WorldEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define WorldEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define WorldEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define WorldEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define WorldEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define WorldEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define WorldEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __WorldEvents_DISPINTERFACE_DEFINED__ */


#ifndef __IVixenWorld_INTERFACE_DEFINED__
#define __IVixenWorld_INTERFACE_DEFINED__

/* interface IVixenWorld */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IVixenWorld;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C5897807-9956-4497-AA0A-51FE65126930")
    IVixenWorld : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IsClass( 
            /* [in] */ BSTR __MIDL__IVixenWorld0000) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClassName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FileName( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FileName( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ImageDir( 
            /* [retval][out] */ BSTR *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ImageDir( 
            /* [in] */ BSTR newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Scene( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Find( 
            /* [in] */ BSTR s,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddEngine( 
            IDispatch *__MIDL__IVixenWorld0001) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Make( 
            BSTR classname,
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Navigator( 
            /* [retval][out] */ IDispatch **pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Navigator( 
            /* [in] */ IDispatch *newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FindAll( 
            /* [in] */ BSTR s,
            /* [retval][out] */ SAFEARRAY * *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IVixenWorldVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVixenWorld * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVixenWorld * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVixenWorld * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IVixenWorld * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IVixenWorld * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IVixenWorld * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IVixenWorld * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsClass )( 
            IVixenWorld * This,
            /* [in] */ BSTR __MIDL__IVixenWorld0000);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClassName )( 
            IVixenWorld * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FileName )( 
            IVixenWorld * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FileName )( 
            IVixenWorld * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ImageDir )( 
            IVixenWorld * This,
            /* [retval][out] */ BSTR *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ImageDir )( 
            IVixenWorld * This,
            /* [in] */ BSTR newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Scene )( 
            IVixenWorld * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Find )( 
            IVixenWorld * This,
            /* [in] */ BSTR s,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AddEngine )( 
            IVixenWorld * This,
            IDispatch *__MIDL__IVixenWorld0001);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Make )( 
            IVixenWorld * This,
            BSTR classname,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Navigator )( 
            IVixenWorld * This,
            /* [retval][out] */ IDispatch **pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Navigator )( 
            IVixenWorld * This,
            /* [in] */ IDispatch *newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *FindAll )( 
            IVixenWorld * This,
            /* [in] */ BSTR s,
            /* [retval][out] */ SAFEARRAY * *pVal);
        
        END_INTERFACE
    } IVixenWorldVtbl;

    interface IVixenWorld
    {
        CONST_VTBL struct IVixenWorldVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVixenWorld_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVixenWorld_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVixenWorld_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVixenWorld_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IVixenWorld_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IVixenWorld_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IVixenWorld_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IVixenWorld_IsClass(This,__MIDL__IVixenWorld0000)	\
    ( (This)->lpVtbl -> IsClass(This,__MIDL__IVixenWorld0000) ) 

#define IVixenWorld_get_ClassName(This,pVal)	\
    ( (This)->lpVtbl -> get_ClassName(This,pVal) ) 

#define IVixenWorld_get_FileName(This,pVal)	\
    ( (This)->lpVtbl -> get_FileName(This,pVal) ) 

#define IVixenWorld_put_FileName(This,newVal)	\
    ( (This)->lpVtbl -> put_FileName(This,newVal) ) 

#define IVixenWorld_get_ImageDir(This,pVal)	\
    ( (This)->lpVtbl -> get_ImageDir(This,pVal) ) 

#define IVixenWorld_put_ImageDir(This,newVal)	\
    ( (This)->lpVtbl -> put_ImageDir(This,newVal) ) 

#define IVixenWorld_get_Scene(This,pVal)	\
    ( (This)->lpVtbl -> get_Scene(This,pVal) ) 

#define IVixenWorld_Find(This,s,pVal)	\
    ( (This)->lpVtbl -> Find(This,s,pVal) ) 

#define IVixenWorld_AddEngine(This,__MIDL__IVixenWorld0001)	\
    ( (This)->lpVtbl -> AddEngine(This,__MIDL__IVixenWorld0001) ) 

#define IVixenWorld_Make(This,classname,pVal)	\
    ( (This)->lpVtbl -> Make(This,classname,pVal) ) 

#define IVixenWorld_get_Navigator(This,pVal)	\
    ( (This)->lpVtbl -> get_Navigator(This,pVal) ) 

#define IVixenWorld_put_Navigator(This,newVal)	\
    ( (This)->lpVtbl -> put_Navigator(This,newVal) ) 

#define IVixenWorld_FindAll(This,s,pVal)	\
    ( (This)->lpVtbl -> FindAll(This,s,pVal) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVixenWorld_INTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_VixenWorld;

#ifdef __cplusplus

class DECLSPEC_UUID("02297D0D-179C-4A68-93BE-DCCA61EC0A22")
VixenWorld;
#endif

EXTERN_C const CLSID CLSID_WorldProp;

#ifdef __cplusplus

class DECLSPEC_UUID("957C50D7-2025-46AE-A7AF-EBD24EE83B64")
WorldProp;
#endif

EXTERN_C const CLSID CLSID_VixenObj;

#ifdef __cplusplus

class DECLSPEC_UUID("7AF3C843-A813-43AE-A9E4-93037FC8D2B7")
VixenObj;
#endif

EXTERN_C const CLSID CLSID_VixenGroup;

#ifdef __cplusplus

class DECLSPEC_UUID("67550657-67A7-491B-BCDD-59AD5E6ECE50")
VixenGroup;
#endif

EXTERN_C const CLSID CLSID_VixenImage;

#ifdef __cplusplus

class DECLSPEC_UUID("4F8F8110-CC51-4381-8757-BABEB85B3BEA")
VixenImage;
#endif

EXTERN_C const CLSID CLSID_VixenMaterial;

#ifdef __cplusplus

class DECLSPEC_UUID("C9FEC818-68DB-498B-B725-0E363D29C009")
VixenMaterial;
#endif

EXTERN_C const CLSID CLSID_VixenAppearance;

#ifdef __cplusplus

class DECLSPEC_UUID("08D6D740-F3CB-4DEF-A372-FD3CDCF2094F")
VixenAppearance;
#endif

EXTERN_C const CLSID CLSID_VixenModel;

#ifdef __cplusplus

class DECLSPEC_UUID("DDF46971-4146-47BE-8D77-CDFB89FA5A88")
VixenModel;
#endif

EXTERN_C const CLSID CLSID_VixenEngine;

#ifdef __cplusplus

class DECLSPEC_UUID("6925DB1A-DBEE-46F5-B894-2D3B690C1EAC")
VixenEngine;
#endif

EXTERN_C const CLSID CLSID_VixenCamera;

#ifdef __cplusplus

class DECLSPEC_UUID("9000E437-BBDB-4263-9B87-93FDB47B41CA")
VixenCamera;
#endif

EXTERN_C const CLSID CLSID_VixenNavigator;

#ifdef __cplusplus

class DECLSPEC_UUID("A66E406F-F74C-4CCF-95F3-B8A39892FB70")
VixenNavigator;
#endif

EXTERN_C const CLSID CLSID_VixenArcBall;

#ifdef __cplusplus

class DECLSPEC_UUID("EE57C28A-3A98-425D-AC8D-3FFF1AB8FBE7")
VixenArcBall;
#endif

EXTERN_C const CLSID CLSID_VixenTrackBall;

#ifdef __cplusplus

class DECLSPEC_UUID("5598EFBE-AA5C-427E-A119-86B12F7E0CE9")
VixenTrackBall;
#endif

EXTERN_C const CLSID CLSID_VixenScriptor;

#ifdef __cplusplus

class DECLSPEC_UUID("f2c23f48-84e2-4d8e-ba70-f639ce715baf")
VixenScriptor;
#endif

EXTERN_C const CLSID CLSID_VixenRayPicker;

#ifdef __cplusplus

class DECLSPEC_UUID("DF62881E-9A86-46E5-A9ED-91F165C79C2F")
VixenRayPicker;
#endif

EXTERN_C const CLSID CLSID_VixenNamePicker;

#ifdef __cplusplus

class DECLSPEC_UUID("EBD46EDD-C68C-4E67-8899-72010BA367F0")
VixenNamePicker;
#endif

EXTERN_C const CLSID CLSID_VixenScene;

#ifdef __cplusplus

class DECLSPEC_UUID("60843662-8E82-4D5A-BC57-63C7507AC2B2")
VixenScene;
#endif
#endif /* __VixenLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


