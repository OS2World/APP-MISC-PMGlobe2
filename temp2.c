
   /*--------------------------------------------------------------------*/
   /* File Dialog application data structure.                            */
   /* ------------------------------------------------------------------ */
   /*                                                                    */
   /* Copyright (c) Mike Cowlishaw, 1993-2019.  All rights reserved.     */
   /* Parts Copyright (c) IBM, 1993-2009.                                */
   */
   /* Permission to use, copy, modify, and distribute this software      */
   /* for any non-commercial purpose without fee is hereby granted,      */
   /* provided that the above copyright notice and this permission       */
   /* notice appear in all copies, and that notice and the date of       */
   /* any modifications be added to the software.                        */
   /*                                                                    */
   /* This software is provided "as is".  No warranties, whether         */
   /* express, implied, or statutory, including, but not limited to,     */
   /* implied warranties of merchantability and fitness for a            */
   /* particular purpose apply to this software.  The author shall       */
   /* not, in any circumstances, be liable for special, incidental,      */
   /* or consequential damages, for any reason whatsoever.               */
   /*                                                                    */
   /* ------------------------------------------------------------------ */
   typedef struct _FILEDLG     /* fildlg */
   {
      ULONG    cbSize;            /* Size of FILEDLG structure.         */
      ULONG    fl;                /* FDS_ flags. Alter behavior of dlg. */
      ULONG    ulUser;            /* User defined field.                */
      LONG     lReturn;           /* Result code from dialog dismissal. */
      LONG     lSRC;              /* System return code.                */
      PSZ      pszTitle;          /* String to display in title bar.    */
      PSZ      pszOKButton;       /* String to display in OK button.    */
      PFNWP    pfnDlgProc;        /* Entry point to custom dialog proc. */
      PSZ      pszIType;          /* Pointer to string containing       */
      /*                               initial EA type filter. Type     */
      /*                               does not have to exist in list.  */
      PAPSZ    papszITypeList;    /* Pointer to table of pointers that  */
      /*                                point to null terminated Type   */
      /*                                strings. End of table is marked */
      /*                                by a NULL pointer.              */
      PSZ      pszIDrive;         /* Pointer to string containing       */
      /*                               initial drive. Drive does not    */
      /*                               have to exist in drive list.     */
      PAPSZ    papszIDriveList;   /* Pointer to table of pointers that  */
      /*                                point to null terminated Drive  */
      /*                                strings. End of table is marked */
      /*                                by a NULL pointer.              */
      HMODULE  hMod;              /* Custom File Dialog template.       */
      CHAR     szFullFile[CCHMAXPATH]; /* Initial or selected fully     */
      /*                                  qualified path and file.      */
      PAPSZ    papszFQFilename;   /* Pointer to table of pointers that  */
      /*                                point to null terminated FQFname*/
      /*                                strings. End of table is marked */
      /*                                by a NULL pointer.              */
      ULONG    ulFQFCount;        /* Number of files selected           */
      USHORT   usDlgId;           /* Custom dialog id.                  */
      SHORT    x;                 /* X coordinate of the dialog         */
      SHORT    y;                 /* Y coordinate of the dialog         */
      SHORT    sEAType;           /* Selected file's EA Type.           */
   } FILEDLG;
   typedef FILEDLG *PFILEDLG;
