#include <iostream>
#include "MacSpeechHandler.h"
#include "Voice_Adapter.hh"
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import "MacSpeechHandlerDelegate.h"

MacSpeechHandler::MacSpeechHandler(Voice_Adapter* adapter) : SpeechHandler(adapter)
{
    pool = [[NSAutoreleasePool alloc] init];
    NSArray *cmds = [NSArray arrayWithObjects:@"Pan Up", @"Pan Left", @"Pan Right", @"Pan Down", @"Full Size", @"Fit Image", @"Zoom In", @"Zoom Out", 
    @"Up", @"Left", @"Right", @"Down", @"Full", @"Fit", @"In", @"Out", @"Enhance", @"Restore", nil];
    recog = [[NSSpeechRecognizer alloc] init]; // recog is an ivar
    [recog setCommands:cmds];
    [recog setDisplayedCommandsTitle:@"Pan and Zoom"];
    delegate = [[MacSpeechHandlerDelegate alloc] init: this]; // passing this?

    if (delegate)
    {
       [recog setDelegate:delegate];
    }
    else
    {
       std::cout << "Unable to initialize NSSpeechHandlerDelegate" << std::endl;
    }
    
    voice_adapter = adapter;
}

MacSpeechHandler::~MacSpeechHandler()
{
  suspend();
  [recog release];
  recog = nil;
  [pool release];
}

void MacSpeechHandler::listen()
{
    std::cout << "I am listening " << std::endl;
    [recog startListening];
}

void MacSpeechHandler::suspend()
{
    std::cout << "I am not listening" << std::endl;
    [recog stopListening];
}

void MacSpeechHandler::handleSpeechCommand(NSString* cmd)
{
   std::cerr << "Received command" << cmd << std::endl;
   
   if ([(NSString *)cmd isEqualToString:@"Full Size"] || [(NSString *)cmd isEqualToString:@"Full"]) 
   {
      voice_adapter->doFullSize();
      return;
   }
   
   if ([(NSString *)cmd isEqualToString:@"Fit Image"] || [(NSString *)cmd isEqualToString:@"Fit"]) 
   {
      voice_adapter->doFitImage();
      return;
   }
   
   if ([(NSString *)cmd isEqualToString:@"Zoom In"] || [(NSString *)cmd isEqualToString:@"In"]) 
   {
      voice_adapter->doZoomIn();
      return;
   }
   
   if ([(NSString *)cmd isEqualToString:@"Zoom Out"] || [(NSString *)cmd isEqualToString:@"Out"]) 
   {
      voice_adapter->doZoomOut();
      return;
   }

   if ([(NSString *)cmd isEqualToString:@"Pan Up"] || [(NSString *)cmd isEqualToString:@"Up"]) 
   {
      voice_adapter->doPanUp();
      return;
   }

   if ([(NSString *)cmd isEqualToString:@"Pan Down"] || [(NSString *)cmd isEqualToString:@"Down"]) 
   {
      voice_adapter->doPanDown();
      return;
   }

   if ([(NSString *)cmd isEqualToString:@"Pan Left"] || [(NSString *)cmd isEqualToString:@"Left"]) 
   {
      voice_adapter->doPanLeft();
      return;
   }

   if ([(NSString *)cmd isEqualToString:@"Pan Right"] || [(NSString *)cmd isEqualToString:@"Right"]) 
   {
      voice_adapter->doPanRight();
      return;
   }
 
   if ([(NSString *)cmd isEqualToString:@"Enhance"]) 
   {
      voice_adapter->doEnhance();
      return;
   }

   if ([(NSString *)cmd isEqualToString:@"Restore"]) 
   {
      voice_adapter->doRestore();
      return;
   }

   std::cerr << "Unhandled command" << cmd << std::endl;
   
}

