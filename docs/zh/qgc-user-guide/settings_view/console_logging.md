# 控制台日志

_控制台_ 可以成为诊断 _QGroundControl_ 问题的有用工具。 可以在 **设置 > 控制台**中找到。

![控制台日志](../../../assets/support/console.jpg)

点击 **设置日志** 按钮来启用/禁用由 _QGroundControl_显示的日志信息。

## 常见日志选项

最常用的日志选项列于下文。

| 选项                                          | 描述                            |
| ------------------------------------------- | ----------------------------- |
| `链接管理器日志`, `多载具管理日志`                        | 调试连接问题。                       |
| `连接管理器冗余日志`                                 | 未检测到调试序列端口。 可用串行端口的连续输出噪音非常大。 |
| `固件升级日志`                                    | 调试固件闪存问题。                     |
| `参数管理器日志`                                   | 调试参数加载问题。                     |
| `参数管理器调试缓存失败日志`                             | 调试参数缓存错误.     |
| `计划管理器日志`、`任务管理器日志`、`地理围栏管理器日志`、`集结点管理器日志`。 | 调试计划上传/下载问题。                  |
| `无线电组件控制器日志`                                | 调试无线电校准问题。                    |

## 从命令行进行日志记录

用于日志记录的替代机制是使用 `--logging` 命令行选项。 如果你试图从 _QGroundControl_ 崩溃的情况下获取日志，这将非常方便。

执行此作的方式以及跟踪的输出位置因作系统而异：

- Windows

  - 您必须打开命令提示符，将目录更改为**qgroundcontrol.exe** 位置，并从那里运行它：

    ```sh
    cd "\Program Files (x86)\qgroundcontrol"
    qgroundcontrol --logging:full
    ```

  - 当_QGroundControl_ 启动时，您应该看到一个单独的控制台窗口打开，它将有日志输出

- OSX

  - 您必须从终端运行 _QGroundControl_ 。 终端应用程序位于应用程序/实用程序。 打开终端后，将以下内容粘贴到其中：

    ```sh
    cd /Applications/qgroundcontrol.app/Contents/MacOS/
    ./qgroundcontrol --logging:full
    ```

  - 记录轨迹将输出到终端窗口。

- Linux

  ```sh
  /qgroundcontrol-start.sh --logging:full
  ```

  - 记录轨迹将输出到您正在运行的 shell。
