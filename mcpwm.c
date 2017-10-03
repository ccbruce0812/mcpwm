#include <esp/timer.h>
#include <mcpwm.h>

#include "common.h"

#define CPU_FREQ	(80*1000*1000)

typedef struct {
	unsigned char pin;
	unsigned int mark;
	bool level;
} PerPinData;

static struct {
	bool isRunning;

	unsigned int freq;
	unsigned int res;

	unsigned int div;
	unsigned int load;
	unsigned int count;

	PerPinData pins[MCPWM_MAX_PINS];
	unsigned char pinCount;
} g_context;

static void onTimer(void *arg) {
	unsigned int max=1<<g_context.res;
	int i=0;

	for(i=0;i<g_context.pinCount;i++) {
		if(g_context.pins[i].mark && g_context.count<=g_context.pins[i].mark) {
			if(!g_context.pins[i].level) {
				g_context.pins[i].level=true;
				gpio_write(g_context.pins[i].pin, g_context.pins[i].level);
			}
		} else {
			if(g_context.pins[i].level) {
				g_context.pins[i].level=false;
				gpio_write(g_context.pins[i].pin, g_context.pins[i].level);
			}
		}
	}
	
	g_context.count=(g_context.count+1)%max;
}

int MCPWM_init(unsigned int freq, unsigned int div, unsigned int res, unsigned char *pins, unsigned char pinCount) {
	const unsigned int divCoffs[]={0x0001, 0x0010, 0x0100};
	unsigned int max=0;
	int i=0;
	
	if(!freq || !res || !pins || pinCount>MCPWM_MAX_PINS) {
		DBG("Bad arguments.\n");
		assert(false);
	}
	
	if(g_context.isRunning) {
		DBG("MCPWM is running.\n");
		return -1;
	}
	
	g_context.freq=freq;
	g_context.div=div;
	g_context.res=res;
	g_context.count=0;

	g_context.pinCount=pinCount;
	for(i=0;i<g_context.pinCount;i++) {
		g_context.pins[i].pin=pins[i];
		gpio_enable(g_context.pins[i].pin, GPIO_OUTPUT);
		gpio_write(g_context.pins[i].pin, false);
		g_context.pins[i].mark=0;
	}

	max=1<<g_context.res;
	g_context.load=CPU_FREQ/freq/divCoffs[g_context.div]/max;
	if(g_context.load) {
		if(g_context.load>TIMER_FRC1_MAX_LOAD)
			g_context.load=TIMER_FRC1_MAX_LOAD;
		DBG("freq=%u, div=%u, res=%u, load=%u\n", g_context.freq, g_context.div, g_context.res, g_context.load);
	} else {
		DBG("Inproper frequency, divider or resolution.\n");
		return -1;
	}

	//Stop
    timer_set_interrupts(FRC1, false);
    timer_set_run(FRC1, false);
	
	//Setup
	timer_set_divider(FRC1, g_context.div);
	timer_set_load(FRC1, g_context.load);
    timer_set_reload(FRC1, true);
	_xt_isr_attach(INUM_TIMER_FRC1, onTimer, NULL);
	
	//Start
    timer_set_interrupts(FRC1, true);
    timer_set_run(FRC1, true);

	g_context.isRunning=true;
	return 0;
}

int MCPWM_setMark(unsigned char pin, unsigned int mark) {
	unsigned int max=1<<g_context.res;

	if(pin>=g_context.pinCount) {
		DBG("Bad arguments.\n");
		assert(false);
	}

	if(!g_context.isRunning) {
		DBG("MCPWM is not running.\n");
		return -1;
	}
	
	if(mark>max-1)
		mark=max-1;
	
    timer_set_interrupts(FRC1, false);
	g_context.pins[pin].mark=mark;
    timer_set_interrupts(FRC1, true);
	
	return 0;
}

void MCPWM_deinit(void) {
	if(!g_context.isRunning) {
		DBG("MCPWM is not running.\n");
		return;
	}

	//Stop
    timer_set_interrupts(FRC1, false);
    timer_set_run(FRC1, false);
	
	g_context.isRunning=false;
}
