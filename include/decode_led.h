#define LED D5

#define STATUS_OFF 0
#define STATUS_NORMAL 1
#define STATUS_ERROR 2

extern bool ledStatus, codeStart, codePause, codeFail;
extern unsigned long ledOnStart, ledOffStart, lastChanged, ledOnDuration, ledOffDuration;
extern int errorCode, systemStatus;

void ledChange(void);
void decodeLED(void);
void pulseLong(void);
void pulseShort(void);
void codeBegin(void);
void nextDigit(void);
