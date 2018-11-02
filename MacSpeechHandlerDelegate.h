#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

class MacSpeechHandler;

@interface MacSpeechHandlerDelegate : NSObject <NSSpeechRecognizerDelegate>
{
     MacSpeechHandler *_handler;
}

- (id) init:(MacSpeechHandler*)handler;

- (void)speechRecognizer:(NSSpeechRecognizer *)sender didRecognizeCommand:(id)command;

@end

