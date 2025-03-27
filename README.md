# Controlled Auto-indent

**Controlled Auto-indent** is an experimental [Notepad++](https://github.com/notepad-plus-plus/notepad-plus-plus) plugin for testing possible extensions to automatic indentation.

Like Notepad++, this software is released under the GNU General Public License (either version 3 of the License, or, at your option, any later version).

This plugin was built using the [NppCppMSVS](https://github.com/Coises/NppCppMSVS/) framework, which is released under the GNU General Public License.

## Purpose and use

This plugin implements a more customizable version of the Auto-indent feature built into Notepad++. I’ve created it to enable some discussion and evaluation of possibly useful extensions to Notepad++ itself, which hopefully will eventually render this plugin obsolete.

**Controlled Auto-indent** will do nothing unless you check the **Enable** item on its menu. This assures that you can get it out of the way quickly if it is causing problems or if you want to use only built-in Notepad++ functionality.

*Before enabling this plugin, please make sure the Notepad++ setting at* **Settings** | **Preferences...** | **Indentation** : **Auto-indent** *is set to* **None**.

**Controlled Auto-indent** does not apply indentation to multiple or rectangular selections. This differs from the Auto-indent built into Notepad++, which computes indentation based on the primary selection or the row selected last and applies that to all selections or rows.

The **Settings...** dialog offers various options to control automatic indentation. These are explained in the [help file](https://coises.github.io/ControlledAutoIndent/help.htm#settings).

## Installation

To install **Controlled Auto-indent**, download the x86 or x64 zip file, depending on whether you're using 32-bit or 64-bit Notepad++, from the [latest release](https://github.com/Coises/ControlledAutoIndent/releases/latest/). Unzip the file to a folder named **ControlledAutoIndent** (the name must be exactly this, or Notepad++ will not load the plugin) and copy that folder into the plugins directory where Notepad++ is installed (usually **C:\Program Files (x86)\Notepad++\plugins** for 32-bit versions or **C:\Program Files\Notepad++\plugins** for 64-bit versions).
