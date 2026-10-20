# Analog Triad

三款独立发声的虚拟模拟合成器插件，共用一个 C++20 / JUCE 9.0.4 代码库：

| 插件 | 对应实体合成器的功能组织 | 格式 |
|---|---|---|
| **MATRIARCH V** | 四振荡器准复音、立体声梯形滤波、双包络、双 LFO、90 类插接点、BBD 立体声延迟 | VST3 / AU / Standalone |
| **POLYBRUTE V** | 六声部、双 VCO + Sub、Metalizer、Steiner + Ladder 双滤波、三包络、三 LFO、调制矩阵、效果链 | VST3 / AU / Standalone |
| **ONE V** | 三个 Synth 部分共享声部池、每声部三 VCO、SVF + Ladder、三个 DAHDSR 包络、四个 LFO、效果 | VST3 / AU / Standalone |

本项目是**独立实现的软件乐器**，不是 Arturia 或 Moog 的官方产品，也不包含原厂固件、预设或界面素材。面板按公开手册与产品照片重绘，音频引擎全部自行实现。手册只描述操作与信号路径，不提供电路容差、非线性元件或专有数字效果的算法；因此本项目不声称“逐电路”或“与实机完全相同”。详见 [COVERAGE.md](docs/COVERAGE.md)。

## 当前状态

三款插件均已能编译为 macOS arm64 的 VST3 / AU / Standalone，并带有独立引擎、实体布局面板、预设与工程状态保存。真实进度以 [docs/COVERAGE.md](docs/COVERAGE.md) 与 [docs/VALIDATION.md](docs/VALIDATION.md) 为准；未写入“已测试”的功能不视为完成。

- 已完成并通过自动测试：三套引擎的确定性音频、音高、参数极值、延音/叠音/弯音/触后/All Notes Off、厂牌预设校验。
- 已完成并通过验证器：三个 VST3 在 pluginval strictness 5 通过；三个 AU 通过 `auval`。
- 已完成：Matriarch 虚拟接线（79 个插孔）、PolyBrute Morph A/B 与 8 槽调制矩阵、Moog One 三部分共享声部池与每部分 8 槽矩阵。
- 未完成：实机音色校准、Windows 构建与 DAW 实测、Developer ID 签名/公证、GitHub 推送、以及覆盖表中列出的硬件细节（Matriarch 12×256 音序、PolyBrute Morphée/Ribbon/64 步音序、One 每部分独立琶音等）。

Matriarch 的键盘按原机 49 键 F1–F5 显示；音高以 A4 = 440 Hz 为基准，并有自动回归测试。

## 构建

依赖：

- macOS 11+ 或 Windows 10+ x64
- CMake 3.22+
- Xcode / Clang（macOS）或 Visual Studio 2022（Windows）
- JUCE **9.0.4**，放在源码树之外
- Ninja（推荐）

```bash
# 1. 获取固定版本的 JUCE
git clone --branch 9.0.4 --depth 1 https://github.com/juce-framework/JUCE.git ../work/JUCE

# 2. 配置（构建目录放在仓库外）
cmake -S . -B ../work/build-triad -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DTRIAD_JUCE_PATH=../work/JUCE

# 3. 编译
cmake --build ../work/build-triad

# 4. 运行确定性引擎测试
ctest --test-dir ../work/build-triad --output-on-failure
```

产物位置：

```
../work/build-triad/Matriarch_artefacts/Release/VST3/MATRIARCH V.vst3
../work/build-triad/Matriarch_artefacts/Release/AU/MATRIARCH V.component
../work/build-triad/Matriarch_artefacts/Release/Standalone/MATRIARCH V.app
```

POLYBRUTE V 的产物名是 `POLYBRUTE V.vst3` / `POLYBRUTE V.component`；ONE V 的产物名带空格：`ONE V.vst3`、`ONE V.component`。构建目标名是 `Matriarch_VST3`、`MoogOne_AU` 等。

Universal 构建：

```bash
cmake -S . -B ../work/build-triad-universal -G Ninja \
      -DCMAKE_BUILD_TYPE=Release -DTRIAD_UNIVERSAL=ON -DTRIAD_JUCE_PATH=../work/JUCE
```

## 安装（自用）

```bash
# 安装当前用户可见的 VST3 与 AU，并复制独立程序到 /Applications
./scripts/package-macos.sh --build ../work/build-triad --install
```

脚本只复制已经编译成功的产物，不会伪造未生成的格式。卸载时删除：

```
~/Library/Audio/Plug-Ins/VST3/MATRIARCH V.vst3
~/Library/Audio/Plug-Ins/VST3/POLYBRUTE V.vst3
~/Library/Audio/Plug-Ins/VST3/ONE V.vst3
~/Library/Audio/Plug-Ins/Components/MATRIARCH V.component
~/Library/Audio/Plug-Ins/Components/POLYBRUTE V.component
~/Library/Audio/Plug-Ins/Components/ONE V.component
```

首次加载未签名插件时，macOS 可能要求在“系统设置 → 隐私与安全性”中允许。正式分发需要 Developer ID 签名与公证，本项目当前没有该身份。

Windows 使用 GitHub Actions 的 Windows runner 构建，产物为 `.vst3` 与 `.exe`；未在本机验证的 Windows DAW 加载结果不会写成“已测试”。

## 使用

- 顶部栏选择预设、INIT、保存/载入 `.triadpreset` 文件；预设按 Bass / Lead / Brass / Pad / Keys / Sequence / Split / FX 分类。每台琴内置 15、17、15 个预设（含 Reese Bass、Acid Bass、Supersaw、Warm Pad、Organ、Pluck 等）。
- 面板按官方手册与产品照片重排：Matriarch 单行模块顺序、PolyBrute 三段式合成区（VCO/滤波/包络/LFO 三行）、Moog One 左/中/右三块面板。资料页码与模块位置见 [docs/LAYOUT.md](docs/LAYOUT.md)。旋转拖动旋钮，双击回到默认值；硬件开关按档位点击。
- 底部键盘可点击演奏；滚轮为 Pitch/Mod（Pitch 松手回中）。Matriarch 显示 49 键 F1–F5，PolyBrute 与 Moog One 显示 61 键 C2–C7。Moog One 的 `MIDI PART` 选择屏幕键盘发送到哪一部分。
- Matriarch 插线：先点一个输出插孔，再点一个输入插孔建立连线；右键点击插孔清除与它相连的线缆。线缆随工程状态保存。
- PolyBrute：`CAPTURE B` 把当前面板存入 B，`MORPH` 在 A/B 之间插值；`SWAP A/B` 交换两侧。
- 所有参数均暴露给宿主自动化、MIDI Learn 与工程保存。宿主自动化按块应用，不宣称样本精确。

## 目录

```
src/core       与框架无关的参数、引擎契约、DSP 基元、琶音/音序器
src/engines    Matriarch / PolyBrute / One 的独立实现（无 JUCE 类型）
src/plugin     JUCE 处理器、参数树、状态、预设、宿主总线
src/ui         矢量绘制的实体面板、控件、键盘、虚拟插线（不嵌入厂商图片）
tests          确定性的离线音频与引擎契约测试
scripts        打包与安装
docs           覆盖表、验证记录、开发约定
```

## 许可与免责

本仓库为自用项目，尚未选择开源许可证；在添加许可证前不要公开分发源码或二进制。JUCE 的许可随所选版本与分发方式决定（参见 JUCE 9 许可条款）。Arturia、Moog、PolyBrute、Matriarch、Moog One、Eventide 等名称属于各自权利人，仅用于说明参照对象。
