
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libopteesharc.h"

static void usage(void) {
	fprintf(stderr, "\n");
	fprintf(stderr, " Usage:\n");
	fprintf(stderr, "    sharc-cli -z [id]           Stop core [id]\n");
	fprintf(stderr, "    sharc-cli -g [id]           Start core [id]\n");
	fprintf(stderr, "    sharc-cli -l [file] [id]    Load [file] to core [id]\n");
	fprintf(stderr, "    sharc-cli -v [file]         Verify signature for [file]\n");
	fprintf(stderr, "\n");
}

enum action {
	ACTION_NONE = 0,
	ACTION_START,
	ACTION_STOP,
	ACTION_LOAD,
	ACTION_VERIFY
};

int load_file(const char *path, void **dest, size_t *size) {
	FILE *fp;
	int ret;
	struct stat stats;
	size_t items;
	void *buffer;

	ret = stat(path, &stats);
	if (ret) {
		fprintf(stderr, "Cannot stat `%s`\n", path);
		return -1;
	}

	buffer = calloc(1, stats.st_size);
	if (!buffer) {
		fprintf(stderr, "Could not allocate %zu bytes for buffer\n", stats.st_size);
		return -2;
	}

	fp = fopen(path, "rb");
	if (!fp) {
		fprintf(stderr, "Cannot open `%s`\n", path);
		ret = -3;
		goto cleanup_mem;
	}

	items = fread(buffer, stats.st_size, 1, fp);
	if (items != 1) {
		fprintf(stderr, "Read failed for file `%s`\n", path);
		ret = -4;
		goto cleanup_file;
	}

	*dest = buffer;
	*size = stats.st_size;
	fclose(fp);
	return 0;

cleanup_file:
	fclose(fp);
cleanup_mem:
	free(buffer);
	return ret;
}

int main(int argc, char **argv) {
	struct adi_optee_sharc *sharc;
	uint32_t id;
	enum action action = ACTION_NONE;
	int opt;
	const char *path = NULL;
	void *buffer;
	size_t size;
	int ret;

	while ((opt = getopt(argc, argv, "hzgl:v:")) != -1) {
		switch (opt) {
		case 'z':
			action = ACTION_STOP;
			break;
		case 'g':
			action = ACTION_START;
			break;
		case 'l':
			action = ACTION_LOAD;
			path = optarg;
			break;
		case 'v':
			action = ACTION_VERIFY;
			path = optarg;
			break;
		case 'h':
		default:
			usage();
			exit(0);
		}
	}

	if (optind >= argc) {
		usage();
		exit(1);
	}

	id = atoi(argv[optind]);

	sharc = adi_optee_sharc_open();
	if (!sharc) {
		fprintf(stderr, "Unable to open SHARC pTA\n");
		exit(1);
	}

	switch (action) {
		case ACTION_STOP:
			if (adi_optee_sharc_stop(sharc, id))
				printf("Failed to stop core %d\n", id);
			break;
		case ACTION_START:
			if (adi_optee_sharc_start(sharc, id))
				printf("Failed to start core %d\n", id);
			break;
		case ACTION_LOAD:
			ret = load_file(path, &buffer, &size);
			if (ret)
				goto cleanup;
			if (adi_optee_sharc_load(sharc, id, buffer, size))
				printf("Failed to load %s to core %d\n", path, id);
			break;
		case ACTION_VERIFY:
			ret = load_file(path, &buffer, &size);
			if (ret)
				goto cleanup;
			if (adi_optee_sharc_verify(sharc, buffer, size))
				printf("LDR signature not verified\n");
			else
				printf("LDR signature verified successfully\n");
			break;
		case ACTION_NONE:
			break;
	}

cleanup:
	adi_optee_sharc_close(sharc);
	return 0;
}
