

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


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


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, LIBID_VixenLib,0x877BD003,0x49F2,0x4586,0xA8,0xCF,0x52,0xDA,0x3F,0xE6,0xA2,0x99);


MIDL_DEFINE_GUID(IID, IID_IVixenObj,0xC09BFB5D,0x4294,0x443F,0x9A,0xC4,0x39,0x32,0x83,0x5E,0x43,0x1E);


MIDL_DEFINE_GUID(IID, IID_IVixenGroup,0x34B8C62E,0x50EC,0x4E82,0xA6,0x53,0x0F,0x0D,0x46,0xF8,0x74,0x05);


MIDL_DEFINE_GUID(IID, IID_IVixenModel,0x9FB8A877,0x51BE,0x4C67,0x82,0xD8,0x5D,0x86,0xFB,0xD4,0xB6,0x07);


MIDL_DEFINE_GUID(IID, IID_IVixenImage,0xEC99A915,0x0CB7,0x43A4,0xB9,0xE7,0x78,0x89,0xCA,0x0F,0xE7,0x2D);


MIDL_DEFINE_GUID(IID, IID_IVixenMaterial,0xFDCB2CB3,0x36EF,0x4CA4,0xA6,0xFE,0xCA,0xF5,0x79,0xBF,0x2E,0x96);


MIDL_DEFINE_GUID(IID, IID_IVixenAppearance,0x43E25B24,0xC304,0x472D,0x8D,0x1D,0x51,0xE1,0xC2,0x2E,0x27,0xEA);


MIDL_DEFINE_GUID(IID, IID_IVixenEngine,0x8B41BFE3,0xD167,0x4704,0x92,0xF8,0x11,0x3B,0x31,0x7B,0x68,0x1B);


MIDL_DEFINE_GUID(IID, IID_IVixenCamera,0x39961685,0x6CC7,0x4CB6,0xBA,0x12,0x96,0x68,0x6A,0x92,0xE8,0xE3);


MIDL_DEFINE_GUID(IID, IID_IVixenScriptor,0xc9103b79,0x15dd,0x4f8e,0x80,0xf6,0x83,0x40,0x23,0x07,0xcc,0xc5);


MIDL_DEFINE_GUID(IID, IID_IVixenTrackBall,0xD593AA6B,0xA10D,0x4602,0x87,0x3B,0x88,0xFA,0x63,0x2D,0x02,0x66);


MIDL_DEFINE_GUID(IID, IID_IVixenRayPicker,0x688348A8,0xC410,0x499D,0xAF,0xF5,0x55,0x7D,0xAD,0xFB,0x14,0xD7);


MIDL_DEFINE_GUID(IID, IID_IVixenNamePicker,0xFD6D579F,0x7ED8,0x4B2A,0x9C,0xBB,0x53,0x27,0x2C,0xCA,0xC2,0x2F);


MIDL_DEFINE_GUID(IID, IID_IVixenArcBall,0x30183A69,0x779C,0x4E7A,0xB6,0xFF,0xD2,0x57,0xE9,0xBD,0x77,0x14);


MIDL_DEFINE_GUID(IID, IID_IVixenNavigator,0x658C1ADB,0x4CE3,0x42DF,0x80,0x05,0x62,0x3F,0x2B,0x90,0xCA,0x0F);


MIDL_DEFINE_GUID(IID, IID_IVixenScene,0x82810AFA,0x3164,0x4715,0x8F,0x1D,0x64,0x70,0xF4,0x6C,0x34,0xED);


MIDL_DEFINE_GUID(IID, DIID_WorldEvents,0x6db72162,0x0aa4,0x4837,0xba,0x22,0xe6,0x17,0x33,0x04,0xbe,0xe1);


MIDL_DEFINE_GUID(IID, IID_IVixenWorld,0xC5897807,0x9956,0x4497,0xAA,0x0A,0x51,0xFE,0x65,0x12,0x69,0x30);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenWorld,0x02297D0D,0x179C,0x4A68,0x93,0xBE,0xDC,0xCA,0x61,0xEC,0x0A,0x22);


MIDL_DEFINE_GUID(CLSID, CLSID_WorldProp,0x957C50D7,0x2025,0x46AE,0xA7,0xAF,0xEB,0xD2,0x4E,0xE8,0x3B,0x64);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenObj,0x7AF3C843,0xA813,0x43AE,0xA9,0xE4,0x93,0x03,0x7F,0xC8,0xD2,0xB7);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenGroup,0x67550657,0x67A7,0x491B,0xBC,0xDD,0x59,0xAD,0x5E,0x6E,0xCE,0x50);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenImage,0x4F8F8110,0xCC51,0x4381,0x87,0x57,0xBA,0xBE,0xB8,0x5B,0x3B,0xEA);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenMaterial,0xC9FEC818,0x68DB,0x498B,0xB7,0x25,0x0E,0x36,0x3D,0x29,0xC0,0x09);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenAppearance,0x08D6D740,0xF3CB,0x4DEF,0xA3,0x72,0xFD,0x3C,0xDC,0xF2,0x09,0x4F);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenModel,0xDDF46971,0x4146,0x47BE,0x8D,0x77,0xCD,0xFB,0x89,0xFA,0x5A,0x88);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenEngine,0x6925DB1A,0xDBEE,0x46F5,0xB8,0x94,0x2D,0x3B,0x69,0x0C,0x1E,0xAC);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenCamera,0x9000E437,0xBBDB,0x4263,0x9B,0x87,0x93,0xFD,0xB4,0x7B,0x41,0xCA);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenNavigator,0xA66E406F,0xF74C,0x4CCF,0x95,0xF3,0xB8,0xA3,0x98,0x92,0xFB,0x70);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenArcBall,0xEE57C28A,0x3A98,0x425D,0xAC,0x8D,0x3F,0xFF,0x1A,0xB8,0xFB,0xE7);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenTrackBall,0x5598EFBE,0xAA5C,0x427E,0xA1,0x19,0x86,0xB1,0x2F,0x7E,0x0C,0xE9);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenScriptor,0xf2c23f48,0x84e2,0x4d8e,0xba,0x70,0xf6,0x39,0xce,0x71,0x5b,0xaf);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenRayPicker,0xDF62881E,0x9A86,0x46E5,0xA9,0xED,0x91,0xF1,0x65,0xC7,0x9C,0x2F);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenNamePicker,0xEBD46EDD,0xC68C,0x4E67,0x88,0x99,0x72,0x01,0x0B,0xA3,0x67,0xF0);


MIDL_DEFINE_GUID(CLSID, CLSID_VixenScene,0x60843662,0x8E82,0x4D5A,0xBC,0x57,0x63,0xC7,0x50,0x7A,0xC2,0xB2);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



