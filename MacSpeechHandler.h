#ifndef __MacSpeechHandler_H__
#define __MacSpeechHandler_H__

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import "MacSpeechHandlerDelegate.h"

#include "Voice_Adapter.hh"
#include "SpeechHandler.hh"

class MacSpeechHandler : public SpeechHandler
{

public:
  MacSpeechHandler(Voice_Adapter* adapter);
  ~MacSpeechHandler();
  void handleSpeechCommand(NSString* cmd);
  void listen();
  void suspend();

private:
    NSAutoreleasePool* pool;
    NSSpeechRecognizer* recog;
    MacSpeechHandlerDelegate* delegate;
};

#endif /* __MacSpeechHandler_H__ */
