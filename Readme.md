# Linux Mint Fullscreen bug fix

This is a workaround to a linux mint(muffin) bug here [#656](https://github.com/linuxmint/muffin/issues/656)

How it works is by intercepting the fullscreen event and forcing the fullscreen to the window your cursor is on

## Setup Guide

Dependancies

```
sudo apt install libx11-dev libxrandr-dev
```

Clone the repo

`git clone https://github.com/IrishBruse/mint_fullscreen_fix.git`

Move to the folder

`cd mint_fullscreen_fix`

then build the application

`make`

to test it out you can run

`./mint_fullscreen_fix`

If it works to fix it you can the set it to run on startup using the starup application app like below

![Startup Applications](Untitled.png)
