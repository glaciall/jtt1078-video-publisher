## jtt1078-video-publisher
通过进程的**stdin**读取符合JT/T 1078协议的RTP消息包，完成分包处理、拆分音视频，并且通过**ffmpeg**完成编码推流到RTMP服务器去。

### 参数说明
1. fifo文件名：`--fifo-name=`
2. 视频编码：`--video-encoding=`
3. 音频编码：`--audio-encoding=`
4. 音频采样率：`--audio-rate=`
5. 音频通道数：`--audio-channel=`
6. rtmp地址：`--rtmp-url=`

举个例子：
```bash
jtt1078 --fifo-name=111222 \
        --video-encoding=h264 \
        --audio-encoding=alaw \
        --audio-rate=8000 \
        --audio-channel=1 \
        --rtmp-url=rtmp://localhost/live/fuck
```

