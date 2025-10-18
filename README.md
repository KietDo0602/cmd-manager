# ⚙️📋 CMD-Manager

CMD-Manager is a lightweight C++ utility software designed to manage and simplify complex terminal commands. Whether you're tired of typing long scripts, choosing and finding files/folders for for command input/output,  or want to organize frequently used commands, this tool helps you streamline your workflow.

Available in 6 languages:
- English
- Spanish
- French
- Vietnamese
- Chinese
- Russian

## 🚀 Features

- Save complex shell commands
- Search and Run saved commands easily
- Manage, save and delete commands
- Cross-platform support (Linux, macOS, Windows)

## ⬇️ Installation

### 🪟 Windows

### 🐧 Linux & Ubuntu

### 🍎 Mac OS


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


## 😃 Example


## ⚙️ Configurations
- JSON file is stored inside "/CMDManager/commands.json"


## 📁 Project Structure

```
CMD-Manager/
├── CMakeLists.txt
├── main.cpp
├── mainwindow.h
├── mainwindow.cpp
├── terminaldialog.h
├── terminaldialog.cpp
├── filerowwidget.h
├── filerowwidget.cpp
└── README.md
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

- [x] Sound / Notification On Command Complete
- [x] Export All Commands to JSON file
- [x] Load Commands from JSON file
- [x] Language Support
- [x] Search for Commands Menu
- [x] Sort by File name / Recently Opened
- [x] Pinned command - will go to the top
- [x] Drag and Drop Support
- [x] Command Validation and Preview
- [ ] Release 1.0 version


## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## 🙌 Acknowledgments

Inspired by the need to reduce repetitive terminal work and improve developer productivity.

---

Manage commands more easily! 🧠💻

