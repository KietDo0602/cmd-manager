# ⚙️🛢️ CMD-Manager

CMD-Manager is a lightweight C++ utility software designed to manage and simplify complex terminal commands. Whether you're tired of typing long scripts, choosing and finding files/folders for for command input/output,  or want to organize frequently used commands, this tool helps you streamline your workflow.

Available in 6 languages:
- English
- Spanish
- French
- Vietnamese
- Chinese
- Russian
  
<img width="1280" height="731" alt="image" src="https://github.com/user-attachments/assets/0fe560fa-d176-4821-ba3a-be7ae53c0a92" />

## 🚀 Features

- Save complex shell commands
- Search and Run saved commands efficiently
- Choose / Create Files easily
- Manage, Save and Delete commands
- Cross-platform support (Linux, macOS, Windows)
- Customizable themes and fonts

## ⬇️ Installation

### 🪟 Windows
- [Download Here](https://github.com/kietdo0602/cmd-manager/releases/download/v1.0.0/cmd-manager-1.0.0-window-setup.exe)

### 🐧 Linux & Ubuntu
- [Download Here](https://github.com/kietdo0602/cmd-manager/releases/download/v1.0.0/cmd-manager-1.0.0-linux.tar.gz)

### 🍎 Mac OS
- Not yet available (contribution needed!)


## 🎨 Screenshots:
<table>
  <tr>
    <td><img src="https://github.com/user-attachments/assets/bdf7cc3e-6008-4c5c-b08f-1f4bd207a319" width="300"/></td>
    <td><img src="https://github.com/user-attachments/assets/c7fea718-eb17-4f86-8a20-13e8eb702c41" width="300"/></td>
    <td><img src="https://github.com/user-attachments/assets/bdac362b-9b90-4ba2-80d3-62f09268b7de" width="300"/></td>
  </tr>
  <tr>
    <td><img src="https://github.com/user-attachments/assets/13971e72-0302-44b7-97bc-77f6b4f8a624" width="300"/></td>
    <td><img src="https://github.com/user-attachments/assets/f59859d8-5db9-43b7-8cbf-912f369ae960" width="300"/></td>
    <td><img src="https://github.com/user-attachments/assets/c0a60973-b22b-44f8-85b9-cac2cb1ee107" width="300"/></td>
  </tr>
</table>

## 📘 Instructions
1. **Open the app** on Linux, macOS, or Windows.  
2. How to use:
   - Enter the **Command String** (e.g., `echo "Hello World"`).  
   - (Optional) Choose a **Working Directory** (This is the folder where the command will be executed).  
   - Click **Validate** to validate and process the **Command String**
   - (Optional) Replace **placeholder file** inside the command, using **Choose File** or **Create File**. Make sure the placeholder has the correct type (input or output)
   - Press **Run Command** to run the command.
3. Press **Save** to store your commands in a JSON file and type in a name.
4. Use **All Commands** to find previously saved commands.
5. Set a **Current Directory** if needed for execution - this is where the command will be executed.  
6. The output will appear in a terminal window.


## ⚙️ Configurations
- JSON file is stored inside "~/.config/CMDManager/commands.json"
- Settings file is stored inside "~/.config/CMDManager/settings.ini"

## 📁 Project Structure

```
▸ assets/
▸ build/
▾ src/
  ▾ core/
      settingsmanager.cpp
      settingsmanager.h
      utils.cpp
      utils.h
  ▾ ui/
      filerowwidget.cpp
      filerowwidget.h
      mainwindow.cpp
      mainwindow.h
      settingsdialog.cpp
      settingsdialog.h
    CMakeLists.txt
    main.cpp
  .gitignore
  CMakeLists.txt
  LICENSE
  README.md
```


## 🛠️ Build Instructions

This project uses [CMake](https://cmake.org/) for building. Make sure you have CMake and a C++ compiler installed.

### 🔧 Requirements

- CMake ≥ 3.5
- C++17 compatible compiler (e.g., GCC, Clang, MSVC)

### 🧱 Build Steps

```bash
# Clone the repository
git clone https://github.com/kietdo0602/CMD-Manager.git
cd CMD-Manager

# Create a build directory
mkdir build && cd build

# Run CMake
cmake ..

# Build the project
cmake --build .
```

### 🧪 Run the Program

After building, the executable `CMD-Manager` will be located inside the `build/src` directory.

```bash
./src/CMD-Manager
```
Run it to start the program!


## 🧹 Clean Build

To remove all build artifacts:

```bash
cd build
rm -rf *
```

Or use CMake's clean target if available:

```bash
cmake --build . --target clean
```

## 🤝 Contributing

Pull requests are welcome! If you have ideas for new features or improvements, feel free to open an issue or submit a PR.

Create new branch, using the following names only:

- /feature/example_feature_name
- /bugs/example_bug_name
- /ui/example-new-ui
- /refactor/refactor-example

Make sure to list any OS-specific problems inside the branch name or description.


## 📝 To Do

- [x] Release 1.0 version


## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## 🙌 Acknowledgments

Inspired by the need to reduce repetitive terminal work and improve developer productivity.

---

Manage commands more easily! 🧠💻

