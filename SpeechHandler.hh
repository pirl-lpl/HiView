#ifndef __SpeechHandler_H__
#define __SpeechHandler_H__


class SpeechHandler
{

public:
  SpeechHandler(Voice_Adapter* adapter);
  
   virtual void listen() = 0;
   virtual void suspend() = 0;
  

protected:
    virtual ~SpeechHandler();
    Voice_Adapter* voice_adapter;
};

#endif /* __SpeechHandler_H__ */
