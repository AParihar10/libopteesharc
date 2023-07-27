#ifndef LIBOPTEE_SHARC_H
#define LIBOPTEE_SHARC_H

#include <inttypes.h>
#include <stddef.h>

#include "adi_sharc_pta.h"

struct adi_optee_sharc;

struct adi_optee_sharc *adi_optee_sharc_open(void);
void adi_optee_sharc_close(struct adi_optee_sharc *);

int adi_optee_sharc_start(struct adi_optee_sharc *, uint32_t sharc);
int adi_optee_sharc_stop(struct adi_optee_sharc *, uint32_t sharc);
int adi_optee_sharc_load(struct adi_optee_sharc *, uint32_t sharc, void *data,
	size_t size);
int adi_optee_sharc_verify(struct adi_optee_sharc *, void *data, size_t size);

#endif
