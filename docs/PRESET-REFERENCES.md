# 原版音色参考与改编记录

更新：2026-10-24。新增 22 个 `Ref` 音色，原有音色保留。`Ref` 表示参考改编，并非原厂文件、精确音色复制或已做实机 A/B。

## One：实际原厂参数参考

来源：[Moog One 官方下载页](https://www.moogmusic.com/downloads/?product=Moog%20One)、[官方 1.8.2 包](https://cdn.inmusicbrands.com/Moog/MoogOne/Moog_One_v1.8.2.zip)。2026-10-23 下载，压缩包 SHA-256：`3904ff4ae73bd8471ac5013301301bf1f972141f64740ef815ab512342e2a1dc`。

包内 `Past Preset Packs` 包含 222 个 `.m1p` 文件：CLASSICS 51、POLYPHONIC DESTINATIONS 22、STEREO FUTURES 45、TRANSFORMERS 53、KING 26、LISA BELLA DONNA 25。本次读取其可见文本参数，选取 12 个做数值映射与独立音色适配。没有提取固件代码，也没有把原厂压缩包、原预设文件或演示录音放入仓库。

| 本插件预设 | 参考原版 |
|---|---|
| Cinema Pulse Ref | CARPENTER BASS |
| Small Ladder Ref | MINIBASS |
| Stacked Foundation Ref | BIG UNISON BASS X2 |
| Sync Crest Ref | SUPERSYNC |
| Vintage Stringfield Ref | OB STRINGPAD |
| Dual Heritage Ref | TOM & BOB'S BRASS |
| Suitcase Bloom Ref | SUITCASE EP |
| Beyond Mood Ref | A MOOD BEYOND |
| Lunar Dawn Ref | MOONDAWN |
| Planet Waves Ref | JUPITER WAVES |
| Ballad Harmonics Ref | 80'S BALLAD GUITAR |
| Glass Assembly Ref | GLASSWORKS |

映射包含各部分启用、音高/波形/脉宽/混合、滤波频率与共振、包络、LFO 和 Mono/Unison。原版以音符单位表示的滤波频率转换为 Hz；硬件 Filter/VCA 包络映射到本引擎对应的包络。波形角度、滤波拓扑和增益按本实现适配。逐振荡器滤波分流、完整调制矩阵、原版时序和私有 FX 没有完整映射，不能因此宣称声音与原版一致。空间效果使用独立的轻量效果设置。源文件 local-port 开关由宿主 MIDI 取代；原版 Ladder HP 音色改用本引擎的 SVF HP 路径。Stacked Foundation 的原版动态调制未完整映射，故将滤波基准独立调整到 180 Hz，避免近乎静音。

复现：`python3 scripts/build-one-reference-bank.py /path/to/Moog_One_v1.8.2.zip`。生成器仅写入适配后的参数，不嵌入原文件或评论。试听参考：[Moog 官方 Lisa Bella Donna 演示](https://soundcloud.com/moogmusicinc/sets/moog-one-lisa-bella-donna-presets-2022)。本轮没有把播放列表当作已完成听辨 A/B 的证据。

## PolyBrute：原版设计说明参考

使用原版六声部型号的 [Arturia sound-design 页面](https://www.arturia.com/products/hardware-synths/polybrute/sound-design) 与 [官方声音示例](https://www.arturia.com/products/hardware-synths/polybrute/overview)。没有混用 PolyBrute 12 的工厂音色。

| 本插件预设 | 参考对象 / 对应设计 |
|---|---|
| A to B Voyage Ref | A_B Trip：长起音 pad 向节奏纹理演进 |
| Malleable Harp Ref | Harpsii：拨弦、缓慢调制与空间感 |
| Diminished Night Ref | Draculas C64：减和弦、琶音、复古游戏感 |
| First Frost Ref | First Snowfall 官方示例中的环境 pad 方向 |
| Juvenile Alloy Ref | Juvenile：演奏中连续改变音色 |
| Pulsed Landscape Ref | Pulsar / Mars Geology 的节奏型模拟音色方向 |

这些是依据公开设计说明重新编制的参数，**未获取上述六个原厂预设的参数文件，未完成音频听辨比较**。其中四个包含真实独立的 B 侧参数，载入后可用 MORPH 扫描，且随工程保存。现有合成结构尚不具备原机全部 Layer、Motion 和效果算法，因此不承诺完整还原参考对象。

## Matriarch：接线音色册参考

原机通过旋钮与接线构造音色，参考 [Moog Matriarch Dark Patchbook](https://cdn.inmusicbrands.com/Moog/Matriarch/Matriarch_Dark_Patchbook.pdf)，本轮实际查看了 PDF 第 4 与 11 页的面板图，其余条目参考同册文字说明。

| 本插件预设 | 参考对象 |
|---|---|
| Night Circuit Ref | Night City，第 4 页；准复音、短包络、滤波与振荡器 FM |
| Loose Mechanism Ref | Loose Sprockets，第 11 页；衰减后的调制、失谐与延迟 |
| Four Colour Keys Ref | 第 7 页的多振荡器不同波形与琶音建议 |
| Descending Echo Field Ref | 第 16 页衰减和弦与立体声回声建议 |

面板旋钮位置转换为本引擎单位时属于近似设计。软件模型和插孔还不完整，连线路径做了适配，并非照图逐线等效。三款 Matriarch Ref 预设带实际连线数据，载入时恢复线缆，不只是改变预设名称。

## 验证与试听

`triad_preset_tests` 检查参数/选择值、B 侧、连线方向及可听/有限/有界输出。

`triad_render_presets OUTPUT_DIR` 对全部 22 个 Ref 音色导出 48 kHz / 16 bit 立体声 WAV 和 `audit.csv`（峰值、RMS、DC、差分 RMS）。PolyBrute 扫描 A/B；使用统一短句演奏，末段留给尾音。Studio 扩展后，工具对已启用的琶音预设使用共享 PatternSequencer，在 110 BPM 下按事件采样位置驱动引擎；这仍不能代替 DAW 内完整音序和 transport 测试。

数值检查能发现静音、失控和响度问题；不能替代听辨或实机音色校准。原有 Continuum 库此次保持不变，另见原有覆盖表。
