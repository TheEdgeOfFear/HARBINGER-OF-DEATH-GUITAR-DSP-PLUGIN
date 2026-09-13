# Installation Guide — HARBINGER OF DEATH
### *By THE EDGE OF FEAR*

---

## 📥 Included Formats

| Format | File Location | Intended Use |
| :--- | :--- | :--- |
| **VST3** | [VST3/HARBINGER OF DEATH.vst3](VST3/) | Modern DAWs (Reaper, Cubase, Studio One, Ableton, FL Studio, Bitwig, Pro Tools via wrapper) |
| **Standalone** | [Standalone/HARBINGER OF DEATH.exe](Standalone/) | Live playing & practice without opening a DAW |
| **DLL** | [DLL/HARBINGER OF DEATH.dll](DLL/) | Custom VST hosts / legacy plugin directories |

---

## 🖥️ 1. VST3 Plugin Installation (DAWs)

### Option A: Automatic 1-Click Batch Installer (Recommended)
1. Navigate to the [VST3/](VST3/) folder in this repository.
2. Right-click **INSTALL_HARBINGER_VST3.bat** and choose ** Run as administrator**.
3. The script will automatically copy HARBINGER OF DEATH.vst3 into the standard Windows 64-bit VST3 location:
   `
   C:\Program Files\Common Files\VST3\
   `
4. Restart or rescan plugins in your DAW.

### Option B: Manual Installation
1. Copy the folder HARBINGER OF DEATH.vst3 from [VST3/](VST3/).
2. Paste it into:
   `
   C:\Program Files\Common Files\VST3\
   `
3. In your DAW settings, click **Rescan VST3 Plugins**.

---

## 🎸 2. Standalone Application Setup (No DAW)

1. Open the [Standalone/](Standalone/) directory.
2. Double-click **HARBINGER OF DEATH.exe**.
3. On first launch, click **Options > Audio/MIDI Settings**:
   - **Audio Device Type**: Select ASIO (recommended) or Windows Audio (Exclusive Mode).
   - **Device**: Select your audio interface (e.g., Focusrite, MOTU, Universal Audio, Behringer).
   - **Sample Rate / Buffer Size**: Set to 44100 / 48000 Hz and 64 - 128 samples for lowest latency.
   - **Active MIDI Inputs**: Check your MIDI foot pedal or USB MIDI keyboard.

---

## ⚙️ 3. DLL Format Installation

1. Open the [DLL/](DLL/) folder.
2. Copy **HARBINGER OF DEATH.dll**.
3. Paste into your custom 64-bit VST plugin folder (for example: C:\Program Files\VstPlugins\ or your DAW's designated custom VST path).
4. Perform a plugin rescan in your host.
