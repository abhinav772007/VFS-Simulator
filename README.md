# VFS Simulator

A simple **Virtual File System** built in C++. It simulates a real disk file (`disk.img`) and lets you create files, folders, read/write data, take snapshots, and inspect how the filesystem works — all from a command-line interface.

---

## What This Project Does

This program creates a fake hard disk on your computer as a single file called `disk.img`. On top of that disk, it builds a small filesystem where you can:

- Create and delete files
- Write and read file content
- Create folders and move between them
- Save and restore snapshots (time travel)
- Visualize disk layout, bitmap, inodes, and directory tree
- Use a block cache to speed up repeated reads
- Benchmark read performance

Everything is stored inside `disk.img`, so your data survives after you close the program (when you use `load` next time).

---

## Project Structure

| File | Purpose |
|------|---------|
| `main.cpp` | Interactive CLI — reads commands and calls filesystem functions |
| `disk.cpp` / `disk.hpp` | Simulated disk — reads/writes 4096-byte blocks to `disk.img`, includes LRU block cache |
| `fs.cpp` / `fs.hpp` | Main filesystem logic — format, load, files, directories, snapshots, viz, bench |
| `inode.cpp` / `inode.hpp` | Inode table — stores file metadata (size, type, block pointers) |
| `bitmap.cpp` / `bitmap.hpp` | Free block tracker — marks blocks as used or free |
| `directory.cpp` / `directory.hpp` | Directory entries — maps filenames to inode numbers |
| `disk.img` | Virtual disk (created automatically) |
| `snapshots/` | Saved snapshot copies of the disk |

---

## How the Disk Is Organized

The virtual disk has **1024 blocks**, each **4096 bytes** (4 KB).

```
Block 0        → Superblock (filesystem settings)
Blocks 1–10    → Inode table (128 inodes)
Block 11       → Bitmap (tracks free/used blocks)
Blocks 12–1023 → Data blocks (file and directory content)
```

### Inode (file metadata)

Each file or folder has an inode that stores:

- `size` — file size in bytes
- `is_dir` — 0 = file, 1 = directory
- `direct_blocks[5]` — up to 5 data block pointers
- `used` — 1 = in use, 0 = free

### Directory

Each directory is stored in one disk block and holds up to **64 entries**. Each entry has:

- `name` — up to 31 characters
- `inode_id` — which inode this entry points to


## Build

Open terminal inside the `VFS-Simulator` folder:

```powershell
cd VFS-Simulator
g++ -std=c++17 -Wall -o fs.exe main.cpp disk.cpp fs.cpp inode.cpp bitmap.cpp directory.cpp
```

This creates `fs.exe`.

---

## Run

```powershell
.\fs.exe
```

On first start you must choose:

```
format   → erase disk and create a fresh filesystem
load     → open existing disk.img and continue where you left off
```

**Important:** Use `load` when you restart the program. Only use `format` when you want to erase everything.

---

## All Commands

After `format` or `load`, you get the `vfs>` prompt.

### General

| Command | Description |
|---------|-------------|
| `help` | Show all commands |
| `debug` | Print filesystem info (blocks, inodes, etc.) |
| `clear` | Clear the terminal screen |
| `exit` | Quit the program |

### Files

| Command | Description |
|---------|-------------|
| `touch <file>` | Create an empty file |
| `create <file>` | Same as `touch` |
| `write <file> <text>` | Write text to a file (inline) |
| `write <file>` | Write mode — prompts `enter content :` on next line |
| `cat <file>` | Read and print file content |
| `read <file>` | Same as `cat` |
| `rm <file>` | Delete a file |
| `delete <file>` | Same as `rm` |
| `ls` | List files in current directory |
| `list` | Same as `ls` |

### Directories

| Command | Description |
|---------|-------------|
| `mkdir <dir>` | Create a folder |
| `cd <dir>` | Enter a folder |
| `cd ..` | Go up (currently jumps to root — see limitations) |
| `pwd` | Show current directory |

### Snapshots

| Command | Description |
|---------|-------------|
| `snapshot save <name>` | Save current disk state as a snapshot |
| `snapshot load <name>` | Restore a saved snapshot |
| `snapshot list` | List all saved snapshots |

Snapshots are stored in `snapshots/<name>.img`. The list is kept in `snapshots/index.txt`.

### Visualization

| Command | Description |
|---------|-------------|
| `viz map` | Show disk layout (superblock, inodes, bitmap, data blocks) |
| `viz bitmap` | Show first 20 blocks of the free-block bitmap |
| `viz tree` | Show directory tree from root |
| `viz inode <id>` | Show details of a specific inode (e.g. `viz inode 0`) |

### Block Cache

| Command | Description |
|---------|-------------|
| `cache on` | Enable block cache |
| `cache off` | Disable cache and clear it |
| `cache stats` | Show hits, misses, and hit rate |
| `cache clear` | Clear cached blocks and reset stats |

The cache uses **LRU** (Least Recently Used) with a default capacity of **64 blocks**. Writes go to disk immediately (write-through), and reads are cached.

### Benchmark

| Command | Description |
|---------|-------------|
| `bench <file> <iter>` | Read a file repeatedly and show timing + cache stats |

Example: `bench notes.txt 10` reads `notes.txt` 10 times and prints total time, average time, and cache hit rate.

---

## Example Session

```
vfs> format
vfs> touch hello.txt
vfs> write hello.txt Hello World
vfs> cat hello.txt
vfs> mkdir docs
vfs> cd docs
vfs> touch notes.txt
vfs> write notes.txt my notes here
vfs> ls
vfs> cd ..
vfs> snapshot save backup1
vfs> write hello.txt updated text
vfs> snapshot load backup1
vfs> cat hello.txt          ← shows old content
vfs> cache on
vfs> bench hello.txt 20
vfs> viz tree
vfs> exit
```

**Next run (persistence test):**

```
vfs> load
vfs> ls
vfs> cat hello.txt
vfs> exit
```

---

## How Each Part Works (Simple Explanation)

### Disk Layer (`disk.cpp`)
- Treats `disk.img` as a raw block device
- Every read/write is 4096 bytes at a time
- Optional LRU cache keeps recently used blocks in memory for faster repeated reads

### Filesystem Layer (`fs.cpp`)
- `format()` — sets up superblock, inode table, bitmap, and root directory
- `load()` — reads superblock and restores root directory info
- File create/write/read/delete — uses inodes, bitmap, and directory entries together
- Tracks **current directory** so `ls`, `touch`, `write`, etc. work inside subfolders

### Inode Table (`inode.cpp`)
- Stores metadata for every file and folder
- Allocates and frees inodes when files are created or deleted

### Bitmap (`bitmap.cpp`)
- Tracks which disk blocks are free (0) or used (1)
- Allocates a block when a file needs space; frees blocks when a file is deleted or shrunk

### Directory (`directory.cpp`)
- Stores filename → inode mappings inside a disk block
- Supports add, remove, find, and list operations

### Snapshots
- Copies the entire `disk.img` to `snapshots/<name>.img`
- Loading a snapshot replaces `disk.img` with the saved copy and reloads the filesystem
- Lets you go back to an earlier state (time travel)

---

## Block Cache Details

- **Policy:** LRU (evicts least recently used block when cache is full)
- **Default capacity:** 64 blocks
- **Write strategy:** Write-through (every write goes to disk, then updates cache)
- **Stats:** hits, misses, hit rate percentage

Typical result after `bench file.txt 10` with cache on:
- First few reads = cache misses
- Later reads = cache hits (~90%+ hit rate)

---
