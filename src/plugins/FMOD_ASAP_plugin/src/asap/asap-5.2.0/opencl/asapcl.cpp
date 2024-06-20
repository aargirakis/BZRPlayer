/*
 * asapcl.cpp - converter of ASAP-supported formats to WAV files
 *
 * Copyright (C) 2021  Piotr Fusik
 *
 * This file is part of ASAP (Another Slight Atari Player),
 * see http://asap.sourceforge.net
 *
 * ASAP is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * ASAP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ASAP; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CL_TARGET_OPENCL_VERSION 200
#include <CL/cl.h>

#include "asap.h"

void check_error(int err)
{
	if (err != CL_SUCCESS) {
		fprintf(stderr, "OpenCL error %d\n", err);
		exit(1);
	}
}

int main(int argc, char **argv)
{
	cl_platform_id platform;
	cl_uint num;
	check_error(clGetPlatformIDs(1, &platform, &num));
	if (num == 0) {
		fprintf(stderr, "No OpenCL platforms\n");
		return 1;
	}

	cl_device_id device;
	check_error(clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, &num));
	if (num == 0) {
		fprintf(stderr, "No OpenCL device\n");
		return 1;
	}

	size_t size;
	check_error(clGetDeviceInfo(device, CL_DEVICE_NAME, 0, nullptr, &size));
	char *name = static_cast<char *>(malloc(size));
	check_error(clGetDeviceInfo(device, CL_DEVICE_NAME, size, name, nullptr));
	fprintf(stderr, "Running on %s\n", name);
	free(name);

	const cl_context_properties properties[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties) platform, 0
	};
	cl_int err;
	cl_context context = clCreateContext(properties, 1, &device, nullptr, nullptr, &err);
	check_error(err);

	const char *source =
#include "asap-cl.h"
		;
	cl_program program = clCreateProgramWithSource(context, 1, &source, nullptr, &err);
	check_error(err);

	check_error(clBuildProgram(program, 1, &device, "-cl-std=CL2.0 -cl-opt-disable", nullptr, nullptr));

	cl_kernel kernel;
	check_error(clCreateKernelsInProgram(program, 1, &kernel, nullptr));

	cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, nullptr, &err);
	check_error(err);

	ASAPInfo *info = ASAPInfo_New();
	int exit_code = 0;

	for (int argi = 1; argi < argc; argi++) {
		const char *input_file = argv[argi];
		const char *input_dot = strrchr(input_file, '.');
		if (input_dot == nullptr) {
			fprintf(stderr, "%s: missing filename extension\n", input_file);
			exit_code = 1;
			continue;
		}
		char output_file[FILENAME_MAX];
		if (snprintf(output_file, sizeof(output_file), "%.*s.wav", (int) (input_dot - input_file), input_file) >= static_cast<int>(sizeof(output_file))) {
			fprintf(stderr, "%s: filename too long\n", input_file);
			exit_code = 1;
			continue;
		}

		FILE *fp = fopen(input_file, "rb");
		if (fp == NULL) {
			fprintf(stderr, "Cannot open %s\n", input_file);
			exit_code = 1;
			continue;
		}
		static unsigned char module[ASAPInfo_MAX_MODULE_LENGTH];
		int module_len = fread(module, 1, sizeof(module), fp);
		fclose(fp);

		if (!ASAPInfo_Load(info, input_file, module, module_len)) {
			fprintf(stderr, "%s: unsupported file\n", input_file);
			exit_code = 1;
			continue;
		}
		int channels = ASAPInfo_GetChannels(info);
		int song = ASAPInfo_GetDefaultSong(info);
		int duration = ASAPInfo_GetDuration(info, song);
		if (duration < 0)
			duration = 180 * 1000;

		cl_mem filename_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_NO_ACCESS, strlen(input_file) + 1, const_cast<char *>(input_file), &err);
		check_error(err);
		check_error(clSetKernelArg(kernel, 0, sizeof(filename_buffer), &filename_buffer));

		cl_mem module_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_NO_ACCESS, module_len, module, &err);
		check_error(err);
		check_error(clSetKernelArg(kernel, 1, sizeof(module_buffer), &module_buffer));
		check_error(clSetKernelArg(kernel, 2, sizeof(module_len), &module_len));

		check_error(clSetKernelArg(kernel, 3, sizeof(song), &song));
		check_error(clSetKernelArg(kernel, 4, sizeof(duration), &duration));

		int wav_len = 44 + duration * (ASAP_SAMPLE_RATE / 100) / 10 * channels * 2;
		cl_mem wav_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, wav_len, nullptr, &err);
		check_error(err);
		check_error(clSetKernelArg(kernel, 5, sizeof(wav_buffer), &wav_buffer));
		check_error(clSetKernelArg(kernel, 6, sizeof(wav_len), &wav_len));

		const size_t one = 1;
		check_error(clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &one, nullptr, 0, nullptr, nullptr));

		uint8_t *wav = static_cast<uint8_t *>(malloc(wav_len));
		check_error(clEnqueueReadBuffer(queue, wav_buffer, false, 0, wav_len, wav, 0, nullptr, nullptr));

		check_error(clFinish(queue));

		if (wav[0] != 'R') {
			fprintf(stderr, "%s: conversion error\n", input_file);
			exit_code = 1;
		}
		else {
			fp = fopen(output_file, "wb");
			if (fp == NULL) {
				fprintf(stderr, "Cannot create %s\n", output_file);
				exit_code = 1;
			}
			else {
				fwrite(wav, 1, wav_len, fp);
				fclose(fp);
			}
		}

		free(wav);

		check_error(clReleaseMemObject(wav_buffer));
		check_error(clReleaseMemObject(module_buffer));
		check_error(clReleaseMemObject(filename_buffer));
	}

	ASAPInfo_Delete(info);

	check_error(clReleaseCommandQueue(queue));
	check_error(clReleaseKernel(kernel));
	check_error(clReleaseProgram(program));
	check_error(clReleaseContext(context));
	return exit_code;
}
