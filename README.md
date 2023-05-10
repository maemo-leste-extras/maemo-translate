Maemo-translate
===================

Translate text without requiring an internet connection. Built on top of [kotki](https://github.com/kroketio/kotki/).

## Models

The language models are available via debian packages on maemo leste, `apt search maemo-translate-data-*` to list 
them. You'll need to restart the GUI after installing new models.

#### hacking

```bash
sudo apt install -y qtbase5-dev ccache cmake libx11-dev zlib1g-dev libpng-dev libpcre2-dev libcli11-dev libcpuinfo-dev libsimde-dev libqt5svg5-dev libqt5maemo5-dev libqt5x11extras5-dev qtdeclarative5-dev qtdeclarative5-dev-tools libkotki
```