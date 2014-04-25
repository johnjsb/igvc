// Handlers for pub/sub
// Kevin Gilbert
// November 30 2013
#ifndef __JSONhandlers_H__
#define __JSONhandlers_H__

#define NUMSUB 5
#define NUMPUB 4
#define BUFFERSIZE 25
#define MAXPWM 0.125
#define MAXMOTOR 0.15
#define MINMOTOR -0.15

/**
* 	Subscriber Handlers
*/
void SPLM_handler(void* data, char *jsonvalue);
void SPRM_handler(void* data, char* jsonvalue);
void SVLX_handler(void* data, char *jsonvalue);
void SVAX_handler(void* data, char* jsonvalue);
void RSTE_handler(void* data, char *jsonvalue);

extern void (*subHandlers[NUMSUB])(void*,char*);

/**
* 	Publisher Handlers
*/
char* SPLMdebug_handler(void* data);
char* SPRMdebug_handler(void* data);
char* SVLXdebug_handler(void* data);
char* SVAXdebug_handler(void* data);

extern char* (*pubHandlers[NUMPUB])(void*);

#endif