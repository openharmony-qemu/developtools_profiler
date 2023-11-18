/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

export class CmdConstant {
  static CMD_TRACE_FILE_SIZE = 'hdc_std shell  stat --format=%s ';
  static CMD_SHELL = 'hdc_std shell ';
  static CMD_MOUNT = 'hdc_std shell mount -o remount,rw /';
  static CMD_GET_PROCESS = 'hdc_std shell ps -A -opid,cmd';
  static CMD_GET_APP_NMAE = 'hdc_std shell ps -A -ocmd';
  static CMD_GET_CPU_COUNT = "hdc_std shell grep -c 'processor' /proc/cpuinfo";
  static CMD_GET_HIPERF_EVENTS = 'hdc_std shell hiperf list';
  static CMD_GET_VERSION = 'hdc_std shell param get const.product.software.version';
  static CMD_GET_DEBUG_PROCESS = `hdc shell netstat -anp |grep Panda |grep -v grep | sed \'s/.* \\([0-9]*\\)\\/.*/\\1/\' |xargs -r ps -A -opid,cmd`;
  static CMD_HDC_DEVICES = 'hdc_std list targets';
  static CMD_MOUNT_DEVICES = 'hdc_std -t {0} shell mount -o remount,rw /';
  static CMD_GET_PROCESS_DEVICES = 'hdc_std -t {0} shell ps -A -opid,cmd';
  static CMD_GET_APP_NMAE_DEVICES = 'hdc_std -t {0} shell ps -A -ocmd';
  static CMD_GET_CPU_COUNT_DEVICES = "hdc_std -t {0} shell grep -c 'processor' /proc/cpuinfo";
  static CMD_GET_HIPERF_EVENTS_DEVICES = 'hdc_std -t {0} shell hiperf list';
  static CMD_FIEL_RECV_DEVICES = 'hdc_std -t {0} file recv {1} ./';
  static CMS_HDC_STOP = 'hdc_std -t {0} shell killall -2 hiprofiler_cmd';
  static CMS_STOP = 'hdc_std shell killall -2 hiprofiler_cmd';
  static CMS_HDC_CANCEL =
    'hdc_std -t {0} shell killall hiprofilerd hiprofiler_plugins native_daemon hiperf hiebpf hiprofiler_cmd';
  static CMS_CANCEL = 'hdc_std shell killall hiprofilerd hiprofiler_plugins native_daemon hiperf hiebpf hiprofiler_cmd';
  static CMD_GET_VERSION_DEVICES = 'hdc_std -t {0} shell param get const.product.software.version';
  static CMD_GET_DEBUG_PROCESS_DEVICES = `hdc_std -t {0} shell netstat -anp |grep Panda 
  |grep -v grep | sed \'s/.* \\([0-9]*\\)\\/.*/\\1/\' |xargs -r ps -A -opid,cmd`;
  static CMD_CLEAR_LONG_FOLD = 'hdc_std shell rm -rf ';
  static CMD_MKDIR_LONG_FOLD = 'hdc_std shell mkdir ';
  static CMD_GET_LONG_FILES = 'hdc_std shell ls ';
}
