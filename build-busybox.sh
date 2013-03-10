#!/bin/bash
cd busybox*
make -j8
cd ..
make mount
sudo cp busybox*/busybox_unstripped fs/bin/sh
