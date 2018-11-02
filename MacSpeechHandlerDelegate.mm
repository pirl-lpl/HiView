#import "MacSpeechHandlerDelegate.h"
#include "MacSpeechHandler.h"

@implementation MacSpeechHandlerDelegate

- (id) init:(MacSpeechHandler*)handler
{
  self = [super init];

  if (self)
  {
     _handler = handler;
  }

  return self;
}

- (void)speechRecognizer:(NSSpeechRecognizer *)sender didRecognizeCommand:(id)command
{
    (void)sender;
    _handler->handleSpeechCommand(command);
}

@end

