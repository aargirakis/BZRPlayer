
#ifndef __MEMORY_H__
#define __MEMORY_H__

extern uint8	*ws_staticRam;
extern uint8	*ws_internalRam;

void	ws_memory_init(uint8 *rom, uint32 romSize);
void	ws_memory_reset(void);
void	ws_memory_done(void);

#endif
