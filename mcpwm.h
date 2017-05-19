#ifndef MCPWM_H
#define MCPWM_H

#ifdef __cplusplus
extern "C" {
#endif

#define MCPWM_MAX_PINS	(8)

int MCPWM_init(unsigned int freq, unsigned int res, unsigned char *pins, unsigned char pinCount);
int MCPWM_setMark(unsigned char pin, unsigned int mark);
void MCPWM_deinit(void);

#ifdef __cplusplus
}
#endif

#endif
