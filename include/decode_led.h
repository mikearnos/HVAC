#define LED D5

#define STATUS_OFF 0
#define STATUS_NORMAL 1
#define STATUS_ERROR 2

extern bool ledStatus;
extern int errorCode, systemStatus;
extern bool codeStart, codePause, codeFail;
extern unsigned long ledOnDuration, ledOffDuration, lastChanged;

void ledChange(void);
void decodeLED(void);
void pulseLong(void);
void pulseShort(void);
void codeBegin(void);
void nextDigit(void);
