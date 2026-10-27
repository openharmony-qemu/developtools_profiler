#include "formatter/html/components/navigation_bar.hpp"
#include <vector>
#include "formatter/html/html_analysis_common.hpp"

using namespace mem_analyzer::html_analysis;

namespace mem_analyzer {

// 设置导航栏格式
std::string nav_bar_format() {
    return R"HTML(
.nav-bar {background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); padding: 0 20px; text-align: center; }
.nav-item { display: inline-block; padding: 14px 28px; cursor: pointer; font-size: 16px; font-weight: 500; color: rgba(255,255,255,0.85); border-bottom: 3px solid transparent; transition: all 0.3s ease; }
.nav-item:hover { color: #fff; background: rgba(255,255,255,0.1); border-radius: 8px 8px 0 0; }
.nav-item.active { color: #fff; border-bottom-color: #fff; background: rgba(255,255,255,0.15); border-radius: 8px 8px 0 0; }

.nav-content { display: none; padding: 24px; background: #f8f9fa; min-height: calc(100vh - 120px); }
.nav-content.active { display: block; }

.section-title { font-size: 20px; font-weight: 600; color: #333; margin: 0 0 20 px 0; padding-bottom: 12px; border-bottom: 2px solid #667eea; }

/* ===== 帮助中心样式 ===== */
.help-container { max-width: 860px; margin: 0 auto; display: flex; flex-direction: column; gap: 18px; }
.help-section { background: #fff; border-radius: 14px; padding: 24px 26px; border: 1px solid rgba(16,24,40,0.06); box-shadow: 0 1px 2px rgba(16,24,40,0.04), 0 4px 16px rgba(16,24,40,0.06); transition: box-shadow .25s ease, transform .25s ease; }
.help-section:hover { box-shadow: 0 2px 4px rgba(16,24,40,0.06), 0 12px 28px rgba(16,24,40,0.10); }
.help-title { font-size: 15px; font-weight: 700; color: #1f2329; letter-spacing: .3px; margin: 0 0 14px 0; padding-bottom: 10px; position: relative; }
.help-title::after { content: ""; position: absolute; left: 0; bottom: 0; width: 32px; height: 3px; border-radius: 2px; background: linear-gradient(90deg, #667eea, #764ba2); }
.help-desc { color: #4b5563; line-height: 1.75; margin: 8px 0; font-size: 14px; }
.help-desc a { color: #667eea; text-decoration: none; border-bottom: 1px solid rgba(102,126,234,0.35); transition: border-color .2s ease, color .2s ease; }
.help-desc a:hover { color: #5a6fd6; border-bottom-color: #5a6fd6; }

/* 关于：引言卡片 */
.help-hero .help-desc { font-size: 15px; color: #374151; }

/* 资源链接列表（整行可点 + 箭头） */
.help-links { padding: 8px 10px; }
.help-link { display: flex; align-items: center; gap: 14px; padding: 14px 14px; border-radius: 10px; color: #333; text-decoration: none; transition: background .2s ease; }
.help-link + .help-link { border-top: 1px solid #f1f3f5; }
.help-link:hover { background: #f6f8ff; }
.help-link-label { font-size: 15px; font-weight: 600; color: #1f2329; flex-shrink: 0; }
.help-link-url { flex: 1; font-size: 13px; color: #8a93a5; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
.help-chevron { width: 8px; height: 8px; border-top: 2px solid #c3c9d4; border-right: 2px solid #c3c9d4; transform: rotate(45deg); flex-shrink: 0; transition: border-color .2s ease, transform .2s ease; }
.help-link:hover .help-chevron { border-color: #667eea; transform: rotate(45deg) translate(-1px, 1px); }

/* 技术支持条目 */
.help-support-block { padding: 14px 0 16px; }
.help-support-block + .help-support-block { border-top: 1px solid #f1f3f5; }
.help-support-block h3 { font-size: 14px; font-weight: 700; color: #1f2329; margin: 0 0 8px 0; padding-left: 14px; position: relative; }
.help-support-block h3::before { content: ""; position: absolute; left: 0; top: 3px; bottom: 3px; width: 4px; border-radius: 2px; background: linear-gradient(180deg, #667eea, #764ba2); }

/* ===== 资料文档样式 ===== */
.doc-container { display: grid; grid-template-columns: repeat(5, 1fr); gap: 24px; max-width: 1750px; margin: 0 auto; }
.doc-category { background: #fff; border-radius: 10px; padding: 18px; box-shadow: 0 2px 8px rgba(0,0,0,0.06); }
.doc-category-title { font-size: 16px; font-weight: 600; color: #333; margin: 0 0 14px 0; padding-bottom: 10px; border-bottom: 2px solid #667eea; }
.doc-subcategory { margin-bottom: 14px; padding-left: 12px; border-left: 2px solid #e0e0e0; }
.doc-subcategory:last-child { margin-bottom: 0; }
.doc-subcategory-title { font-size: 14px; font-weight: 700; color: #555; margin: 0 0 8px 0; }
.doc-links { list-style: none; padding: 0; margin: 0; }
.doc-links li { padding: 5px 0; border-bottom: 1px solid #f5f5f5; }
.doc-links li:last-child { border-bottom: none; }
.doc-links a { color: #555; text-decoration: none; font-size: 14px; transition: color 0.2s; display: block; }
.doc-links a:hover { color: #667eea; }
)HTML";
}

// 构建标题
std::string nav_bar_title() {
    return R"HTML(
<div class="nav-bar" style="display: flex; justify-content: center;">
  <span class="nav-item active" data-nav="navReport" onclick="switchNav('navReport', this)">分析报告</span>
  <span class="nav-item" data-nav="navDoc" onclick="switchNav('navDoc', this)">资料文档</span>
  <span class="nav-item" data-nav="navHelp" onclick="switchNav('navHelp', this)">帮助中心</span>
</div>
)HTML";
}


// ========== 构建资料文档导航栏内容 ==========
std::string nav_bar_doc_content() {
    return R"HTML(
<div class="doc-container">
  <div class="doc-category">
    <h2 class="doc-category-title">资源泄漏故障模式</h2>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">ArkTS内存泄漏故障模式</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-overview-of-arkts-memory-leaks-overview" target="_blank">ArkTS内存泄漏故障模式概述</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-memleak-arkts-globalhandle-mode" target="_blank">GlobalHandle类型内存泄漏故障模式</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-memleak-arkts-localhandle-mode" target="_blank">LocalHandle类型内存泄漏故障模式</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-memleak-arkts-vmroot-mode" target="_blank">VMRoot类型内存泄漏故障模式</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-memleak-arkts-huge-mode" target="_blank">申请超大对象导致OOM故障模式</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">RSS内存泄漏故障模式</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-rssleak-fault-mode-overreview" target="_blank">RSS内存泄漏故障模式概述</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-nativeheap-fault-mode" target="_blank">NativeHeap堆过大导致内存泄漏故障模式</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-anon-fault-mode" target="_blank">匿名映射过大导致内存泄漏故障模式</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-filepage-fault-mode" target="_blank">文件映射过大导致内存泄漏故障模式</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-stack-fault-mode" target="_blank">栈内存过大导致内存泄漏故障模式</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-association-fault-mode" target="_blank">组合使用过大导致内存泄漏故障模式</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">泛PSS内存泄漏故障模式</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-extpssleak-fault-mode-overreview" target="_blank">泛PSS内存泄漏故障模式概述</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-app-extpssleak-fault-mode" target="_blank">应用泛PSS内存泄漏故障模式</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">DMA内存泄漏故障模式</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-dmaleak-fault-mode-overreview" target="_blank">DMA内存泄漏故障模式概述</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-app-dmaleak-fault-mode" target="_blank">应用DMA内存泄漏故障模式</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">ASHMEM内存泄漏故障模式</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-ashmemleak-fault-mode-overreview" target="_blank">ASHMEM内存泄漏故障模式概述</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-app-ashmemleak-fault-mode" target="_blank">应用ASHMEM内存泄漏故障模式</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">GPU内存泄漏故障模式</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-gpuleak-fault-mode-overreview" target="_blank">GPU内存泄漏故障模式概述</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-gpuleak-self-rendering-fault-mode" target="_blank">单应用自渲染GPU内存泄漏故障模式</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">ArkWeb OOM故障模式</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-overview-of-arkweb-oom-fault-modes" target="_blank">ArkWeb V8 OOM故障模式概述</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-arkweb-oom-js-object-held-by-js-object" target="_blank">JS对象长期被JS持有导致内存泄漏故障模式</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">JSVM OOM故障模式</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-overview-of-jsvm-oom-fault-modes" target="_blank">JSVM OOM故障模式概述</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-jsvm-oom-js-object-held-by-native" target="_blank">JS对象被Native持有导致内存泄漏故障模式</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-jsvm-oom-js-object-held-by-js-object" target="_blank">JS对象长期被JS持有导致内存泄漏故障模式</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">线程泄漏故障模式</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-threadleak-fault-mode-overreview" target="_blank">线程泄漏故障模式概述</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-app-threadleak-fault-mode" target="_blank">应用线程泄漏故障模式</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">句柄泄漏故障模式</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-fdleak-fault-mode-overreview" target="_blank">句柄泄漏故障模式概述</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-app-fdleak-fault-mode" target="_blank">应用句柄泄漏故障模式</a></li>
      </ul>
    </div>
  </div>

  <div class="doc-category">
    <h2 class="doc-category-title">资源泄漏问题分析</h2>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">ArkTS内存泄漏</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/forum/topic/0201216402239166435?fid=0109140870620153026" target="_blank">开发态快速定位ArkTS泄漏</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/forum/topic/0201217016241279525?ha_source=rrdscpjl&ha_sourceId=89000499" target="_blank">解决异步任务内存泄漏问题</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-js-leak-watcher" target="_blank">JSLeakWatcher开发实践</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/forum/topic/0207217013856434057?fid=0109140870620153026" target="_blank">JSLeakWatcher特性指导</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/forum/topic/0201217013290146523" target="_blank">ArkTS内存快照聚类分析规则</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">RSS内存泄漏</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-native-memleak-detection" target="_blank">Native内存泄漏问题检测方法</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-native-leak-in-develop" target="_blank">开发态快速定位Native泄漏</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-native-leak-in-operation" target="_blank">运维态高效处理Native泄漏</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/ide-native-allocation-case" target="_blank">Native内存泄漏分析案例</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">其他</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-memory-basic-knowledge" target="_blank">内存基础知识</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/resource-leak-guidelines" target="_blank">资源泄漏检测原理及日志规格</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/forum/topic/0203216483644522067" target="_blank">开发态快速定位DMA(ION)泄漏</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/best-practices/bpta-stability-leak-way" target="_blank">句柄、线程等内存泄漏分析</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/harmonyos-faqs/faq-stability-71" target="_blank">PSS_MEMORY内存泄漏问题排查</a></li>
      </ul>
    </div>
  </div>

  <div class="doc-category">
    <h2 class="doc-category-title">资源泄漏分析工具</h2>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">日志解析及分析工具</h3>
      <ul class="doc-links">
        <li><a href="https://gitcode.com/ancimoon/developtools_dfx_mem_analyzer" target="_blank">资源泄漏日志采集&分析工具-mem-analyzer</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/forum/topic/0204223139430059948?fid=0109140870620153026" target="_blank">mem-analyzer工具概览和使用指导</a></li>
        <li><a href="https://gitcode.com/openharmony-sig/developtools_dfx_skills" target="_blank">AI自动化扫描与分析工具</a></li>
        <li><a href="https://mp.weixin.qq.com/s/NNBLp_LPl2hSNchsdEXBag" target="_blank">HarmonyOS故障定位Skills</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/rawheap-translator" target="_blank">rawheap-translator工具</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/js-rawheap-translator" target="_blank">js-rawheap-translator工具</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">DevEco Studio等工具</h3>
      <ul class="doc-links">
        <li><a href="https://gitcode.com/openharmony/developtools_smartperf_host/releases" target="_blank">SmartPerf工具源码仓</a></li>
        <li><a href="https://gitcode.com/openharmony/developtools_smartperf_host/blob/master/smartperf_host/README_zh.md" target="_blank">SmartPerf工具使用介绍</a></li>
        <li><a href="https://mp.weixin.qq.com/s/-IMwIBxABwFdvBFA5grTfQ" target="_blank">Profiler跨语言内存调优</a></li>
        <li><a href="https://mp.weixin.qq.com/s/SGptFBGA4DpxJ8IBywx6eQ" target="_blank">Local Handle泄漏检测工具</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/stability-testing#section1896912152183" target="_blank">使用Testing检测内存泄漏</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/ide-code-scanner" target="_blank">Code Scanner代码检查</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">命令行工具</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/hiprofiler#%E5%B8%B8%E7%94%A8%E5%91%BD%E4%BB%A4" target="_blank">hiprofiler抓取调用栈数据</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/hidumper#查询进程内存" target="_blank">hidumper查询进程内存</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/hidumper#查询虚拟机堆内存" target="_blank">hidumper导出虚拟机快照</a></li>
      </ul>
    </div>
  </div>

  <div class="doc-category">
    <h2 class="doc-category-title">资源泄漏API能力开放</h2>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">资源泄漏事件订阅</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/hiappevent-watcher-resourceleak-events" target="_blank">资源泄漏事件介绍</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/forum/topic/0202217015059997961?fid=0109140870620153026" target="_blank">资源采集API特性指导</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">ArkTS内存泄漏</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/harmonyos-references/js-apis-hidebug#hidebugdumpjsrawheapdata18" target="_blank">异步导出ArkTS快照API</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/harmonyos-references/js-apis-hidebug#hidebugsetprocdumpinsharedoom24" target="_blank">进程级ArkTS快照导出</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/harmonyos-references/js-apis-jsleakwatcher" target="_blank">JSLeakWatcher对外开放API</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">其他API能力</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/doc/app/agc-help-apms-gray-scale-collect-0000002619401669" target="_blank">HiRetrieval应用灰度采集接口</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/doc/harmonyos-references/js-apis-hidebug#hidebuggetrssinfo24" target="_blank">获取应用物理内存(RSS)</a></li>
      </ul>
    </div>
  </div>

  <div class="doc-category">
    <h2 class="doc-category-title">资源泄漏视频资料</h2>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">ArkTS内存泄漏与分析</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/training/course/live/C301783664865179515" target="_blank">ArkTS内存泄漏故障模式介绍</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/training/course/live/C101787207224535476?pathId=401788146257334517" target="_blank">ArkTS内存泄漏定位分析方法</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/training/course/live/C101765353484147091" target="_blank">ArkTS内存泄漏分析与案例</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/training/course/live/C201784623786393705" target="_blank">揭秘JSLeakWatcher-精准发现ArkTS内存泄漏</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/training/course/live/C301779952542462179" target="_blank">ArkTS易调易测能力分享</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/training/course/live/C201786670125807161?pathId=401788146257334517" target="_blank">ArkTS快照新特性介绍</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">Native内存泄漏与分析</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/training/course/live/C201785979005407921" target="_blank">RSS内存泄漏故障模式库与问题分析介绍</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">资源采集API实践</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/training/course/live/C401787799158154021" target="_blank">内存泄漏-资源采集API实践</a></li>
      </ul>
    </div>

    <div class="doc-subcategory">
      <h3 class="doc-subcategory-title">资源泄漏日志自动化分析</h3>
      <ul class="doc-links">
        <li><a href="https://developer.huawei.com/consumer/cn/training/course/live/C201787126686067895" target="_blank">mem-analyzer工具指导、效果展示与案例分析</a></li>
        <li><a href="https://developer.huawei.com/consumer/cn/training/course/live/C101786437351753212" target="_blank">AI辅助内存泄漏动静态定位案例与实战</a></li>
      </ul>
    </div>
  </div>
</div>
)HTML";
}

// 构建帮助中心导航栏内容
std::string nav_bar_help_center_content() {
    std::string content = R"HTML(
<div class="help-container">
  <div class="help-section help-hero">
    <h2 class="help-title">关于</h2>
    <p class="help-desc">mem-analyzer是专为HarmonyOS设计的命令行工具，主要用于资源泄漏日志的采集和深度分析。其分析对象涵盖 ArkTS 堆快照、NativeHeap 及 DMA 内存调用栈信息、ArkTS 全局/局部句柄记录，以及文件描述符（FD）、线程（Thread）等各类系统资源日志。</p>
  </div>

  <div class="help-section help-links">
    <a class="help-link" href="https://gitcode.com/ancimoon/developtools_dfx_mem_analyzer/releases" target="_blank">
      <span class="help-link-label">工具下载</span>
      <span class="help-link-url">https://gitcode.com/ancimoon/developtools_dfx_mem_analyzer/releases</span>
      <span class="help-chevron" aria-hidden="true"></span>
    </a>
    <a class="help-link" href="https://gitcode.com/ancimoon/developtools_dfx_mem_analyzer/blob/master/docs/usage.md" target="_blank">
      <span class="help-link-label">使用手册</span>
      <span class="help-link-url">https://gitcode.com/ancimoon/developtools_dfx_mem_analyzer/blob/master/docs/usage.md</span>
      <span class="help-chevron" aria-hidden="true"></span>
    </a>
    <a class="help-link" href="https://gitcode.com/ancimoon/developtools_dfx_mem_analyzer" target="_blank">
      <span class="help-link-label">源码下载</span>
      <span class="help-link-url">https://gitcode.com/ancimoon/developtools_dfx_mem_analyzer</span>
      <span class="help-chevron" aria-hidden="true"></span>
    </a>
  </div>

  <div class="help-section">
    <h2 class="help-title">技术支持</h2>

    <div class="help-support-block">
      <h3>问题反馈</h3>
      <p class="help-desc">工具问题、功能易用性反馈、新功能诉求等，欢迎直接到【项目讨论】板块<a href="https://gitcode.com/ancimoon/developtools_dfx_mem_analyzer/discussions/type" target="_blank">https://gitcode.com/ancimoon/developtools_dfx_mem_analyzer/discussions/type</a>按类型发起讨论，收到您的反馈后，我们将第一时间分析并解决问题，以帮助更好地优化工具产品。</p>
    </div>
  </div>
</div>
)HTML";
    return content;
}

// 设置切换导航栏函数
std::string nav_bar_switch_nav_func() {
    return R"HTML(
    window.switchNav = function(navId, el) {
    var target = document.getElementById(navId);
    if (!target) {
      console.error('Nav content not found:', navId);
      return;
    }

    var allNavs = document.querySelectorAll('.nav-content');
    for (var i = 0; i < allNavs.length; i++) {
      allNavs[i].style.display = 'none';
      allNavs[i].classList.remove('active');
    }

    target.style.display = 'block';
    target.classList.add('active');

    var allItems = document.querySelectorAll('.nav-item');
    for (var i = 0; i < allItems.length; i++) {
      allItems[i].classList.remove('active');
    }
    if (el) {
      el.classList.add('active');
    } else {
      var items = document.querySelectorAll('.nav-item');
      for (var i = 0; i < items.length; i++) {
        if (items[i].getAttribute('data-nav') === navId) {
          items[i].classList.add('active');
        }
      } 
    }
  }; 
)HTML";
}

}