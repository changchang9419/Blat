/*
    reg.cpp
*/

#include "declarations.h"

#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "blat.h"
#include "common_data.h"


#ifdef BLATDLL_EXPORTS // this is blat.dll, not blat.exe
#if defined(__cplusplus)
extern "C" {
#endif
extern void (*pMyPrintDLL)(LPTSTR);
#if defined(__cplusplus)
}
#endif
#else
#define pMyPrintDLL _tprintf
#endif

extern void base64_encode(_TUCHAR * in, size_t length, LPTSTR out, int inclCrLf);
extern int  base64_decode(_TUCHAR * in, LPTSTR out);
extern void printMsg(COMMON_DATA & CommonData, LPTSTR p, ... );              // Added 23 Aug 2000 Craig Morrison

extern LPTSTR       defaultSMTPPort;

#if INCLUDE_NNTP
extern LPTSTR       defaultNNTPPort;
#endif

#if INCLUDE_POP3
extern LPTSTR       defaultPOP3Port;
#endif
#if INCLUDE_IMAP
extern LPTSTR       defaultIMAPPort;
#endif

static const _TCHAR SoftwareRegKey[]     = __T("SOFTWARE");

#if defined(_WIN64)
static const _TCHAR Wow6432NodeKey[]     = __T("\\Wow6432Node");
#endif
static const _TCHAR MainRegKey[]         = __T("\\Public Domain\\Blat");

static const _TCHAR RegKeySMTPHost[]     = __T("SMTP server");
static const _TCHAR RegKeySMTPPort[]     = __T("SMTP Port");
#if INCLUDE_NNTP
static const _TCHAR RegKeyNNTPHost[]     = __T("NNTP server");
static const _TCHAR RegKeyNNTPPort[]     = __T("NNTP Port");
#endif
#if INCLUDE_POP3
static const _TCHAR RegKeyPOP3Host[]     = __T("POP3 server");
static const _TCHAR RegKeyPOP3Port[]     = __T("POP3 Port");
static const _TCHAR RegKeyPOP3Login[]    = __T("POP3 Login");
static const _TCHAR RegKeyPOP3Password[] = __T("POP3 Pwd");
#endif
#if INCLUDE_IMAP
static const _TCHAR RegKeyIMAPHost[]     = __T("IMAP server");
static const _TCHAR RegKeyIMAPPort[]     = __T("IMAP Port");
static const _TCHAR RegKeyIMAPLogin[]    = __T("IMAP Login");
static const _TCHAR RegKeyIMAPPassword[] = __T("IMAP Pwd");
#endif
static const _TCHAR RegKeyLogin[]        = __T("Login");
static const _TCHAR RegKeyPassword[]     = __T("Pwd");
static const _TCHAR RegKeySender[]       = __T("Sender");
static const _TCHAR RegKeyTry[]          = __T("Try");

#if defined(_UNICODE) || defined(UNICODE)

extern void convertPackedUnicodeToUTF( Buf & sourceText, Buf & outputText, int * utf, LPTSTR charset, int utfRequested );


static void encodeThis( _TUCHAR * in, LPTSTR out, int inclCrLf )
{
    size_t byteCount;
    Buf    tmpStr;
    Buf    msg;
    size_t len;
    Buf    inputString;
    int    tempUTF;


    if ( !in || !out )
        return;

    tmpStr.Clear();
    *out = __T('\0');

    for ( len = 0; ; len++ ) {
        if ( in[len] == __T('\0') )
            break;
        if ( in[len] > 0x00FF ) {
            tempUTF = sizeof(_TCHAR);
            inputString = (LPTSTR)in;
            convertPackedUnicodeToUTF( inputString, tmpStr, &tempUTF, NULL, 8 );
            break;
        }
        tmpStr.Add( (_TCHAR)in[len] );
    }

    byteCount = tmpStr.Length();
    if ( byteCount ) {
        msg.AllocExact( (byteCount+1) * 3 );
        base64_encode( (_TUCHAR *)tmpStr.Get(), (size_t)byteCount, msg.Get(), inclCrLf );
        _tcscpy( out, msg.Get() );
    }

    inputString.Free();
    msg.Free();
    tmpStr.Free();
}

//static void decodeThis(LPTSTR in, LPTSTR out)
//{
//    Buf    tmpStr;
//    Buf    msg;
//    size_t len;
//    size_t ix;
//    LPTSTR pTcharString;
//    DWORD  tlc;
//    int    count;
//
//    *out = __T('\0');
//    len = _tcslen(in);
//    if ( len ) {
//        len++;
//        for ( ix = 0; ix < len; ix++ ) {
//            if ( in[ix] == __T('\0') ) {
//               tmpStr.Add( __T('\0') );
//               ix++;
//               break;
//            }
//            if ( in[ix] & ~0x00FF ) {
//                tmpStr.Free();
//                return;
//            }
//            tmpStr.Add( (char)in[ix] );
//        }
//        len = ix;
//        msg.AllocExact( len );
//        pTcharString = msg.Get();
//        base64_decode( (_TUCHAR *)tmpStr.Get(), pTcharString );
//        tmpStr.Free();
//        for ( len = 0; ; len++ ) {
//            if ( pTcharString[len] == __T('\0') ) {
//                tmpStr.Add( __T('\0') );
//                break;
//            }
//            if ( (_TUCHAR)pTcharString[len] >= 0xFE ) {
//                msg.Free();
//                tmpStr.Free();
//                return;
//            }
//            if ( (_TUCHAR)pTcharString[len] < 128 ) {
//                tmpStr.Add( pTcharString[len] );
//                continue;
//            }
//
//            count = 0;
//            if ( (((_TUCHAR)pTcharString[len] & 0xE0) == 0xC0) &&
//                 ( (_TUCHAR)pTcharString[len]         >= 0xC1) ) {
//                tlc = (_TCHAR)(pTcharString[len] & 0x1Fl);
//                count = 1;
//            } else
//            if ( ((_TUCHAR)pTcharString[len] & 0xF0) == 0xE0 ) {
//                tlc = (_TCHAR)(pTcharString[len] & 0x0Fl);
//                count = 2;
//            } else
//            if ( ((_TUCHAR)pTcharString[len] & 0xF8) == 0xF0 ) {
//                tlc = (_TCHAR)(pTcharString[len] & 0x07l);
//                count = 3;
//            } else
//            if ( ((_TUCHAR)pTcharString[len] & 0xFC) == 0xF8 ) {
//                tlc = (_TCHAR)(pTcharString[len] & 0x03l);
//                count = 4;
//            } else
//            if ( ((_TUCHAR)pTcharString[len] & 0xFE) == 0xFC ) {
//                tlc = (_TCHAR)(pTcharString[len] & 0x01l);
//                count = 5;
//            } else {
//                msg.Free();
//                tmpStr.Free();
//                return;
//            }
//            while ( count ) {
//                if ( (pTcharString[++len] & 0xC0) != 0x80 ) {
//                    msg.Free();
//                    tmpStr.Free();
//                    return;
//                }
//                tlc <<= 6;
//                tlc |= (_TCHAR)(pTcharString[++len] & 0x3F);
//                count--;
//            }
//            tmpStr.Add( (_TCHAR)tlc );
//        }
//        _tcscpy( out, tmpStr.Get() );
//    }
//    msg.Free();
//    tmpStr.Free();
//}
//
#else
#define encodeThis(s,o,crlf) base64_encode((s), _tcslen((char *)(s)), (LPTSTR)(o), (crlf))
#endif
#define decodeThis(s,o)      base64_decode((_TUCHAR *)(s), (o))

// create registry entries for this program
static int CreateRegEntry( COMMON_DATA & CommonData, HKEY rootKeyLevel )
{
    HKEY   hKey1;
    DWORD  dwDisposition;
    LONG   lRetCode;
    _TCHAR strRegisterKey[256];
    _TCHAR tmpstr[MAXOUTLINE * 2];

#if defined(_WIN64)
    _stprintf(strRegisterKey, __T("%s%s%s"), SoftwareRegKey, Wow6432NodeKey, MainRegKey);
    if ( CommonData.Profile[0] != __T('\0') ) {
        _tcscat(strRegisterKey, __T("\\"));
        _tcscat(strRegisterKey, CommonData.Profile);
    }

    /* try to create the .INI file key */
    lRetCode = RegCreateKeyEx ( rootKeyLevel,
                                strRegisterKey,
                                0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey1, &dwDisposition
                              );

    /* if we failed, note it, and leave */
    if ( lRetCode != ERROR_SUCCESS )
#endif
    {
        _stprintf(strRegisterKey, __T("%s%s"), SoftwareRegKey, MainRegKey);
        if ( CommonData.Profile[0] != __T('\0') ) {
            _tcscat(strRegisterKey, __T("\\"));
            _tcscat(strRegisterKey, CommonData.Profile);
        }

        /* try to create the .INI file key */
        lRetCode = RegCreateKeyEx ( rootKeyLevel,
                                    strRegisterKey,
                                    0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey1, &dwDisposition
                                  );
    }

    /* if we failed, note it, and leave */
    if ( lRetCode != ERROR_SUCCESS ) {
        if ( !CommonData.quiet ) {
            if ( (lRetCode == ERROR_CANTWRITE) || (lRetCode == ERROR_ACCESS_DENIED) )
                pMyPrintDLL( __T("Write access to the registry was denied.\n") );
            else
                pMyPrintDLL( __T("Error in creating blat key in the registry\n") );
        }

        return(10);
    }

    if ( CommonData.SMTPHost[0] ) {
        if ( !_tcscmp(CommonData.SMTPHost, __T("-")) ) {
            CommonData.SMTPHost[0] = __T('\0');
            CommonData.SMTPPort[0] = __T('\0');
        }

        /* try to set a section value */
        lRetCode = RegSetValueEx( hKey1, RegKeySMTPHost, 0, REG_SZ, (LPBYTE)CommonData.SMTPHost, (DWORD)((_tcslen(CommonData.SMTPHost)+1)*sizeof(_TCHAR)) );

        /* if we failed, note it, and leave */
        if ( lRetCode != ERROR_SUCCESS ) {
            if ( !CommonData.quiet )  pMyPrintDLL( __T("Error in setting SMTP server value in the registry\n"));
            return(11);
        }

        /* try to set another section value */
        lRetCode = RegSetValueEx( hKey1, RegKeySMTPPort, 0, REG_SZ, (LPBYTE)CommonData.SMTPPort, (DWORD)((_tcslen(CommonData.SMTPPort)+1)*sizeof(_TCHAR)) );

        /* if we failed, note it, and leave */
        if ( lRetCode != ERROR_SUCCESS ) {
            if ( !CommonData.quiet )  pMyPrintDLL( __T("Error in setting port value in the registry\n") );
            return(11);
        }

        /* try to set another section value */
        lRetCode = RegSetValueEx( hKey1, RegKeyTry, 0, REG_SZ, (LPBYTE)CommonData.Try, (DWORD)((_tcslen(CommonData.Try)+1)*sizeof(_TCHAR)) );

        /* if we failed, note it, and leave */
        if ( lRetCode != ERROR_SUCCESS ) {
            if ( !CommonData.quiet )  pMyPrintDLL( __T("Error in setting number of try value in the registry\n") );
            return(11);
        }

        /* try to set another section value */
        encodeThis( (_TUCHAR *)CommonData.AUTHLogin.Get(), tmpstr, FALSE );
        lRetCode = RegSetValueEx( hKey1, RegKeyLogin, 0, REG_SZ, (LPBYTE)tmpstr, (DWORD)((_tcslen(tmpstr)+1)*sizeof(_TCHAR)) );

        /* try to set another section value */
        encodeThis( (_TUCHAR *)CommonData.AUTHPassword.Get(), tmpstr, FALSE );
        lRetCode = RegSetValueEx( hKey1, RegKeyPassword, 0, REG_SZ, (LPBYTE)tmpstr, (DWORD)((_tcslen(tmpstr)+1)*sizeof(_TCHAR)) );

        /* try to set another section value */
        lRetCode = RegSetValueEx( hKey1, RegKeySender, 0, REG_SZ, (LPBYTE)CommonData.Sender.Get(), (DWORD)((_tcslen(CommonData.Sender.Get())+1)*sizeof(_TCHAR)) );

        /* if we failed, note it, and leave */
        if ( lRetCode != ERROR_SUCCESS ) {
            if ( !CommonData.quiet )  pMyPrintDLL( __T("Error in setting sender address value in the registry\n") );
            return(11);
        }
    }

#if INCLUDE_NNTP
    if ( CommonData.NNTPHost[0] ) {
        if ( !_tcscmp(CommonData.NNTPHost, __T("-")) ) {
            CommonData.NNTPHost[0] = __T('\0');
            CommonData.NNTPPort[0] = __T('\0');
        }

        /* try to set a section value */
        lRetCode = RegSetValueEx( hKey1, RegKeyNNTPHost, 0, REG_SZ, (LPBYTE)CommonData.NNTPHost, (DWORD)((_tcslen(CommonData.NNTPHost)+1)*sizeof(_TCHAR)) );

        /* if we failed, note it, and leave */
        if ( lRetCode != ERROR_SUCCESS ) {
            if ( !CommonData.quiet )  pMyPrintDLL( __T("Error in setting NNTP server value in the registry\n") );
            return(11);
        }

        /* try to set another section value */
        lRetCode = RegSetValueEx( hKey1, RegKeyNNTPPort, 0, REG_SZ, (LPBYTE)CommonData.NNTPPort, (DWORD)((_tcslen(CommonData.NNTPPort)+1)*sizeof(_TCHAR)) );

        /* if we failed, note it, and leave */
        if ( lRetCode != ERROR_SUCCESS ) {
            if ( !CommonData.quiet )  pMyPrintDLL( __T("Error in setting port value in the registry\n") );
            return(11);
        }

        /* try to set another section value */
        lRetCode = RegSetValueEx( hKey1, RegKeyTry, 0, REG_SZ, (LPBYTE)CommonData.Try, (DWORD)((_tcslen(CommonData.Try)+1)*sizeof(_TCHAR)) );

        /* if we failed, note it, and leave */
        if ( lRetCode != ERROR_SUCCESS ) {
            if ( !CommonData.quiet )  pMyPrintDLL( __T("Error in setting number of try value in the registry\n") );
            return(11);
        }

        /* try to set another section value */
        encodeThis( (_TUCHAR *)CommonData.AUTHLogin.Get(), tmpstr, FALSE );
        lRetCode = RegSetValueEx( hKey1, RegKeyLogin, 0, REG_SZ, (LPBYTE)tmpstr, (DWORD)((_tcslen(tmpstr)+1)*sizeof(_TCHAR)) );

        /* try to set another section value */
        encodeThis( (_TUCHAR *)CommonData.AUTHPassword.Get(), tmpstr, FALSE );
        lRetCode = RegSetValueEx( hKey1, RegKeyPassword, 0, REG_SZ, (LPBYTE)tmpstr, (DWORD)((_tcslen(tmpstr)+1)*sizeof(_TCHAR)) );

        /* try to set another section value */
        lRetCode = RegSetValueEx( hKey1, RegKeySender, 0, REG_SZ, (LPBYTE)CommonData.Sender.Get(), (DWORD)((_tcslen(CommonData.Sender.Get())+1)*sizeof(_TCHAR)) );

        /* if we failed, note it, and leave */
        if ( lRetCode != ERROR_SUCCESS ) {
            if ( !CommonData.quiet )  pMyPrintDLL( __T("Error in setting sender address value in the registry\n") );
            return(11);
        }
    }
#endif

#if INCLUDE_POP3
    if ( CommonData.POP3Host[0] ) {
        if ( !_tcscmp(CommonData.POP3Host, __T("-")) ) {
            CommonData.POP3Host[0] = __T('\0');
            CommonData.POP3Port[0] = __T('\0');
        }

        /* try to set a section value */
        lRetCode = RegSetValueEx( hKey1, RegKeyPOP3Host, 0, REG_SZ, (LPBYTE)CommonData.POP3Host, (DWORD)((_tcslen(CommonData.POP3Host)+1)*sizeof(_TCHAR)) );

        /* if we failed, note it, and leave */
        if ( lRetCode != ERROR_SUCCESS ) {
            if ( !CommonData.quiet )  pMyPrintDLL( __T("Error in setting POP3 server value in the registry\n") );
            return(11);
        }

        /* try to set another section value */
        lRetCode = RegSetValueEx( hKey1, RegKeyPOP3Port, 0, REG_SZ, (LPBYTE)CommonData.POP3Port, (DWORD)((_tcslen(CommonData.POP3Port)+1)*sizeof(_TCHAR)) );

        /* if we failed, note it, and leave */
        if ( lRetCode != ERROR_SUCCESS ) {
            if ( !CommonData.quiet )  pMyPrintDLL( __T("Error in setting port value in the registry\n") );
            return(11);
        }

        encodeThis( (_TUCHAR *)CommonData.POP3Login.Get(), tmpstr, FALSE );
        (void)RegSetValueEx( hKey1, RegKeyPOP3Login, 0, REG_SZ, (LPBYTE)tmpstr, (DWORD)((_tcslen(tmpstr)+1)*sizeof(_TCHAR)) );

        /* try to set another section value */
        encodeThis( (_TUCHAR *)CommonData.POP3Password.Get(), tmpstr, FALSE );
        (void)RegSetValueEx( hKey1, RegKeyPOP3Password, 0, REG_SZ, (LPBYTE)tmpstr, (DWORD)((_tcslen(tmpstr)+1)*sizeof(_TCHAR)) );
    }
#endif

#if INCLUDE_IMAP
    if ( CommonData.IMAPHost[0] ) {
        if ( !_tcscmp(CommonData.IMAPHost, __T("-")) ) {
            CommonData.IMAPHost[0] = __T('\0');
            CommonData.IMAPPort[0] = __T('\0');
        }

        /* try to set a section value */
        lRetCode = RegSetValueEx( hKey1, RegKeyIMAPHost, 0, REG_SZ, (LPBYTE)CommonData.IMAPHost, (DWORD)((_tcslen(CommonData.IMAPHost)+1)*sizeof(_TCHAR)) );

        /* if we failed, note it, and leave */
        if ( lRetCode != ERROR_SUCCESS ) {
            if ( !CommonData.quiet )  pMyPrintDLL( __T("Error in setting IMAP server value in the registry\n") );
            return(11);
        }

        /* try to set another section value */
        lRetCode = RegSetValueEx( hKey1, RegKeyIMAPPort, 0, REG_SZ, (LPBYTE)CommonData.IMAPPort, (DWORD)((_tcslen(CommonData.IMAPPort)+1)*sizeof(_TCHAR)) );

        /* if we failed, note it, and leave */
        if ( lRetCode != ERROR_SUCCESS ) {
            if ( !CommonData.quiet )  pMyPrintDLL( __T("Error in setting port value in the registry\n") );
            return(11);
        }

        encodeThis( (_TUCHAR *)CommonData.IMAPLogin.Get(), tmpstr, FALSE );
        (void)RegSetValueEx( hKey1, RegKeyIMAPLogin, 0, REG_SZ, (LPBYTE)tmpstr, (DWORD)((_tcslen(tmpstr)+1)*sizeof(_TCHAR)) );

        /* try to set another section value */
        encodeThis( (_TUCHAR *)CommonData.IMAPPassword.Get(), tmpstr, FALSE );
        (void)RegSetValueEx( hKey1, RegKeyIMAPPassword, 0, REG_SZ, (LPBYTE)tmpstr, (DWORD)((_tcslen(tmpstr)+1)*sizeof(_TCHAR)) );
    }
#endif

    return(0);
}

// create registry entries for this program
int CreateRegEntry( COMMON_DATA & CommonData, int useHKCU )
{
    int retval;

    retval = 1;     // assume useHKCU set, so the 2nd if() works.
    if ( !useHKCU )
        retval = CreateRegEntry( CommonData, HKEY_LOCAL_MACHINE );

    if ( retval )
        retval = CreateRegEntry( CommonData, HKEY_CURRENT_USER );

    return retval;
}

/**************************************************************
 **************************************************************/

void ShowRegHelp( COMMON_DATA & CommonData )
{
    printMsg( CommonData, __T("BLAT PROFILE EDITOR\n") );
    printMsg( CommonData, __T("To modify SMTP: blat -install SMTPHost Sender [Try [Port [Profile [Login name\n") );
    printMsg( CommonData, __T("                     [Password]]]]]\n") );
    printMsg( CommonData, __T("            or: blat -installSMTP SMTPHost Sender [Try [Port [Profile\n") );
    printMsg( CommonData, __T("                     [Login name [Password]]]]]\n") );
#if INCLUDE_NNTP
    printMsg( CommonData, __T("To modify NNTP: blat -installNNTP NNTPHost Sender [Try [Port [Profile\n") );
    printMsg( CommonData, __T("                     [Login name [Password]]]]]\n") );
#endif
#if INCLUDE_POP3
    printMsg( CommonData, __T("To modify POP3: blat -installPOP3 POP3Host - - [Port [Profile\n") );
    printMsg( CommonData, __T("                     [Login name [Password]]]]\n") );
#endif
#if INCLUDE_IMAP
    printMsg( CommonData, __T("To modify IMAP: blat -installIMAP IMAPHost - - [Port [Profile\n") );
    printMsg( CommonData, __T("                     [Login name [Password]]]]\n") );
#endif
    printMsg( CommonData, __T("To delete:      blat -profile -delete Profile\n") );
    printMsg( CommonData, __T("Profiles are listed as in the -install option:\n") );
    printMsg( CommonData, __T("Host Sender Try Port Profile login_name password\n") );
    printMsg( CommonData, __T("\n") );
}


static int DeleteRegTree( COMMON_DATA & CommonData, HKEY rootKeyLevel, LPTSTR pstrProfile )
{
    HKEY     hKey1=NULL;
    DWORD    dwBytesRead;
    LONG     lRetCode;
    DWORD    dwIndex;                               // index of subkey to enumerate
    FILETIME lftLastWriteTime;
    LPTSTR   newProfile;

    // open the registry key in read mode
    lRetCode = RegOpenKeyEx( rootKeyLevel, pstrProfile, 0, KEY_ALL_ACCESS, &hKey1 );
    dwIndex = 0;
    for ( ; lRetCode == 0; ) {
        dwBytesRead = (sizeof(CommonData.Profile)/sizeof(CommonData.Profile[0]));
        lRetCode = RegEnumKeyEx(  hKey1,     // handle of key to enumerate
                                  dwIndex++, // index of subkey to enumerate
                                  CommonData.Profile,   // address of buffer for subkey name
                                  &dwBytesRead,    // address for size of subkey buffer
                                  NULL,      // reserved
                                  NULL,      // address of buffer for class string
                                  NULL,      // address for size of class buffer
                                  &lftLastWriteTime
                                  // address for time key last written to);
                               );
        if ( lRetCode == ERROR_NO_MORE_ITEMS ) {
            lRetCode = 0;
            break;
        }

        if ( lRetCode == 0 ) {
            newProfile = new _TCHAR[_tcslen(pstrProfile)+_tcslen(CommonData.Profile)+2];
            _stprintf(newProfile, __T("%s\\%s"),pstrProfile,CommonData.Profile);
            lRetCode = DeleteRegTree( CommonData, rootKeyLevel, newProfile );
            delete [] newProfile;
        }
        if ( lRetCode == 0 ) {
            lRetCode = RegDeleteKey( hKey1, CommonData.Profile );
            if ( lRetCode != ERROR_SUCCESS ) {
                if ( !CommonData.quiet ) {
                    pMyPrintDLL( __T("Error in deleting profile ") );
                    pMyPrintDLL( pstrProfile);
                    pMyPrintDLL( __T(" in the registry\n") );
                }
                return(11);
            }
            dwIndex--;
        }
    }

    RegCloseKey(hKey1);
    return lRetCode;
}

// Delete registry entries for this program
int DeleteRegEntry( COMMON_DATA & CommonData, LPTSTR pstrProfile, int useHKCU )
{
    HKEY   rootKeyLevel;
    HKEY   hKey1=NULL;
    LONG   lRetCode;
    _TCHAR strRegisterKey[256];


    if ( useHKCU )
        rootKeyLevel = HKEY_CURRENT_USER;
    else
        rootKeyLevel = HKEY_LOCAL_MACHINE;

#if defined(_WIN64)
    _stprintf(strRegisterKey, __T("%s%s%s"), SoftwareRegKey, Wow6432NodeKey, MainRegKey);
    if ( RegOpenKeyEx ( rootKeyLevel, strRegisterKey, 0, KEY_ALL_ACCESS, &hKey1 ) == ERROR_SUCCESS )
        RegCloseKey(hKey1);
    else
#endif
        _stprintf(strRegisterKey, __T("%s%s"), SoftwareRegKey, MainRegKey);

    if ( !_tcscmp(pstrProfile, __T("<default>")) ) {
        DWORD dwBytesRead;
        DWORD dwIndex;      // index of subkey to enumerate

        lRetCode = RegOpenKeyEx ( rootKeyLevel, strRegisterKey, 0, KEY_ALL_ACCESS, &hKey1 );

        /* if we failed, note it, and leave */
        if ( lRetCode != ERROR_SUCCESS ) {
            if ( !CommonData.quiet )  pMyPrintDLL( __T("Error in finding blat default profile in the registry\n") );
            return(10);
        }

        dwIndex = 0;
        for ( ; lRetCode == 0; ) {
            dwBytesRead = (sizeof(CommonData.Profile)/sizeof(CommonData.Profile[0]));
            lRetCode = RegEnumValue(  hKey1,     // handle of value to enumerate
                                      dwIndex++, // index of subkey to enumerate
                                      CommonData.Profile,   // address of buffer for subkey name
                                      &dwBytesRead,    // address for size of subkey buffer
                                      NULL,      // reserved
                                      NULL,      // address of buffer for key type
                                      NULL,      // address of buffer for value data
                                      NULL       // address for length of value data );
                                   );
            if ( lRetCode == ERROR_NO_MORE_ITEMS ) {
                lRetCode = 0;
                break;
            }

            if ( lRetCode == 0 ) {
                lRetCode = RegDeleteValue( hKey1, CommonData.Profile );
                if ( lRetCode != ERROR_SUCCESS ) {
                    if ( !CommonData.quiet ) {
                        pMyPrintDLL( __T("Error in deleting profile ") );
                        pMyPrintDLL( pstrProfile);
                        pMyPrintDLL( __T(" in the registry\n") );
                    }
                    return(11);
                }
                dwIndex--;
            }
        }

        RegCloseKey(hKey1);
    } else
    if ( !_tcscmp(pstrProfile, __T("<all>")) ) {
        DeleteRegTree( CommonData, rootKeyLevel, strRegisterKey );

        // Attempt to delete the main Blat key.
        lRetCode = RegOpenKeyEx ( rootKeyLevel, strRegisterKey, 0, KEY_ALL_ACCESS, &hKey1 );
        if ( lRetCode == ERROR_SUCCESS ) {
            lRetCode = RegDeleteKey( hKey1, __T("") );
            RegCloseKey(hKey1);
            *_tcsrchr(strRegisterKey,__T('\\')) = __T('\0');

            // Attempt to delete the Public Domain key
            lRetCode = RegOpenKeyEx ( rootKeyLevel, strRegisterKey, 0, KEY_ALL_ACCESS, &hKey1 );
            if ( lRetCode == ERROR_SUCCESS ) {
                lRetCode = RegDeleteKey( hKey1, __T("") );
                RegCloseKey(hKey1);
            }
        }
    } else
    if ( pstrProfile[0] != __T('\0') ) {
        _tcscat(strRegisterKey, __T("\\") );
        _tcscat(strRegisterKey, pstrProfile);

        lRetCode = DeleteRegTree( CommonData, rootKeyLevel, strRegisterKey );

        /* if we failed, note it, and leave */
        if ( lRetCode != ERROR_SUCCESS ) {
            if ( !CommonData.quiet ) {
                pMyPrintDLL( __T("Error in deleting profile ") );
                pMyPrintDLL( pstrProfile);
                pMyPrintDLL( __T(" in the registry\n") );
            }
            return(11);
        }

        lRetCode = RegOpenKeyEx ( rootKeyLevel, strRegisterKey, 0, KEY_ALL_ACCESS, &hKey1 );
        if ( lRetCode == ERROR_SUCCESS ) {
            lRetCode = RegDeleteKey( hKey1, __T("") );
            RegCloseKey(hKey1);
        }
        /* if we failed, note it, and leave */
        if ( lRetCode != ERROR_SUCCESS ) {
            if ( !CommonData.quiet ) {
                pMyPrintDLL( __T("Error in finding blat profile ") );
                pMyPrintDLL( pstrProfile);
                pMyPrintDLL( __T(" in the registry\n") );
            }
            return(10);
        }

    }

    return(0);
}

/**************************************************************
 **************************************************************/

// get the registry entries for this program
static int GetRegEntryKeyed( COMMON_DATA & CommonData, HKEY rootKeyLevel, LPTSTR pstrProfile )
{
    HKEY   hKey1=NULL;
    DWORD  dwType;
    DWORD  dwBytesRead;
    LONG   lRetCode;
    Buf    register_key;
    _TCHAR tmpstr[(MAXOUTLINE + 2) * 2];
    int    retval;


    hKey1 = NULL;

#if defined(_WIN64)
    register_key = SoftwareRegKey;
    register_key.Add( Wow6432NodeKey );
    register_key.Add( MainRegKey );

    if ( pstrProfile && *pstrProfile ) {
        register_key.Add( __T("\\") );
        register_key.Add( pstrProfile );
    }

    // open the registry key in read mode
    lRetCode = RegOpenKeyEx( rootKeyLevel, register_key.Get(), 0, KEY_READ, &hKey1 );
    if ( lRetCode != ERROR_SUCCESS )
#endif
    {
        register_key = SoftwareRegKey;
        register_key.Add( MainRegKey );

        if ( pstrProfile && *pstrProfile ) {
            register_key.Add( __T("\\") );
            register_key.Add( pstrProfile );
        }

        // open the registry key in read mode
        lRetCode = RegOpenKeyEx( rootKeyLevel, register_key.Get(), 0, KEY_READ, &hKey1 );
    }
    register_key.Free();

    if ( lRetCode != ERROR_SUCCESS ) {
//       printMsg( CommonData, __T("Failed to open registry key for Blat\n") );
        if ( pstrProfile && *pstrProfile )
            return GetRegEntryKeyed( CommonData, rootKeyLevel, NULL );

        return(12);
    }

    // set the size of the buffer to contain the data returned from the registry
    // thanks to Beverly Brown "beverly@datacube.com" and "chick@cyberspace.com" for spotting it...
    dwBytesRead = SENDER_SIZE;
    // read the value of the SMTP server entry
    lRetCode = RegQueryValueEx( hKey1, RegKeySender, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );    //lint !e545
    if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) )
        CommonData.Sender = tmpstr;
    else
        CommonData.Sender.Clear();

    dwBytesRead = TRY_SIZE;
    // read the value of the number of try entry
    lRetCode = RegQueryValueEx( hKey1, RegKeyTry, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );    //lint !e545
    // if we failed, assign a default value
    if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) )
        _tcscpy( CommonData.Try, tmpstr );
    else
        _tcscpy(CommonData.Try, __T("1") );

    dwBytesRead = (sizeof(tmpstr)/sizeof(tmpstr[0])) - 1;
    // read the value of the SMTP server entry
    lRetCode = RegQueryValueEx( hKey1, RegKeyLogin, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );
    if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) ) {
        CommonData.AUTHLogin.Clear();
        CommonData.AUTHLogin.Alloc(dwBytesRead*4);
        decodeThis( tmpstr, CommonData.AUTHLogin.Get() );
        CommonData.AUTHLogin.SetLength();
    }

    dwBytesRead = (sizeof(tmpstr)/sizeof(tmpstr[0])) - 1;
    // read the value of the SMTP server entry
    lRetCode = RegQueryValueEx( hKey1, RegKeyPassword, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );
    if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) ) {
        CommonData.AUTHPassword.Clear();
        CommonData.AUTHPassword.Alloc(dwBytesRead*4);
        decodeThis( tmpstr, CommonData.AUTHPassword.Get() );
        CommonData.AUTHPassword.SetLength();
    }

    dwBytesRead = SERVER_SIZE;
    // read the value of the SMTP server entry
    lRetCode = RegQueryValueEx( hKey1, RegKeySMTPHost, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );
    // if we got it, then get the smtp port.
    if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) ) {
        _tcscpy( CommonData.SMTPHost, tmpstr );

        dwBytesRead = SERVER_SIZE;
        // read the value of the SMTP port entry
        lRetCode = RegQueryValueEx( hKey1, RegKeySMTPPort, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );
        // if we got it, then convert it back to Unicode
        if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) )
            _tcscpy( CommonData.SMTPPort, tmpstr );
        else
            _tcscpy( CommonData.SMTPPort, defaultSMTPPort );
    } else {
        CommonData.SMTPHost[0] = __T('\0');
        CommonData.SMTPPort[0] = __T('\0');
    }

#if INCLUDE_NNTP
    dwBytesRead = SERVER_SIZE;
    // read the value of the NNTP server entry
    lRetCode = RegQueryValueEx( hKey1, RegKeyNNTPHost, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );
    // if we got it, then get the nntp port.
    if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) ) {
        _tcscpy( CommonData.NNTPHost, tmpstr );

        dwBytesRead = SERVER_SIZE;
        // read the value of the NNTP port entry
        lRetCode = RegQueryValueEx( hKey1, RegKeyNNTPPort, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );
        // if we got it,
        if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) )
            _tcscpy( CommonData.NNTPPort, tmpstr );
        else
            _tcscpy( CommonData.NNTPPort, defaultNNTPPort );
    } else {
        CommonData.NNTPHost[0] = __T('\0');
        CommonData.NNTPPort[0] = __T('\0');
    }
#endif

#if INCLUDE_POP3
    dwBytesRead = SERVER_SIZE;
    // read the value of the POP3 server entry
    lRetCode = RegQueryValueEx( hKey1, RegKeyPOP3Host, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );
    // if we got it, then get the POP3 port.
    if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) ) {
        _tcscpy( CommonData.POP3Host, tmpstr );

        dwBytesRead = SERVER_SIZE;
        // read the value of the POP3 port entry
        lRetCode = RegQueryValueEx( hKey1, RegKeyPOP3Port, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );
        // if we got it,
        if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) )
            _tcscpy( CommonData.POP3Port, tmpstr );
        else
            _tcscpy( CommonData.POP3Port, defaultPOP3Port );

        dwBytesRead = (sizeof(tmpstr)/sizeof(tmpstr[0])) - 1;
        // read the value of the POP3 login id
        lRetCode = RegQueryValueEx( hKey1, RegKeyPOP3Login, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );
        // if we got it,
        if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) ) {
            CommonData.POP3Login.Clear();
            CommonData.POP3Login.Alloc(dwBytesRead*4);
            decodeThis( tmpstr, CommonData.POP3Login.Get() );
            CommonData.POP3Login.SetLength();
        } else
            CommonData.POP3Login = CommonData.AUTHLogin;

        dwBytesRead = (sizeof(tmpstr)/sizeof(tmpstr[0])) - 1;
        // read the value of the POP3 password
        lRetCode = RegQueryValueEx( hKey1, RegKeyPOP3Password, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );
        // if we got it,
        if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) ) {
            CommonData.POP3Password.Clear();
            CommonData.POP3Password.Alloc(dwBytesRead*4);
            decodeThis( tmpstr, CommonData.POP3Password.Get() );
            CommonData.POP3Password.SetLength();
        } else
            CommonData.POP3Password = CommonData.AUTHPassword;
    } else {
        CommonData.POP3Host[0] = __T('\0');
        CommonData.POP3Port[0] = __T('\0');
    }
#endif

#if INCLUDE_IMAP
    dwBytesRead = SERVER_SIZE;
    // read the value of the IMAP server entry
    lRetCode = RegQueryValueEx( hKey1, RegKeyIMAPHost, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );
    // if we got it, then get the IMAP port.
    if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) ) {
        _tcscpy( CommonData.IMAPHost, tmpstr );

        dwBytesRead = SERVER_SIZE;
        // read the value of the IMAP port entry
        lRetCode = RegQueryValueEx( hKey1, RegKeyIMAPPort, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );
        // if we got it,
        if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) )
            _tcscpy( CommonData.IMAPPort, tmpstr );
        else
            _tcscpy( CommonData.IMAPPort, defaultIMAPPort );

        dwBytesRead = (sizeof(tmpstr)/sizeof(tmpstr[0])) - 1;
        // read the value of the IMAP login id
        lRetCode = RegQueryValueEx( hKey1, RegKeyIMAPLogin, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );
        // if we got it,
        if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) ) {
            CommonData.IMAPLogin.Clear();
            CommonData.IMAPLogin.Alloc(dwBytesRead*4);
            decodeThis( tmpstr, CommonData.IMAPLogin.Get() );
            CommonData.IMAPLogin.SetLength();
       } else
            CommonData.IMAPLogin = CommonData.AUTHLogin;

        dwBytesRead = (sizeof(tmpstr)/sizeof(tmpstr[0])) - 1;
        // read the value of the IMAP password
        lRetCode = RegQueryValueEx( hKey1, RegKeyIMAPPassword, NULL, &dwType, (LPBYTE)tmpstr, &dwBytesRead );
        // if we got it,
        if ( dwBytesRead && (lRetCode == ERROR_SUCCESS) ) {
            CommonData.IMAPPassword.Clear();
            CommonData.IMAPPassword.Alloc(dwBytesRead*4);
            decodeThis( tmpstr, CommonData.IMAPPassword.Get() );
            CommonData.IMAPPassword.SetLength();
       } else
            CommonData.IMAPPassword = CommonData.AUTHPassword;
    } else {
        CommonData.IMAPHost[0] = __T('\0');
        CommonData.IMAPPort[0] = __T('\0');
    }
#endif

    if ( hKey1 )
        RegCloseKey(hKey1);

    // if the host is not specified, or if the sender is not specified,
    // leave the routine with error 12.
    retval = 12;
    if ( CommonData.Sender.Get()[0] ) {
        if ( CommonData.SMTPHost[0] )
            retval = 0;

#if INCLUDE_POP3
        if ( CommonData.POP3Host[0] )
            retval = 0;
#endif

#if INCLUDE_IMAP
        if ( CommonData.IMAPHost[0] )
            retval = 0;
#endif

#if INCLUDE_NNTP
        if ( CommonData.NNTPHost[0] )
            retval = 0;
#endif
    }

    return retval;
}


int GetRegEntry( COMMON_DATA & CommonData, LPTSTR pstrProfile )
{
    int lRetVal;

    lRetVal = GetRegEntryKeyed( CommonData, HKEY_CURRENT_USER, pstrProfile );
    if ( lRetVal )
        lRetVal = GetRegEntryKeyed( CommonData, HKEY_LOCAL_MACHINE, pstrProfile );

    return lRetVal;
}

static void DisplayThisProfile( COMMON_DATA & CommonData, HKEY rootKeyLevel, LPTSTR pstrProfile )
{
    LPTSTR tmpstr;

    GetRegEntryKeyed( CommonData, rootKeyLevel, pstrProfile );

    tmpstr = __T("");
    if ( CommonData.AUTHLogin.Get()[0] || CommonData.AUTHPassword.Get()[0] )
        tmpstr = __T(" ***** *****");

    if ( CommonData.SMTPHost[0] ) {
        //printMsg(CommonData, __T("SMTP: %s %s %s %s %s %s %s\n"),CommonData.SMTPHost,CommonData.Sender.Get(),CommonData.Try,CommonData.SMTPPort,CommonData.Profile,CommonData.AUTHLogin.Get(),CommonData.AUTHPassword.Get());
        printMsg(CommonData, __T("%s: %s \"%s\" %s %s %s%s\n"),
                 __T("SMTP"), CommonData.SMTPHost, CommonData.Sender.Get(), CommonData.Try, CommonData.SMTPPort, CommonData.Profile, tmpstr);
    }

#if INCLUDE_NNTP
    if ( CommonData.NNTPHost[0] ) {
        //printMsg(CommonData, __T("NNTP: %s %s %s %s %s %s %s\n"),CommonData.NNTPHost,CommonData.Sender.Get(),CommonData.Try,CommonData.NNTPPort,CommonData.Profile,CommonData.AUTHLogin.Get(),CommonData.AUTHPassword.Get());
        printMsg(CommonData, __T("%s: %s \"%s\" %s %s %s%s\n"),
                 __T("NNTP"), CommonData.NNTPHost, CommonData.Sender.Get(), CommonData.Try, CommonData.NNTPPort, CommonData.Profile, tmpstr);
    }
#endif
#if INCLUDE_POP3
    tmpstr = __T("");
    if ( CommonData.POP3Login.Get()[0] || CommonData.POP3Password.Get()[0] )
        tmpstr = __T(" ***** *****");

    if ( CommonData.POP3Host[0] ) {
        //printMsg(CommonData, __T("POP3: %s - - %s %s %s %s\n"),CommonData.POP3Host,CommonData.POP3Port,CommonData.Profile,CommonData.POP3Login.Get(),CommonData.POP3Password.Get());
        printMsg(CommonData, __T("%s: %s - - %s %s%s\n"),
                 __T("POP3"), CommonData.POP3Host, CommonData.POP3Port, CommonData.Profile, tmpstr);
    }
#endif
#if INCLUDE_IMAP
    tmpstr = __T("");
    if ( CommonData.IMAPLogin.Get()[0] || CommonData.IMAPPassword.Get()[0] )
        tmpstr = __T(" ***** *****");

    if ( CommonData.IMAPHost[0] ) {
        //printMsg(CommonData, __T("IMAP: %s - - %s %s %s %s\n"),CommonData.IMAPHost,CommonData.IMAPPort,CommonData.Profile,CommonData.IMAPLogin.Get(),CommonData.IMAPPassword.Get());
        printMsg(CommonData, __T("%s: %s - - %s %s%s\n"),
                 __T("IMAP"), CommonData.IMAPHost, CommonData.IMAPPort, CommonData.Profile, tmpstr);
    }
#endif
}

static void DumpProfiles( COMMON_DATA & CommonData, HKEY rootKeyLevel, LPTSTR pstrProfile )
{
    HKEY     hKey1=NULL;
    DWORD    dwBytesRead;
    LONG     lRetCode;
    DWORD    dwIndex;                               // index of subkey to enumerate
    FILETIME lftLastWriteTime;                      // address for time key last written to
    _TCHAR   strRegisterKey[256];


#if defined(_WIN64)
    _stprintf(strRegisterKey, __T("%s%s%s"), SoftwareRegKey, Wow6432NodeKey, MainRegKey);
    /* try to open the registry key */
    lRetCode = RegOpenKeyEx( rootKeyLevel,
                             strRegisterKey,
                             0, KEY_READ, &hKey1
                           );
    /* if we failed, check the standard 64-bit tree */
    if ( lRetCode != ERROR_SUCCESS )
#endif
    {
        _stprintf(strRegisterKey, __T("%s%s"), SoftwareRegKey, MainRegKey);
        /* try to open the registry key */
        lRetCode = RegOpenKeyEx( rootKeyLevel,
                                 strRegisterKey,
                                 0, KEY_READ, &hKey1
                               );
    }
    if ( lRetCode != ERROR_SUCCESS ) {
        printMsg( CommonData, __T("Failed to open registry key for Blat using default.\n") );
    } else {
        CommonData.quiet      = FALSE;
        CommonData.Profile[0] = __T('\0');

        if ( !_tcscmp(pstrProfile, __T("<default>")) || !_tcscmp(pstrProfile, __T("<all>")) ) {
            DisplayThisProfile( CommonData, rootKeyLevel, __T("") );
        }

        dwIndex = 0;
        do {
            dwBytesRead = (sizeof(CommonData.Profile)/sizeof(CommonData.Profile[0]));
            lRetCode = RegEnumKeyEx(  hKey1,     // handle of key to enumerate
                                      dwIndex++, // index of subkey to enumerate
                                      CommonData.Profile,   // address of buffer for subkey name
                                      &dwBytesRead,    // address for size of subkey buffer
                                      NULL,      // reserved
                                      NULL,      // address of buffer for class string
                                      NULL,      // address for size of class buffer
                                      &lftLastWriteTime
                                      // address for time key last written to);
                                   );
            if ( lRetCode == 0 ) {
                if ( !_tcscmp(pstrProfile,CommonData.Profile) || !_tcscmp(pstrProfile, __T("<all>")) ) {
                    DisplayThisProfile( CommonData, rootKeyLevel, CommonData.Profile );
                }
            }
        } while ( lRetCode == 0 );

        RegCloseKey(hKey1);
    }
}

// List all profiles
void ListProfiles( COMMON_DATA & CommonData, LPTSTR pstrProfile )
{
    HKEY     hKey1=NULL;
    LONG     lRetCode;
    _TCHAR   strRegisterKey[256];


#if defined(_WIN64)
    _stprintf(strRegisterKey, __T("%s%s%s"), SoftwareRegKey, Wow6432NodeKey, MainRegKey);
    // open the HKCU registry key in read mode
    lRetCode = RegOpenKeyEx( HKEY_CURRENT_USER,
                             strRegisterKey,
                             0, KEY_READ, &hKey1
                           );
    /* if we failed, check the standard 64-bit tree */
    if ( lRetCode != ERROR_SUCCESS )
#endif
    {
        _stprintf(strRegisterKey, __T("%s%s"), SoftwareRegKey, MainRegKey);
        // open the HKCU registry key in read mode
        lRetCode = RegOpenKeyEx( HKEY_CURRENT_USER,
                                 strRegisterKey,
                                 0, KEY_READ, &hKey1
                               );
    }
    if ( lRetCode == ERROR_SUCCESS ) {
        RegCloseKey(hKey1);
        printMsg( CommonData, __T("Profile(s) for current user --\n") );
        DumpProfiles( CommonData, HKEY_CURRENT_USER, pstrProfile );
        printMsg( CommonData, __T("\n") );
    }

    printMsg( CommonData, __T("Profile(s) for all users of this computer --\n") );
    DumpProfiles( CommonData, HKEY_LOCAL_MACHINE, pstrProfile );
}
