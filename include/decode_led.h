#define LED D5

extern bool ledStatus;
extern unsigned long ledOnStart, ledOffStart, lastChanged;
extern int errorCode;

void ledChange(void);

void decodeLED(void);
void pulseLong(void);
void pulseShort(void);
void codeBegin(void);
void nextDigit(void);
