# Build Instructions for ZynqDataTransferClient

## Option 1: Using Qt Creator (Recommended)

1. Open Qt Creator
2. File → Open File or Project
3. Navigate to: `D:\Qt_projects\ZynqDataTransferClient`
4. Select: `ZynqDataTransferClient.pro`
5. Configure the project with your Qt kit
6. Build and Run (Ctrl+B, then Ctrl+R)

## Option 2: Using Command Line with CMake

### Prerequisites:
- Qt6 or Qt5 installed
- CMake installed
- MinGW or Visual Studio compiler

### Steps:

1. **Set Qt Environment Variables:**
   ```powershell
   # Find your Qt installation (usually in C:\Qt\)
   # Set the Qt directory path
   $env:CMAKE_PREFIX_PATH = "C:\Qt\6.9.1\mingw_64"
   ```

2. **Configure and Build:**
   ```powershell
   cd D:\Qt_projects\ZynqDataTransferClient
   Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
   mkdir build
   cd build
   cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:\Qt\6.9.1\mingw_64" ..
   cmake --build .
   ```

3. **Run the Application:**
   ```powershell
   .\Debug\ZynqDataTransferClient.exe
   ```

## Option 3: Using qmake (Alternative)

If you have Qt in your PATH:

```powershell
cd D:\Qt_projects\ZynqDataTransferClient
qmake ZynqDataTransferClient.pro
make
```

## Troubleshooting

### If Qt is not found:
1. Check Qt installation path (usually `C:\Qt\6.x.x\mingw_64`)
2. Add Qt bin directory to system PATH
3. Restart command prompt/IDE

### If build fails:
1. Ensure you have the correct Qt version (6.x.x)
2. Check that all required modules are installed (Core, Widgets, Network)
3. Verify compiler compatibility (MinGW vs MSVC)

## Expected Output

After successful build, you should see:
- `ZynqDataTransferClient.exe` in the build directory
- Application window with connection interface
- Ready to connect to Zynq server at 192.168.1.10:8888
