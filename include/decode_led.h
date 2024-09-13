#define LED D5

#define STATUS_OFF 0
#define STATUS_NORMAL 1
#define STATUS_ERROR 2

extern unsigned long ledOnStart, ledOffStart, lastChanged;
extern int errorCode, systemStatus;

void ledChange(void);
void decodeLED(void);
void pulseLong(void);
void pulseShort(void);
void codeBegin(void);
void nextDigit(void);
