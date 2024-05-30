#ifndef __VOL_H__
#define __VOL_H__

#define	MAX_VOLUMES	2

typedef struct {
	void		(*read)(void* buf, unsigned int block);
	void		(*write)(void* buf, unsigned int block);

	unsigned int	vtoc_size;
	unsigned int	vtoc_block;
} VOLUME;

extern unsigned int current_volume;
extern unsigned int current_block;

void	VOLInit(void);
void	VOLAddVolume(unsigned int id, void (*read)(void*, unsigned int),
		void (*write)(void*, unsigned int));
VOLUME*	VOLGetVolume(unsigned int volume);
BOOL	VOLIsActive(unsigned int volume);
void	VOLReadBlock(unsigned int volume, void* buf, unsigned int block);
void	VOLWriteBlock(unsigned int volume, void* buf, unsigned int block);

#endif
