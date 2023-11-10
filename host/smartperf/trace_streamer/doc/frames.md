# 帧渲染数据解析的逻辑
帧的解析分为应用帧的解析和渲染帧的业务解析。  
## 应用帧
应用帧解析主要是进行应用帧业务的开始和结束，并对其帧编号进行记录，以便在RS的业务中进行匹配（RenderService简称为RS）。  
应用帧会有下面一些调用栈：  
H:ReceiveVsync  
H:OnVsyncEvent  
H:MarshRSTransactionData  
H:ReceiveVsync包含H:OnVsyncEvent, H:OnVsyncEvent包含H:MarshRSTransactionData.  
特别地，当一个栈没有H:MarshRSTransactionData事件时，是无效的帧。  
H:MarshRSTransactionData内会包含线程号和应用的帧标号。  例如： H:MarshRSTransactionData cmdCount:3 transactionFlag:[32402,146] 
## 渲染帧
渲染帧会有下面一些调用栈：  
H:ReceiveVsync  
H:RSMainThread::DoComposition
H:RSMainThread::ProcessCommandUni  
H:ReceiveVsync包含H:RSMainThread::DoComposition, H:RSMainThread::DoComposition包含H:RSMainThread::ProcessCommandUni.  
特别地：当渲染帧不包含H:RSMainThread::ProcessCommandUni时，为无效的渲染帧。  
H:RSMainThread::ProcessCommandUni内包含被渲染帧所属的线程号和帧编号。例如： H:RSMainThread::ProcessCommandUni [32402,146]   
## 应用帧和渲染帧关联
应用帧和渲染帧通过H:MarshRSTransactionData和H:RSMainThread::ProcessCommandUni事件挟带的[tid, id]关联。  
## 应用帧数据计算
expect: 开始时间为应用帧H:ReceiveVsync事件中now字段指定的时间， 结束时间为end字段指定的时间。  
        需要注意这里的now和end指定的时间并非boottime，需要经过时钟同步后使用。 在文本格式的数据中无时钟源信息，所以无法进行时钟同步。进而导致文本和htrace解析结果不一致。  
actural: 开始时间为应用帧H:ReceiveVsync事件B子事件发生时间，结束时间为应用帧H:ReceiveVsync事件E子事件发生时间。  
## 渲染帧数据计算
expect: 开始时间为渲染帧H:ReceiveVsync事件中now字段指定的时间， 结束时间为end字段指定的时间。  
        需要注意这里的now和end指定的时间并非boottime，需要经过时钟同步后使用。 在文本格式的数据中无时钟源信息，所以无法进行时钟同步。进而导致文本和htrace解析结果不一致。  
actural: 开始时间为渲染帧H:ReceiveVsync事件B子事件发生时间，结束时间为渲染帧H:ReceiveVsync事件E子事件发生时间。  
## FrameTimeline的数据计算
expect: 开始时间为应用帧expect的开始时间， 结束时间为渲染帧expect的结束时间。  
actrual: 开始时间为应用帧H:ReceiveVsync事件B子事件发生时间，结束时间为渲染帧H:ReceiveVsync事件E子事件发生时间。  
## GPU渲染时长
和渲染帧同步并发的，是渲染线程的gpu信息，在事件中是H:M: Frame queued事件。  
只有当H:M: Frame queued事件的开始时间在渲染帧的H:ReceiveVsync时间的开始到结束的时间范围时，将该H:M: Frame queued事件和H:ReceiveVsync做关联，两者构成一个完整的渲染事件（也可能没有H:M: Frame queued事件）。  
而应用的渲染帧编号，和渲染线程中H:RSMainThread::ProcessCommandUni事件所携带的被渲染的帧编号，所属的线程号关联之后，形成应用帧的一个完整渲染事件。  
## 关于帧卡顿的定义
关于帧卡顿的定义：帧的实际渲染时间晚于期望被渲染的时间，认为帧卡顿。数据库中标识为1，否则为0。  
期望帧不做任何标识，为默认的255(max u8)，导出db时为空。  
特别地：以上事件发出的线程，必须是进程中的主线程，即线程号和进程号一致，否则，不做处理。  
另外，|渲染帧actural开始时间 -  应用帧actural结束时间| > 1ms时， 认为渲染状态异常。 数据库中标识为3.  
## 关于无效帧的定义
如果应用帧不包含FrameNum，则其为无效帧。  
无效帧的标识为2。  
当帧为无效帧时，会同时标识其实际帧和期望帧为无效帧。  