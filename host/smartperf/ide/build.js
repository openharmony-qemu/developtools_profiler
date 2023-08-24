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

const path = require('path');
const fs = require('fs');
const childProcess = require('child_process');
const os = require('os');
const log4js = require('log4js');

const compileServer = true;
const outDir = 'dist';

const sdkWams = [
  'trace_streamer_sdk_builtin.js',
  'trace_streamer_sdk_builtin.wasm',
  'trace_streamer_dubai_builtin.js',
  'trace_streamer_dubai_builtin.wasm',
];

const staticPath = ['/src/img', '/server/cert', '/src/doc', '/src/figures'];

const staticFiles = [
  '/server/version.txt',
  '/src/index.html',
  '/src/base-ui/icon.svg',
  '/server/wasm.json',
  '/server/server-config.txt',
];

const thirdParty = [
  {
    srcFilePath: '/third-party/sql-wasm.wasm',
    distFilePath: '/trace/database/sql-wasm.wasm',
  },
  {
    srcFilePath: '/third-party/sql-wasm.js',
    distFilePath: '/trace/database/sql-wasm.js',
  },
  {
    srcFilePath: '/third-party/worker.sql-wasm.js',
    distFilePath: '/trace/database/worker.sql-wasm.js',
  },
];

let log;

function cpFile(from, to) {
  if (fs.existsSync(from)) {
    fs.writeFileSync(to, fs.readFileSync(from));
    log.info('cp file %s  to  %s', from, to);
  } else {
    log.warn('file %s  is not exists', from, to);
  }
}

function checkEnvironment() {
  let goVersion = childProcess.execSync('go version', {
    encoding: 'utf-8',
  });
  log.info('go is', goVersion);
  let nodeVersion = childProcess.execSync('node -v', {
    encoding: 'utf-8',
  });
  log.info('node version is', nodeVersion);
  let tscVersion = childProcess.execSync('tsc -v', {
    encoding: 'utf-8',
  });
  log.info('tsc version is', tscVersion);
  if (goVersion == '' || nodeVersion == '' || tscVersion == '') {
    return false;
  }
  let traceStreamer = path.normalize(path.join(__dirname, '/bin'));
  if (!checkDirExist(traceStreamer + '/trace_streamer_builtin.js')) {
    log.error(traceStreamer + '/trace_streamer_builtin.js' + ' Must exist');
    return false;
  }
  if (!checkDirExist(traceStreamer + '/trace_streamer_builtin.wasm')) {
    log.error(traceStreamer + '/trace_streamer_builtin.wasm' + ' Must exist');
    return false;
  }
  return true;
}

function initLog() {
  log4js.configure({
    appenders: {
      out: { type: 'stdout' },
    },
    categories: {
      default: { appenders: ['out'], level: 'debug' },
    },
  });
  return log4js.getLogger('smartPerf');
}

function main() {
  log = initLog();
  if (!checkEnvironment()) {
    return;
  }
  // clean outDir
  let outPath = path.normalize(path.join(__dirname, '/', outDir));
  if (checkDirExist(outPath)) {
    log.info('delete the last compilation result');
    removeDir(outPath);
    log.info('delete the last compilation success');
  }
  // run tsc compile
  log.info('start compiling typeScript code');
  let rootPath = path.join(__dirname, '/');
  childProcess.execSync('tsc -p ' + rootPath, {
    encoding: 'utf-8',
  });
  log.info('compiling typeScript code success');
  // run cp to mv all staticFile
  staticFiles.forEach((value) => {
    let filePath = path.join(__dirname, value);
    let distFile;
    if (value.startsWith('/src')) {
      distFile = path.join(__dirname, outDir, value.substring(4, value.length + 1));
    } else if (value.startsWith('/server')) {
      distFile = path.join(__dirname, outDir, value.substring(7, value.length + 1));
    }
    cpFile(filePath, distFile);
  });
  staticPath.forEach((value) => {
    let pa = path.join(__dirname, value);
    let distPath;
    if (value.startsWith('/src')) {
      distPath = path.join(__dirname, outDir, value.substring(4, value.length + 1));
    } else if (value.startsWith('/server')) {
      distPath = path.join(__dirname, outDir, value.substring(7, value.length + 1));
    }
    copyDirectory(pa, distPath);
  });
  thirdParty.forEach((value) => {
    let thirdFile = path.join(__dirname, value.srcFilePath);
    let thirdDistFile = path.join(__dirname, outDir, value.distFilePath);
    cpFile(thirdFile, thirdDistFile);
  });
  let traceStreamer = path.normalize(path.join(__dirname, '/bin'));
  if (checkDirExist(traceStreamer)) {
    let dest = path.normalize(path.join(__dirname, outDir, '/bin'));
    copyDirectory(traceStreamer, dest);
    // to mv traceStream Wasm and js
    cpFile(
      traceStreamer + '/trace_streamer_builtin.js',
      rootPath + outDir + '/trace/database/trace_streamer_builtin.js'
    );
    cpFile(
      traceStreamer + '/trace_streamer_builtin.wasm',
      rootPath + outDir + '/trace/database/trace_streamer_builtin.wasm'
    );
    if (sdkWams.length > 0) {
      sdkWams.forEach((fileName) => {
        cpFile(traceStreamer + '/' + fileName, rootPath + outDir + '/trace/database/' + fileName);
      });
    }
  } else {
    log.error('traceStreamer dir is not Exits');
    return;
  }
  // compile server
  if (compileServer) {
    log.log('start compile server');
    let serverSrc = path.normalize(path.join(__dirname, '/server/main.go'));
    let rs;
    if (os.type() === 'Windows_NT') {
      rs = childProcess.spawnSync('go', ['build', '-o', outPath, serverSrc], {
        encoding: 'utf-8',
      });
    } else {
      rs = childProcess.spawnSync('go', ['build', '-o', outPath + '/main', serverSrc], {
        encoding: 'utf-8',
      });
    }
    if (rs.status == 0) {
      log.log('compile server success');
    } else {
      log.error('compile server failed', rs);
    }
  } else {
    log.warn('skip compile server');
  }
  log.log('smartPerf compile success');
}

function copyDirectory(src, dest) {
  if (checkDirExist(dest) == false) {
    fs.mkdirSync(dest);
  }
  if (checkDirExist(src) == false) {
    return false;
  }
  let directories = fs.readdirSync(src);
  directories.forEach((value) => {
    let filePath = path.join(src, value);
    let fileSys = fs.statSync(filePath);
    if (fileSys.isFile()) {
      let destPath = path.join(dest, value);
      log.info('cp file %s  to  %s', filePath, destPath);
      fs.copyFileSync(filePath, destPath);
    } else if (fileSys.isDirectory()) {
      copyDirectory(filePath, path.join(dest, value));
    }
  });
}

function checkDirExist(dirPath) {
  return fs.existsSync(dirPath);
}

function removeDir(outPath) {
  let files = [];
  if (fs.existsSync(outPath)) {
    files = fs.readdirSync(outPath);
    files.forEach((file, index) => {
      let curPath = outPath + '/' + file;
      if (fs.statSync(curPath).isDirectory()) {
        removeDir(curPath);
      } else {
        fs.unlinkSync(curPath);
      }
    });
    fs.rmdirSync(outPath);
  }
}

main();
