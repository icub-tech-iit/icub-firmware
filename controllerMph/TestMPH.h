#include <18F448.h>
#device ICD=TRUE
#device adc=10
#use delay(clock=16000000)
#fuses NOWDT,WDT128,HS, NOPROTECT, NOOSCSEN, BROWNOUT, BORV20, NOPUT, NOCPD, STVREN, NODEBUG, LVP, NOWRT, NOWRTD, NOWRTB, NOCPB, NOWRTC, NOEBTR, NOEBTRB

