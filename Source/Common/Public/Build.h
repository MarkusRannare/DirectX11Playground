#pragma once

#ifdef __INTELLISENSE__
#define MOGET_WINDOWS
#endif

// Set how we should declare imports and exports
#ifdef MOGET_WINDOWS
#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_IMPORT
#define DLL_EXPORT
#endif