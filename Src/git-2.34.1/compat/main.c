#include "../git-compat-util.h"
#include "win32.h"
#include <conio.h>
#include <wchar.h>
#include "../strbuf.h"
#include "../run-command.h"
#include "../cache.h"
#include "win32/lazyload.h"
#include "../config.h"
#include "dir.h"

// https://github.com/coderforlife/mingw-unicode-main/blob/master/mingw-unicode.c
// Code below is released into the public domain. Absolutely no warranty is provided.
// See http://www.coderforlife.com/projects/utilities.
//
// This is for the MinGW compiler which does not support wmain.
// It is a wrapper for _tmain when _UNICODE is defined (wmain).
//
// !! Do not compile this file, but instead include it right before your _tmain function like:
// #include "mingw-unicode.c"
// int _tmain(int argc, _TCHAR *argv[]) {
//
// If you wish to have enpv in your main, then define the following before including this file:
// #define MAIN_USE_ENVP
//
// This wrapper adds ~300 bytes to the program and negligible overhead

#ifndef __MSVCRT__
#error Unicode main function requires linking to MSVCRT
#endif

#include <wchar.h>
#include <stdlib.h>

extern int _CRT_glob;
void __wgetmainargs(int*,wchar_t***,wchar_t***,int,int*);

__declspec(dllimport) int git_wmain(int argc, const wchar_t **wargv);

int main()
{
	wchar_t **enpv, **argv;
	int argc, si = 0;
	__wgetmainargs(&argc, &argv, &enpv, _CRT_glob, &si); // this also creates the global variable __wargv
	return git_wmain(argc, argv);
}
