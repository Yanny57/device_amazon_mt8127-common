Device Tree For Amazon Fire 7
=============================

Basic   | Spec Sheet
-------:|:-------------------------
CPU     | Quad 1.3 GHz Cortex-A7
CHIPSET | Mediatek MT8127
GPU     | Mali-450 MP
Memory  | 1GB RAM
Android | 5.0
Storage | 8 / 16 GB
MicroSD | yes
Battery | 2980 mAh
Display | 7"
Front Camera  | 0.3 MP
Rear Camera  | 2 MP

![Fire 7](https://cnet1.cbsistatic.com/img/xQi3z27APNet9u6_jXURLg8YP7U=/770x433/2017/06/01/91b72246-686f-4be5-baf7-3bb26a6ba518/amazon-fire-7-15.jpg "Fire 7")

This branch is for building LineageOS 18.1. See
[device_amazon_ford](https://github.com/Yanny57/device_amazon_ford) for
status, install and build instructions.

- `patches/`: changes to LineageOS repositories, applied by `patches/patch.sh`
- `repartition/`: `ford_repart` and the TWRP zips that switch the eMMC between
  the stock and the treble (`/vendor`) partition layouts
