# Trace解析能力增强
Trace解析能力增强主要是提高Trace的解析能力。
### 焦点问题trace切割，选择焦点trace，聚焦问题发生时间范围，单独打开
焦点问题切割是将现有的trace加载到界面时，对指定时间进行框选，截取框选时间范围内的trace数据通过跳转的方式去打开新的trace界面，新界面的trace数据就是按照这段时间范围去切割的。如下图框选一段时间范围，点击右上方的小剪刀图标对框选时间范围数据进行剪切。
![GitHub Logo](../../figures/parsingability/cuttrace_bytime.jpg)
### 超大trace分段打开，超大trace按照固定大小切割，分段打开显示
超大trace分段打开是对现有配置抓取的同一种时钟源数据源的。也就是配置命令下发的抓取有效。Htrace数据按照端测切割的大小的分成多个文件，并将不能切割的hiperf、ebpf、arkts的数据单独存储。IDE通过将这几种源文件数据存储在IndexedDB，并调用trace streamer去进行切割数据，并在IDE测过滤筛选出切割的数据进行展示。
#### 超大trace浏览器用户数据目录配置说明
由于大文件切割的数据是保存在浏览器的IndexedDb中，默认存储在系统盘，所以需要修改用户目录数据存放位置。
##### 找到浏览器的exe执行文件位置(或者浏览器exe的快捷方式位置)， 在cmd窗口执行以下命令(edge浏览器)
	D:\deskTop\msedge.exe.lnk --user-data-dir=D:\Edgedata
		D:\deskTop\msedge.exe.lnk     浏览器的exe执行文件位置(或者浏览器exe的快捷方式位置)
		--user-data-dir=D:\Edgedata   指定用户目录数据位置,本地除系统盘外的位置都可以，内存尽量大一点
##### 浏览器exe或者exe快捷方式目录和名称不能有带空格
	错误：D:\desk Top\Microsoft Edge.exe.lnk --user-data-dir=D:\Edgedata 
	正确：D:\deskTop\MicrosoftEdge.exe.lnk --user-data-dir=D:\Edgedata
##### 配置完成后, 查看【用户配置路径】是否是配置的路径
	edge浏览器:   edge://version/
	chrome浏览器: chrome://version/
#### 超大trace抓取配置说明
![GitHub Logo](../../figures/parsingability/bigtracerecord.jpg)
+     Long Trace Mode： 超大trace抓取模式。
+     Single file max size： trace文件分割的大小。
#### 本地导入超大trace
本地导入超大trace是导入文件夹，将该文件夹中所有文件导入。如下图从Open long trace file入口导入。
![GitHub Logo](../../figures/parsingability/longtraceload.jpg)
本地导入以后，点击右上方的切页图标可以切页。
![GitHub Logo](../../figures/parsingability/longtraceswitch.jpg)
### web端支持已打开的trace文件转换成systrace，并可下载
已打开的trace界面，增加trace conver按钮，支持将htrace和row trace转为systrace。
![GitHub Logo](../../figures/parsingability/traceconvert.jpg)
### tracestreamer离线执行sql和metrics语句，对标trace_processor_shell –q和—run-metrics 
trace_streamer_shell –q可以直接将sql查询结果显示在命令行里，如下图bytrace.ftrace是需要解析的文件，a.txt是sql语句。
![GitHub Logo](../../figures/parsingability/tracestreamer_q.jpg)
trace_streamer_shell –m可以直接将metric接口的查询结果显示在命令行里，如下图bytrace.ftrace是需要解析的文件，trace_stats是metric接口名。
![GitHub Logo](../../figures/parsingability/tsmetric.jpg)