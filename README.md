# Watchpoint

Watchpoint is a small experimental puzzle game running on real Nintendo 64 hardware. It explores a new interaction model where classic game systems are observed and manipulated indirectly through modern AI-driven tools.

The player does not control a character. Instead, they operate a remote security camera observing a sealed room. Objects inside the room respond only to external actions, issued by an AI through a command interface. The player’s role is to observe outcomes, infer hidden rules, and guide experimentation.

At its core, Watchpoint is a verb–object discovery puzzle. Progress is made by learning which actions can be applied to which objects, and in what order. The game provides no instructions, no UI text, and no explicit feedback. Understanding emerges entirely through observation and consequence.

## How to Play

To play Watchpoint, you will need:

1. A Nintendo 64 (or compatible hardware)
2. An EverDrive-64 (currently tested with the X7)
3. A PC with a USB port
4. A local or remote Large Language Model (LLM) capable of tool calling via the Model Context Protocol (MCP)

### Setup

1. Download the latest Watchpoint ROM from the Releases page and copy it to your EverDrive’s microSD card.
2. Connect the EverDrive to your PC via USB.
3. Clone this repository and register the MCP server (`mcp/server.py`) with your preferred LLM client.
4. Boot the ROM on your N64.

Once running, you control the in-game camera using the N64 controller. All interactions with the room are performed by the LLM through MCP tools. Gameplay emerges from collaborating with the LLM, experimenting with available actions, and observing how the room responds.

This prototype is intentionally constrained. Its long-term goal is to explore whether this interaction model can evolve into a lightweight, LLM-assisted “vibe coding” game engine for N64-era hardware.

## Development

Watchpoint is currently a solo project, and the development setup is tailored to my personal workflow. Expect things to change as the project evolves.

Current setup:

* A Windows XP VirtualBox environment for compiling N64 ROMs using `libultra`
* [UNFLoader](https://github.com/buu342/N64-UNFLoader) to automatically load newly compiled ROMs onto an EverDrive X7
* Testing primarily performed on an [Analogue 3D](https://www.analogue.co/3d)
