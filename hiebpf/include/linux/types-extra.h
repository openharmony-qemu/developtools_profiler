/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef TYPES_EXTRA_H
#define TYPES_EXTRA_H

/*
 * This file exports kernel type definitions, which usually are transparent to
 * user space, to BPF user program. Most of these definitions are specific to arm64
 * , which is actually the resean why they are not exposed to user space
*/

// kernel types from arm64 linux-5.10
#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
#define __bitwise __bitwise__
typedef int __bitwise       __kernel_rwf_t;
typedef __kernel_rwf_t      rwf_t;
typedef int                 __kernel_mqd_t;
typedef __kernel_mqd_t		mqd_t;

typedef __u32 				compat_size_t;
typedef __s32 				compat_ssize_t;
typedef __s32 				compat_clock_t;
typedef __s32 				compat_pid_t;
typedef __u32 				compat_ino_t;
typedef __s32 				compat_off_t;
typedef __s64 				compat_loff_t;
typedef __s32 				compat_daddr_t;
typedef __s32 				compat_timer_t;
typedef __s32 				compat_key_t;
typedef __s16 				compat_short_t;
typedef __s32 				compat_int_t;
typedef __s32 				compat_long_t;
typedef __u16 				compat_ushort_t;
typedef __u32 				compat_uint_t;
typedef __u32 				compat_ulong_t;
typedef __u32 				compat_uptr_t;
typedef __u32 				compat_aio_context_t;

typedef __u16		        compat_mode_t;
typedef unsigned short		umode_t;


#ifdef CONFIG_COMPAT_FOR_U64_ALIGNMENT
typedef __s64 __attribute__((aligned(4))) compat_s64;
typedef __u64 __attribute__((aligned(4))) compat_u64;
#else
typedef __s64 compat_s64;
typedef __u64 compat_u64;
#endif

typedef unsigned short      __kernel_old_uid_t;
typedef unsigned short      __kernel_old_gid_t;
#define __kernel_old_uid_t  __kernel_old_uid_t
// #ifdef CONFIG_HAVE_UID16
/* This is defined by include/asm-{arch}/posix_types.h */
typedef __kernel_old_uid_t	old_uid_t;
typedef __kernel_old_gid_t	old_gid_t;
// #endif /* CONFIG_UID16 */

#ifndef loff_t
typedef long long	__kernel_loff_t;
typedef __kernel_loff_t		loff_t;
#endif

#endif