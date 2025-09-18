# ⚙️📋 CMD-Manager

CMD-Manager is a lightweight C++ utility designed to simplify and save complex terminal commands. Whether you're tired of typing long scripts or want to organize frequently used commands, this tool helps you streamline your workflow.

## 🚀 Features

- Save complex shell commands
- Run saved commands with a single keyword
- Organize commands into categories
- Cross-platform support (Linux, macOS, Windows)

## ⬇️ Installation


## 📘 Instructions
1. **Open the app** on Linux, macOS, or Windows.  
2. How to use:
   - Enter the **command string** (e.g., `echo "Hello World"`).  
   - (Optional) Choose a **working directory**.  
   - Click **Start** to process the **command string**
   - (Optional) Replace placeholder file inside command, make sure it has the correct type (input or output)
   - Press **Execute** to run the command.
3. Press **Save** to store your commands in a JSON file and type in a name.
4. Use **All Commands** to find previously saved commands.  
5. Set a **Current Directory** if needed for execution - this is where the command will be executed.  
6. The output will appear in a terminal window.


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

After building, the executable will be located inside the `build` directory:

```bash
./CMD-Manager
```

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

## 📝 To Do

- [ ] Fix Save behavior when opening an existing settings
- [ ] Add Settings
- [ ] Add color theme
- [ ] Add Help and About section


## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## 🙌 Acknowledgments

Inspired by the need to reduce repetitive terminal work and improve developer productivity.

---

Manage commands more easily! 🧠💻

