# KW_Operating_System

광운대학교 운영체제 과제

우분투 뻑갔을 때 재설치

sudo apt-get install vim

sudo apt update

sudo su

cd /home/os2018202065/Downloads

sudo wget https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-4.19.67.tar.xz

tar -Jxvf linux-4.19.67.tar.xz

cd linux-4.19.67/

vi Makefile 수정 ‘-본인학번’

sudo apt install build-essential libncurses5-dev bison flex libssl-dev libelf-dev

sudo make menuconfig

“Enable loadable module support” ➔ “Forced module loading” 체크
“Device Drivers” ➔ “Staging drivers” 체크 해제

make -jn

make modules\_\_install

make install

sudo vi /etc/default/grub

true -> false

reboot

sudo apt install exuberant-ctags

apt install cscope
