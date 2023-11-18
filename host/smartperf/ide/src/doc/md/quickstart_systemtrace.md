# web端加载trace说明
从web端查看trace文件，进行性能检测的分析。
## 文件加载入口
将抓取的trace导入查看。
![GitHub Logo](../../figures/Web/opentrace.jpg)

说明：
+ Open trace file：导入离线trace文件入口。
+ Open long trace file：导入大文件入口。
+ Record new trace：抓取新的trace文件入口。
+ Record template：抓取指定模块的trace入口。

## 导入trace文件后显示页面
![GitHub Logo](../../figures/Web/trace.jpg)
说明：
+ 操作说明：在当前页面可以通过键盘上的wasd四个键位操纵当前的时间轴进行缩放，w为放大，s为缩小，a为左移，d为右移。
## trace功能介绍
trace模块从上往下主要展示时间轴、cpu使用率、cpu使用情况、进程间通讯数据的方法调用情况、进程、线程和方法调用情况。
### 时间轴和cpu使用率
![GitHub Logo](../../figures/Web/time.jpg)
最上方带刻度的为时间轴，主要展示当前抓取数据的总时长和时间刻度的分布情况，如上图所示，左下角展示总时长。
中间区域展示的是抓取数据时间段内的cpu使用率，颜色越深代表cpu使用率越高，颜色越浅代表cpu使用率越低。
![GitHub Logo](../../figures/Web/highlit.jpg)
在白色背景时间轴区域内可以点击后拖拽鼠标，可以对从鼠标按下到拖拽完成鼠标松开的区域内的数据进行筛选，高亮显示的部分为当前所选区域，如上图所示。
### cpu使用情况
![GitHub Logo](../../figures/Web/cpu.jpg)
 
如上图所示，当前抓取数据有4个cpu工作，前四组数据对应的是当前调用cpu的线程和对应的进程情况，以颜色作为区分。后四组数据则为cpu的使用频率信息。鼠标移动到相应的线程上还会将当前选中的进程信息全部置为高亮，其他的进程会置灰，如下图所示。
![GitHub Logo](../../figures/Web/gray.jpg)
#### cpu使用情况的框选功能
可以对cpu的数据进行框选，框选后在最下方的弹出层中会展示框选数据的统计表格,总共有六个tab页。
CPU by thread的Tab页，主要显示了在框选时间区间内的进程名、进程号、线程名、线程号、总运行时长、平均运行时长和调度次数信息。
![GitHub Logo](../../figures/Web/cpubythread.jpg)
CPU by process的Tab页，主要显示了在框选时间区间内的进程名、进程号、总运行时长、平均运行时长和调度次数信息。
![GitHub Logo](../../figures/Web/cpubyprocess.jpg)
CPU Usage的Tab页，主要显示了在框选时间区间内，该频率时间占比前三的信息。
![GitHub Logo](../../figures/Web/cpusage.jpg)
Thread Switches的Tab页，按照状态>进程>线程，统计对应状态下的次数，持续时长，最小时长，平均时长，最大时长信息。
![GitHub Logo](../../figures/Web/threadswitches.jpg)
Thread States的Tab页，按进程>线程>状态的维度去统计，需要呈现该状态的线程名、进入该状态次数、该状态下时长、最小时长、平均时长、最大时长。
![GitHub Logo](../../figures/Web/threadstates.jpg)
Sched Priority的Tab页，按优先级显示调度，显示框选范围内所有Running以及Running之前的Runnable线程的最小，最大，平均耗时。 
![GitHub Logo](../../figures/Web/schedpritab.jpg)
Thread States、Thread Switches的2个Tab页，点击移动到某一行，鼠标会变成一个小手的标志，并且会高亮，点击一下，就会进入辅助信息界面，会将选中行的辅助信息展示出来，包括开始时间、进程、线程、线程状态、对应的CPU、优先级等信息如下图。
![GitHub Logo](../../figures/Web/details.jpg)
#### cpu使用情况的单选功能
单选CPU使用情况数据会在选中的色块外层加上深色边框，能够突出当前选中色块，弹出层中会展示当前CPU上的进程名，线程名，开始时间的相对时间，开始时间的绝对时间，运行时长，线程结束状态信息。
![GitHub Logo](../../figures/Web/cpuclick.jpg)
### 线程唤醒关系树
线程唤醒关系树功能是点击GetWakeupList会以链式结构和表格分别在泳道图和tab上展示线程的唤醒关系树信息,点击GetWakeupList按钮旁边的星号按钮可以对唤醒树对应的泳道图一键收藏，对优先级最低且运行时长最长或者运行时长最长的线程会以红色字体高亮显示。
![GitHub Logo](../../figures/Web/threadtree.jpg)
### 进程、线程和方法数据
下图是进程数据，左边部分展示进程名称和id，右边显示线程切换关系，线程的调用方法，进程间内存信息等。
![GitHub Logo](../../figures/Web/process.jpg)
点击进程名前面箭头可以展开对应的线程进行查看，展开后的线程如下图，如果存在堆内存占用情况，会显示出对应泳道图，如果出现两个名字和id一样的线程，则第一个为线程的使用情况，第二个为线程内的方法栈调用情况。
![GitHub Logo](../../figures/Web/threadinfo.jpg)
#### 进程、线程和方法数据的框选功能
可以对线程的数据进行框选，框选后在最下方的弹出层中会展示框选数据的统计表格，包含线程运行状态，线程调用栈的统计情况。当框选的数据中同时存在线程运行状态和线程调用栈数据，下方的弹出层中就会出现多个tab选项，可以进行切换。

下图是线程运行状态框选统计信息,包括进程名、进程号、线程名、线程号、线程状态、状态持续时间、平均持续时间、该线程状态发生的次数。
![GitHub Logo](../../figures/Web/threadselect.jpg)

下图是线程调用栈框选统计信息,包括方法名、持续时间、平均持续时间、调用的次数。
![GitHub Logo](../../figures/Web/callstackselect.jpg)
#### 进程、线程和方法数据的单选功能
单选线程的state数据时，会展示当前选中线程的状态数据，开始时间(相对和绝对时间)和持续时长，线程状态，所在进程名称，选中状态的前一个状态，选中状态的后一个状态。
![GitHub Logo](../../figures/Web/threadclick.jpg)
单选调用栈数据，单击方法会在选中的方法外层加上黑色边框，能够突出当前选中的方法，弹出层中会展示当前方法的名称、开始时间(相对和绝对时间)和运行时长信息，调用栈深度。
![GitHub Logo](../../figures/Web/callstackclick.jpg)
#### 线程的跳转功能
点击画红框处的带箭头的标志，会从CPU的线程概览视图跳转到线程的详情视图，同时从线程的详情视图也能跳转到CPU的线程概览视图。
![GitHub Logo](../../figures/Web/jumpthread.jpg)
### trace的其他功能
#### 小旗标志的功能
将选中数据的时间点通过小旗的方式在时间轴上展示，直观的展示选中数据的时间。
![GitHub Logo](../../figures/Web/flag.jpg)
#### 勾选功能
框选某一区域，该区域左边会出现CheckBox的复选框，选中的区域的复选框会出现打勾的状态，可以取消勾选，也可以重新勾选。
![GitHub Logo](../../figures/Web/checkbox.jpg)
#### 搜索功能
在搜索框中，可以输入线程，线程号等搜索自己想要的信息，搜索完成会高亮显示。
![GitHub Logo](../../figures/Web/search.jpg)
在搜索框中输入调用栈的方法名，会跳转到对应的调用栈。
![GitHub Logo](../../figures/Web/searchcallstack.jpg)
### Tab页信息和泳道图可跳转(点击和框选场景,框选类似搜索)
泳道图高亮场景：框选Cpu Frequency 或者Cpu State泳道图后，弹出Cpu Frequency或Cpu State Tab页，在点击Tab页表格的行时，框选范围泳道图的当前行的Value值一样的部分上方会出现一条以圆点开头颜色比趋势图颜色同色但稍深的粗线条，如下图:
![GitHub Logo](../../figures/Web/Tabskill.jpg)
搜索场景：框选函数调用栈的泳道图，弹出Slices Tab页，点击表格行，会跳转到框选范围内的第一条调用栈的位置，点击下图Slices Tab页的Background concurrent copying GC调用栈。
![GitHub Logo](../../figures/Web/Tabskillcalltack.jpg)
会在搜索栏显示出该调用栈的搜索结果，并且会跳转到框选范围内第一条Background concurrent copying GC调用栈的位置。
![GitHub Logo](../../figures/Web/Tabcallstackskip.jpg)
#### M键测量功能
放大trace中的色块，选中色块，键盘按下M，会出现像标尺一样的形状。
![GitHub Logo](../../figures/Web/M.jpg)