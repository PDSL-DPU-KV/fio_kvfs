#include <math.h>
#include <string.h>
#include <stdio.h>
#include "../hash.h"
#include "gauss.h"

#define GAUSS_ITERS	12

static int gauss_dev(struct gauss_state *gs)
{
	unsigned int r;
	int vr;

	if (!gs->stddev)
		return 0;

	r = __rand(&gs->r);
	vr = gs->stddev * (r / (FRAND_MAX + 1.0));

	return vr - gs->stddev / 2;
}

unsigned long long gauss_next(struct gauss_state *gs)
{
	unsigned long long sum = 0;
	int i;

	for (i = 0; i < GAUSS_ITERS; i++)
		sum += __rand(&gs->r) % (gs->nranges + 1);

	sum = (sum + GAUSS_ITERS - 1) / GAUSS_ITERS;

	if (gs->stddev) {
		int dev = gauss_dev(gs);

		while (dev + sum >= gs->nranges)
			dev /= 2;
		sum += dev;
	}

	return __hash_u64(sum) % gs->nranges;
}

void gauss_init(struct gauss_state *gs, unsigned long nranges, unsigned int d,
		unsigned int seed)
{
	memset(gs, 0, sizeof(*gs));
	init_rand_seed(&gs->r, seed);
	gs->nranges = nranges;
	if (d) {
		gs->stddev = (nranges * 100) / d;
		if (gs->stddev > nranges / 2)
			gs->stddev = nranges / 2;
	}
}
