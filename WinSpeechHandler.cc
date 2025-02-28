/*	Script_Evaluator.cc

HiROC CVS ID: $Id: Image_Info_Panel.hh,v 1.14 2014/08/05 17:58:09 stephens Exp $

Copyright (C) 2010-2011  Arizona Board of Regents on behalf of the
Planetary Image Research Laboratory, Lunar and Planetary Laboratory at
the University of Arizona.

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License, version 2.1,
as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.

*******************************************************************************/
#include "WinSpeechHandler.hh"
#include "Voice_Adapter.hh"

#ifndef QT_NO_DEBUG_OUTPUT
#include <QDebug>
#endif

#include <sphelper.h>
#include <sapi.h>

#include <string>
#include <codecvt>
//#include <iostream>

// deprecated, use Windows MultiByteToWideChar
static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> cvt;

WinSpeechHandler::WinSpeechHandler(Voice_Adapter* adapter) : SpeechHandler(adapter)
{
    recoContext = nullptr;
    recoGrammar = nullptr;
    recoInstance = nullptr;
}

WinSpeechHandler::~WinSpeechHandler()
{
    suspend();
}

void WinSpeechHandler::listen()
{
#ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Initializing speech recognizer";
#endif
    //CComPtr<ISpRecognizer> recoEngine;
    ::CoInitialize(nullptr);

    /* Initialize COM library
    if (FAILED(::CoInitialize(nullptr)))
    {
        throw "Failed to initialize Windows Speech API";
    }
    */
/*
    HRESULT result;

    // https://docs.microsoft.com/en-us/windows/desktop/api/combaseapi/nf-combaseapi-coinitializeex
    if (!SUCCEEDED(result = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
    {
        qDebug() << hex << (result & 0xFFFF);
        throw "Failed to initialize Windows Speech API";
    }

    HRESULT result = CoInitializeEx(nullptr, COINIT_SPEED_OVER_MEMORY);
    if (result != S_OK && result != S_FALSE)
    {
        throw "Failed to initialize Windows Speech API";
    }
*/
    //recoEngine.CoCreateInstance(CLSID_SpSharedRecognizer);
    //recoEngine->CreateRecoContext(&recoContext);
    /* TODO sometimes it fails here */
    if (!SUCCEEDED(CoCreateInstance(
            CLSID_SpSharedRecognizer, // class ID
            nullptr, // aggregate
            CLSCTX_ALL, // context type
            IID_ISpRecognizer, // interface
            reinterpret_cast<void**>(&recoInstance) // address of receiver
    ))
    )
    {
        throw  "Failed to create instance of Windows Speech Recognizer";
    }

    if (!SUCCEEDED(recoInstance->CreateRecoContext(&recoContext)))
    {
        throw "Failed to initialize context of Windows Speech Recognizer";
    }

    if (!SUCCEEDED(recoContext->Pause(0))) // TODO(guym) needed?
    {
        throw "Failure in context for Windows Speech Recognizer";
    }

    if (!SUCCEEDED(recoContext->CreateGrammar(0, &recoGrammar))) // id
    {
        throw "Failed to create grammar for Windows Speech Recognizer";
    }

    SPSTATEHANDLE state;
    auto ruleName1 = L"ruleName1";
    //auto attributes = SPRAF_TopLevel | SPRAF_Active | SPRAF_UserDelimited;
    // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee450815(v%3dvs.85)
    if (!SUCCEEDED(recoGrammar->GetRule(ruleName1, 0, SPRAF_TopLevel | SPRAF_Active, true, &state)))
    {
        throw "Could not create grammar ruleset for Windows Speech Recognizer";
    }

    for (const auto & cmd : Voice_Adapter::COMMANDS)
    {
        const std::wstring str = cvt.from_bytes(cmd);
        //std::wcerr << str.c_str() << std::endl;

        // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee450811(v%3dvs.85)
        if (!SUCCEEDED(recoGrammar->AddWordTransition(state, nullptr, str.c_str(), L" ", SPWT_LEXICAL_NO_SPECIAL_CHARS, 1, nullptr)))
        {
            throw "Could not initialize grammar for Windows Speech Recognizer";
        }
        // TODO(guym) release str?
    }

    // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee450813(v%3dvs.85)
    if (!SUCCEEDED(recoGrammar->Commit(0)))
    {
        throw "Could not commit grammar for Windows Speech Recognizer";
    }

    if (!SUCCEEDED(recoContext->SetNotifyWin32Event()))
    {
        throw "Could not set up event handling for Windows Speech Recognizer";
    }

    HANDLE handleEvent = recoContext->GetNotifyEventHandle();
    if (handleEvent == INVALID_HANDLE_VALUE)
    {
        throw "Failed to obtain Windows Speech Recognizer event handler";
    }

    /*auto interest = SPFEI(SPEI_RECOGNITION) | SPFEI(SPEI_HYPOTHESIS) | SPFEI(SPEI_FALSE_RECOGNITION);

    // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee450801%28v%3Dvs.85%29
    if (!SUCCEEDED(recoContext->SetInterest(interest, interest)))
    {
        throw "Could not set event handling flags for Windows Speech Recognizer";

    }*/

    // Activate Grammar
    if (!SUCCEEDED(recoGrammar->SetRuleState(ruleName1, 0, SPRS_ACTIVE)))
    {
        throw "Could not activate grammar for Windows Speech Recognizer";
    }

    // Enable context
    if (!SUCCEEDED(recoContext->Resume(0)))
    {
        throw "Failed to start context for Windows Speech Recognizer";
    }

    qDebug() << "Waiting for speech input";

    // Wait for reco
    //HANDLE handles[1];
    //handles[0] = handleEvent;
    //WaitForMultipleObjects(1, handles, FALSE, INFINITE);

    // use ISpNotifyCallback instead?
    recoContext->SetNotifyCallbackInterface(this, 0, NULL);

}

HRESULT WinSpeechHandler::NotifyCallback(WPARAM   wParam,
   LPARAM   lParam
        )
{
    //qDebug() << "Callback Notified";

    const ULONG maxEvents = 10;
    SPEVENT events[maxEvents];

    ULONG eventCount;
    HRESULT hr;

    if (!SUCCEEDED(recoContext->GetEvents(maxEvents, events, &eventCount)))
    {
#ifndef QT_NO_DEBUG_OUTPUT
        qDebug() << " Failed to get any speech events";
#endif
        return E_FAIL;
    }


    ISpRecoResult* recoResult;
    recoResult = reinterpret_cast<ISpRecoResult*>(events[0].lParam);

    wchar_t* text;

    // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee450916%28v%3dvs.85%29
    if (SUCCEEDED(recoResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, FALSE, &text, nullptr)))
    {
        voice_adapter->receiveCommand(cvt.to_bytes(text));
    }
#ifndef QT_NO_DEBUG_OUTPUT
    else
    {
        qDebug() << "Failed to get text from speech event";
    }
#endif

    recoResult->Release();
    CoTaskMemFree(text);

    return S_OK;
}

void WinSpeechHandler::suspend()
{
#ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Suspending Speech Recognizer";
#endif
    if (recoGrammar != nullptr) recoGrammar->Release();
    if (recoContext != nullptr) recoContext->Release();
    if (recoInstance != nullptr)recoInstance->Release();
    ::CoUninitialize();
    CoFreeUnusedLibraries();

#ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "Suspended.";
#endif
}
