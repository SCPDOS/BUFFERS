
;-------------------------------------------------------;
; This include file contains miscellaneous internal     ;
; SCP/DOS structures.                                   ;
;-------------------------------------------------------;

;------------------------------------------------------------------------;
; Each mass storage device up to LASTDRIVE gets a DPB, describing 
; its file system structure. This is used to locate files and structures
; on the device itself and to translate file requests to cluster chains
; to LBAs to pass to the disk driver..
; The DPBs form a linked list, with each DPB formed after the devices 
; drivers Initialise routine has been called.
;------------------------------------------------------------------------;
struc dpb        ;Drive Parameter Block

    .bDriveNumber            resb 1  ;Drive number (0=A, 1=B ...) x
    .bUnitNumber             resb 1  ;Unit number in device x
    .wBytesPerSector         resw 1  ;min 512, max 4096 x
    .bMaxSectorInCluster     resb 1  ;(Maximum sector in cluster) - 1 x
    .bSectorsPerClusterShift resb 1  ;Sectors per cluster exponent x
    .wFAToffset              resw 1  ;Number of reserved sectors in partition x
    .bNumberOfFATs           resb 1  ;Number of FATs x
    .wNumberRootDirEntries   resw 1  ;In 32 byte units, this is 0 for FAT32 x
    .dClusterHeapOffset      resd 1  ;Start sector of data area x
    .dClusterCount           resd 1  ;Total number of clusters in data area x
    .dFATlength              resd 1  ;FAT length, in sectors x
    .dFirstUnitOfRootDir     resd 1  
    ;Unit=Sector/Cluster of Root Directory (FAT12/16 = Sector, FAT32 = Cluster)x
    .qDriverHeaderPtr        resq 1  ;Pointer to device driver header x
    .bMediaDescriptor        resb 1  ;Media descriptor x
    .bAccessFlag             resb 1  ;Access Flag (0 if accessed, else -1) x
    .qNextDPBPtr             resq 1  ;Pointer to next DPB, -1 if at end
    .dFirstFreeCluster       resd 1  ;Starting cluster of free space search x
    .dNumberOfFreeClusters   resd 1  ;Number of free clusters, -1 unknown x

endstruc

fat12MaxClustCnt    equ 4085    ;Cluster count less than this is FAT12
fat16MaxClustCnt    equ 65525   ;Cluster count less than this is FAT16
;------------------------------------------------------------------------;
; Each mass storage device up to LASTDRIVE has a CDS associated with it.
; All disk pathing requests go via the CDS for that drive.
; The default drive is the last accessed drive, this number is stored
; in the DOS data area.
; The CDS is stored as an array with the offset into the array being 
; given by the drive letter's offset into the alphabet (with A=0).
;
; DEFAULT LASTDRIVE=E, MINIMUM LASTDRIVE=B
;------------------------------------------------------------------------;
struc cds       ;Current Directory structure
    .sCurrentPath        db 67 dup (?)    ;Current Directory Path String
    .wFlags              dw ?   ;CDS Flags - 
    .qDPBPtr             dq ?   ;Ptr to the DPB of the dev using this CDS
    .dStartCluster       dd ?   ;Starting cluster of the directory on disk
    .qIFSPtr             dq ?   ;Reserved for IFS driver expansions
    .dNetStore           dd ?   ;For storing a owner dword for IFS (41h/5F03h)
    .wBackslashOffset    dw ?   ;How many chars to skip for getting the path
; currentPath (usually 2 to skip C: in C:\, but when SUBSTing a drive letter to 
; another, we save the number of chars to skip to that deep level).
; I.E. if we SUBST A:\DOS G: then this value for the CDS for drive G is 6
; Only affects for SUBST drives, not JOIN
; Flags: Bit 15 set = Network drive
;        Bit 14 set = Physical drive
;        Bit 13 set = JOIN'ed drive
;        Bit 12 set = SUBST'ed drive
;        Bit 7  set = Redirected, not net. drive (This is how MSCDEX worked)
endstruc
;CDS equates
cdsRedirDrive   equ 8000h   ;All drives which use the Redirector interface
cdsValidDrive   equ 4000h   ;CDS valid
cdsJoinDrive    equ 2000h   ;CDS for joined drives
cdsSubstDrive   equ 1000h   ;Drives formed by Subst
cdsRdirLocDrive equ 0080h   ;Redirector but Local
;------------------------------------------------------------------------;
; The SFT is a way to allow applications to open file handles to files 
; and devices. As things stand, the SFT chain will have 5 file handles
; under the first header, and then a second header will be linked to the 
; first one after CONFIG.SYS has been read, to allow for FILES= to specify
; a size for the second SFT (default 10 files). Same for System FCBs, 
; using the FCBS= command. Default FCBS=5.
;
; A file handle describes the file, and the location within the file that
; we are reading. The file handle can be manipulated without the file
; being in memory, thus allowing the user to access data without needing 
; to take up large portions of memory. Similarly, file handles allow for
; accessing devices, by treating them as files with their drivers 
; responding and translating the manipulations of the file handle itself.
; Neither the driver nor the application should see or use the SFT for
; the handle, as it is subject to change (and will change when file 
; sharing provisions are included). The DOS will make requests to the 
; driver in a generic way and the driver will respond, with DOS keeping
; track of the information in the SFT entry for that handle. 
;
; One SFT entry can have multiple handles and multiple SFT entries
; can point to the same file. It all depends on what the applications 
; want to do with the File.
;
; !!!!IMPORTANT!!!!
; Currently there are plans for File Sharing, but I include minimal 
; provisions for it. I really just wanna get a DOS out the door.
;
;Each SFT may look like this:
;sysvars-->sfth------------>sfth------------>|ENDOFLIST|
;            |                |
;       wNumFiles*sft    wNumFiles*sft
;
; Two SFT chains are built; one for files, and a one for so called 
; System FCBs.
;
; Each SFT header has an array of SFT's below it, with the number of 
; SFTs given by wNumFiles. The SFT headers form a linked list.
;
; DEFAULT FILES=8, MINIMUM FILES=5
; DEFAULT FCBS=5,  MINIMUM FCBS=5
;------------------------------------------------------------------------;
struc sfth   ;System file table header
; This is the SFT header.
; This structure comes before the start of the first file in the array/table.
    .qNextSFTPtr resq 1    ;Pointer to the next SFT
    .wNumFiles   resw 1    ;Number of files in this table
endstruc

struc sft
;This is the file struc itself
    .wNumHandles resw 1    ;Number of handles pointing to this file
    .wOpenMode   resw 1    ;File access permissions and behvaiours
    .bFileAttrib resb 1    ;Regular File Attributes
    .wDeviceInfo resw 1    ;Includes unit number if a block device
    .qPtr        resq 1    ;IF char, ptr to device driver header, ELSE DPBptr
    .dStartClust resd 1    ;Start cluster of file pointed to by SFT entry
    .wTime       resw 1    ;File Time, updated on open and writes
    .wDate       resw 1    ;File Date, updated on open and writes
    .dFileSize   resd 1    ;File Size, in bytes
    .dCurntOff   resd 1    ;Current Offset pointed to by SFT entry, in bytes
    .dRelClust   resd 1    ;Rel cluster pointed to by SFT entry
    .dAbsClusr   resd 1    ;Absolute cluster pointed to by SFT entry
    .qDirSect    resq 1    ;Abs number of sector containing directory entry 
    .bNumDirEnt  resb 1    ;Directory entry within dir sector (byte offset/32)
    .sFileName   resb 11   ;8.3 Filename
    .qSharePtr   resq 1    ;SHARE pointer to previous SFT sharing same file
    .wMachNum    resw 1    ;SHARE number of network machine opening file
    .qPSPOwner   resq 1    ;Addr of PSP of program which first opened this file
    .wShareRec   resw 1    ;offset within SHARE code segment of sharing record
;                           0 means no record
endstruc
;Open Mode equates
;Access modes   Bits[2:0]
ReadAccess      equ 0h
WriteAccess     equ 1h
RWAccess        equ 2h
;Bit 3 reserved 0
;Sharing modes  Bits[6:4]
CompatShare     equ 0h
denyRWShare     equ 10h
denyWriteShare  equ 20h
denyReadShare   equ 30h
denyNoneShare   equ 40h
netFCBShare     equ 70h
noInherit       equ 80h
;Bits 8-12 reserved 0

diskFullFail    equ 2000h   ;DO NOT USE. IF set, Trigger Int 24h on disk full.
;Buffering on writes
noBufferWrites  equ 4000h   ;If set, dont buffer writes, always update disk
;FCB opened file, shouldn't see this as we are depreciating FCB's completely now
FCBopenedFile   equ 8000h   ;If set, file is opened by FCB, else SFT
;Attribute Byte
readOnlyFile    equ 1h
hiddenFile      equ 2h
systemFile      equ 4h
volumeLabel     equ 8h
directoryFile   equ 10h ;Directory entry
archiveFile     equ 20h
;Device Info equates
;For a block device, bits [5:0] contain the drive number
charDevConIn    equ 1h  ;If set, device is CON input
charDevConOut   equ 2h  ;If set, device is CON output
charDevNulDev   equ 4h  ;If set, device is NULL device
charDevClockDev equ 8h  ;If set, device is CLOCK device
charDevFastOut  equ 10h ;If set, device supports Int 29h
charDevBinary   equ 20h ;If set, r/w char device in Binary mode!!!
charDevNoEOF    equ 40h ;If set, no EOF when reading from device!!
blokFileNoFlush equ 40h ;Set if file has not yet been accessed (sol no flush!)
devCharDev      equ 80h ;If set, it is a char device!
charDevNetSpool equ 800h    ;If set, device is a network spooler
devNoInherit    equ 1000h   ;If set, handle not inherited by child processes
charDevNamePipe equ 2000h   ;If set, device is a named pipe
blokNoDTonClose equ 4000h   ;If set, dont update date and time when closing
devRedirDev     equ 8000h   ;If set, dev/file is redirected!
 
struc bufferHdr
    .nextBufPtr  resq  1   ;Pointer to the next buffer in the chain or -1 at end
    .driveNumber resb  1   ;Drive number the buffer is used for (or -1 for free)
    .bufferFlags resb  1   ;Standard Flags
    .bufferLBA   resq  1   ;LBA the buffer contains
    .bufFATcopy  resb  1   ;Number of copies of the FAT to write (1 if non-FAT)
    .bufFATsize  resd  1   ;Number of sectors per FAT (or 0 if non-FAT)
    .driveDPBPtr resq  1   ;Pointer to the device DPB block
    .owningFile  resq  1   ;Ptr to SFT/FCBS which owns buff (-1 if not in chain)
    .reserved    resb  9   ;Alignment bytes
    .dataarea:             ;Points to the start of the data area
endstruc
;Buffer Flag equates
dosBuffer   equ 1h  ;Sector 0 (BPB/VPB) in buffer
fatBuffer   equ 2h  ;FAT sector in buffer
dirBuffer   equ 4h  ;Directory sector in buffer
dataBuffer  equ 8h  ;File Data sector in buffer
refBuffer   equ 20h ;Data in buffer has been referenced by an application
dirtyBuffer equ 40h ;Data in buffer modified by an application
;Note! A buffer is marked as referenced only when the application 
; is done with that buffer OR if a hard error has occured (both read and write)

struc sysVars
    .dpbHeadPtr  dq ?
    .sftHeadPtr  dq ?    
    .clockPtr    dq ? 
    .vConPtr     dq ?
    .maxBytesSec dw ?
    .bufHeadPtr  dq ?
endstruc

struc cmdLineArgPtr
    .fcb1       db 16 dup(?)    ;First FCB,    argument 1 
    .fcb2       db 20 dup(?)    ;Second FCB,   argument 2
    .dta:   ;Pointer to the default DTA in the PSP
    .parmList   db ?   ;Number of characters in command tail
    .progTail   db 127 dup(?) ;Default DTA/Program tail
endstruc