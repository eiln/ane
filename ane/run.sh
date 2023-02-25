#!/usr/bin/env bash

function logs(){
	dmesg -k --color=always | grep ane
}

function status(){
	kstat=$(lsmod | grep "^ane ")
	if [ -n "$kstat" ]; then
		lsmod | head -n1 # header
		echo "$kstat"
		echo "module is installed"
	else
		echo "module NOT installed"
	fi
}

function uninstall(){
	kstat=$(lsmod | grep "^ane ")
	if [ -n "$kstat" ]; then
		make uninstall
		echo "uninstalled module"
	else
		echo "module NOT installed"
	fi
}

function install(){
	kstat=$(lsmod | grep "^ane ")
	if [ -n "$kstat" ]; then
		echo "already installed; removing then re-installing"
		make uninstall && make && make install
		logs
		echo "installed module"
	else
		make && make install
		logs
		echo "installed module"
	fi
}

function ktest(){
	uninstall
	install
	status
}

$1
