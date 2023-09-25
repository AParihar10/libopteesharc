/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2023, Analog Devices, Inc. All rights reserved.
 */

#include <errno.h>
#include <libopteesharc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tee_client_api.h>

struct adi_optee_sharc {
	TEEC_Context ctx;
	TEEC_Session session;
};

struct adi_optee_sharc *adi_optee_sharc_open(void) {
	TEEC_UUID uuid = PTA_ADI_SHARC_UUID;
	uint32_t origin;
	struct adi_optee_sharc *ret;
	TEEC_Result res;

	ret = calloc(1, sizeof(*ret));
	if (!ret)
		return NULL;

	res = TEEC_InitializeContext(NULL, &ret->ctx);
	if (TEEC_SUCCESS != res)
		goto cleanup;

	res = TEEC_OpenSession(&ret->ctx, &ret->session, &uuid, TEEC_LOGIN_PUBLIC,
		NULL, NULL, &origin);
	if (TEEC_SUCCESS != res)
		goto finalize;

	return ret;

finalize:
	TEEC_FinalizeContext(&ret->ctx);

cleanup:
	free(ret);
	return NULL;
}

void adi_optee_sharc_close(struct adi_optee_sharc *state) {
	if (!state)
		return;

	TEEC_CloseSession(&state->session);
	TEEC_FinalizeContext(&state->ctx);
}

int adi_optee_sharc_start(struct adi_optee_sharc *state, uint32_t sharc) {
	uint32_t origin;
	TEEC_Result res;
	TEEC_Operation op;

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(
		TEEC_VALUE_INPUT,
		TEEC_NONE,
		TEEC_NONE,
		TEEC_NONE
	);

	op.params[0].value.a = sharc;

	res = TEEC_InvokeCommand(&state->session, ADI_SHARC_CMD_START, &op, &origin);
	if (TEEC_SUCCESS != res) {
		fprintf(stderr, "SHARC start for core %d failed, ret = %d\n", sharc, res);
		return (int) res;
	}

	return 0;
}

int adi_optee_sharc_stop(struct adi_optee_sharc *state, uint32_t sharc) {
	uint32_t origin;
	TEEC_Result res;
	TEEC_Operation op;

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(
		TEEC_VALUE_INPUT,
		TEEC_NONE,
		TEEC_NONE,
		TEEC_NONE
	);

	op.params[0].value.a = sharc;

	res = TEEC_InvokeCommand(&state->session, ADI_SHARC_CMD_STOP, &op, &origin);
	if (TEEC_SUCCESS != res) {
		fprintf(stderr, "SHARC stop for core %d failed, ret = %d\n", sharc, res);
		return (int) res;
	}

	return 0;
}

int adi_optee_sharc_load(struct adi_optee_sharc *state, uint32_t sharc, void *data,
	size_t size)
{
	uint32_t origin;
	TEEC_Result res;
	TEEC_Operation op;

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(
		TEEC_VALUE_INPUT,
		TEEC_MEMREF_TEMP_INPUT,
		TEEC_NONE,
		TEEC_NONE
	);

	op.params[0].value.a = sharc;
	op.params[1].tmpref.buffer = data;
	op.params[1].tmpref.size = size;

	res = TEEC_InvokeCommand(&state->session, ADI_SHARC_CMD_LOAD, &op, &origin);
	if (TEEC_SUCCESS != res) {
		fprintf(stderr, "SHARC load for core %d failed, ret = %d\n", sharc, res);
		return (int) res;
	}

	return 0;
}

int adi_optee_sharc_verify(struct adi_optee_sharc *state, void *data, size_t size) {
	uint32_t origin;
	TEEC_Result res;
	TEEC_Operation op;

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(
		TEEC_MEMREF_TEMP_INPUT,
		TEEC_NONE,
		TEEC_NONE,
		TEEC_NONE
	);

	op.params[0].tmpref.buffer = data;
	op.params[0].tmpref.size = size;

	res = TEEC_InvokeCommand(&state->session, ADI_SHARC_CMD_VERIFY, &op, &origin);
	if (TEEC_SUCCESS != res) {
		fprintf(stderr, "SHARC verify failed, ret = %d\n", res);
		return (int) res;
	}

	return 0;
}
