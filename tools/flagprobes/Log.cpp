// probe: Log.dll (bin/player/Log.dll, the Oct 15 Player's copy; the exe imports FLog::* from it). Exported
// functions, names from the export table. Bodies from the bytes, intent from the 2016 FastLog.cpp.
namespace FLog {
typedef unsigned char Channel;
typedef double (*TimeFunc)();
typedef void (*ExternalLogFunc)(Channel channel, const char* message);
static TimeFunc timeF;
static ExternalLogFunc logF;
extern int g_LogCounters[5];
inline void* flf(float value) { return reinterpret_cast<void*&>(value); }
void FastLog(Channel level, const char* message, const void* arg0, const void* arg1, const void* arg2, const void* arg3, const void* arg4);
void FastLog(Channel level, const char* message, const void* arg0)                                    // 0x10002770
{
  FastLog(level, message, arg0, 0, 0, 0, 0);
}
void FastLog(Channel level, const char* message, const void* arg0, const void* arg1, const void* arg2)  // 0x10002790
{
  FastLog(level, message, arg0, arg1, arg2, 0, 0);
}
void Init(TimeFunc tF) { timeF = tF; }                                                                // 0x10002920
void SetExternalLogFunc(ExternalLogFunc lF) { logF = lF; }                                            // 0x10002930
void FastLogF(Channel channel, const char* message, float arg1, float arg2, float arg3, float arg4)    // 0x10002940
{
  FastLog(channel, message, (void*)0xFFFF10AD, flf(arg1), flf(arg2), flf(arg3), flf(arg4));
}
int GetFastLogCounter(Channel channel)                                                                // 0x10002e70
{
  if (channel > 5)
    channel = 1;
  return g_LogCounters[channel - 1];
}
double NowFast() { return timeF(); }                                                                  // 0x10003370
}
