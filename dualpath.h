/* Include to aid dual-pathing of PM applications for 16/32-bit. */
/* Assumes "B16" is defined for the 16-bit case.                 */
/* ------------------------------------------------------------- */
/*                                                               */
/* Copyright (c) Mike Cowlishaw, 1993-2019.  All rights reserved.*/
/* Parts Copyright (c) IBM, 1993-2009.                           */
/* ------------------------------------------------------------- */
/* Specials:                                                     */
/*                                                               */
/*   USLONG  -- USHORT for 16-bit, ULONG for 32-bit              */
/*   SLONG   -- SHORT for 16-bit,  LONG for 32-bit               */
/*   PSLONG  -- PSHORT for 16-bit, PLONG for 32-bit              */
/*                                                               */
/*                                                               */

#if defined(B16)
  /* 16-bit */
  #define USLONG  USHORT
  #define SLONG   SHORT
  #define PSLONG  PSHORT
  #define PUSLONG PUSHORT
  /* Now let us use some 32-bit functions in the 16-bit version */
  /* Event and Mutex semaphore stuff -- replaces RAM semaphores */
  #define HEV unsigned long
  #define DosCreateEventSem(x,y,z,f) if (f) *(y)=0; else *(y)=1;
  #define DosPostEventSem(x)        DosSemClear(&x)
  #define DosResetEventSem(x,y)     DosSemSet(&x)
  #define DosWaitEventSem(x,y)      DosSemWait(&x,(signed long)(y))

  #define DosCreateMutexSem(x,y,z,f) if (f) *(y)=1; else *(y)=0;
  #define DosRequestMutexSem(x,y)   DosSemRequest(&x,(signed long)(y))
  #define DosReleaseMutexSem(x)     DosSemClear(&x)

  /* Misc */
  #define DosDelete(x)              DosDelete(x,0L)
  #define WinQueryWindow(x,y)       WinQueryWindow(x,y,NULL)
  #define WinWindowFromPoint(x,y,z) WinWindowFromPoint(x,y,z,NULL)
  #define WinEnableControl(hwndDlg, id, fEnable) \
          WinEnableWindow(WinWindowFromID(hwndDlg, id), fEnable)
  #define BITMAPINFOHEADER2         BITMAPINFOHEADER
  #define BITMAPINFO2               BITMAPINFO
  #define PBITMAPINFO2              PBITMAPINFO
  #define RGB2                      RGB
  #define _timezone                 timezone
  #define RexxStart                 -REXXSAA
  #define RexxVariablePool          RxVar
  #define _ltoa(x,y,z)              ltoa((x),(y),(z))
  #define _itoa(x,y,z)              itoa((x),(y),(z))

#else
  /* 32-bit */
  #define USLONG  ULONG
  #define SLONG   LONG
  #define PSLONG  PLONG
  #define PUSLONG PULONG

  #define DosWaitEventSem(x,y)      DosWaitEventSem((x),(unsigned)(y))
  #define DosRequestMutexSem(x,y)   DosRequestMutexSem((x),(unsigned)(y))

  #undef  NULL
  #define NULL   0

  #define halloc(x,y) malloc((unsigned long)(x)*(unsigned long)(y))
  #define hfree(x)    free(x)

  #define far
  #define near
  #define huge
  #define cdecl
  #define _loadds

  #define ltoa(x,y,z) _ltoa((x),(y),(z))

#endif
