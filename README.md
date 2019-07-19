# OSU-Referee

#### HEXCHAT COMMAND

/BB: Usage: BB \<message> 给BanchoBot发送消息<br/>
/INI: Usage: INI <key> [value] 查看或修改referee.ini配置<br/>
  
#### PRIVMSG

!y: 回复 “斯哈斯哈”<br/>

## Building

HexChat Plugin Interface: https://hexchat.readthedocs.io/en/latest/plugins.html#plugins-on-windows-win32

HexChat Building: https://hexchat.readthedocs.io/en/latest/building.html

注意事项：
* 确保 gtk-build 在 C:\
* 移动 gtk-build 会导致 GTK+ 路径错误，且无法通过 hexchat.props 设置
* 别忘记修改 hexchat.props 中 IsccPath 的值
* 所有 python 版本都要安装 cffi
* 鉴于 WIN32 下 python3 插件无法加载，已取消其生成
* 该项目仅支持 Windows
